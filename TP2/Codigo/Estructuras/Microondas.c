#include "Microondas.h"
#include "Errores.h"

// Helpers de cambio de estado!

int8_t cambiarEstado(struct Microondas *self, uint8_t estado)
{
    self->estadoMicroondas = estado;
    return SIN_ERROR;
}

int8_t CambiarEstadoPuerta(struct Microondas *self, uint8_t estado)
{
    self->estadoPuerta = estado;
    return SIN_ERROR;
}

// Cambios de estados para las pulsaciones!

int8_t MicroondasIntentarVolverIngreso(struct Microondas *self)
{
    self->estadoMicroondas = MICROONDAS_INGRESO;

    return SIN_ERROR;
}

int8_t MicroondasIntentarInicioCoccion(struct Microondas *self)
{
    // Chequear tiempo y intentar cocina
    self->estadoMicroondas = MICROONDAS_COCINANDO;

    /*
        TERMINAR!
        Deberia iniciar clock y pasarle una funcion que:
        Actualice el display y cuando el tiempo sea 0 ->
        int8_t MicroondasTermino(struct Microondas *self);
    */

    return SIN_ERROR;
}

int8_t MicroondasIntentarPausaCoccion(struct Microondas *self)
{
    self->estadoMicroondas = MICROONDAS_PAUSADO;

    return SIN_ERROR;
}

int8_t MicroondasIntentarFinCoccion(struct Microondas *self)
{
    self->estadoMicroondas = MICROONDAS_FIN_COCCION;

    /*
        Deberia INICIAR CLOCK de 5 segs
        Y pasarle la funcion IntentarVolverIngreso;
    */

    return SIN_ERROR;
}

int8_t PuertaEstaAbierta(const struct Microondas *self)
{
    return self->estadoPuerta == PUERTA_ABIERTA;
}
int8_t PuertaEstaCerrada(const struct Microondas *self)
{
    return self->estadoPuerta == PUERTA_CERRADA;
}

int8_t BotonA(struct Microondas *self)
{

    // Si tiempo es mayor a cero y la puerta esta cerrada
    return MicroondasIntentarInicioCoccion(self);
}

int8_t BotonB(struct Microondas *self)
{

    switch (self->estadoMicroondas)
    {
    case MICROONDAS_INGRESO:
        // Borrar tiempo
        return SIN_ERROR;
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

int8_t BotonC(struct Microondas *self)
{
    switch (self->estadoMicroondas)
    {
    case MICROONDAS_INGRESO:
    case MICROONDAS_PAUSADO:
    case MICROONDAS_COCINANDO:
    case MICROONDAS_FIN_COCCION:
        // Suma +30 en el timer
        // Si esta en ingreso, pausado, fin_cocina encender... para mi si!
        return PuertaEstaCerrada(self) ? MicroondasIntentarInicioCoccion(self) : SIN_ERROR;
    default:
        return ERROR_ESTADO_DESCONOCIDO;
    }
}

int8_t BotonD(struct Microondas *self)
{
    // Este boton solo cambia el estado de la puerta!
    // Luego se fija que si la puerta esta abierta se debe pausar!

    uint8_t estadoSiguiente = PuertaEstaAbierta(self) ? PUERTA_CERRADA : PUERTA_ABIERTA;
    return CambiarEstadoPuerta(self, estadoSiguiente);
}

int8_t Encender(const struct Microondas *self, int8_t encendidos)
{
    *self->pinLuzTerminado.port = ((encendidos & ENCENDIDO_LUZ_TERMINADO) != 0) << self->pinLuzTerminado.pin;
    *self->pinLuzInterior.port = ((encendidos & ENCENDIDO_LUZ) != 0) << self->pinLuzInterior.pin;
    *self->pinMagnetron.port = ((encendidos & ENCENDIDO_MAGNETRON) != 0) << self->pinMagnetron.pin;

    return SIN_ERROR;
}

// Funciones visibles por el usuario

void MicroondasInicializarMicroondas(struct Microondas *self,
                                     struct PuertoPin pinMagnetron,
                                     struct PuertoPin pinLuzInterior,
                                     struct PuertoPin pinLuzTerminado)
{
    self->estadoMicroondas = MICROONDAS_INGRESO;
    self->estadoPuerta = PUERTA_CERRADA;
    self->pinMagnetron = pinMagnetron;
    self->pinLuzInterior = pinLuzInterior;
    self->pinLuzTerminado = pinLuzTerminado;
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
        // return SumarTimer(self, tecla); HACER ALGO QUEQUE
        return SIN_ERROR;
    case TECLA_ASTERISCO:
    case TECLA_NUMERAL:
        return SIN_ERROR; // NO HACER NADA QUEQUE
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

int8_t MicroondasTermino(struct Microondas *self)
{

    /*
        TERMINAR!

        Deberia IniciarClock(); con 5 segundos
        Y a los 5 segudos cambiar a
    */

    return self->estadoMicroondas == MICROONDAS_FIN_COCCION;
}

int8_t MicroondasAplicarCambiosDeEstado(const struct Microondas *self)
{
    int8_t encendidos;

    switch (self->estadoMicroondas)
    {
    case MICROONDAS_FIN_COCCION:
        encendidos = ENCENDIDO_LUZ_TERMINADO;
        break;
    case MICROONDAS_INGRESO:
    case MICROONDAS_PAUSADO:
        encendidos = 0;
        break;
    case MICROONDAS_COCINANDO:
        encendidos = ENCENDIDO_LUZ | ENCENDIDO_MAGNETRON;
        break;
    default:
        return ERROR_ESTADO_DESCONOCIDO;
        break;
    }

    return Encender(self, encendidos);
}