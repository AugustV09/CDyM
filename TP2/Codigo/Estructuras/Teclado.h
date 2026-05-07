#ifndef TECLADO_H
#define TECLADO_H

#include <stdint.h>

enum Teclas
{
    TECLA_0,
    TECLA_1,
    TECLA_2,
    TECLA_3,
    TECLA_4,
    TECLA_5,
    TECLA_6,
    TECLA_7,
    TECLA_8,
    TECLA_9,
    TECLA_ASTERISCO,
    TECLA_NUMERAL,
    TECLA_A,
    TECLA_B,
    TECLA_C,
    TECLA_D,
};

struct Teclado
{
    // Datos como pines y demas
};

void inicializarTeclado(struct Teclado *self);
int8_t obtenerTeclaPresionada(struct Teclado *self, uint8_t *tecla);

#endif