#include "IncisoD.h"

void NeoPixelRotarSalidaVerdes(volatile uint8_t *port, uint8_t pin)
{
    static uint8_t pos_verde = 0;

    uint8_t colores[TOTAL_LEDS_NEOPIXEL * 3];

    // Calcula los colores
    for (uint8_t i = 0; i < TOTAL_LEDS_NEOPIXEL; i++)
    {
        uint8_t g = (i == pos_verde) ? 255 : 0;
        uint8_t r = 0;
        uint8_t b = 0;
        colores[i * 3 + 0] = g; // G
        colores[i * 3 + 1] = r; // R
        colores[i * 3 + 2] = b; // B
    }

    // Ya calculados los envia
    NeoPixelSendColorArray(port, pin, colores, TOTAL_LEDS_NEOPIXEL);

    pos_verde = (pos_verde + TOTAL_LEDS_NEOPIXEL - 1) % TOTAL_LEDS_NEOPIXEL;
}