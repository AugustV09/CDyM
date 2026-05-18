#ifndef MEF_MICROONDAS_H
#define MEF_MICROONDAS_H

#include <stdint.h>

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
enum EstadosMicroondas MEF_GetEstadoMicroondas(void);
uint16_t MEF_GetTiempoPantalla(void);
uint8_t MEF_GetPosTimerPantalla(void);

// Interfaz pública (Ya no requiere parámetros)
void MEF_Microondas_Init(void);
uint8_t MEF_ActualizarEstado(uint8_t tecla);
void MEF_ActualizarSalidas(void);

#endif