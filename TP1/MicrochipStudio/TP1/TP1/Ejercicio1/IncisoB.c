#include "IncisoB.h"

void LEDsRebote(volatile uint8_t *port)
{
    static uint8_t moviendoIzquierda = 0;

    // 1. Decidir dirección antes de mover
    if (*port & (1 << (TOTAL_LEDS_INDIVIDUALES - 1)))
    {
        moviendoIzquierda = 0;
    }
    else if (*port & 1)
    {
        moviendoIzquierda = 1;
    }

    // 2. Ejecutar el movimiento
    if (moviendoIzquierda)
    {
        *port <<= 1;
    }
    else
    {
        *port >>= 1;
    }
}