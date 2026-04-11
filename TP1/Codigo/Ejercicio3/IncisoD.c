#include "IncisoD.h"

void NeoPixelRotarSalidaVerdes(volatile uint8_t *port, uint8_t pin)
{
    static uint8_t pos_verde = 0;

    uint8_t frame[TOTAL_LEDS_NEOPIXEL * 3];

    // Calcula los colores
    for (uint8_t i = 0; i < TOTAL_LEDS_NEOPIXEL; i++)
    {
        uint8_t g = (i == pos_verde) ? 255 : 0;
        frame[i * 3 + 0] = g; // G
        frame[i * 3 + 1] = 0; // R
        frame[i * 3 + 2] = 0; // B
    }

    // Ya calculados los envia
    NeoPixelSendColorArray(port, pin, frame, TOTAL_LEDS_NEOPIXEL * 3);

    pos_verde = (pos_verde + 1) % TOTAL_LEDS_NEOPIXEL;
}