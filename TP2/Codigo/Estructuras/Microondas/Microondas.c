#include "Microondas.h"
#include "../Reloj/Reloj.h"

// --- Helpers de conversión ---------------------------------------------

static const uint16_t POTENCIAS_10[] = {1000, 100, 10, 1};

/* tiempoPantalla (MMSS, ej: 130 = 1:30) → tiempoSegundos (ej: 90) */
static void TiempoSegundosDesdeTiempoPantalla(struct Microondas *self)
{
    uint16_t minutos = self->tiempoPantalla / 100;
    uint16_t segundos = self->tiempoPantalla % 100;
    self->tiempoSegundos = (uint16_t)(minutos * 60u + segundos);
}

/* tiempoSegundos → tiempoPantalla (MMSS) */
static void TiempoPantallaDesdeTiempoSegundos(struct Microondas *self)
{
    uint32_t minutos = self->tiempoSegundos / 60;
    uint32_t segundos = self->tiempoSegundos % 60;
    self->tiempoPantalla = (uint16_t)(minutos * 100u + segundos);
}

// --- Helpers de estado de puerta ---------------------------------------------

static int8_t PuertaEstaAbierta(const struct Microondas *self)
{
    return self->estadoPuerta == PUERTA_ABIERTA;
}

static int8_t PuertaEstaCerrada(const struct Microondas *self)
{
    return self->estadoPuerta == PUERTA_CERRADA;
}

// --- Callback del reloj (llamado cada 1 segundo por la ISR) ------

static void SegunderoMicroondas(void *ctx);

// --- Cambios de estado ---------------------------------------------

int8_t MicroondasIntentarVolverIngreso(struct Microondas *self)
{
    Reloj_PararClock();

    self->estadoMicroondas = MICROONDAS_INGRESO;
    self->posTimerPantalla = 0;
    self->tiempoPantalla = 0;
    self->tiempoSegundos = 0;

    return SIN_ERROR;
}

int8_t MicroondasIntentarInicioCoccion(struct Microondas *self)
{
    // tiempoSegundos ya debe estar sincronizado antes de llamar aquí.
    self->estadoMicroondas = MICROONDAS_COCINANDO;
    Reloj_SetFunc(SegunderoMicroondas, self);
    Reloj_IniciarClock();
    return SIN_ERROR;
}

int8_t MicroondasIntentarPausaCoccion(struct Microondas *self)
{
    Reloj_PararClock();
    self->estadoMicroondas = MICROONDAS_PAUSADO;
    return SIN_ERROR;
}

int8_t MicroondasIntentarFinCoccion(struct Microondas *self)
{
    self->estadoMicroondas = MICROONDAS_FIN_COCCION;
    self->tiempoSegundosFin = 5;
    Reloj_SetFunc(SegunderoMicroondas, self);
    Reloj_IniciarClock();
    return SIN_ERROR;
}

// --- Implementación del callback ---------------------------------------------

static void SegunderoMicroondas(void *ctx)
{
    struct Microondas *self = (struct Microondas *)ctx;

    switch (self->estadoMicroondas)
    {
    case MICROONDAS_COCINANDO:
        if (self->tiempoSegundos > 0)
        {
            --self->tiempoSegundos;
            TiempoPantallaDesdeTiempoSegundos(self);
        }
        if (self->tiempoSegundos == 0)
        {
            MicroondasIntentarFinCoccion(self);
        }
        break;

    case MICROONDAS_FIN_COCCION:
        if (self->tiempoSegundosFin > 0)
        {
            --self->tiempoSegundosFin;
        }
        if (self->tiempoSegundosFin == 0)
        {
            MicroondasIntentarVolverIngreso(self);
        }
        break;

    default:
        /* Estado inesperado: frenar el reloj como salvaguarda */
        Reloj_PararClock();
        break;
    }
}

// --- Lógica de botones ---------------------------------------------

