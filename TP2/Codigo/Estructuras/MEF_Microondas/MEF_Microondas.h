#ifndef MEF_MICROONDAS_H
#define MEF_MICROONDAS_H

#include <stdint.h>

/*
 * A partir de aca empieza la personalizacion del usuario
 * Puede cambiar:
 * Los pines y puertos de las filas y columnas
 */

#define DD_MAGNETRON DDRB
#define PUERTO_MAGNETRON PORTB
#define PIN_MAGNETRON PB5

#define DD_LUZ_INTERIOR DDRC
#define PUERTO_LUZ_INTERIOR PORTC
#define PIN_LUZ_INTERIOR PC4

#define DD_LUZ_TERMINADO DDRC
#define PUERTO_LUZ_TERMINADO PORTC
#define PIN_LUZ_TERMINADO PC5

/*
 * Aca termina la personalizacion del usuario
 */

#define MAX_TIME (99 * 60 + 99)

#define TICKS_POR_SEGUNDOS 40

enum EstadosMicroondas
{
    MEF_MICROONDAS_INGRESO,
    MEF_MICROONDAS_COCINANDO,
    MEF_MICROONDAS_PAUSADO,
    MEF_MICROONDAS_FIN_COCCION,
};

enum EstadosPuerta
{
    MEF_MICROONDAS_PUERTA_CERRADA,
    MEF_MICROONDAS_PUERTA_ABIERTA,
};

// Getters públicos para el LCD

/**
 * @brief Devuelve el estado actual del microondas
 * @return Estado actual del microondas
 */
enum EstadosMicroondas MEF_GetEstadoMicroondas(void);

/**
 * @brief Devuelve el tiempo actual del microondas
 * @return Tiempo actual del microondas
 */
uint16_t MEF_GetTiempoPantalla(void);

/**
 * @brief Inicializa el microondas
 */
void MEF_Microondas_Init(void);

/**
 * @brief Actualiza el estado del microondas
 * @param tecla Tecla pulsada
 */
void MEF_ActualizarEstado(uint8_t tecla);

/**
 * @brief Actualiza las salidas del microondas
 */
void MEF_ActualizarSalidas(void);

/**
 * @brief Obtiene los segundos restantes del microondas
 * @return Segundos restantes del microondas
 */
uint16_t MEF_ObtenerSegundosRestantes(void);

#endif