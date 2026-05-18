#include "MEF_Microondas.h"
#include <avr/io.h>
#include "../Errores.h"

#define DD_MAGNETRON DDRB
#define PUERTO_MAGNETRON PORTB
#define PIN_MAGNETRON PB5

#define DD_LUZ_INTERIOR DDRC
#define PUERTO_LUZ_INTERIOR PORTC
#define PIN_LUZ_INTERIOR PC4

#define DD_LUZ_TERMINADO DDRC
#define PUERTO_LUZ_TERMINADO PORTC
#define PIN_LUZ_TERMINADO PC5

enum Encendidos
{
    ENCENDIDO_MAGNETRON = (1 << 0),
    ENCENDIDO_LUZ = (1 << 1),
    ENCENDIDO_LUZ_TERMINADO = (1 << 2),
};

struct TemporizadorTicks
{
    uint8_t ticksASegundos;
    uint8_t ticks;
    uint16_t tiempoSegundos;
};

struct EstructuraInternaMEF
{
    enum EstadosMicroondas estadoMicroondas;
    enum EstadosPuerta estadoPuerta;
    uint16_t tiempoPantalla;
    struct TemporizadorTicks temporizador;
    struct TemporizadorTicks temporizadorFin;
};

static struct EstructuraInternaMEF instancia_mef;

enum EstadosMicroondas MEF_GetEstadoMicroondas(void) { return instancia_mef.estadoMicroondas; }
uint16_t MEF_GetTiempoPantalla(void) { return instancia_mef.tiempoPantalla; }

static void TiempoSegundosDesdeTiempoPantalla(uint16_t *tiempoSegundos, uint16_t tiempoPantalla)
{
    uint16_t minutos = tiempoPantalla / 100;
    uint16_t segundos = tiempoPantalla % 100;
    *tiempoSegundos = (uint16_t)(minutos * 60u + segundos);
}

