/**
 * @file fade.h
 * @brief Lógica de fade (desvanecimiento) del LED RGB.
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 *
 * El fade produce la siguiente forma de onda de intensidad:
 *
 *   Intensidad
 *   255 |      ___________
 *       |     /           \
 *       |    /             \
 *     0 |___/               \___
 *       |<1s><---1s---><1s->|
 *       |<--------T-------->|
 *
 * Estados internos de la máquina de estados:
 *   FADE_SUBIENDO:  intensidad aumenta de 0 a 255 en ~1 segundo
 *   FADE_MESETA:    intensidad permanece en 255 durante ~1 segundo
 *   FADE_BAJANDO:   intensidad disminuye de 255 a 0 en ~1 segundo
 *   FADE_PAUSA:     intensidad en 0 durante (T - 3s) segundos
 *
 * El periodo T es modificable en tiempo de ejecución por el módulo ADC (inciso
 * 4). El rango es 3 s (máx iluminación) a 6 s (mín iluminación): la PAUSA varía
 * entre 0 s y 3 s.
 *
 * Cada llamada a fade_tick() avanza la máquina un paso; debe llamarse desde
 * el foreground al consumir el tick del Timer2 (cada ~10 ms).
 *
 * EXTENSIBILIDAD (incisos 3 y 4):
 *   - fade_tick(): implementar en el inciso 3
 *   - fade_set_periodo_ms(): llamada desde el módulo ADC en el inciso 4
 */

#ifndef FADE_H
#define FADE_H

#include <stdint.h>

/**
 * @brief Inicializa la máquina de estados del fade.
 * @param periodo_ms Periodo total T en milisegundos (rango: 3000-6000).
 * @details Arranca la máquina en estado FADE_SUBIENDO con intensidad 0.
 */
void fade_init(uint16_t periodo_ms);

/**
 * @brief Avanza la máquina de estados del fade un paso (llamar cada ~10 ms).
 * @details Usa un switch sobre el estado actual y un contador de ticks
 *   como "entrada" para decidir las transiciones.
 *   Actualiza la intensidad del LED vía led_rgb_set_intensidad().
 */
void fade_tick(void);

/**
 * @brief Modifica el periodo total del fade en tiempo de ejecución.
 * @param periodo_ms Nuevo periodo en ms (se acepta 3000-6000, se satura fuera
 * de rango). Llamada desde el módulo ADC en el inciso 4.
 */
void fade_set_periodo_ms(uint16_t periodo_ms);

#endif /* FADE_H */
