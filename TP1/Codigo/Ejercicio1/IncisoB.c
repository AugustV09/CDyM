#include "IncisoB.h"

void LEDsRebote(volatile uint8_t *port)
{
    static uint8_t moviendoIzquierda = 0;

    if (moviendoIzquierda)
    {

        *port <<= 1;

        if (*port & (1 << (TOTAL_LEDS_INDIVIDUALES - 1)))
        {
            moviendoIzquierda = 0;
        }
    }
    else
    {
        *port >>= 1;

        if (*port & 1)
        {
            moviendoIzquierda = 1;
        }
    }
}