static void TiempoPantallaDesdeTiempoSegundos(uint16_t *tiempoPantalla, uint16_t tiempoSegundos)
{
    uint32_t minutos = tiempoSegundos / 60;
    uint32_t segundos = tiempoSegundos % 60;

    // Si por la locura del ingreso el tiempo supera los 99 minutos reales (5940 segundos),
    // lo topamos en 99 minutos para que no desborde los 4 dígitos del display (99:59).
    if (minutos > 99)
    {
        minutos = 99;
        segundos += 60;
    }

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

static uint8_t MEF_MicroondasSumarTiempo(uint16_t segundos)
{
    TiempoSegundosDesdeTiempoPantalla(&instancia_mef.temporizador.tiempoSegundos, instancia_mef.tiempoPantalla);
    instancia_mef.temporizador.tiempoSegundos += segundos;
    TiempoPantallaDesdeTiempoSegundos(&instancia_mef.tiempoPantalla, instancia_mef.temporizador.tiempoSegundos);
    return SIN_ERROR;
}

static uint8_t MEF_MicroondasSumarTiempoTecla(uint8_t tecla)
{
    if (instancia_mef.estadoMicroondas != MEF_MICROONDAS_INGRESO)
        return ERROR_SUMAR_TIEMPO_CUANDO_ESTA_ANDANDO;

    // Si ya tiene 4 dígitos (ej: 9999), cualquier otra tecla lo pasaría a 5 dígitos.
    // El máximo valor representable en el display será 9999.
    if (instancia_mef.tiempoPantalla >= 1000)
        return ERROR_SUPERAR_TIEMPO_MAXIMO;

    uint8_t valorNumerico = tecla - '0';

    // Desplazamiento clásico hacia la izquierda
    instancia_mef.tiempoPantalla = (instancia_mef.tiempoPantalla * 10) + valorNumerico;

    return SIN_ERROR;
}

static uint8_t MEF_MicroondasReinicio(void)
{
    instancia_mef.estadoMicroondas = MEF_MICROONDAS_INGRESO;
    instancia_mef.tiempoPantalla = 0;

    instancia_mef.temporizador.tiempoSegundos = 0;
    instancia_mef.temporizadorFin.tiempoSegundos = 0;

    instancia_mef.temporizador.ticks = 0;
    instancia_mef.temporizadorFin.ticks = 0;

    return SIN_ERROR;
}

static uint8_t MEF_MicroondasIniciarCoccion(void)
{
    if (instancia_mef.tiempoPantalla == 0)
        return ERROR_TIEMPO_INVALIDO;

    if (instancia_mef.estadoPuerta == MEF_MICROONDAS_PUERTA_ABIERTA)
        return ERROR_INICIAR_MICROONDAS_PUERTA_ABIERTA;

    TiempoSegundosDesdeTiempoPantalla(&instancia_mef.temporizador.tiempoSegundos, instancia_mef.tiempoPantalla);
    instancia_mef.estadoMicroondas = MEF_MICROONDAS_COCINANDO;

    return SIN_ERROR;
}

static uint8_t MEF_MicroondasPausar(void)
{
    instancia_mef.estadoMicroondas = MEF_MICROONDAS_PAUSADO;
    return SIN_ERROR;
}

static uint8_t MEF_MicroondasFinalizar(void)
{
    instancia_mef.estadoMicroondas = MEF_MICROONDAS_FIN_COCCION;
    instancia_mef.temporizadorFin.tiempoSegundos = 5;
    instancia_mef.temporizadorFin.ticks = 0;
    return SIN_ERROR;
}

static uint8_t MEF_MicroondasPuerta(void)
{
    if (instancia_mef.estadoPuerta == MEF_MICROONDAS_PUERTA_CERRADA)
    {
        uint8_t error = SIN_ERROR;
        if (instancia_mef.estadoMicroondas == MEF_MICROONDAS_COCINANDO)
            error = MEF_MicroondasPausar();

        instancia_mef.estadoPuerta = MEF_MICROONDAS_PUERTA_ABIERTA;
        return error;
    }
    else
    {
        instancia_mef.estadoPuerta = MEF_MICROONDAS_PUERTA_CERRADA;
    }
    return SIN_ERROR;
}

static uint8_t MEF_MicroondasIngreso(uint8_t tecla)
{
    switch (tecla)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return MEF_MicroondasSumarTiempoTecla(tecla);
    case 'A':
        return MEF_MicroondasIniciarCoccion();
    case 'B':
        return MEF_MicroondasReinicio();
    case 'C':
    {
        uint8_t error = MEF_MicroondasSumarTiempo(30);
        if (error == SIN_ERROR)
            return MEF_MicroondasIniciarCoccion();
        else
            return error;
    }
    case 'D':
        return MEF_MicroondasPuerta();
    default:
        return SIN_ERROR;
    }
}

static uint8_t MEF_MicroondasCocinado(uint8_t tecla)
{
    TemporizadorTicksDecrementar(&instancia_mef.temporizador);
    TiempoPantallaDesdeTiempoSegundos(&instancia_mef.tiempoPantalla, instancia_mef.temporizador.tiempoSegundos);

    if (instancia_mef.temporizador.tiempoSegundos == 0)
        return MEF_MicroondasFinalizar();

    switch (tecla)
    {
    case 'A':
        return SIN_ERROR;
    case 'B':
        return MEF_MicroondasPausar();
    case 'C':
        return MEF_MicroondasSumarTiempo(30);
    case 'D':
        return MEF_MicroondasPuerta();
    default:
        return SIN_ERROR;
    }
}

static uint8_t MEF_MicroondasPausado(uint8_t tecla)
{
    switch (tecla)
    {
    case 'A':
        return MEF_MicroondasIniciarCoccion();
    case 'B':
        return MEF_MicroondasReinicio();
    case 'C':
    {
        uint8_t error = MEF_MicroondasSumarTiempo(30);
        if (error == SIN_ERROR)
            return MEF_MicroondasIniciarCoccion();
        else
            return error;
    }
    case 'D':
        return MEF_MicroondasPuerta();
    default:
        return SIN_ERROR;
    }
}

