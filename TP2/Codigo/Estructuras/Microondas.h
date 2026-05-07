#ifndef MICROONDAS_H
#define MICROONDAS_H

#include <stdint.h>

#include "Teclado.h"

enum EstadosMicroondas
{
    MICROONDAS_INGRESO,
    MICROONDAS_COCINANDO,
    MICROONDAS_PAUSADO,
    MICROONDAS_FIN_COCCION,
};

enum EstadosPuerta
{
    PUERTA_CERRADA,
    PUERTA_ABIERTA,
};

enum Encendidos
{
    ENCENDIDO_MAGNETRON = 1 << 0,
    ENCENDIDO_LUZ = 1 << 1,
    ENCENDIDO_LUZ_TERMINADO = 1 << 2,
};

struct PuertoPin
{
    volatile uint8_t *port;
    uint8_t pin;
};

struct Microondas
{
    uint8_t estadoMicroondas;
    uint8_t estadoPuerta;

    struct PuertoPin pinMagnetron;
    struct PuertoPin pinLuzInterior;
    struct PuertoPin pinLuzTerminado;
};

// Iniciar estructura
void MicroondasInicializarMicroondas(struct Microondas *self,
                                     struct PuertoPin pinMagnetron,
                                     struct PuertoPin pinLuzInterior,
                                     struct PuertoPin pinLuzTerminado);

// Pulsacion teclado
// Cuando se le pasa la tecla ACTUALIZA EL ESTADO NO, (NO!!) LA SALIDA.
int8_t MicroondasTecladoPresionado(struct Microondas *self, uint8_t tecla);

// Cambios SI O SI, actualiza el estado.
// El unico que por ahora puede no cambiar es el
// IntentarInicioCoccion ya que depende del timer que sea mayor a 0.

int8_t MicroondasIntentarVolverIngreso(struct Microondas *self);

int8_t MicroondasIntentarInicioCoccion(struct Microondas *self);

int8_t MicroondasIntentarPausaCoccion(struct Microondas *self);

int8_t MicroondasIntentarFinCoccion(struct Microondas *self);

// Aca se aplican los cambios de estados, la salida
// termina cambiando, es decir, se encienden los leds y el magnetron

int8_t MicroondasAplicarCambiosDeEstado(const struct Microondas *self);

#endif