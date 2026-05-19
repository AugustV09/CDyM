#ifndef RELOJ_H
#define RELOJ_H

#include <stdint.h>

/*
 * A partir de aca empieza la personalizacion del usuario
 * Puede cambiar:
 * Cada cuantas interrupciones de 5ms es un Tick
 */

#define INTERRUPCIONES_POR_TICK 5 // Este caso un tick equivale a 25ms

/*
 * Aca termina la personalizacion del usuario
 */

/**
 * @brief Inicializa el reloj, interrupciones cada 5ms con TIMER2
 * @return void
 */
void Reloj_Init(void);

/**
 * @brief Comienza la cuenta de segundos
 * @return void
 */
void Reloj_StartCountingSeconds(void);

/**
 * @brief Detiene la cuenta de segundos
 * @return void
 */
void Reloj_StopCountingSeconds(void);

/**
 * @brief Reinicia la cuenta de segundos
 * @return void
 */
void Reloj_RestartSeconds(void);

/**
 * @brief Indica si hay un tick disponible
 * @return uint8_t
 */
uint8_t Reloj_HayTick(void);

/**
 * @brief Limpia el tick
 * @return void
 */
void Reloj_LimpiarTick(void);

/**
 * @brief Indica si hay un segundo disponible
 * @return uint8_t
 */
uint8_t Reloj_HaySegundo(void);
void Reloj_LimpiarSegundo(void);

#endif // RELOJ_H