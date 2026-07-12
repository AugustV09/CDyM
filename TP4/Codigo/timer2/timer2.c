/**
 * @file timer2.c
 * @brief Implementación del tick periódico con Timer/Counter 2.
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 */

#include "timer2.h"
/* Sin includes de otros módulos: la ISR solo gestiona flags.
 * El acoplamiento con led_rgb y fade ocurre exclusivamente en el foreground. */
#include <avr/interrupt.h>
#include <avr/io.h>

/* =========================================================================
 * Constantes
 * ====================================================================== */

/*
 * Configuración del Timer2 en modo CTC (WGM21=1, WGM20=0, WGM22=0):
 *   TOP = OCR2A = 63
 *   Prescaler CS21=1 → divisor 8
 *   f_interrupción = F_CPU / (prescaler × (OCR2A + 1))
 *                  = 16.000.000 / (8 × 64) = 31.250 Hz  →  período = 32 µs
 */
#define OCR2A_VAL 63u

/*
 * Interrupciones del Timer2 entre dos ticks de fade consecutivos.
 * Con f_tick = 31250 Hz, para un tick de fade cada 10 ms:
 *   N = 31250 × 0.010 = 312.5 → usamos 313  (≈ 10.016 ms, error < 0.2 %)
 */
#define TICKS_POR_FADE_MS 313u

/* =========================================================================
 * Estado interno
 * ====================================================================== */

/** Contador descendente hasta el próximo tick de fade. */
static volatile uint16_t contador_fade = TICKS_POR_FADE_MS;

/**
 * Bandera de tick de SW-PWM.
 * Se levanta en cada interrupción del Timer2 (31250 Hz).
 * La limpia el foreground con Timer2_LimpiarTickPWM().
 * Al ser una bandera binaria (no un contador), si el foreground no la consume
 * antes de la siguiente interrupción, el tick intermedio se fusiona (se pierde
 * la cuenta exacta pero no la señal de que ocurrió al menos uno).
 */
static volatile uint8_t tick_sw_pwm_flag = 0u;

/**
 * Bandera de tick de fade (~10 ms).
 * Se levanta cada TICKS_POR_FADE_MS interrupciones del Timer2.
 * La limpia el foreground con Timer2_LimpiarTick().
 */
static volatile uint8_t tick_fade_flag = 0u;

/* =========================================================================
 * API pública
 * ====================================================================== */

void Timer2_Init(void) {
  contador_fade = TICKS_POR_FADE_MS;
  tick_sw_pwm_flag = 0u;
  tick_fade_flag = 0u;

  TCNT2 = 0u;
  OCR2A = OCR2A_VAL;

  /*
   * TCCR2A: WGM21=1, WGM20=0 → modo CTC (TOP = OCR2A)
   * No se usan salidas OC2A/OC2B en pines (COM2x = 0).
   */
  TCCR2A = (1u << WGM21);

  /*
   * TCCR2B: WGM22=0 (modo CTC no lo requiere), CS21=1 → prescaler 8
   */
  TCCR2B = (1u << CS21);

  /* Habilita la interrupción por Compare Match A del Timer2 */
  TIMSK2 |= (1u << OCIE2A);
}

uint8_t Timer2_HayTickPWM(void) { return tick_sw_pwm_flag; }
void Timer2_LimpiarTickPWM(void) { tick_sw_pwm_flag = 0u; }

uint8_t Timer2_HayTick(void) { return tick_fade_flag; }
void Timer2_LimpiarTick(void) { tick_fade_flag = 0u; }

/* =========================================================================
 * ISR del Timer2 – se ejecuta a 31250 Hz (cada 32 µs)
 * ====================================================================== */

ISR(TIMER2_COMPA_vect) {
  /*
   * ROL 1 – Señalizar tick de SW-PWM.
   *
   * En lugar de llamar directamente a led_rgb_tick_sw_pwm() (lo que
   * acoplaría este módulo con led_rgb), levantamos una bandera que
   * el foreground consume en el siguiente ciclo del main loop.
   *
   * Consecuencia: si el main loop está ocupado (por ej. procesando
   * un comando UART), el SW-PWM puede perder ticks intermedios.
   * En una aplicación visual esto es aceptable; si se necesitara
   * precisión estricta, habría que mover la llamada de vuelta a la ISR.
   */
  tick_sw_pwm_flag = 1u;

  /*
   * ROL 2 – Generar el tick de fade cada ~10 ms.
   * Contador de 16 bits porque 313 > 255.
   */
  if (--contador_fade == 0u) {
    contador_fade = TICKS_POR_FADE_MS;
    tick_fade_flag = 1u;
  }
}