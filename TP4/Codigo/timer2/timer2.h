/**
 * @file timer2.h
 * @brief Módulo de tick periódico usando el Timer/Counter 2 del ATmega328P.
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 *
 * El Timer2 cumple dos roles simultáneos en el TP4, ambos gestionados
 * mediante flags levantadas en la ISR y consumidas en el foreground:
 *
 *   ROL 1 – Tick de software PWM (flag de alta frecuencia):
 *     La ISR levanta tick_sw_pwm_flag en cada interrupción.
 *     El main llama a led_rgb_tick_sw_pwm() al consumirla, desacoplando
 *     el módulo Timer2 del módulo led_rgb.
 *     Cadencia: f_tick = F_CPU / (prescaler × (OCR2A + 1))
 *     Con prescaler=8 y OCR2A=63: f_tick = 16e6 / (8 × 64) = 31250 Hz
 *
 *   ROL 2 – Tick de fade (~10 ms):
 *     Cada 313 interrupciones del Timer2 se levanta tick_fade_flag,
 *     consumida por el main para avanzar la máquina de estados de fade.
 *     Con f_tick = 31250 Hz → período tick_fade ≈ 313/31250 ≈ 10.016 ms.
 *
 * Jerarquía de flags en el foreground ("anidamiento de flags"):
 *
 *   if (Timer2_HayTickPWM())        <- cada interrupción de Timer2
 *   {
 *       Timer2_LimpiarTickPWM();
 *       led_rgb_tick_sw_pwm();      <- siempre al entrar a la primera flag
 *
 *       if (Timer2_HayTick())       <- cada ~10 ms (313 interrupciones)
 *       {
 *           Timer2_LimpiarTick();
 *           fade_tick();            <- avanza la máquina de estados
 *       }
 *   }
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
 *   La ISR levanta tick_sw_pwm_flag en cada interrupción y
 *   tick_fade_flag cada ~313 interrupciones (~10 ms).
 */
void Timer2_Init(void);

/* -------------------------------------------------------------------------
 * API de tick SW-PWM (alta frecuencia – una por interrupción de Timer2)
 * ---------------------------------------------------------------------- */

/**
 * @brief Consulta si hay al menos un tick de SW-PWM pendiente.
 * @return 1 si la flag está levantada, 0 si no.
 * @note La flag se levanta en cada ISR del Timer2 (31250 Hz).
 *   Si el foreground no la consume a tiempo, los ticks intermedios
 *   se fusionan (la flag es binaria, no acumula).
 */
uint8_t Timer2_HayTickPWM(void);

/**
 * @brief Limpia la flag de tick de SW-PWM.
 * @note Llamar desde el foreground antes de invocar led_rgb_tick_sw_pwm().
 */
void Timer2_LimpiarTickPWM(void);

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