/**
 * @file comandos.h
 * @brief Parser de comandos recibidos por UART.
 *
 * Comandos soportados:
 *   SET_TIME=HH:MM:SS  ->  Actualiza la hora del RTC DS3231.
 *   SET_TM=SS          ->  Cambia el intervalo de reporte (2-60 segundos).
 */

#ifndef COMANDOS_H
#define COMANDOS_H

/**
 * @brief Vacía el buffer UART, acumula caracteres y procesa el comando
 *        cuando detecta un '\n'.
 * @note  Debe llamarse desde el foreground loop al detectar uart_has_message().
 */
void procesar_comando(void);

#endif /* COMANDOS_H */
