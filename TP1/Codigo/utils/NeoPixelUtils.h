#ifndef NEOPIXELUTILS_H
#define NEOPIXELUTILS_H

/*
    Funciones basicas para poder mandar los colores
    sin tener que estar pensando en la arquitectura

    Solo se debe usar para la maxima abstraccion y sin problemas
    la funcion: 
    
        NeoPixelSendColorArray

    void NeoPixelSendColorArray(volatile uint8_t *port, uint8_t pin, uint8_t* colores, uint8_t cantLeds);
*/

#include "AtmegaThings.h"
#include "../Definitions.h" // Importante para los calculos de ciclos
#include <stdint.h>

// Definiciones basicas:

/* Tiempos para WS2812B 16MHz (1 ciclo = 62.5ns)
 * Basado en el datasheet:
 * T0H: 0.4us  (aprox 6 ciclos)
 * T1H: 0.8us  (aprox 12 ciclos)
 * T0L: 0.85us (aprox 13 ciclos)
 * T1L: 0.45us (aprox 7 ciclos)
 */

// Tiempos datasheet en microsegundos
#define _T0H_TIME 0.40
#define _T1H_TIME 0.80
#define _T0L_TIME 0.85
#define _T1L_TIME 0.45
#define _RES_TIME 300.0

// Overhead medido 5 ciclos
#define _BIT_OVERHEAD_US (5 * (1000000.0 / F_CPU))

// Tiempos netos que debe esperar _delay_us (descontando overhead del bit)
#define _T0H_DELAY_US (_T0H_TIME - _BIT_OVERHEAD_US)
#define _T1H_DELAY_US (_T1H_TIME - _BIT_OVERHEAD_US)
#define _T0L_DELAY_US (_T0L_TIME - _BIT_OVERHEAD_US) 
#define _T1L_DELAY_US (_T1L_TIME - _BIT_OVERHEAD_US)

#define NEOPIXEL_SEND_BIT_1(PORT, PIN) \
    do                                 \
    {                                  \
        set_bit(PORT, PIN);            \
        _delay_us(_T1H_DELAY_US);      \
        clear_bit(PORT, PIN);          \
        _delay_us(_T1L_DELAY_US);      \
    } while (0)

#define NEOPIXEL_SEND_BIT_0(PORT, PIN) \
    do                                 \
    {                                  \
        set_bit(PORT, PIN);            \
        _delay_us(_T0H_DELAY_US);      \
        clear_bit(PORT, PIN);          \
        _delay_us(_T0L_DELAY_US);      \
    } while (0)

#define NEOPIXEL_SEND_RES(PORT, PIN) \
    do                               \
    {                                \
        clear_bit(PORT, PIN);        \
        _delay_us(_RES_TIME);        \
    } while (0)

/**
 * Envía un byte completo al NeoPixel
 */
#define NEOPIXEL_SEND_PIXEL_BYTE(byte, reg, pin) \
    do                                           \
    {                                            \
        for (int8_t i = 7; i >= 0; i--)          \
        {                                        \
            if ((byte) & (1 << i))               \
            {                                    \
                NEOPIXEL_SEND_BIT_1(reg, pin);   \
            }                                    \
            else                                 \
            {                                    \
                NEOPIXEL_SEND_BIT_0(reg, pin);   \
            }                                    \
        }                                        \
    } while (0)



// volatile: que el compilador no haga optimizaciones en esto...

/*
    Ej: escribo un 1, luego de 100 ms un 0 y asi.
    Ve que nadie lee esa variable port... entonces escribe
    todo de una en esa variable y hace 100101 de una
*/

// Escribe todo el NeoPixel con los colores elegidos
void NeoPixelSendColorArray(volatile uint8_t *port, uint8_t pin, uint8_t* colores, uint8_t cantLeds);

#endif