#include "Microondas.h"

#include "../Errores.h"
#include "../Teclas.h"

#include <stdio.h>

static void formatearTiempo(uint16_t tiempo, char *out_str)
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

    error = MEF_ActualizarEstado(&self->mef, /*tecla*/ 0);
    MEF_ActualizarSalidas(&self->mef);

    // Idea para el error + display (Si te parece):

    /*
        Cuando se reciba un error
        se muestra en el display en lugar del tiempo
        hasta que se presione cualquier tecla, if(tecla != NO_TECLA)
        y vuelve a mostrar el tiempo.

        O tambien por X tiempo transcurrido!
    */

    // Escribir en el display

    char mostrarDisplay[6]; // 00:00 + null

    formatearTiempo(self->mef.tiempoPantalla, mostrarDisplay);

    // LCD_MostrarString(display, mostrarDisplay);
}