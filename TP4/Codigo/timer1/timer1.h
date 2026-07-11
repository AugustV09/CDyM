/**
 * @file timer1.h
 * @brief Módulo de temporización periódica usando el Timer/Counter 1 del AVR.
 * @note Diseñado para el ATmega328P a 16MHz. Genera eventos cada T segundos
 * configurables en tiempo de ejecución (rango válido: 2 a 60 segundos).
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 */
#ifndef TIMER1_H
#define TIMER1_H

#include <stdint.h>

/**
 * @brief Inicializa el Timer 1 en modo CTC con interrupción cada 1 segundo.
 * @details Configura OCR1A para generar una interrupción por segundo con prescaler
 * 1024. Internamente acumula segundos hasta alcanzar el intervalo T indicado,
 * momento en que levanta la bandera de evento.
 * @param intervalo_seg Intervalo inicial en segundos (recomendado: entre 2 y 60).
 */
void Timer1_Init(uint8_t intervalo_seg);

/**
 * @brief Modifica el intervalo de reporte en tiempo de ejecución.
 * @details Solo acepta valores entre 2 y 60 segundos. Si el valor está fuera de
 * rango, la función no hace nada. Reinicia el contador interno para que el nuevo
 * periodo empiece a contar desde el momento del cambio.
 * @param seg Nuevo intervalo en segundos.
 */
void Timer1_SetIntervalo(uint8_t seg);

/**
 * @brief Consulta si ya transcurrió el intervalo T configurado.
 * @return 1 si el evento está pendiente, 0 si todavía no.
 */
uint8_t Timer1_HayEvento(void);

/**
 * @brief Limpia la bandera de evento pendiente.
 * @details Debe llamarse desde el main loop luego de procesar el evento,
 * para evitar que se ejecute más de una vez por periodo.
 */
void Timer1_LimpiarEvento(void);

#endif /* TIMER1_H */