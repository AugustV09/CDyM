#include "MEF_Microondas.h"
#include <avr/io.h>

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

static void MEF_MicroondasSumarTiempo(uint16_t segundos)
{
    TiempoSegundosDesdeTiempoPantalla(&instancia_mef.temporizador.tiempoSegundos, instancia_mef.tiempoPantalla);

    instancia_mef.temporizador.tiempoSegundos += segundos;

    if (instancia_mef.temporizador.tiempoSegundos > MAX_TIME)
        instancia_mef.temporizador.tiempoSegundos = MAX_TIME;

    TiempoPantallaDesdeTiempoSegundos(&instancia_mef.tiempoPantalla, instancia_mef.temporizador.tiempoSegundos);
}

static void MEF_MicroondasSumarTiempoTecla(uint8_t tecla)
{
    if (instancia_mef.estadoMicroondas != MEF_MICROONDAS_INGRESO)
        return;

    // Si ya tiene 4 dígitos (ej: 9999), cualquier otra tecla lo pasaría a 5 dígitos.
    // El máximo valor representable en el display será 9999.
    if (instancia_mef.tiempoPantalla >= 1000)
        return;

    uint8_t valorNumerico = tecla - '0';

    // Desplazamiento clásico hacia la izquierda
    instancia_mef.tiempoPantalla = (instancia_mef.tiempoPantalla * 10) + valorNumerico;
}

static void MEF_MicroondasReinicio(void)
{
    instancia_mef.estadoMicroondas = MEF_MICROONDAS_INGRESO;
    instancia_mef.tiempoPantalla = 0;

    instancia_mef.temporizador.tiempoSegundos = 0;
    instancia_mef.temporizadorFin.tiempoSegundos = 0;

    instancia_mef.temporizador.ticks = 0;
    instancia_mef.temporizadorFin.ticks = 0;
}

static void MEF_MicroondasIniciarCoccion(void)
{
    if (instancia_mef.tiempoPantalla == 0)
        return;

    if (instancia_mef.estadoPuerta == MEF_MICROONDAS_PUERTA_ABIERTA)
        return;

    TiempoSegundosDesdeTiempoPantalla(&instancia_mef.temporizador.tiempoSegundos, instancia_mef.tiempoPantalla);
    instancia_mef.estadoMicroondas = MEF_MICROONDAS_COCINANDO;
}

static void MEF_MicroondasPausar(void)
{
    instancia_mef.estadoMicroondas = MEF_MICROONDAS_PAUSADO;
}

static void MEF_MicroondasFinalizar(void)
{
    instancia_mef.estadoMicroondas = MEF_MICROONDAS_FIN_COCCION;
    instancia_mef.temporizadorFin.tiempoSegundos = 5;
    instancia_mef.temporizadorFin.ticks = 0;
}

static void MEF_MicroondasPuerta(void)
{
    if (instancia_mef.estadoPuerta == MEF_MICROONDAS_PUERTA_CERRADA)
    {
        if (instancia_mef.estadoMicroondas == MEF_MICROONDAS_COCINANDO)
            MEF_MicroondasPausar();

        instancia_mef.estadoPuerta = MEF_MICROONDAS_PUERTA_ABIERTA;
    }
    else
    {
        instancia_mef.estadoPuerta = MEF_MICROONDAS_PUERTA_CERRADA;
    }
}

static void MEF_MicroondasIngreso(uint8_t tecla)
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
        MEF_MicroondasSumarTiempoTecla(tecla);
        break;
    case 'A':
        MEF_MicroondasIniciarCoccion();
        break;
    case 'B':
        MEF_MicroondasReinicio();
        break;
    case 'C':
        MEF_MicroondasSumarTiempo(30);
        MEF_MicroondasIniciarCoccion();
        break;
    case 'D':
        MEF_MicroondasPuerta();
        break;
    default:
        break;
    }
}

static void MEF_MicroondasCocinado(uint8_t tecla)
{
    TemporizadorTicksDecrementar(&instancia_mef.temporizador);
    TiempoPantallaDesdeTiempoSegundos(&instancia_mef.tiempoPantalla, instancia_mef.temporizador.tiempoSegundos);

    if (instancia_mef.temporizador.tiempoSegundos == 0)
    {
        MEF_MicroondasFinalizar();
        return;
    }

    switch (tecla)
    {
    case 'A':
        break;
    case 'B':
        MEF_MicroondasPausar();
        break;
    case 'C':
        MEF_MicroondasSumarTiempo(30);
        MEF_MicroondasIniciarCoccion();
        break;
    case 'D':
        MEF_MicroondasPuerta();
        break;
    default:
        break;
    }
}

static void MEF_MicroondasPausado(uint8_t tecla)
{
    switch (tecla)
    {
    case 'A':
        MEF_MicroondasIniciarCoccion();
        break;
    case 'B':
        MEF_MicroondasReinicio();
        break;
    case 'C':
        MEF_MicroondasSumarTiempo(30);
        MEF_MicroondasIniciarCoccion();
        break;
    case 'D':
        MEF_MicroondasPuerta();
        break;
    default:
        break;
    }
}

static void MEF_MicroondasFin(uint8_t tecla)
{
    TemporizadorTicksDecrementar(&instancia_mef.temporizadorFin);

    if (instancia_mef.temporizadorFin.tiempoSegundos == 0)
    {
        MEF_MicroondasReinicio();
        return;
    }

    switch (tecla)
    {
    case 'A':
        break;
    case 'B':
        MEF_MicroondasReinicio();
        break;
    case 'C':
        MEF_MicroondasSumarTiempo(30);
        MEF_MicroondasIniciarCoccion();
        break;
    case 'D':
        MEF_MicroondasPuerta();
        break;
    default:
        break;
    }
}

static void Encender(int8_t encendidos)
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

void MEF_ActualizarEstado(uint8_t tecla)
{
    switch (instancia_mef.estadoMicroondas)
    {
    case MEF_MICROONDAS_INGRESO:
        MEF_MicroondasIngreso(tecla);
        break;
    case MEF_MICROONDAS_COCINANDO:
        MEF_MicroondasCocinado(tecla);
        break;
    case MEF_MICROONDAS_PAUSADO:
        MEF_MicroondasPausado(tecla);
        break;
    case MEF_MICROONDAS_FIN_COCCION:
        MEF_MicroondasFin(tecla);
        break;
    default:
        break;
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

uint16_t MEF_ObtenerSegundosRestantes(void)
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