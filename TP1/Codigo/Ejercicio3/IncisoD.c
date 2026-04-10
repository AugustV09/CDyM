#include "IncisoD.h"

void NeoPixelRotarSalidaVerdes(volatile uint8_t *port, uint8_t pin)
{

    static uint8_t pos_verde = 0;

    for (uint8_t i = 0; i < TOTAL_LEDS_NEOPIXEL; i++)
    {
        if (i == pos_verde)
        {
            // Enviamos Verde Máximo (GRB: 255, 0, 0)
            NEOPIXEL_SEND_MAX_GREEN(*port, pin);
        }
        else
        {
            // Enviamos Apagado (GRB: 0, 0, 0)
            NEOPIXEL_SEND_OFF(*port, pin);
        }
    }

    ++pos_verde;
    pos_verde %= TOTAL_LEDS_NEOPIXEL;

    NEOPIXEL_SEND_RES(*port, pin);
}