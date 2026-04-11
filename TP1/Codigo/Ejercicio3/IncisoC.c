#include "IncisoC.h"

void NeoPixelRotarRojoAzul(volatile uint8_t *port, uint8_t pin)
{
    static uint8_t rojosPrendidos = 0;

    uint8_t colores[TOTAL_LEDS_NEOPIXEL * 3];

    //Calcula los colores
    for (uint8_t i = 0; i < TOTAL_LEDS_NEOPIXEL; i++)
    {
        uint8_t esImpar = i & 1;

        uint8_t g = 0;
        uint8_t r = (!rojosPrendidos && esImpar) ? 255 : 0; // Rojos en pares
        uint8_t b = (rojosPrendidos && !esImpar) ? 255 : 0;   // Azules en impares

        colores[i * 3 + 0] = g; // G
        colores[i * 3 + 1] = r; // R
        colores[i * 3 + 2] = b; // B
    }

    //Ya calculados los envia
    NeoPixelSendColorArray(port, pin, colores, TOTAL_LEDS_NEOPIXEL);

    rojosPrendidos = !rojosPrendidos;
}