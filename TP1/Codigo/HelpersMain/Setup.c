
#include "Setup.h"

#include "Definitions.h"


void configurarPuertos()
{
    // 1. Configurar LEDs individuales como salida
    ConfigPuertoLEDs = 0b11111111;

    // 2. Configurar NeoPixel como salida (bit 0)
    ConfigNeoPixel |= (1 << PORTB0);

    // 3. Configurar Pulsadores como entrada (bits 0 y 1)
    ConfigPulsadores &= ~((1 << PulsadorLeds) | (1 << PulsadorNeoPixel));

    // 4. ACTIVAR PULL-UPS (Escribiendo 1 en el registro de Salida siendo entrada)
    PuertoPulsadoresSalida |= (1 << PulsadorLeds) | (1 << PulsadorNeoPixel);
}