#ifndef MEF_MICROONDAS_H
#define MEF_MICROONDAS_H

#include <stdint.h>

enum EstadosMicroondas
{
    MEF_MICROONDAS_INGRESO,
    MEF_MICROONDAS_COCINANDO,
    MEF_MICROONDAS_PAUSADO,
    MEF_MICROONDAS_FIN_COCCION,
};

enum EstadosPuerta
{
    MEF_MICROONDAS_PUERTA_CERRADA,
    MEF_MICROONDAS_PUERTA_ABIERTA,
};

struct PuertoPin
{
    volatile uint8_t *port;
    uint8_t pin;
};

struct TemporizadorTicks
{
    uint8_t ticksASegundos;
    uint8_t ticks;
    uint16_t tiempoSegundos;
};

struct MEF_Microondas
{
    enum EstadosMicroondas estadoMicroondas;
    enum EstadosPuerta estadoPuerta;

    struct PuertoPin pinMagnetron;
    struct PuertoPin pinLuzInterior;
    struct PuertoPin pinLuzTerminado;

    uint8_t posTimerPantalla;
    uint16_t tiempoPantalla; // Segundos en pantalla: Si aparece 130 -> 1:30

    struct TemporizadorTicks temporizador;

    struct TemporizadorTicks temporizadorFin;
};

void MEF_Microondas_Init(struct MEF_Microondas *self,
                         uint8_t ticksASegundos,
                         struct PuertoPin pinMagnetron,
                         struct PuertoPin pinLuzInterior,
                         struct PuertoPin pinLuzTerminado);

uint8_t MEF_ActualizarEstado(struct MEF_Microondas *self, uint8_t tecla);
void MEF_ActualizarSalidas(const struct MEF_Microondas *self);

#endif