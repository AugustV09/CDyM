#include "IncisoA.h"

void LEDsRotar(volatile uint8_t *port)
{

    if (*port & (1 << (TOTAL_LEDS_INDIVIDUALES - 1)))
    {
        *port = 1;
    }

    else
        *port <<= 1;
}