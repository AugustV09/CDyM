#include "MEF_Microondas.h"
#include "../KeyPad/KeyPad.h"
#include "../LCD/lcd.h"
#include "../Reloj/Reloj.h"
#include <avr/io.h>
#include <stdio.h>

#define POSICION_MEDIO_TIMER 5
#define MAX_TIME_SEGUNDOS (99u * 60u + 59u) // 5999 s  →  99:59

/* --- Enumeraciones Internas --- */
enum EstadosMicroondas
{
    MICROONDAS_INGRESO,
    MICROONDAS_COCINANDO,
    MICROONDAS_PAUSADO,
    MICROONDAS_FIN_COCCION,
};

enum EstadosPuerta
{
    MICROONDAS_PUERTA_CERRADA,
    MICROONDAS_PUERTA_ABIERTA,
};

enum Encendidos
{
    ENCENDIDO_MAGNETRON = (1 << 0),
    ENCENDIDO_LUZ = (1 << 1),
    ENCENDIDO_LUZ_TERMINADO = (1 << 2),
};

/* --- Estructura de Datos Interna --- */
struct EstructuraMicroondas
{
    enum EstadosMicroondas estado;
    enum EstadosPuerta puerta;
    uint16_t tiempoPantalla;   // BCD decimal MMSS: 0130 == 01:30
    uint16_t tiempoSegundos;   // Segundos reales restantes de coccion
    uint8_t tiempoFinSegundos; // Cuenta regresiva 5 s al finalizar
};

static struct EstructuraMicroondas self;

/* --- Conversion y Formateo --- */

static const char *EstadosToString(enum EstadosMicroondas estado)
{
    switch (estado)
    {
    case MICROONDAS_INGRESO:
        return "Ingrese tiempo";
    case MICROONDAS_COCINANDO:
        return "Cocinando...";
    case MICROONDAS_PAUSADO:
        return "Pausado";
    case MICROONDAS_FIN_COCCION:
        return "Finalizado";
    default:
        return "ESTADO_DESCONOCIDO";
    }
}

static void formatearTiempoDisplay(uint16_t tiempo, uint8_t *out_str)
{
    uint8_t minutos = (uint8_t)(tiempo / 100u);
    uint8_t segundos = (uint8_t)(tiempo % 100u);
    sprintf((char *)out_str, "%02d:%02d", minutos, segundos);
}

/*
 * Pantalla -> segundos reales (MM:SS estandar).
 * Si el usuario ingreso SS >= 60, se saturan a 59.
 */
static void TiempoSegundosDesdePantalla(uint16_t *tiempoSegundos,
                                        uint16_t tiempoPantalla)
{
    uint16_t minutos = tiempoPantalla / 100u;
    uint16_t segundos = tiempoPantalla % 100u;

    if (segundos > 59u)
        segundos = 59u;
    if (minutos > 99u)
        minutos = 99u;

    *tiempoSegundos = (uint16_t)(minutos * 60u + segundos);
    if (*tiempoSegundos > MAX_TIME_SEGUNDOS)
        *tiempoSegundos = MAX_TIME_SEGUNDOS;
}

/*
 * Segundos reales -> pantalla MM:SS estandar.
 * Segundos siempre en [0, 59].
 */
static void TiempoPantallaDesdeSegundos(uint16_t *tiempoPantalla,
                                        uint16_t tiempoSegundos)
{
    uint16_t minutos = tiempoSegundos / 60u;
    uint16_t segundos = tiempoSegundos % 60u;

    if (minutos > 99u)
    {
        minutos = 99u;
        segundos = 59u;
    }

    *tiempoPantalla = (uint16_t)(minutos * 100u + segundos);
}

/* --- Salidas de Hardware --- */

