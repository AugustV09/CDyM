#include "Microondas.h"

#include "../Errores.h"

#include "../KeyPad/KeyPad.h"

#include "../LCD/lcd.h"

#include <stdio.h>

static void formatearTiempo(uint16_t tiempo, uint8_t *out_str)
{
    uint8_t horas = tiempo / 100;
    uint8_t minutos = tiempo % 100;
    sprintf(out_str, "%02d:%02d", horas, minutos);
}

void Microondas_Init(struct Microondas *self,
                     uint8_t ticksASegundos,
                     struct PuertoPin pinMagnetron,
                     struct PuertoPin pinLuzInterior,
                     struct PuertoPin pinLuzTerminado)
{
    MEF_Microondas_Init(&self->mef,
                        ticksASegundos,
                        pinMagnetron,
                        pinLuzInterior,
                        pinLuzTerminado);
}

void Microondas_Actualizar(struct Microondas *self)
{
    // preguntar al teclado si recibio algo
    uint8_t tecla;
    uint8_t error;

    if (!KEYPAD_Scan(&tecla))
        tecla = 0;

    error = MEF_ActualizarEstado(&self->mef, tecla);
    MEF_ActualizarSalidas(&self->mef);

    uint8_t mostrarDisplay[6]; // 00:00 + null

    formatearTiempo(self->mef.tiempoPantalla, mostrarDisplay);

    // LCD_MostrarString(display, mostrarDisplay);

    LCDclearAndWriteString(mostrarDisplay);
}