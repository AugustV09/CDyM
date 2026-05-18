#ifndef MEF_MICROONDAS_H
#define MEF_MICROONDAS_H

#include <stdint.h>

/*
 * A partir de aca empieza la personalizacion del usuario
 * Puede cambiar:
 * Los pines y puertos del:
 * - Magnetron
 * - Luz interior
 * - Luz de terminado
 */

#define MEF_MICROONDAS_DD_MAGNETRON DDRB
#define MEF_MICROONDAS_PUERTO_MAGNETRON PORTB
#define MEF_MICROONDAS_PIN_MAGNETRON PB5

#define MEF_MICROONDAS_DD_LUZ_INTERIOR DDRC
#define MEF_MICROONDAS_PUERTO_LUZ_INTERIOR PORTC
#define MEF_MICROONDAS_PIN_LUZ_INTERIOR PC4

#define MEF_MICROONDAS_DD_LUZ_TERMINADO DDRC
#define MEF_MICROONDAS_PUERTO_LUZ_TERMINADO PORTC
#define MEF_MICROONDAS_PIN_LUZ_TERMINADO PC5

/*
 * Aca termina la personalizacion del usuario
 */

/* Configuración de temporización (Depende de la frecuencia de llamada) */
#define TICKS_POR_SEGUNDOS 40

/**
 * @brief Inicializa el microondas
 */
void Microondas_Init(void);

/**
 * @brief Actualiza el estado del microondas
 */
void Microondas_Actualizar(void);

#endif /* MICROONDAS_H */