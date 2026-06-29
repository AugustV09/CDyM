#include "dht11.h"
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define PIN_DHT11 PC0
#define TIMEOUT_LOOPS ((uint16_t)(F_CPU / 1000000UL * 100UL / 4UL))

static const dht_data_t DHT_ERROR = {0u, 0u, 0u, 0u, 0xFFu};

static uint8_t wait_pin_low(void)
{
    uint16_t t = TIMEOUT_LOOPS;
    while (PINC & (1u << PIN_DHT11))
    {
        if (--t == 0u)
            return 0u;
    }
    return 1u;
}

static uint8_t wait_pin_high(void)
{
    uint16_t t = TIMEOUT_LOOPS;
    while (!(PINC & (1u << PIN_DHT11)))
    {
        if (--t == 0u)
            return 0u;
    }
    return 1u;
}

void dht11_init(void)
{
    DDRC &= ~(1u << PIN_DHT11);
    PORTC |= (1u << PIN_DHT11);
}

dht_data_t dht11_read(void)
{
    uint8_t bytes[5] = {0u};
    uint8_t sreg_saved;
    uint8_t ok = 1u;

    /* --- 1. Pulso de inicio: mínimo 18 ms en bajo --- */
    DDRC |= (1u << PIN_DHT11);
    PORTC &= ~(1u << PIN_DHT11);
    _delay_ms(18);
    PORTC |= (1u << PIN_DHT11);
    DDRC &= ~(1u << PIN_DHT11);
    _delay_us(20);

    /* --- 2. Deshabilitar interrupciones --- */
    sreg_saved = SREG;
    cli();

    /* --- 3. Handshake --- */
    ok = ok && wait_pin_low();
    ok = ok && wait_pin_high();

    /* --- 4. Leer 40 bits --- */
    for (uint8_t i = 0u; i < 5u && ok; i++)
    {
        for (uint8_t bit = 0u; bit < 8u && ok; bit++)
        {
            ok = ok && wait_pin_low();
            ok = ok && wait_pin_high();
            _delay_us(40);
            bytes[i] <<= 1u;
            if (PINC & (1u << PIN_DHT11))
                bytes[i] |= 1u;
        }
    }

    /* --- 5. Restaurar interrupciones --- */
    SREG = sreg_saved;

    /* --- 6. Timeout: único motivo para devolver DHT_ERROR --- */
    if (!ok)
        return DHT_ERROR;

    /* --- 7. Devolver datos crudos sin validar checksum.
     *        El llamador es responsable de verificarlo.     --- */
    {
        dht_data_t d;
        d.rh_int = bytes[0];
        d.rh_dec = bytes[1];
        d.temp_int = bytes[2];
        d.temp_dec = bytes[3];
        d.checksum = bytes[4];
        return d;
    }
}

uint8_t dht11_checksum_ok(const dht_data_t *d)
{
    uint8_t suma = d->rh_int + d->rh_dec + d->temp_int + d->temp_dec;
    return (suma == d->checksum) ? 1u : 0u;
}
