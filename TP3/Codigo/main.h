#ifndef MAIN_H
#define MAIN_H

#include <avr/io.h>
#include <avr/interrupt.h>

// Frecuencia del reloj para util/delay (y para calcular los baudios de la UART)
#define F_CPU 16000000UL
#include <util/delay.h>

// Conexiones UART (Mapeo por hardware del ATmega328P)
#define PIN_RX_UART PD0
#define PIN_TX_UART PD1

// Conexión del sensor DHT11 (Bus de datos único)
#define PIN_DHT11 PC0

// Conexiones I2C para el RTC DS3231 (Mapeo por hardware TWI)
#define PIN_SDA_I2C PC4
#define PIN_SCL_I2C PC5

#endif /* MAIN_H */