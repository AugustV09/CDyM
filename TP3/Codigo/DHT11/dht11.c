#include "dht11.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define PIN_DHT11 PC0
#define PIN_DHT11_DD DDRC
#define PIN_DHT11_PORT PORTC
#define PIN_DHT11_PIN PINC
#define BITS_DATA_DHT11 40

// Espera 20ms (asegura los >18ms requeridos al inicio de la comunicacion por parte del MCU)
#define DELAY_EN_BAJO _delay_ms(20);
// Espera 30µs (punto medio entre los 20µs-40µs requeridos durante el primer PULL-UP)
#define DELAY_EN_ALTO _delay_us(30);
// Espera 40µs (punto medio de los dos tiempos posibles (28µs < 40µs < 70µs) para el tiempo en alto entre 0 y 1)
#define DELAY_MEDIO _delay_us(40);

void dht11_init()
{
    PIN_DHT11_PORT |= (1 << PIN_DHT11); // Se prepara la activación del pull-up del pin del DHT11
    PIN_DHT11_DD &= ~(1 << PIN_DHT11);  // PC0 como entrada (definitivamente se activa el pull-up)
}

dht_data_t dht11_read()
{
    uint8_t i;

    // Inicialización en 0 para no tener que hacer nada cuando el bit recibido del DHT11 sea 0
    dht_data_t data = {0, 0, 0, 0, 0};

    // Punteros a los bytes de la estructura para poder modificarlos
    uint8_t *bytes_struct[5] = {
        &data.rh_int,
        &data.rh_dec,
        &data.temp_int,
        &data.temp_dec,
        &data.checksum};

    PIN_DHT11_DD |= (1 << PIN_DHT11);    // Pin del DHT11 como salida
    PIN_DHT11_PORT &= ~(1 << PIN_DHT11); // Pin del DHT11 en BAJO
    DELAY_EN_BAJO

    PIN_DHT11_PORT |= (1 << PIN_DHT11); // Se prepara la activación del pull-up del pin del DHT11
    PIN_DHT11_DD &= ~(1 << PIN_DHT11);  // PC0 como entrada (definitivamente se activa el pull-up)
    DELAY_EN_ALTO

    // Esperar a que el DHT11 baje la línea (inicio de la respuesta)
    while (PIN_DHT11_PIN & (1 << PIN_DHT11))
        ;

    // Esperar a que la línea vuelva a subir (fin de los 80µs en BAJO)
    while (!(PIN_DHT11_PIN & (1 << PIN_DHT11)))
        ;

    // Esperar a que la línea vuelva a bajar (fin de los 80µs en ALTO, comienza el primer bit)
    while (PIN_DHT11_PIN & (1 << PIN_DHT11))
        ;

    for (i = 0; i < BITS_DATA_DHT11; i++)
    {
        // Esperar a que pase el estado BAJO de 50µs y comience el ALTO
        while (!(PIN_DHT11_PIN & (1 << PIN_DHT11)))
            ;

        DELAY_MEDIO

        // Evaluar el estado del pin
        if (PIN_DHT11_PIN & (1 << PIN_DHT11))
        {
            // Sigue en ALTO, es un '1'

            *(bytes_struct[i / 8]) |= (1 << (7 - (i % 8)));

            // Esperamos a que baje para no alterar el siguiente bit
            while (PIN_DHT11_PIN & (1 << PIN_DHT11))
                ;
        }
    }
    return data;
}