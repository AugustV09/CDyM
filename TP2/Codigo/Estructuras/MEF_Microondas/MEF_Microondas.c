#include "MEF_Microondas.h"

#include "../Teclas.h"
#include "../Errores.h"

enum Encendidos
{
    ENCENDIDO_MAGNETRON = (1 << 0),
    ENCENDIDO_LUZ = (1 << 1),
    ENCENDIDO_LUZ_TERMINADO = (1 << 2),
};

static const uint16_t POTENCIAS_10[] = {1000, 100, 10, 1};

/* tiempoPantalla (MMSS, ej: 130 = 1:30) → tiempoSegundos (ej: 90) */
static void TiempoSegundosDesdeTiempoPantalla(uint16_t *tiempoSegundos, uint16_t tiempoPantalla)
{
    uint16_t minutos = tiempoPantalla / 100;
    uint16_t segundos = tiempoPantalla % 100;
    *tiempoSegundos = (uint16_t)(minutos * 60u + segundos);
}

/* tiempoSegundos → tiempoPantalla (MMSS) */
static void TiempoPantallaDesdeTiempoSegundos(uint16_t *tiempoPantalla, uint16_t tiempoSegundos)
{
    uint32_t minutos = tiempoSegundos / 60;
    uint32_t segundos = tiempoSegundos % 60;
    *tiempoPantalla = (uint16_t)(minutos * 100u + segundos);
}

static void TemporizadorTicksDecrementar(struct TemporizadorTicks *self)
{
    self->ticks++;
    if (self->ticks >= self->ticksASegundos)
    {
        self->ticks = 0;
        if (self->tiempoSegundos > 0)
            self->tiempoSegundos--;
    }
}

static uint8_t MEF_MicroondasSumarTiempo(struct MEF_Microondas *self, uint16_t segundos)
{
    TiempoSegundosDesdeTiempoPantalla(&self->temporizador.tiempoSegundos, self->tiempoPantalla);

    self->temporizador.tiempoSegundos += segundos;

    TiempoPantallaDesdeTiempoSegundos(&self->tiempoPantalla, self->temporizador.tiempoSegundos);

    return SIN_ERROR;
}

static uint8_t MEF_MicroondasSumarTiempoTecla(struct MEF_Microondas *self, uint8_t tecla)
{
    if (self->estadoMicroondas != MEF_MICROONDAS_INGRESO)
        return ERROR_SUMAR_TIEMPO_CUANDO_ESTA_ANDANDO;

    if (self->posTimerPantalla > 3)
        return ERROR_SUPERAR_TIEMPO_MAXIMO;

    uint8_t valorNumerico = tecla - TECLA_0;

    if (self->posTimerPantalla == 2 && valorNumerico > 5) // Máx 99:59
        return ERROR_TIEMPO_INVALIDO;

    self->tiempoPantalla += (uint16_t)valorNumerico * POTENCIAS_10[self->posTimerPantalla];
    self->posTimerPantalla++;

    return SIN_ERROR;
}

static uint8_t MEF_MicroondasReinicio(struct MEF_Microondas *self)
{
    self->estadoMicroondas = MEF_MICROONDAS_INGRESO;
    self->posTimerPantalla = 0;
    self->tiempoPantalla = 0;

    self->temporizador.tiempoSegundos = 0;
    self->temporizadorFin.tiempoSegundos = 0;

    self->temporizador.ticks = 0;
    self->temporizadorFin.ticks = 0;

    return SIN_ERROR;
}

static uint8_t MEF_MicroondasIniciarCoccion(struct MEF_Microondas *self)
{
    if (self->tiempoPantalla == 0)
        return ERROR_TIEMPO_INVALIDO;

    if (self->estadoPuerta == MEF_MICROONDAS_PUERTA_ABIERTA)
        return ERROR_INICIAR_MICROONDAS_PUERTA_ABIERTA;

    // Convertir lo que el usuario ingresó en pantalla a segundos reales
    TiempoSegundosDesdeTiempoPantalla(&self->temporizador.tiempoSegundos, self->tiempoPantalla);

    self->estadoMicroondas = MEF_MICROONDAS_COCINANDO;

    return SIN_ERROR;
}

