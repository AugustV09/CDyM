#ifndef ERRORES_H
#define ERRORES_H

enum EstadosErrores
{
    SIN_ERROR = 0,
    ERROR_TECLA_DESCONOCIDA,
    ERROR_ESTADO_DESCONOCIDO, // No deberia ni aparecer ya que seria fallo de hardware
    ERROR_INICIAR_MICROONDAS_TIMER,
    ERROR_INICIAR_MICROONDAS_PUERTA_ABIERTA,
    ERROR_SUMAR_TIMER_TIEMPO_MAXIMO,
};

#endif