static uint8_t MEF_MicroondasFin(uint8_t tecla)
{
    TemporizadorTicksDecrementar(&instancia_mef.temporizadorFin);

    if (instancia_mef.temporizadorFin.tiempoSegundos == 0)
        return MEF_MicroondasReinicio();

    switch (tecla)
    {
    case 'A':
        return SIN_ERROR;
    case 'B':
        return MEF_MicroondasReinicio();
    case 'C':
    {
        uint8_t error = MEF_MicroondasSumarTiempo(30);
        if (error == SIN_ERROR)
            return MEF_MicroondasIniciarCoccion();
        else
            return error;
    }
    case 'D':
        return MEF_MicroondasPuerta();
    default:
        return SIN_ERROR;
    }
}

static int8_t Encender(int8_t encendidos)
{
    if (encendidos & ENCENDIDO_MAGNETRON)
        PUERTO_MAGNETRON |= (1 << PIN_MAGNETRON);
    else
        PUERTO_MAGNETRON &= ~(1 << PIN_MAGNETRON);

    if (encendidos & ENCENDIDO_LUZ)
        PUERTO_LUZ_INTERIOR |= (1 << PIN_LUZ_INTERIOR);
    else
        PUERTO_LUZ_INTERIOR &= ~(1 << PIN_LUZ_INTERIOR);

    if (encendidos & ENCENDIDO_LUZ_TERMINADO)
        PUERTO_LUZ_TERMINADO |= (1 << PIN_LUZ_TERMINADO);
    else
        PUERTO_LUZ_TERMINADO &= ~(1 << PIN_LUZ_TERMINADO);

    return SIN_ERROR;
}

// --- Interfaz Pública ---

void MEF_Microondas_Init(void)
{
    // Usamos el define para cargar la configuración de los timers internos
    instancia_mef.temporizador.ticksASegundos = TICKS_POR_SEGUNDOS;
    instancia_mef.temporizadorFin.ticksASegundos = TICKS_POR_SEGUNDOS;

    DD_MAGNETRON |= (1 << PIN_MAGNETRON);
    DD_LUZ_INTERIOR |= (1 << PIN_LUZ_INTERIOR);
    DD_LUZ_TERMINADO |= (1 << PIN_LUZ_TERMINADO);

    MEF_MicroondasReinicio();

    instancia_mef.estadoPuerta = MEF_MICROONDAS_PUERTA_CERRADA;
}

uint8_t MEF_ActualizarEstado(uint8_t tecla)
{
    switch (instancia_mef.estadoMicroondas)
    {
    case MEF_MICROONDAS_INGRESO:
        return MEF_MicroondasIngreso(tecla);
    case MEF_MICROONDAS_COCINANDO:
        return MEF_MicroondasCocinado(tecla);
    case MEF_MICROONDAS_PAUSADO:
        return MEF_MicroondasPausado(tecla);
    case MEF_MICROONDAS_FIN_COCCION:
        return MEF_MicroondasFin(tecla);
    default:
        return ERROR_ESTADO_DESCONOCIDO;
    }
}

void MEF_ActualizarSalidas(void)
{
    int8_t encendidos;

    switch (instancia_mef.estadoMicroondas)
    {
    case MEF_MICROONDAS_COCINANDO:
        encendidos = ENCENDIDO_LUZ | ENCENDIDO_MAGNETRON;
        break;

    case MEF_MICROONDAS_FIN_COCCION:
        encendidos = (instancia_mef.temporizadorFin.tiempoSegundos % 2) ? ENCENDIDO_LUZ_TERMINADO : 0;
        break;

    default:
        encendidos = 0;
        break;
    }

    Encender(encendidos);
}

uint8_t MEF_ObtenerSegundosRestantes(void)
{
    switch (instancia_mef.estadoMicroondas)
    {
    case MEF_MICROONDAS_COCINANDO:
        return instancia_mef.temporizador.tiempoSegundos;
    case MEF_MICROONDAS_FIN_COCCION:
        return instancia_mef.temporizadorFin.tiempoSegundos;
    default:
        return 0;
    }
}