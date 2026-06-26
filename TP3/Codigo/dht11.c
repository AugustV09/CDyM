#include "dht11.h"

// Espera 20ms (asegura los >18ms requeridos al inicio de la comunicacion por parte del MCU)
#define DELAY_EN_BAJO _delay_ms(20);
// Espera 30µs (punto medio entre los 20µs-40µs requeridos durante el primer PULL-UP)
#define DELAY_EN_ALTO _delay_us(30);
// Espera 40µs (punto medio de los dos tiempos posibles (28µs < 40µs < 70µs) para el tiempo en alto entre 0 y 1)
#define DELAY_BAJO _delay_us(40);

void dht11_read(dht_data_t *data)
{
    DDRC |= (1 << DDC0);    // PC0 como salida
    PORTC &= ~(1 << PORTC0); // PC0 en BAJO
    DELAY_EN_BAJO

    PORTC |= (1 << PORTC0); // Opcional: activa pull-up interno transitorio
    DDRC &= ~(1 << DDC0);   // PC0 como entrada
    DELAY_EN_ALTO

    // 1. Esperar a que el DHT11 baje la línea (Inicio de respuesta)
    while(PINC & (1 << PINC0)); 

    // 2. Esperar a que la línea vuelva a subir (Fin de los 80µs en BAJO)
    while(!(PINC & (1 << PINC0))); 

    // 3. Esperar a que la línea vuelva a bajar (Fin de los 80µs en ALTO, comienza el primer bit)
    while(PINC & (1 << PINC0));

    // 1. Esperar a que pase el estado BAJO de 50µs y comience el ALTO
    while(!(PINC & (1 << PINC0)));
    
    // 2. Retraso estratégico en medio de los dos tiempos posibles (28µs < 40µs < 70µs)
    DELAY_BAJO
    
    // 3. Evaluar el estado del pin
    if(PINC & (1 << PINC0)) {
        // Sigue en ALTO, es un '1'. Esperamos a que baje para no alterar el siguiente bit.
        while(PINC & (1 << PINC0)); 
        return 1;///////////////////////////////////////////////////////////////////
    } else {
        // Ya bajó, es un '0'.
        return 0;///////////////////////////////////////////////////////////////////
    }
}