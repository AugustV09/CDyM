#ifndef INCISOC_H
#define INCISOC_H

#include "../Definitions.h"

#include "../utils/NeoPixelUtils.h"

#include <stdint.h>

// Prende los led rojos, luego los azules

// volatile: que el compilador no haga optimizaciones en esto...

/*
    Ej: escribo un 1, luego de 100 ms un 0 y asi.
    Ve que nadie lee esa variable port... entonces escribe
    todo de una en esa variable y hace 100101 de una
*/

void NeoPixelRotarRojoAzul(volatile uint8_t *port, uint8_t pin);

#endif