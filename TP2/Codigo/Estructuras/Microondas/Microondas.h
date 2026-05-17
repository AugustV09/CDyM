#ifndef MICROONDAS_H
#define MICROONDAS_H

#include "../MEF_Microondas/MEF_Microondas.h"

struct Microondas
{
    struct MEF_Microondas mef;

    // struct Teclado teclado;
    // struct Display display;
};

void Microondas_Init(struct Microondas *self,
                     uint8_t ticksASegundos,
                     struct PuertoPin pinMagnetron,
                     struct PuertoPin pinLuzInterior,
                     struct PuertoPin pinLuzTerminado);

void Microondas_Actualizar(struct Microondas *self);

#endif