static uint8_t MEF_MicroondasPausar(struct MEF_Microondas *self)
{
    self->estadoMicroondas = MEF_MICROONDAS_PAUSADO;

    return SIN_ERROR;
}

static uint8_t MEF_MicroondasFinalizar(struct MEF_Microondas *self)
{
    self->estadoMicroondas = MEF_MICROONDAS_FIN_COCCION;

    self->temporizadorFin.tiempoSegundos = 5;
    self->temporizadorFin.ticks = 0;

    return SIN_ERROR;
}

static uint8_t MEF_MicroondasPuerta(struct MEF_Microondas *self)
{
    if (self->estadoPuerta == MEF_MICROONDAS_PUERTA_CERRADA)
    {
        uint8_t error = SIN_ERROR;
        if (self->estadoMicroondas == MEF_MICROONDAS_COCINANDO)
            error = MEF_MicroondasPausar(self);

        self->estadoPuerta = MEF_MICROONDAS_PUERTA_ABIERTA;
        return error;
    }

    else
        self->estadoPuerta = MEF_MICROONDAS_PUERTA_CERRADA;

    return SIN_ERROR;
}

static uint8_t MEF_MicroondasIngreso(struct MEF_Microondas *self, uint8_t tecla)
{
    switch (tecla)
    {
    case TECLA_0:
    case TECLA_1:
    case TECLA_2:
    case TECLA_3:
    case TECLA_4:
    case TECLA_5:
    case TECLA_6:
    case TECLA_7:
    case TECLA_8:
    case TECLA_9:
        return MEF_MicroondasSumarTiempoTecla(self, tecla);
    case TECLA_A:
        return MEF_MicroondasIniciarCoccion(self);
    case TECLA_B:
        return MEF_MicroondasReinicio(self);
    case TECLA_C:
    {
        uint8_t error = MEF_MicroondasSumarTiempo(self, 30);
        if (error == SIN_ERROR)
            return MEF_MicroondasIniciarCoccion(self);
        else
            return error;
    }
    case TECLA_D:
        return MEF_MicroondasPuerta(self);

    default:
        return SIN_ERROR;
        break;
    }
}

static uint8_t MEF_MicroondasCocinado(struct MEF_Microondas *self, uint8_t tecla)
{

    TemporizadorTicksDecrementar(&self->temporizador);
    TiempoPantallaDesdeTiempoSegundos(&self->tiempoPantalla, self->temporizador.tiempoSegundos);

    if (self->temporizador.tiempoSegundos == 0)
        return MEF_MicroondasFinalizar(self);

    switch (tecla)
    {
    case TECLA_A:
        return SIN_ERROR;
    case TECLA_B:
        return MEF_MicroondasPausar(self);
    case TECLA_C:
        return MEF_MicroondasSumarTiempo(self, 30);
    case TECLA_D:
        return MEF_MicroondasPuerta(self);
    default:
        return SIN_ERROR;
        break;
    }
}

static uint8_t MEF_MicroondasPausado(struct MEF_Microondas *self, uint8_t tecla)
{
    switch (tecla)
    {
    case TECLA_A:
        return MEF_MicroondasIniciarCoccion(self);
    case TECLA_B:
        return MEF_MicroondasReinicio(self);
    case TECLA_C:
    {
        uint8_t error = MEF_MicroondasSumarTiempo(self, 30);
        if (error == SIN_ERROR)
            return MEF_MicroondasIniciarCoccion(self);
        else
            return error;
    }
    case TECLA_D:
        return MEF_MicroondasPuerta(self);

    default:
        return SIN_ERROR;
        break;
    }
}

