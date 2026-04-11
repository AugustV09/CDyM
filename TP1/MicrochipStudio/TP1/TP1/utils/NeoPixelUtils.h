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
#define _T0H_TIME 0.40 // 6.4 ciclos -> 6 ciclos
#define _T1H_TIME 0.80 // 12.8 ciclos -> 13 ciclos
#define _T0L_TIME 0.85 // 13.6 ciclos -> 14 ciclos
#define _T1L_TIME 0.45 // 7.2 ciclos -> 7 ciclos
#define _RES_TIME 100

#define NOP1 "nop \n\t"
#define NOP2 NOP1 NOP1
#define NOP4 NOP2 NOP2
#define NOP8 NOP4 NOP4

// Macro para enviar un '1' lógico (Pulso Largo en Alto)
#define NEOPIXEL_SEND_BIT_1(PORT_PTR, MASK)                                                             \
    asm volatile(                                                                                       \
        "ld   r16, %a0  \n\t" /* [2 ciclos] Lee el estado actual del puerto desde el puntero */         \
        "or   r16, %1   \n\t" /* [1 ciclo]  Pone a '1' el bit correspondiente en el registro r16 */     \
        "st   %a0, r16  \n\t" /* [2 ciclos] ESCRIBE al puerto: El pin sube a ALTO aquí */               \
        NOP8                  /* [8 ciclos] Mantiene el pin en ALTO (Total T1H: 2+1+2+8 = 13 ciclos) */ \
        "ld   r16, %a0  \n\t" /* [2 ciclos] Prepara la bajada: Lee el puerto otra vez */                \
        "and  r16, %2   \n\t" /* [1 ciclo]  Aplica la máscara invertida (pone el bit en '0') */         \
        "st   %a0, r16  \n\t" /* [2 ciclos] ESCRIBE al puerto: El pin baja a BAJO aquí */               \
        NOP4 NOP2 NOP1        /* [7 ciclos] Espera mínima en bajo antes de que el loop de C siga */     \
        ::                                                                                              \
            "e"(PORT_PTR),    /* %a0: Usa un registro de puntero (X, Y o Z) */                          \
        "r"(MASK),            /* %1:  Máscara para el OR (poner a 1) */                                 \
        "r"((uint8_t)~(MASK)) /* %2:  Máscara invertida para el AND (poner a 0) */                      \
        : "r16")              /* Avisa que usamos r16 para que el compilador no lo use para otra cosa */

// Macro para enviar un '0' lógico (Pulso Corto en Alto)
#define NEOPIXEL_SEND_BIT_0(PORT_PTR, MASK)                                                           \
    asm volatile(                                                                                     \
        "ld   r16, %a0  \n\t" /* [2 ciclos] Lee puerto */                                             \
        "or   r16, %1   \n\t" /* [1 ciclo]  Prepara el bit en 1 */                                    \
        "st   %a0, r16  \n\t" /* [2 ciclos] ESCRIBE al puerto: El pin sube a ALTO aquí */             \
        NOP1                  /* [1 ciclo]  Espera mínima (Total T0H: 2+1+2+1 = 6 ciclos) */          \
        "ld   r16, %a0  \n\t" /* [2 ciclos] Prepara la bajada */                                      \
        "and  r16, %2   \n\t" /* [1 ciclo]  Prepara el bit en 0 */                                    \
        "st   %a0, r16  \n\t" /* [2 ciclos] ESCRIBE al puerto: El pin baja a BAJO aquí */             \
        NOP8 NOP4 NOP2        /* [14 ciclos] Mantiene en bajo (T0L) mientras esperamos el sig. bit */ \
        ::                                                                                            \
            "e"(PORT_PTR),                                                                            \
        "r"(MASK),                                                                                    \
        "r"((uint8_t)~(MASK))                                                                         \
        : "r16")

#define NEOPIXEL_SEND_RES(PORT, PIN) \
    do                               \
    {                                \
        clear_bit(*PORT, PIN);       \
        _delay_us(_RES_TIME);        \
    } while (0)

/**
 * Envía un byte completo al NeoPixel
 */
#define NEOPIXEL_SEND_PIXEL_BYTE(byte, reg, pin) \
    do                                           \
    {                                            \
        uint8_t mask = 1 << pin;                 \
        for (int8_t i = 7; i >= 0; i--)          \
        {                                        \
            if ((byte) & (1 << i))               \
            {                                    \
                NEOPIXEL_SEND_BIT_1(reg, mask);  \
            }                                    \
            else                                 \
            {                                    \
                NEOPIXEL_SEND_BIT_0(reg, mask);  \
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
void NeoPixelSendColorArray(volatile uint8_t *port, const uint8_t pin, const uint8_t *colores, const uint8_t cantLeds);

#endif