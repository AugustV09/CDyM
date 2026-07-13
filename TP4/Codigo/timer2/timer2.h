/**
 * @file timer2.h
 * @brief Módulo de tick periódico usando el Timer/Counter 2 del ATmega328P.
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 *
 * El Timer2 cumple dos roles simultáneos en el TP4:
 *
 *   ROL 1 – Tick de software PWM (contador acumulador de alta frecuencia):
 *     La ISR incrementa tick_sw_pwm_count en cada interrupción, en lugar
 *     de levantar una flag binaria. Esto evita perder ticks si el
 *     foreground tarda en consumirlos: Timer2_ConsumirTickPWM() devuelve
 *     y descuenta de a uno, así que el foreground puede recuperarlos todos
 *     llamando en un while() (ver ejemplo más abajo).
 *     Cadencia: f_tick = F_CPU / (prescaler × (OCR2A + 1))
 *     Con prescaler=8 y OCR2A=63: f_tick = 16e6 / (8 × 64) = 31250 Hz
 *
 *   ROL 2 – Tick de fade (~10 ms):
 *     Cada 313 interrupciones del Timer2 se levanta tick_fade_flag,
 *     consumida por el main para avanzar la máquina de estados de fade.
 *     Con f_tick = 31250 Hz → período tick_fade ≈ 313/31250 ≈ 10.016 ms.
 *     Esta sigue siendo una flag binaria: el fade es una animación visual
 *     de 1s por rampa, perder o fusionar un tick de 10ms ocasional no
 *     tiene impacto perceptible, así que no se justifica la complejidad
 *     de un contador acá.
 *
 * Jerarquía de consumo en el foreground ("anidamiento de ticks"):
 *
 *   while (Timer2_ConsumirTickPWM())   <- drena TODOS los ticks pendientes,
 *   {                                     uno por uno, sin perder cuenta
 *       led_rgb_tick_sw_pwm();
 *       adc_tick();
 *
 *       if (Timer2_HayTick())          <- cada ~10 ms (313 interrupciones)
 *       {
 *           Timer2_LimpiarTick();
 *           fade_tick();
 *       }
 *   }
 *
 * Por qué el cambio de flag binaria a contador (para el tick de SW-PWM):
 *   led_rgb_tick_sw_pwm() y adc_tick() dependen de contar exactamente
 *   cuántas veces se llaman (256 llamadas = 1 ciclo de SW-PWM; 3125
 *   llamadas = 1 muestreo de ADC). Si el foreground se demora más de
 *   32 µs en una vuelta del while(1) (por ejemplo procesando un comando
 *   UART largo) y se usara una flag binaria, los ticks intermedios se
 *   fusionarían en uno solo, atrasando el PWM por software y el
 *   muestreo del ADC. Con un contador acumulador, ese atraso se
 *   recupera íntegramente en la siguiente vuelta del while(1), porque
 *   el foreground drena todos los ticks pendientes antes de seguir.
 *
 * Separación de módulos:
 *   La ISR solo maneja contadores y flags; no invoca funciones de otros
 *   módulos. El acoplamiento entre módulos ocurre exclusivamente en el
 *   foreground, donde el llamador tiene visibilidad de todos ellos.
 */

#ifndef TIMER2_H
#define TIMER2_H

#include <stdint.h>

/**
 * @brief Inicializa el Timer2 en modo CTC con prescaler 8.
 * @details OCR2A = 63 → f_interrupción = 31250 Hz.
 *   La ISR incrementa tick_sw_pwm_count en cada interrupción y levanta
 *   tick_fade_flag cada ~313 interrupciones (~10 ms).
 */
void Timer2_Init(void);

/* -------------------------------------------------------------------------
 * API de tick SW-PWM (alta frecuencia – contador acumulador, no flag)
 * ---------------------------------------------------------------------- */

/**
 * @brief Consume un tick de SW-PWM pendiente, si lo hay.
 * @return 1 si había al menos un tick pendiente (y lo descontó), 0 si el
 *   contador ya está en cero.
 * @note A diferencia de una flag binaria, esta función NO pierde ticks:
 *   si la ISR incrementó el contador varias veces mientras el foreground
 *   estaba ocupado, cada llamada a Timer2_ConsumirTickPWM() devuelve 1 y
 *   descuenta uno, hasta agotar el contador. Se recomienda usarla en un
 *   while() para drenar todos los ticks pendientes en cada vuelta del
 *   loop principal:
 *     while (Timer2_ConsumirTickPWM()) { led_rgb_tick_sw_pwm(); ... }
 */
uint8_t Timer2_ConsumirTickPWM(void);

/* -------------------------------------------------------------------------
 * API de tick de fade (~10 ms – una cada 313 interrupciones)
 * ---------------------------------------------------------------------- */

/**
 * @brief Consulta si ha transcurrido un tick de fade (~10 ms).
 * @return 1 si hay un tick pendiente, 0 si no.
 */
uint8_t Timer2_HayTick(void);

/**
 * @brief Limpia la flag de tick de fade.
 * @note Llamar desde el foreground después de procesar fade_tick().
 */
void Timer2_LimpiarTick(void);

#endif /* TIMER2_H */