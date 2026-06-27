/**
 * @file uart.h
 * @brief Driver UART0 asincrónico (9600 bps, 8N1) con transmisión y recepción por interrupciones.
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>

/** Tamaño del buffer circular de recepción (comandos desde la PC). */
#define UART_RX_BUF_SIZE 64
// Valor que debe tener UBRR para que el BAUD RATE sea de ~9600 bps
#define BAUD 103

/**
 * @brief Inicializa USART0: 9600 bps, 8N1, habilita TX/RX e interrupción de recepción.
 * @details UDRIE queda deshabilitada hasta que se solicite una transferencia.
 */
void uart_init(void);

/**
 * @brief Inicia la transmisión de una cadena terminada en '\\0' byte a byte por interrupción UDRE.
 * @param str Puntero al mensaje. No debe modificarse ni liberarse hasta que uart_tx_busy() sea 0.
 * @return 1 si la transferencia comenzó, 0 si el transmisor aún está ocupado.
 */
uint8_t uart_transfer(const char *str);

/**
 * @brief Indica si hay una transferencia en curso.
 * @return 1 ocupado, 0 libre.
 */
uint8_t uart_tx_busy(void);

/**
 * @brief Cantidad de bytes disponibles en el buffer de recepción.
 */
uint8_t uart_rx_available(void);

/**
 * @brief Lee un byte del buffer de recepción (foreground).
 * @return Byte leído (0–255) o -1 si el buffer está vacío.
 */
int16_t uart_read(void);

#endif /* UART_H */
