/**
 * @file led_rgb.h
 * @brief Driver PWM para LED RGB de ánodo común (RGBLED-CA).
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 *
 * Conexionado (ánodo común, cátodos a los pines vía 220 ohm):
 *   PB1 = OC1A = canal AZUL   (Timer1, modo 1 - Phase Correct PWM 8-bit,
 * invertido) PB2 = OC1B = canal VERDE  (Timer1, modo 1 - Phase Correct PWM
 * 8-bit, invertido) PB5 = canal ROJO          (PWM por software, lógica
 * equivalente al modo invertido)
 *
 * ============================================================================
 * ELECCIÓN DE MODO: Phase Correct PWM 8-bit (Modo 1), INVERTIDO
 * ============================================================================
 *
 * Modo 1 vs. Modo 5 (Fast PWM):
 *   El Modo 1 (Phase Correct PWM) cuenta UP y DOWN simétricamente (0->255->0),
 *   lo que produce una señal centrada y sin el glitch de actualización del
 *   Fast PWM. Más importante: en Phase Correct PWM los valores extremos OCR=0
 *   y OCR=TOP=255 producen duty cycles exactos (0 % y 100 %), permitiendo
 *   alcanzar tanto el apagado como el encendido máximo perfectos.
 *
 *   Frecuencia con prescaler 8:
 *     f_PWM = F_CPU / (N x 2 x TOP) = 16e6 / (8 x 510) ~ 3922 Hz >> 30 Hz OK
 *
 * Por qué el modo INVERTIDO (COM1x = 0b11):
 *   En el modo NO INVERTIDO, el pin está HIGH mientras TCNT < OCR (sentido UP)
 *   y HIGH mientras TCNT > OCR (sentido DOWN), de modo que:
 *     DC_HIGH = OCR / TOP
 *   Como el LED conduce cuando el pin está LOW (ánodo a VCC), se necesitaría
 *   invertir la relación:  OCR = 255 - val_final
 *
 *   En el modo INVERTIDO, el pin está LOW mientras TCNT < OCR (y viceversa),
 *   de modo que:
 *     DC_LOW  = OCR / TOP
 *   Como el LED conduce cuando el pin está LOW, la relación es DIRECTA:
 *     OCR = val_final = (color x intensidad) / 255
 *
 * Casos límite (ánodo común, modo INVERTIDO, TOP=255):
 *   OCR = 0   -> DC_LOW = 0 %   -> pin siempre HIGH -> LED APAGADO PERFECTO  OK
 *   OCR = 255 -> DC_LOW = 100 % -> pin siempre LOW  -> LED MÁXIMO BRILLO     OK
 *
 * Conversión color->OCR:
 *   val_final = (color x intensidad) / 255
 *   OCR       = val_final              <- relación directa, sin inversión
 *
 * Software PWM (PB5, canal rojo) - misma lógica:
 *   sw_pwm_umbral = val_final_r
 *   cnt < umbral -> pin LOW  (LED encendido)
 *   cnt >= umbral -> pin HIGH (LED apagado)
 *   Esto replica la semántica del modo invertido del Timer1.
 *
 * API de doble capa (color x intensidad):
 *   - led_rgb_set_color(r, g, b)    -> fija el color base (0-255 por canal)
 *   - led_rgb_set_intensidad(val)   -> fija el factor global (0-255)
 *   Valor final = (color x intensidad) / 255
 *   Esto permite que el módulo fade/ ajuste la intensidad sin conocer el color.
 */

#ifndef LED_RGB_H
#define LED_RGB_H

#include <stdint.h>

/* -------------------------------------------------------------------------
 * Inicialización
 * ---------------------------------------------------------------------- */

/**
 * @brief Inicializa los tres canales PWM del LED RGB.
 * @details Configura el Timer1 en modo 1 (Phase Correct PWM 8-bit) con
 *   salidas INVERTIDAS en OC1A y OC1B:
 *   - OC1A (PB1, azul):  invertido, OCR1A = 0 (apagado inicial)
 *   - OC1B (PB2, verde): invertido, OCR1B = 0 (apagado inicial)
 *   - PB5  (rojo):       configurado como salida, software PWM mediante
 *                        contador interno actualizado en led_rgb_tick_sw_pwm()
 * Color inicial = {0,0,0}, intensidad inicial = 0 -> todo apagado.
 */
void led_rgb_init(void);

/* -------------------------------------------------------------------------
 * Control de color e intensidad
 * ---------------------------------------------------------------------- */

/**
 * @brief Fija el color base de los tres canales.
 * @param r  Componente rojo  (0 = apagado, 255 = máxima intensidad de ese
 * color).
 * @param g  Componente verde (0 = apagado, 255 = máxima intensidad de ese
 * color).
 * @param b  Componente azul  (0 = apagado, 255 = máxima intensidad de ese
 * color).
 * @note Actualiza los registros OCR y el umbral de SW-PWM de inmediato.
 */
void led_rgb_set_color(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Fija el factor global de intensidad.
 * @param val  0 = completamente apagado, 255 = brillo completo del color base.
 * @note Actualiza los registros OCR y el umbral de SW-PWM de inmediato.
 */
void led_rgb_set_intensidad(uint8_t val);

/* -------------------------------------------------------------------------
 * Software PWM para PB5 (canal rojo)
 * Debe llamarse desde el foreground al consumir el tick de SW-PWM del Timer2.
 * ---------------------------------------------------------------------- */

/**
 * @brief Avanza el contador del software PWM y actualiza el pin PB5.
 * @details Implementa la misma lógica invertida que Timer1:
 *   cnt < umbral -> pin LOW  (LED encendido, ánodo común)
 *   cnt >= umbral -> pin HIGH (LED apagado)
 *   umbral = val_final_r = (color_r x intensidad) / 255
 *   Frecuencia resultante: f_sw_pwm = f_llamada / 256
 *   Con la llamada manejada desde el tick de SW-PWM del Timer2, la frecuencia
 *   es aproximadamente la del tick de Timer2 (~ 31250 Hz) / 256 ~ 122 Hz >> 30
 * Hz.
 */
void led_rgb_tick_sw_pwm(void);

#endif /* LED_RGB_H */