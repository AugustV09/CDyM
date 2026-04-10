#ifndef NEOPIXELUTILS_H
#define NEOPIXELUTILS_H

/*
    Funciones basicas para poder mandar los colores
    sin tener que estar pensando en la arquitectura
*/

#include "AtmegaThings.h"
#include "../Definitions.h" // Importante para los calculos de ciclos


// Definiciones basicas:

/* Tiempos para WS2812B 16MHz (1 ciclo = 62.5ns)
 * Basado en el datasheet:
 * T0H: 0.4us  (aprox 6 ciclos)
 * T1H: 0.8us  (aprox 12 ciclos)
 * T0L: 0.85us (aprox 13 ciclos)
 * T1L: 0.45us (aprox 7 ciclos)
 */

// En microsegundos:

#define _T0H_TIME 0.4
#define _T1H_TIME 0.8
#define _T0L_TIME 0.85
#define _T1L_TIME 0.45
#define _RES_TIME 1 // Arriba de 0.5

#define NEOPIXEL_SEND_BIT_1(PORT, PIN) \
    do                                 \
    {                                  \
        set_bit(PORT, PIN);            \
        _delay_us(_T1H_TIME);          \
        clear_bit(PORT, PIN);          \
        _delay_us(_T1L_TIME);          \
    } while (0)

#define NEOPIXEL_SEND_BIT_0(PORT, PIN) \
    do                                 \
    {                                  \
        set_bit(PORT, PIN);            \
        _delay_us(_T0H_TIME);          \
        clear_bit(PORT, PIN);          \
        _delay_us(_T0L_TIME);          \
    } while (0)

#define NEOPIXEL_SEND_RES(PORT, PIN) \
    do                               \
    {                                \
        clear_bit(PORT, PIN);        \
        _delay_us(_RES_TIME);        \
    } while (0)

// Funciones hardcodeadas
// Prende una luz en verde intenso (0, 255, 0 en formato GRB)
#define NEOPIXEL_SEND_MAX_RED(PORT, PIN) \
    do                                   \
    {                                    \
        /* VERDE: 8 bits en 0 */         \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);           \
        /* ROJO: 8 bits en 1 */          \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);           \
        /* AZUL: 8 bits en 0 */          \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);           \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);           \
    } while (0)

// Prende una luz en verde intenso (255, 0, 0 en formato GRB)
#define NEOPIXEL_SEND_MAX_GREEN(PORT, PIN) \
    do                                     \
    {                                      \
        /* VERDE: 8 bits en 1 */           \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);             \
        /* ROJO: 8 bits en 0 */            \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);             \
        /* AZUL: 8 bits en 0 */            \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);             \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);             \
    } while (0)

// Prende una luz en azul intenso (0, 0, 255 en formato GRB)
#define NEOPIXEL_SEND_MAX_BLUE(PORT, PIN) \
    do                                    \
    {                                     \
        /* VERDE: 8 bits en 0 */          \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);            \
        /* ROJO: 8 bits en 0 */           \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_0(PORT, PIN);            \
        /* AZUL: 8 bits en 1 */           \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);            \
        NEOPIXEL_SEND_BIT_1(PORT, PIN);            \
    } while (0)

// Envía 24 bits en 0 para apagar un LED por completo (GRB: 0, 0, 0)
#define NEOPIXEL_SEND_OFF(PORT, PIN)    \
    do                                  \
    {                                   \
        /* VERDE: 8 bits en 0 */        \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        /* ROJO: 8 bits en 0 */         \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        /* AZUL: 8 bits en 0 */         \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
        NEOPIXEL_SEND_BIT_0(PORT, PIN); \
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
                NEOPIXEL_SEND_BIT_1(reg, pin);            \
            }                                    \
            else                                 \
            {                                    \
                NEOPIXEL_SEND_BIT_0(reg, pin);            \
            }                                    \
        }                                        \
    } while (0)

#endif