#ifndef MICROONDAS_H
#define MICROONDAS_H

#include "../Teclas.h"

#include "Errores.h"

#include <stdint.h>

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
    ENCENDIDO_MAGNETRON,
    ENCENDIDO_LUZ,
    ENCENDIDO_LUZ_TERMINADO,
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

    uint8_t posTimerPantalla;
    uint16_t tiempoPantalla; // Segundos en pantalla: Si aparece 130 -> 1:30
    uint16_t tiempoSegundos; // Si aparece 1:30 -> 90 segs
    uint8_t tiempoSegundosFin;
};

// Iniciar estructura
void MicroondasInicializarMicroondas(struct Microondas *self,
                                     struct PuertoPin pinMagnetron,
                                     struct PuertoPin pinLuzInterior,
                                     struct PuertoPin pinLuzTerminado);

// Pulsacion teclado
// Cuando se le pasa la tecla ACTUALIZA EL ESTADO NO, (NO!!) LA SALIDA.
int8_t MicroondasTecladoPresionado(struct Microondas *self, uint8_t tecla);

// ACA SE ACTUALIZAN LAS SALIDAS, LOS PINES ACA SON ENERGIZADOS!
int8_t MicroondasAplicarCambiosDeEstado(const struct Microondas *self);

static inline uint16_t MicroondasObtenerTiempoPantalla(const struct Microondas *self)
{
    return self->tiempoPantalla;
}

static inline uint16_t MicroondasObtenerTiempoSegundos(const struct Microondas *self)
{
    return self->tiempoSegundos;
}

static inline enum EstadosMicroondas MicroondasObtenerEstadoMicroondas(const struct Microondas *self)
{
    return self->estadoMicroondas;
}

static inline enum EstadosPuerta MicroondasObtenerEstadoPuerta(const struct Microondas *self)
{
    return self->estadoPuerta;
}

#endif