#include "IncisoC.h"

#define esImpar(num) ((num) & 1)

void NeoPixelRotarRojoAzul(volatile uint8_t *port, uint8_t pin)
{

    static uint8_t rojosPrendidos = 0;

    for (uint8_t ledActual = 0; ledActual < TOTAL_LEDS_NEOPIXEL; ++ledActual)
    {
        if (rojosPrendidos) // Ahora tocara prender los azules
        {
            if (esImpar(ledActual))            
                NEOPIXEL_SEND_MAX_BLUE(*port, pin);            
            else            
                NEOPIXEL_SEND_OFF(*port, pin);
            
        }
        else // Ahora tocara prender los rojos
        {
            if (esImpar(ledActual))
                NEOPIXEL_SEND_OFF(*port, pin);
            else
                NEOPIXEL_SEND_MAX_RED(*port, pin);
            
        }
    }

    NEOPIXEL_SEND_RES(*port, pin);

    rojosPrendidos = !rojosPrendidos;
}

#undef esImpar