static void AplicarSalidas(int8_t encendidos)
{
    if (encendidos & ENCENDIDO_MAGNETRON)
        MEF_MICROONDAS_PUERTO_MAGNETRON |= (1 << MEF_MICROONDAS_PIN_MAGNETRON);
    else
        MEF_MICROONDAS_PUERTO_MAGNETRON &= ~(1 << MEF_MICROONDAS_PIN_MAGNETRON);

    if (encendidos & ENCENDIDO_LUZ)
        MEF_MICROONDAS_PUERTO_LUZ_INTERIOR |= (1 << MEF_MICROONDAS_PIN_LUZ_INTERIOR);
    else
        MEF_MICROONDAS_PUERTO_LUZ_INTERIOR &= ~(1 << MEF_MICROONDAS_PIN_LUZ_INTERIOR);

    if (encendidos & ENCENDIDO_LUZ_TERMINADO)
        MEF_MICROONDAS_PUERTO_LUZ_TERMINADO |= (1 << MEF_MICROONDAS_PIN_LUZ_TERMINADO);
    else
        MEF_MICROONDAS_PUERTO_LUZ_TERMINADO &= ~(1 << MEF_MICROONDAS_PIN_LUZ_TERMINADO);
}

static void ActualizarSalidasHardware(void)
{
    int8_t encendidos = 0;
    switch (self.estado)
    {
    case MICROONDAS_COCINANDO:
        encendidos = ENCENDIDO_LUZ | ENCENDIDO_MAGNETRON;
        break;
    case MICROONDAS_PAUSADO:
        encendidos = ENCENDIDO_LUZ;
        break;
    case MICROONDAS_FIN_COCCION:
        encendidos = (self.tiempoFinSegundos % 2) ? ENCENDIDO_LUZ_TERMINADO : 0;
        break;
    default:
        encendidos = 0;
        break;
    }
    AplicarSalidas(encendidos);
}

/* --- Logica Interna --- */

static void ResetVariablesTiempo(void)
{
    Reloj_StopCountingSeconds();
    self.estado = MICROONDAS_INGRESO;
    self.tiempoPantalla = 0;
    self.tiempoSegundos = 0;
    self.tiempoFinSegundos = 0;
}

static void IniciarCoccion(void)
{
    if (self.tiempoPantalla == 0 || self.puerta == MICROONDAS_PUERTA_ABIERTA)
        return;

    if (self.estado == MICROONDAS_INGRESO)
        TiempoSegundosDesdePantalla(&self.tiempoSegundos, self.tiempoPantalla);

    Reloj_RestartSeconds();
    Reloj_StartCountingSeconds();
    self.estado = MICROONDAS_COCINANDO;
}

static void SumarTiempoRapido(uint16_t segundos)
{
    if (self.estado == MICROONDAS_INGRESO)
        TiempoSegundosDesdePantalla(&self.tiempoSegundos, self.tiempoPantalla);

    self.tiempoSegundos += segundos;
    if (self.tiempoSegundos > MAX_TIME_SEGUNDOS)
        self.tiempoSegundos = MAX_TIME_SEGUNDOS;

    TiempoPantallaDesdeSegundos(&self.tiempoPantalla, self.tiempoSegundos);
}

static void RegistrarTeclaIngreso(uint8_t tecla)
{
    if (self.estado != MICROONDAS_INGRESO || self.tiempoPantalla >= 1000u)
        return;

    uint8_t valorNumerico = tecla - '0';
    self.tiempoPantalla = (uint16_t)(self.tiempoPantalla * 10u + valorNumerico);
}

static void ControlarPuerta(void)
{
    if (self.puerta == MICROONDAS_PUERTA_CERRADA)
    {
        if (self.estado == MICROONDAS_COCINANDO)
        {
            Reloj_StopCountingSeconds();
            self.estado = MICROONDAS_PAUSADO;
        }
        else if (self.estado == MICROONDAS_FIN_COCCION)
        {
            ResetVariablesTiempo();
        }
        self.puerta = MICROONDAS_PUERTA_ABIERTA;
    }
    else
    {
        self.puerta = MICROONDAS_PUERTA_CERRADA;
    }
}

/* --- Manejadores por Estado --- */

static void EstadoIngreso(uint8_t tecla)
{
    if (tecla >= '0' && tecla <= '9')
        RegistrarTeclaIngreso(tecla);
    else
    {
        switch (tecla)
        {
        case 'A':
            IniciarCoccion();
            break;
        case 'B':
            ResetVariablesTiempo();
            break;
        case 'C':
            SumarTiempoRapido(30);
            IniciarCoccion();
            break;
        case 'D':
            ControlarPuerta();
            break;
        }
    }
}

