/**
 * @file uart.c
 */

#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/* --- Buffer de recepción (productor: ISR RX, consumidor: main) --- */
static volatile uint8_t rx_buf[UART_RX_BUF_SIZE];
static volatile uint8_t rx_head;
static volatile uint8_t rx_tail;

/* --- Estado de transmisión (productor: main, consumidor: ISR UDRE) --- */
static volatile const char *tx_ptr;
static volatile uint8_t tx_busy;

static uint8_t rx_push(uint8_t data)
{
    uint8_t next_head = (uint8_t)((rx_head + 1) % UART_RX_BUF_SIZE);

    if (next_head == rx_tail)
        return 0;

    rx_buf[rx_head] = data;
    rx_head = next_head;
    return 1;
}

static uint8_t rx_pop(uint8_t *data)
{
    if (rx_head == rx_tail)
        return 0;

    *data = rx_buf[rx_tail];
    rx_tail = (uint8_t)((rx_tail + 1) % UART_RX_BUF_SIZE);
    return 1;
}

void uart_init(void)
{
    rx_head = 0;
    rx_tail = 0;
    tx_ptr = 0;
    tx_busy = 0;

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

uint8_t uart_transfer(const char *str)
{
    uint8_t sreg;

    if (str == 0)
        return 0;

    sreg = SREG;
    cli();

    if (tx_busy)
    {
        SREG = sreg;
        return 0;
    }

    tx_busy = 1;
    tx_ptr = str;

    /* Primer byte: carga directa en UDR (doble buffer del hardware) */
    UDR0 = (uint8_t)str[0];
    tx_ptr++;

    /*
     * UDRE=1 mientras UDR está vacío. Si UDRIE quedara siempre habilitada,
     * la ISR se ejecutaría en bucle sin transmitir nada útil.
     * Solo se habilita UDRIE cuando quedan bytes por enviar; la ISR la
     * deshabilita al agotar la cadena.
     */
    if (*tx_ptr != '\0')
        UCSR0B |= (1 << UDRIE0);
    else
    {
        tx_busy = 0;
        tx_ptr = 0;
    }

    SREG = sreg;
    return 1;
}

uint8_t uart_tx_busy(void)
{
    return tx_busy;
}

uint8_t uart_rx_available(void)
{
    return (uint8_t)((rx_head - rx_tail + UART_RX_BUF_SIZE) % UART_RX_BUF_SIZE);
}

int16_t uart_read(void)
{
    uint8_t data;
    uint8_t sreg = SREG;

    cli();
    if (!rx_pop(&data))
    {
        SREG = sreg;
        return -1;
    }
    SREG = sreg;

    return (int16_t)data;
}

ISR(USART_UDRE_vect)
{
    if (tx_ptr == 0)
    {
        UCSR0B &= ~(1 << UDRIE0);
        tx_busy = 0;
        return;
    }

    if (*tx_ptr != '\0')
    {
        UDR0 = (uint8_t)*tx_ptr;
        tx_ptr++;
    }

    if (*tx_ptr == '\0')
    {
        UCSR0B &= ~(1 << UDRIE0);
        tx_busy = 0;
        tx_ptr = 0;
    }
}

ISR(USART_RX_vect)
{
    uint8_t data = UDR0;
    (void)rx_push(data);
}
