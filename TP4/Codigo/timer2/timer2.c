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
 * Contador acumulador de ticks de SW-PWM pendientes.
 * Se incrementa en cada interrupción del Timer2 (31250 Hz) y se
 * decrementa de a uno en cada Timer2_ConsumirTickPWM() exitoso.
 *
 * A diferencia de una flag binaria, este contador NO pierde ticks si el
 * foreground se demora: si mientras el main loop procesaba un comando
 * UART ocurrieron, por ejemplo, 5 interrupciones del Timer2, el contador
 * queda en 5 y el foreground las recupera todas la próxima vez que
 * consulte Timer2_ConsumirTickPWM() en un while().
 *
 * uint8_t alcanza porque el foreground se espera a drenarlo en cada
 * vuelta del while(1); aun en el peor caso previsible (un uart_transfer()
 * bloqueado esperando espacio en el buffer TX, del orden de unas pocas
 * decenas de µs) no debería acumular más de unas pocas unidades. Si se
 * acumulara más de 255 sin ser consumido, el contador se satura en 255
 * en lugar de desbordar (ver ISR más abajo), perdiendo el excedente en
 * vez de reiniciar en 0 silenciosamente.
 */
static volatile uint8_t tick_sw_pwm_count = 0u;

/**
 * Bandera de tick de fade (~10 ms).
 * Se levanta cada TICKS_POR_FADE_MS interrupciones del Timer2.
 * La limpia el foreground con Timer2_LimpiarTick().
 * Sigue siendo una flag binaria (ver justificación en timer2.h): el fade
 * es una animación visual donde perder un tick de 10 ms ocasional no se
 * nota, así que no se justifica llevar un contador acumulador para esto.
 */
static volatile uint8_t tick_fade_flag = 0u;

/* =========================================================================
 * API pública
 * ====================================================================== */

void Timer2_Init(void)
{
  contador_fade = TICKS_POR_FADE_MS;
  tick_sw_pwm_count = 0u;
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

uint8_t Timer2_ConsumirTickPWM(void)
{
  /*
   * Sección crítica: tick_sw_pwm_count también se modifica desde la ISR
   * del Timer2, así que la lectura-y-decremento debe ser atómica para
   * evitar perder un incremento concurrente (read-modify-write no es
   * atómico de por sí en AVR para variables de 8 bits accedidas desde
   * ISR y foreground a la vez).
   */
  uint8_t sreg = SREG;
  cli();

  uint8_t hay_tick = (tick_sw_pwm_count > 0u);
  if (hay_tick)
    tick_sw_pwm_count--;

  SREG = sreg;
  return hay_tick;
}

uint8_t Timer2_HayTick(void) { return tick_fade_flag; }
void Timer2_LimpiarTick(void) { tick_fade_flag = 0u; }

/* =========================================================================
 * ISR del Timer2 – se ejecuta a 31250 Hz (cada 32 µs)
 * ====================================================================== */

ISR(TIMER2_COMPA_vect)
{
  /*
   * ROL 1 – Acumular un tick de SW-PWM pendiente.
   *
   * En lugar de levantar una flag binaria (que fusionaría ticks perdidos
   * si el foreground no llega a tiempo), se incrementa un contador que
   * el foreground drena por completo en cada vuelta del while(1) mediante
   * Timer2_ConsumirTickPWM(). Saturado en 255 en vez de desbordar a 0,
   * para no perder silenciosamente la señal de que hay ticks pendientes
   * en el escenario (muy improbable) de que se acumulen más de 255.
   */
  if (tick_sw_pwm_count < 255u)
    tick_sw_pwm_count++;

  /*
   * ROL 2 – Generar el tick de fade cada ~10 ms.
   * Contador de 16 bits porque 313 > 255.
   */
  if (--contador_fade == 0u)
  {
    contador_fade = TICKS_POR_FADE_MS;
    tick_fade_flag = 1u;
  }
}