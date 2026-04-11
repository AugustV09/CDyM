#include "Definitions.h"
#include "HelpersMain/Setup.h"
#include "HelpersMain/LectorBotones.h"

#include "Ejercicio1/IncisoA.h"
#include "Ejercicio1/IncisoB.h"
#include "Ejercicio3/IncisoC.h"
#include "Ejercicio3/IncisoD.h"

#include <stdint.h>

int main(void)
{

    configurarPuertos();

    uint64_t contadorCiclos = 0;

    // Leds
    uint8_t seApretoBotonLeds = 0;
    uint8_t cambiarPatronLeds = 0;
    uint8_t LEDsPatronA = 0;

    // NeoPixel
    uint8_t seApretoBotonNeoPixel = 0;
    uint8_t cambiarPatronNeoPixel = 0;
    uint8_t NeoPixelPatronA = 0;

    PuertoLEDs = 0b00000001; // Un led al comienzo debe estar encendido

    while (1)
    {

        detectarSoltado(estaPulsadoElBotonLeds(), seApretoBotonLeds, cambiarPatronLeds);
        detectarSoltado(estaPulsadoElBotonNeoPixel(), seApretoBotonNeoPixel, cambiarPatronNeoPixel);

        if ((contadorCiclos % CiclosLeds) == 0)
        {
            if (cambiarPatronLeds)
            {
                LEDsPatronA = !LEDsPatronA;
                cambiarPatronLeds = 0;
            }

            if (LEDsPatronA)
                LEDsRebote(&PuertoLEDs);
            else
                LEDsRotar(&PuertoLEDs);
        }

        if ((contadorCiclos % CiclosNeoPixel) == 0)
        {
            if (cambiarPatronNeoPixel)
            {
                NeoPixelPatronA = !NeoPixelPatronA;
                cambiarPatronNeoPixel = 0;
            }

            if (NeoPixelPatronA)
                NeoPixelRotarSalidaVerdes(&PuertoNeoPixel, PinNeoPixel);
            else
                NeoPixelRotarRojoAzul(&PuertoNeoPixel, PinNeoPixel);
        }

        _delay_ms(TiempoChequeos);
        ++contadorCiclos;
    }
}