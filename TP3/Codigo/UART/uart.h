/**
 * @file uart.h
 * @brief Driver UART0 asincrónico (9600 bps, 8N1) con transmisión y recepción
 * por interrupciones y buffer circular.
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>

/**
 * @brief Inicializa USART0: 9600 bps, 8N1, habilita TX/RX y la interrupción de
 * recepción.
 */
void uart_init(void);

/**
 * @brief Habilita la interrupción por registro de datos de transmisión vacío
 * (UDRE).
 */
void uart_tx_interrupt_enable(void);

/**
 * @brief Deshabilita la interrupción por registro de datos de transmisión vacío
 * (UDRE).
 */
void uart_tx_interrupt_disable(void);

/**
 * @brief Encola un carácter en el buffer circular de transmisión.
 * @param c Carácter a transmitir.
 * @return 1 si se encoló con éxito, 0 si el buffer de transmisión está lleno.
 */
uint8_t uart_send_char(uint8_t c);

/**
 * @brief Encola una cadena de texto completa terminada en '\0' y activa la
 * transmisión.
 * @param str Puntero a la cadena a transmitir.
 * @return 1 si toda la cadena fue encolada con éxito, 0 si no hay espacio
 * suficiente.
 */
uint8_t uart_transfer(const char *str);

/**
 * @brief Lee un carácter del buffer de recepción.
 * @param data Puntero donde se almacena el byte leído.
 * @return 1 si se leyó un byte, 0 si el buffer está vacío.
 */
uint8_t uart_read(uint8_t *data);

/**
 * @brief Indica si se ha recibido un mensaje completo terminado en '\n'.
 * @return 1 si hay un mensaje listo, 0 en caso contrario.
 */
uint8_t uart_has_message(void);

/**
 * @brief Limpia el flag de mensaje recibido.
 */
void uart_clear_message(void);

#endif /* UART_H */