static int8_t BotonNumerico(struct Microondas *self, uint8_t tecla)
{
    if (self->estadoMicroondas != MICROONDAS_INGRESO)
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

static int8_t BotonA(struct Microondas *self)
{
    if (self->tiempoPantalla == 0)
        return ERROR_TIEMPO_INVALIDO;

    if (PuertaEstaAbierta(self))
        return ERROR_INICIAR_MICROONDAS_PUERTA_ABIERTA;

    // Convertir lo que el usuario ingresó en pantalla a segundos reales
    TiempoSegundosDesdeTiempoPantalla(self);
    return MicroondasIntentarInicioCoccion(self);
}

static int8_t BotonB(struct Microondas *self)
{
    switch (self->estadoMicroondas)
    {
    case MICROONDAS_INGRESO:
        return MicroondasIntentarVolverIngreso(self); // Borra el tiempo ingresado
    case MICROONDAS_COCINANDO:
        return MicroondasIntentarPausaCoccion(self);
    case MICROONDAS_PAUSADO:
        return MicroondasIntentarVolverIngreso(self);
    case MICROONDAS_FIN_COCCION:
        return MicroondasIntentarVolverIngreso(self);
    default:
        return ERROR_ESTADO_DESCONOCIDO;
    }
}

static int8_t BotonC(struct Microondas *self)
{
    switch (self->estadoMicroondas)
    {
    case MICROONDAS_INGRESO:
        // Inicio rápido: sincronizar pantalla → segundos, sumar 30 y arrancar
        TiempoSegundosDesdeTiempoPantalla(self);
        self->tiempoSegundos += 30;
        TiempoPantallaDesdeTiempoSegundos(self);
        return PuertaEstaCerrada(self) ? MicroondasIntentarInicioCoccion(self) : SIN_ERROR;

    case MICROONDAS_COCINANDO:
        // Sumar tiempo en caliente; el reloj sigue corriendo normalmente
        self->tiempoSegundos += 30;
        TiempoPantallaDesdeTiempoSegundos(self);
        return SIN_ERROR;

    case MICROONDAS_PAUSADO:
        // Sumar tiempo pero no reanudar automáticamente (el usuario pausó a propósito)
        self->tiempoSegundos += 30;
        TiempoPantallaDesdeTiempoSegundos(self);
        return SIN_ERROR;

    case MICROONDAS_FIN_COCCION:
        return SIN_ERROR; // Ignorar en fin de cocción

    default:
        return ERROR_ESTADO_DESCONOCIDO;
    }
}

static int8_t BotonD(struct Microondas *self)
{
    if (PuertaEstaAbierta(self))
    {
        // Cerrar puerta: solo cambia el estado, no reanuda cocción
        self->estadoPuerta = PUERTA_CERRADA;
        return SIN_ERROR;
    }
    else
    {
        // Abrir puerta
        self->estadoPuerta = PUERTA_ABIERTA;
        // Si estaba cocinando, pausar automáticamente (enunciado punto 3)
        if (self->estadoMicroondas == MICROONDAS_COCINANDO)
        {
            return MicroondasIntentarPausaCoccion(self);
        }
        return SIN_ERROR;
    }
}

// --- Salidas: LEDs y magnetrón ---------------------------------------------

static int8_t Encender(const struct Microondas *self, int8_t encendidos)
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

// --- API pública -----------------------------------------------------------
void MicroondasInicializarMicroondas(struct Microondas *self,
                                     struct PuertoPin pinMagnetron,
                                     struct PuertoPin pinLuzInterior,
                                     struct PuertoPin pinLuzTerminado)
{
    self->pinMagnetron = pinMagnetron;
    self->pinLuzInterior = pinLuzInterior;
    self->pinLuzTerminado = pinLuzTerminado;

    self->estadoPuerta = PUERTA_CERRADA;
    MicroondasIntentarVolverIngreso(self); // Deja todo en 0 y para el reloj

    MicroondasAplicarCambiosDeEstado(self);
}

int8_t MicroondasTecladoPresionado(struct Microondas *self, uint8_t tecla)
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
        return BotonNumerico(self, tecla);
    case TECLA_ASTERISCO:
    case TECLA_NUMERAL:
        return SIN_ERROR; // Sin función asignada
    case TECLA_A:
        return BotonA(self);
    case TECLA_B:
        return BotonB(self);
    case TECLA_C:
        return BotonC(self);
    case TECLA_D:
        return BotonD(self);
    default:
        return ERROR_TECLA_DESCONOCIDA;
    }
}

int8_t MicroondasAplicarCambiosDeEstado(const struct Microondas *self)
{
    int8_t encendidos;

    switch (self->estadoMicroondas)
    {
    case MICROONDAS_COCINANDO:
        encendidos = ENCENDIDO_LUZ | ENCENDIDO_MAGNETRON;
        break;

    case MICROONDAS_FIN_COCCION:
        // LED de fin parpadea usando la paridad de los segundos restantes
        encendidos = (self->tiempoSegundosFin % 2) ? ENCENDIDO_LUZ_TERMINADO : 0;
        break;

    case MICROONDAS_INGRESO:
    case MICROONDAS_PAUSADO:
        encendidos = 0;
        break;

    default:
        return ERROR_ESTADO_DESCONOCIDO;
    }

    return Encender(self, encendidos);
}