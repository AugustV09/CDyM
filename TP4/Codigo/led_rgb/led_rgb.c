/**
 * @file led_rgb.c
 * @brief Implementación del driver PWM para LED RGB de ánodo común.
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 */

#include "led_rgb.h"
#include <avr/io.h>

/* =========================================================================
 * Constantes de hardware - Timer1 Modo 1, INVERTIDO
 * ====================================================================== */

/*
 * Timer1 - Modo 1: Phase Correct PWM 8-bit
 *   WGM = 0b0001  ->  WGM13=0 WGM12=0 WGM11=0 WGM10=1  ->  TOP = 0xFF
 *   Distribucion de bits:
 *     WGM10, WGM11 -> TCCR1A[1:0]
 *     WGM12, WGM13 -> TCCR1B[4:3]
 *   Para Modo 1: solo WGM10=1; WGM11,WGM12,WGM13 quedan en 0.
 *
 *   El contador cuenta 0->255->0 (doble rampa) cada ciclo PWM.
 *   Período = 2 x N x TOP = 2 x 8 x 255 = 4080 ciclos de reloj
 *   f_PWM = F_CPU / 4080 = 16.000.000 / 4080 ~ 3922 Hz >> 30 Hz OK
 *
 * Modo INVERTIDO (COM1x = 0b11):
 *   - SET   en compare match durante UP-count   (TCNT llega a OCR subiendo)
 *   - CLEAR en compare match durante DOWN-count (TCNT llega a OCR bajando)
 *   -> pin LOW  durante  0 <= TCNT < OCR  (ambas rampas)
 *   -> pin HIGH durante OCR <= TCNT <= TOP (ambas rampas)
 *
 *   DC_LOW (tiempo que el pin está LOW) = OCR / TOP ~ OCR / 255
 *   Como el LED conduce con el pin LOW (ánodo común -> VCC, cátodo -> MCU):
 *     DC_LED = DC_LOW = OCR / 255  <- relación DIRECTA
 *
 *   Valores extremos (sin glitches en Phase Correct PWM):
 *     OCR = 0   -> DC_LED = 0 %   -> LED APAGADO PERFECTO    OK
 *     OCR = 255 -> DC_LED ~ 100 % -> LED AL MÁXIMO BRILLO    OK
 *
 *   Conversión val_final -> OCR:
 *     OCR = val_final = (color x intensidad) / 255  <- SIN inversión
 *
 * TCCR1A:
 *   COM1A1=1, COM1A0=1 -> OC1A (PB1, azul)  modo INVERTIDO
 *   COM1B1=1, COM1B0=1 -> OC1B (PB2, verde) modo INVERTIDO
 *   WGM11=0, WGM10=1   -> Modo 1 parte baja
 */
#define TCCR1A_VAL                                                             \
  ((1u << COM1A1) | (1u << COM1A0) | (1u << COM1B1) | (1u << COM1B0) |         \
   (1u << WGM10))

/*
 * TCCR1B:
 *   WGM13=0, WGM12=0 -> Modo 1 parte alta (NO se setea WGM12, a diferencia
 *                       del Modo 5 donde WGM12=1).
 *   CS11=1            -> prescaler 8
 */
#define TCCR1B_VAL (1u << CS11)

/* Pin PB5 (rojo, software PWM) */
#define PIN_R PB5

/* =========================================================================
 * Estado interno
 * ====================================================================== */

/** Color base de cada canal (0-255). */
static uint8_t color_r = 0u;
static uint8_t color_g = 0u;
static uint8_t color_b = 0u;

/** Factor global de intensidad (0-255). */
static uint8_t intensidad = 0u;

/**
 * Umbral del software PWM para PB5.
 * Actualizado en aplicar_pwm() cada vez que cambia color_r o intensidad.
 * umbral = val_final_r = (color_r x intensidad) / 255
 *
 * Lógica invertida (espeja el Timer1 en modo INVERTIDO):
 *   cnt < umbral -> pin LOW  (LED encendido, ánodo común)
 *   cnt >= umbral -> pin HIGH (LED apagado)
 *
 * Casos extremos:
 *   umbral = 0   -> cnt nunca < 0 (unsigned) -> pin siempre HIGH -> LED apagado  OK
 *   umbral = 255 -> cnt < 255 en 255/256 pasos -> pin LOW 99.6 % -> LED casi
 * máximo OK
 */
static volatile uint8_t sw_pwm_umbral = 0u;

/** Contador libre del software PWM (0-255, desbordamiento natural). */
static volatile uint8_t sw_pwm_cnt = 0u;

/* =========================================================================
 * Funciones privadas
 * ====================================================================== */

/**
 * @brief Calcula (a x b) / 255 con aritmética de 16 bits (~ >> 8).
 * @details El error máximo es 1 LSB, aceptable para PWM visual.
 *   Evita la rutina de división soft del compilador AVR.
 */
