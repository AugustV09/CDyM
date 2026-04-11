#ifndef INCISOB_H
#define INCISOB_H

#include "../Definitions.h"

#include <stdint.h>

// Hace que el led rebot
// DEBE TENER UN LED SI O SI UN LED (1, UNO!! -> UNO!!!) ENCENDIDO ASI FUNCIONA!

// volatile: que el compilador no haga optimizaciones en esto...

/*
    Ej: escribo un 1, luego de 100 ms un 0 y asi.
    Ve que nadie lee esa variable port... entonces escribe
    todo de una en esa variable y hace 100101 de una
*/

void LEDsRebote(volatile uint8_t *port);

#endif