static void EstadoCocinando(uint8_t tecla)
{
    if (Reloj_HaySegundo())
    {
        Reloj_LimpiarSegundo();

        if (self.tiempoSegundos > 0)
        {
            self.tiempoSegundos--;
            TiempoPantallaDesdeSegundos(&self.tiempoPantalla, self.tiempoSegundos);
        }

        if (self.tiempoSegundos == 0)
        {
            self.estado = MICROONDAS_FIN_COCCION;
            self.tiempoFinSegundos = 5;
            Reloj_RestartSeconds();
            Reloj_StartCountingSeconds();
            return;
        }
    }

    switch (tecla)
    {
    case 'B':
        Reloj_StopCountingSeconds();
        self.estado = MICROONDAS_PAUSADO;
        break;
    case 'C':
        SumarTiempoRapido(30);
        break;
    case 'D':
        ControlarPuerta();
        break;
    }
}

static void EstadoPausado(uint8_t tecla)
{
    switch (tecla)
    {
    case 'A':
        IniciarCoccion();
        break;
    case 'B':
        ResetVariablesTiempo();
        break;
    case 'C':
        SumarTiempoRapido(30);
        IniciarCoccion();
        break;
    case 'D':
        ControlarPuerta();
        break;
    }
}

static void EstadoFinCoccion(uint8_t tecla)
{
    if (Reloj_HaySegundo())
    {
        Reloj_LimpiarSegundo();
        if (self.tiempoFinSegundos > 0)
            self.tiempoFinSegundos--;

        if (self.tiempoFinSegundos == 0)
        {
            ResetVariablesTiempo();
            return;
        }
    }

    switch (tecla)
    {
    case 'B':
        ResetVariablesTiempo();
        break;
    case 'C':
        SumarTiempoRapido(30);
        IniciarCoccion();
        break;
    case 'D':
        ControlarPuerta();
        break;
    }
}

static void ActualizarEstadoMEF(uint8_t tecla)
{
    switch (self.estado)
    {
    case MICROONDAS_INGRESO:
        EstadoIngreso(tecla);
        break;
    case MICROONDAS_COCINANDO:
        EstadoCocinando(tecla);
        break;
    case MICROONDAS_PAUSADO:
        EstadoPausado(tecla);
        break;
    case MICROONDAS_FIN_COCCION:
        EstadoFinCoccion(tecla);
        break;
    }
}

/* ========================================================================== */
/* INTERFAZ PUBLICA                                                           */
/* ========================================================================== */

void Microondas_Init(void)
{
    MEF_MICROONDAS_DD_MAGNETRON |= (1 << MEF_MICROONDAS_PIN_MAGNETRON);
    MEF_MICROONDAS_DD_LUZ_INTERIOR |= (1 << MEF_MICROONDAS_PIN_LUZ_INTERIOR);
    MEF_MICROONDAS_DD_LUZ_TERMINADO |= (1 << MEF_MICROONDAS_PIN_LUZ_TERMINADO);

    Reloj_Init();
    ResetVariablesTiempo();
    self.puerta = MICROONDAS_PUERTA_CERRADA;

    LCD_Init();
    LCDcursorOFF();
    LCDWriteString((const uint8_t *)EstadosToString(self.estado), 0, 1);
}

void Microondas_Actualizar(void)
{
    uint8_t tecla;
    if (!KEYPAD_Scan(&tecla))
        tecla = 0;

    enum EstadosMicroondas estadoAnterior = self.estado;
    enum EstadosPuerta puertaAnterior = self.puerta;

    ActualizarEstadoMEF(tecla);
    ActualizarSalidasHardware();

    uint8_t mostrarDisplay[6];
    formatearTiempoDisplay(self.tiempoPantalla, mostrarDisplay);

    if (estadoAnterior != self.estado || puertaAnterior != self.puerta)
    {
        LCDclr();
        LCDvisible();
        if (self.puerta == MICROONDAS_PUERTA_ABIERTA)
            LCDWriteString((const uint8_t *)"PUERTA ABIERTA", 0, 1);
        else
            LCDWriteString((const uint8_t *)EstadosToString(self.estado), 0, 1);
    }

    LCDWriteString(mostrarDisplay, POSICION_MEDIO_TIMER, 0);

    if (self.estado == MICROONDAS_FIN_COCCION)
    {
        if (self.tiempoFinSegundos % 2)
            LCDvisible();
        else
            LCDblank();
    }
}