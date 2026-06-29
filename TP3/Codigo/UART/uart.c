/**
 * @file uart.c
 */

#include "uart.h"
#include <avr/interrupt.h>
#include <avr/io.h>

/** Tamaños de los buffers circulares (DEBEN ser potencias de 2 para poder usar
 * (& (SIZE - 1)) en lugar del operador módulo %) */
#define UART_RX_BUF_SIZE 64
#define UART_TX_BUF_SIZE 64

// Valor que debe tener UBRR para que el BAUD RATE sea de ~9600 bps (a 16MHz)
#define BAUD 103

/* --- Buffer de recepción (productor: ISR RX, consumidor: main) --- */
static volatile uint8_t rx_buf[UART_RX_BUF_SIZE];
static volatile uint8_t rx_head;
static volatile uint8_t rx_tail;
static volatile uint8_t rx_msg_flag;

/* --- Buffer de transmisión (productor: main, consumidor: ISR UDRE) --- */
static volatile uint8_t tx_buf[UART_TX_BUF_SIZE];
static volatile uint8_t tx_head;
static volatile uint8_t tx_tail;

void uart_init(void)
{
  rx_head = 0;
  rx_tail = 0;
  rx_msg_flag = 0;

  tx_head = 0;
  tx_tail = 0;

  UBRR0H = (BAUD >> 8);
  UBRR0L = BAUD;
#if USE_2X
  UCSR0A |= (1 << U2X0);
#else
  UCSR0A &= ~(1 << U2X0);
#endif

  /* 8 bits de datos, sin paridad, 1 bit de stop, modo asíncrono */
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

  /* TX/RX habilitados; solo interrupción de recepción al inicio */
  UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
}

void uart_tx_interrupt_enable(void) { UCSR0B |= (1 << UDRIE0); }

void uart_tx_interrupt_disable(void) { UCSR0B &= ~(1 << UDRIE0); }

uint8_t uart_send_char(uint8_t c)
{
  uint8_t next_head = (tx_head + 1) & (UART_TX_BUF_SIZE - 1);

  if (next_head == tx_tail)
  {
    return 0; // Buffer lleno
  }

  tx_buf[tx_head] = c;
  tx_head = next_head;
  return 1;
}

uint8_t uart_transfer(const char *str)
{
  if (str == 0)
  {
    return 0;
  }

  // Contar longitud de la cadena
  uint8_t len = 0;
  while (str[len] != '\0')
  {
    len++;
  }

  // Calcular espacio disponible en el buffer TX
  uint8_t used = (tx_head - tx_tail) & (UART_TX_BUF_SIZE - 1);
  uint8_t free_space = (UART_TX_BUF_SIZE - 1) - used;

  if (len > free_space)
  {
    return 0; // No cabe toda la cadena
  }

  // Encolar caracteres
  for (uint8_t i = 0; i < len; i++)
  {
    uart_send_char(str[i]);
  }

  // Activar transmisión por interrupción
  uart_tx_interrupt_enable();
  return 1;
}

uint8_t uart_read(uint8_t *data)
{
  if (rx_head == rx_tail)
  {
    return 0;
  }

  *data = rx_buf[rx_tail];
  rx_tail = (rx_tail + 1) & (UART_RX_BUF_SIZE - 1);
  return 1;
}

uint8_t uart_has_message(void) { return rx_msg_flag; }

void uart_clear_message(void) { rx_msg_flag = 0; }

ISR(USART_UDRE_vect)
{
  if (tx_head != tx_tail)
  {
    UDR0 = tx_buf[tx_tail];
    tx_tail = (tx_tail + 1) & (UART_TX_BUF_SIZE - 1);
  }

  // Si el buffer quedó vacío, deshabilitar interrupción
  if (tx_head == tx_tail)
  {
    uart_tx_interrupt_disable();
  }
}

ISR(USART_RX_vect)
{
  uint8_t data = UDR0;
  uint8_t next_head = (rx_head + 1) & (UART_RX_BUF_SIZE - 1);

  if (next_head != rx_tail)
  {
    rx_buf[rx_head] = data;
    rx_head = next_head;

    if (data == '\n' || data == '\r')
    {
      rx_msg_flag = 1;
    }
  }
}
