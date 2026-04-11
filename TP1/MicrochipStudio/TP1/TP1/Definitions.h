#ifndef DEFINITIONS_H

#define DEFINITIONS_H

// Hardware disponible

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>

#define TOTAL_LEDS_INDIVIDUALES 8
#define TOTAL_LEDS_NEOPIXEL 8

// Puertos a utilizar por nosotros

#define ConfigNeoPixel DDRB
#define PuertoNeoPixel PORTB
#define PinNeoPixel PORTB0

#define ConfigPulsadores DDRC
#define PuertoPulsadoresEntrada PINC
#define PuertoPulsadoresSalida PORTC
#define PulsadorLeds PORTC0
#define PulsadorNeoPixel PORTC1

#define ConfigPuertoLEDs DDRD
#define PuertoLEDs PORTD

// Tiempos pedidos en ms

#define TiempoLeds 100

#define TiempoNeoPixel 150

#define TiempoChequeos 25

#define CiclosLeds (TiempoLeds / TiempoChequeos)
#define CiclosNeoPixel (TiempoNeoPixel / TiempoChequeos)

#endif