static inline uint8_t escalar(uint8_t a, uint8_t b) {
  return (uint8_t)(((uint16_t)a * (uint16_t)b) >> 8u);
}

/**
 * @brief Recalcula y escribe los tres registros de duty cycle con los
 *   valores actuales de color e intensidad.
 *
 * @note Modo INVERTIDO de Timer1: OCR = val_final (relación directa).
 *   val=0   -> OCR=0   -> DC_LED=0 %   -> LED apagado  OK
 *   val=255 -> OCR=255 -> DC_LED~100 % -> LED máximo   OK
 */
static void aplicar_pwm(void) {
  uint8_t val_r = escalar(color_r, intensidad);
  uint8_t val_g = escalar(color_g, intensidad);
  uint8_t val_b = escalar(color_b, intensidad);

  /*
   * Timer1 modo INVERTIDO, Phase Correct PWM:
   *   OCR = val_final  (sin inversión).
   * OC1A -> PB1 -> azul
   * OC1B -> PB2 -> verde
   */
  OCR1A = val_b;
  OCR1B = val_g;

  /*
   * Software PWM (PB5, rojo) - misma lógica invertida:
   *   umbral = val_final_r
   *   cnt < umbral -> LOW (LED encendido)
   *   cnt >= umbral -> HIGH (LED apagado)
   */
  sw_pwm_umbral = val_r;
}

/* =========================================================================
 * API pública
 * ====================================================================== */

void led_rgb_init(void) {
  /* --- Configurar pines como salida --- */
  DDRB |= (1u << PB1) | (1u << PB2) | (1u << PIN_R);

  /* Estado inicial: todos HIGH -> LEDs apagados (ánodo común a VCC). */
  PORTB |= (1u << PB1) | (1u << PB2) | (1u << PIN_R);

  /*
   * Timer1 - Modo 1 INVERTIDO: OCR = 0 -> apagado.
   * En modo invertido con OCR=0, el pin permanece HIGH durante la totalidad
   * del ciclo (excepto el instante del turnaround en BOTTOM), lo que
   * corresponde a DC_LED = 0 % -> LED apagado perfecto.
   */
  OCR1A = 0u; /* azul  apagado */
  OCR1B = 0u; /* verde apagado */

  TCCR1A = TCCR1A_VAL;
  TCCR1B = TCCR1B_VAL;

  /* No se habilitan OCIE1A/OCIE1B: Timer1 genera PWM en hardware sin ISR. */

  /* Software PWM inicial: umbral=0 -> cnt nunca < 0 -> pin HIGH -> apagado. */
  sw_pwm_cnt = 0u;
  sw_pwm_umbral = 0u;

  color_r = 0u;
  color_g = 0u;
  color_b = 0u;
  intensidad = 0u;
}

void led_rgb_set_color(uint8_t r, uint8_t g, uint8_t b) {
  color_r = r;
  color_g = g;
  color_b = b;
  aplicar_pwm();
}

void led_rgb_set_intensidad(uint8_t val) {
  intensidad = val;
  aplicar_pwm();
}

void led_rgb_tick_sw_pwm(void) {
  /*
   * Contador libre de 8 bits: desborda de 255 a 0 automáticamente con ++.
   * Genera 256 llamadas por ciclo completo de SW-PWM.
   *
   * Lógica INVERTIDA (replica el comportamiento del Timer1 en modo invertido):
   *   cnt < umbral -> pin LOW  -> LED encendido  (corriente fluye por el cátodo)
   *   cnt >= umbral -> pin HIGH -> LED apagado    (ánodo y cátodo al mismo
   * potencial)
   *
   * umbral = val_final_r  (calculado en aplicar_pwm())
   *   val_final=0   -> umbral=0   -> cnt nunca < 0 -> pin siempre HIGH -> apagado OK
   *   val_final=255 -> umbral=255 -> cnt < 255 en 255/256 pasos -> pin LOW 99.6 %
   * OK
   *
   * Nota sobre frecuencia:
   *   f_sw_pwm = f_llamada / 256
   *   Si led_rgb_tick_sw_pwm() se llama desde el foreground al consumir el
   *   tick de SW-PWM del Timer2 (~ 31250 Hz), entonces:
   *   f_sw_pwm ~ 31250 / 256 ~ 122 Hz >> 30 Hz OK
   */
  sw_pwm_cnt++;

  if (sw_pwm_cnt < sw_pwm_umbral)
    PORTB &= ~(1u << PIN_R); /* LOW  -> LED encendido */
  else
    PORTB |= (1u << PIN_R); /* HIGH -> LED apagado   */
}

uint8_t led_rgb_get_r(void) { return color_r; }
uint8_t led_rgb_get_g(void) { return color_g; }
uint8_t led_rgb_get_b(void) { return color_b; }
uint8_t led_rgb_get_intensidad(void) { return intensidad; }