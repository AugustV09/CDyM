#include "Reloj.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// 5 ms base con Prescaler 1024 a 16MHz => OCR2A = 77
#define VALOR_COMPARE_MATCH 77
#define INTERRUPCIONES_POR_TICK 5
#define TICKS_POR_SEGUNDO 40

// Variables volátiles para sincronización con la ISR
static volatile uint8_t tick_pendiente = 0;
static volatile uint8_t segundo_pendiente = 0;
static volatile uint8_t contador_interrupciones = 0;
static volatile uint8_t contador_ticks_segundo = 0;

// Bandera de control de estado
static volatile uint8_t contar_segundos = 0;

void Reloj_Init(void)
{
    // Asegurar estado inicial de variables
    tick_pendiente = 0;
    segundo_pendiente = 0;
    contador_interrupciones = 0;
    contador_ticks_segundo = 0;
    contar_segundos = 0;

    TCNT2 = 0;

    // Configura Modo CTC (Clear Timer on Compare Match)
    TCCR2A = (1 << WGM21);

    // Carga el valor de comparación para 5ms
    OCR2A = VALOR_COMPARE_MATCH;

    // Prescaler 1024 -> El clock empieza a correr inmediatamente
    TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);

    // Habilita interrupción por comparación de canal A (OCIE2A)
    TIMSK2 |= (1 << OCIE2A);

    // Habilita interrupciones globales
    sei();
}

void Reloj_StartCountingSeconds(void)
{
    contar_segundos = 1;
}

void Reloj_StopCountingSeconds(void)
{
    contar_segundos = 0;
}

void Reloj_RestartSeconds(void)
{
    // Protegemos la escritura de la variable compartida con la ISR
    uint8_t sreg = SREG;
    cli();
    contador_ticks_segundo = 0;
    segundo_pendiente = 0;
    SREG = sreg;
}

uint8_t Reloj_HayTick(void)
{
    return tick_pendiente;
}

void Reloj_LimpiarTick(void)
{
    tick_pendiente = 0;
}

uint8_t Reloj_HaySegundo(void)
{
    return segundo_pendiente;
}

void Reloj_LimpiarSegundo(void)
{
    segundo_pendiente = 0;
}

// Interrupción por comparación del Timer2 (Cada 5ms fijo)
ISR(TIMER2_COMPA_vect)
{
    // 1. Manejo del Tick del sistema (Cada 5 interrupciones = 25ms)
    if (++contador_interrupciones >= INTERRUPCIONES_POR_TICK)
    {
        contador_interrupciones = 0;
        tick_pendiente = 1; // Despierta a la MEF/Main cada 25ms

        // 2. Manejo de la cuenta de segundos (Solo si está habilitada)
        if (contar_segundos)
        {
            if (++contador_ticks_segundo >= TICKS_POR_SEGUNDO)
            {
                contador_ticks_segundo = 0;
                segundo_pendiente = 1; // Avisa que pasó 1 segundo entero
            }
        }
    }
}