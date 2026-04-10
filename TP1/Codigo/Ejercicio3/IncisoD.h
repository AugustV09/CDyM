#ifndef INCISOD_H
#define INCISOD_H

#include "../Definitions.h"

#include "../utils/NeoPixelUtils.h"

#include <stdint.h>

/*
    Prender los leds verdes de derecha a izquierda
*/

// Agarra la posicion previa en la que estaba y la mueve a la izquierda

// ES RELATIVO... DEPENDE COMO SE ORIENTE LA LED!

// volatile: que el compilador no haga optimizaciones en esto...

/*
    Ej: escribo un 1, luego de 100 ms un 0 y asi.
    Ve que nadie lee esa variable port... entonces escribe
    todo de una en esa variable y hace 100101 de una
*/

void NeoPixelRotarSalidaVerdes(volatile uint8_t *port, uint8_t pin);

#endif