static uint8_t MEF_MicroondasFin(struct MEF_Microondas *self, uint8_t tecla)
{

    TemporizadorTicksDecrementar(&self->temporizadorFin);

    if (self->temporizadorFin.tiempoSegundos == 0)
        return MEF_MicroondasReinicio(self);

    switch (tecla)
    {
    case TECLA_A:
        return SIN_ERROR;
    case TECLA_B:
        return MEF_MicroondasReinicio(self);
    case TECLA_C:
    {
        uint8_t error = MEF_MicroondasSumarTiempo(self, 30);
        if (error == SIN_ERROR)
            return MEF_MicroondasIniciarCoccion(self);
        else
            return error;
    }
    case TECLA_D:
        return MEF_MicroondasPuerta(self);

    default:
        return SIN_ERROR;
        break;
    }
}

static int8_t Encender(const struct MEF_Microondas *self, int8_t encendidos)
{
    // Magnetrón — PORTB
    if (encendidos & ENCENDIDO_MAGNETRON)
        *self->pinMagnetron.port |= (1 << self->pinMagnetron.pin);
    else
        *self->pinMagnetron.port &= ~(1 << self->pinMagnetron.pin);

    // Luz interior — PORTC
    if (encendidos & ENCENDIDO_LUZ)
        *self->pinLuzInterior.port |= (1 << self->pinLuzInterior.pin);
    else
        *self->pinLuzInterior.port &= ~(1 << self->pinLuzInterior.pin);

    // Luz terminado — PORTC
    if (encendidos & ENCENDIDO_LUZ_TERMINADO)
        *self->pinLuzTerminado.port |= (1 << self->pinLuzTerminado.pin);
    else
        *self->pinLuzTerminado.port &= ~(1 << self->pinLuzTerminado.pin);

    return SIN_ERROR;
}

// Lo publico:

void MEF_Microondas_Init(struct MEF_Microondas *self,
                         uint8_t ticksASegundos,
                         struct PuertoPin pinMagnetron,
                         struct PuertoPin pinLuzInterior,
                         struct PuertoPin pinLuzTerminado)
{
    self->temporizador.ticksASegundos = ticksASegundos;
    self->temporizadorFin.ticksASegundos = ticksASegundos;

    self->pinMagnetron = pinMagnetron;
    self->pinLuzInterior = pinLuzInterior;
    self->pinLuzTerminado = pinLuzTerminado;

    MEF_MicroondasReinicio(self);

    self->estadoPuerta = MEF_MICROONDAS_PUERTA_CERRADA;
}

uint8_t MEF_ActualizarEstado(struct MEF_Microondas *self, uint8_t tecla)
{

    switch (self->estadoMicroondas)
    {
    case MEF_MICROONDAS_INGRESO:
        return MEF_MicroondasIngreso(self, tecla);
    case MEF_MICROONDAS_COCINANDO:
        return MEF_MicroondasCocinado(self, tecla);
    case MEF_MICROONDAS_PAUSADO:
        return MEF_MicroondasPausado(self, tecla);
    case MEF_MICROONDAS_FIN_COCCION:
        return MEF_MicroondasFin(self, tecla);
    default:
        return ERROR_ESTADO_DESCONOCIDO;
    }
}

void MEF_ActualizarSalidas(const struct MEF_Microondas *self)
{
    int8_t encendidos;

    switch (self->estadoMicroondas)
    {
    case MEF_MICROONDAS_COCINANDO:
        encendidos = ENCENDIDO_LUZ | ENCENDIDO_MAGNETRON;
        break;

    case MEF_MICROONDAS_FIN_COCCION:
        encendidos = (self->temporizadorFin.tiempoSegundos % 2) ? ENCENDIDO_LUZ_TERMINADO : 0;
        break;

    default:
        encendidos = 0;
        break;
    }

    Encender(self, encendidos);
}