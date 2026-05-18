#include "Reloj.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// 5 ms base con Prescaler 1024 a 16MHz => OCR2A = 77
#define VALOR_COMPARE_MATCH 77
// Para llegar a 25ms (40 ticks por segundo), necesitamos 5 pasos de 5ms
#define INTERRUPCIONES_POR_TICK 5
// Para llegar a un segundo entero, necesitamos 40 ticks
#define TICKS_POR_SEGUNDO 40

static volatile uint8_t tick_pendiente = 0;
static volatile uint8_t contador_interrupciones = 0;
static volatile uint8_t contador_ticks_segundo = 0;

void Reloj_IniciarClock(void)
{
    TCNT2 = 0;

    // Configura Modo CTC (Clear Timer on Compare Match)
    TCCR2A = (1 << WGM21);

    // Carga el valor de comparación para 5ms
    OCR2A = VALOR_COMPARE_MATCH;

    // Prescaler 1024
    TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);

    // Habilita interrupción por comparación de canal A (OCIE2A)
    TIMSK2 |= (1 << OCIE2A);

    sei();
}

void Reloj_PararClock(void)
{
    TCCR2B = 0;
    TIMSK2 &= ~(1 << OCIE2A);
}

uint8_t Reloj_HayTick(void)
{
    return tick_pendiente;
}

void Reloj_LimpiarTick(void)
{
    tick_pendiente = 0;
}

// Interrupción por comparación del Timer2
ISR(TIMER2_COMPA_vect)
{
    // Cada vez que entra acá, pasaron ~5ms. El CPU se despierta del sleep.
    if (++contador_interrupciones >= INTERRUPCIONES_POR_TICK)
    {
        contador_interrupciones = 0;
        tick_pendiente = 1; // Avisa al main que pasaron los 25ms

        // Control opcional del segundo exacto por hardware si el Microondas lo requiere
        if (++contador_ticks_segundo >= TICKS_POR_SEGUNDO)
        {
            contador_ticks_segundo = 0;
            // Acá adentro podés ejecutar un flag de segundo si la MEF lo prefiere
        }
    }
}