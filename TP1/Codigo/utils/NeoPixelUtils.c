#include "NeoPixelUtils.h"

/*

    La idea de usar unicamente el NeoPixelSendColorArray es que:
        - Usa cli y sei el mismo
        - Ya esta calculado el tiempo para un *port sin importar cual sea ya que
            suma tiempo y se calcularon los ciclos. Asegura que sea un ptr siempre
        - El tiempo para setear todos los leds esta definido, no se puede hacer mas rapido
            por lo que no importa llamar una funcion o macro...
*/

void NeoPixelSendColorArray(volatile uint8_t *port, const uint8_t pin, const uint8_t *colores, const uint8_t cantLeds)
{
    cli();
    for (uint8_t n = 0; n < cantLeds * 3; n++)
        NEOPIXEL_SEND_PIXEL_BYTE(colores[n], port, pin);

    NEOPIXEL_SEND_RES(port, pin);
    sei();
}