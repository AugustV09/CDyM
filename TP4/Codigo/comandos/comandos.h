/**
 * @file comandos.h
 * @brief Parser de comandos recibidos por UART (adaptado para TP4).
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 *
 * Comandos soportados en TP4:
 *   SET_COLOR=R,G,B   -> Fija el color del LED RGB (valores 0-255 por canal).
 *                        Ejemplo: SET_COLOR=0,255,255\r\n
 *
 */

#ifndef COMANDOS_H
#define COMANDOS_H

/**
 * @brief Vacía el buffer UART, acumula caracteres y procesa el comando
 *        cuando detecta un '\\n' o '\\r'.
 * @note  Debe llamarse desde el foreground loop al detectar uart_has_message().
 */
void procesar_comando(void);

#endif /* COMANDOS_H */
