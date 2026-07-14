/**
 * @file main.h
 * @brief Definiciones globales del TP4 - Control de LED RGB con ADC y UART.
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 */

#ifndef MAIN_H
#define MAIN_H

#include <avr/io.h>
#include <avr/interrupt.h>

/* Frecuencia del reloj del MCU */
#define F_CPU 16000000UL
#include <util/delay.h>

/*
 * Mapa de pines del ATmega328P para el TP4
 * =========================================
 * PB1 = OC1A = canal AZUL  del LED RGB (Timer1, PWM hardware)
 * PB2 = OC1B = canal VERDE del LED RGB (Timer1, PWM hardware)
 * PB5 = canal ROJO del LED RGB (PWM por software, Timer2)
 *
 * PC3 = ADC3 = entrada del sensor LDR (divisor de tensión con 100k a GND)
 *
 * PD0 = RXD0 (UART)
 * PD1 = TXD0 (UART)
 */
#define PIN_LED_B PB1 /* OC1A - azul,  Timer1 */
#define PIN_LED_G PB2 /* OC1B - verde, Timer1 */
#define PIN_LED_R PB5 /* software PWM - rojo */
#define PIN_LDR PC3   /* ADC3 - sensor de luz */
#define PIN_RX_UART PD0
#define PIN_TX_UART PD1

#endif /* MAIN_H */
