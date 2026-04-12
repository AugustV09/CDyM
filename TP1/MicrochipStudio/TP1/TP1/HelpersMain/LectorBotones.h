#ifndef LECTORBOTONES_H
#define LECTORBOTONES_H

#include "../Definitions.h"

#define estaPulsadoElBotonLeds() (!(PuertoPulsadoresEntrada & (1 << PulsadorLeds)))
#define estaPulsadoElBotonNeoPixel() (!(PuertoPulsadoresEntrada & (1 << PulsadorNeoPixel)))

#define detectarSoltado(actual, memoria, bandera) \
    do                                            \
    {                                             \
        if (memoria && !actual)                   \
        {                                         \
            bandera = !bandera;                   \
        }                                         \
        memoria = actual;                         \
    } while (0)

#endif