#include "Reloj.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>

// Con F_CPU=16MHz y prescaler 1024, Timer2 (8 bits) desborda cada:
// 256 * 1024 / 16.000.000 ≈ 16.384 ms → ~61 overflows por segundo
#define OVERFLOWS_POR_SEGUNDO 61

static volatile uint8_t contador_overflows = 0;
static clockCallback callback = NULL;
static void *contexto_guardado = NULL;

void Reloj_IniciarClock(void)
{
    TCNT2 = 0;
    TCCR2A = 0x00;
    TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20); // Prescaler 1024
    TIMSK2 |= (1 << TOIE2);
    sei();
}

void Reloj_PararClock(void)
{
    TCCR2B = 0;
    TIMSK2 &= ~(1 << TOIE2);
}

void Reloj_SetFunc(clockCallback funcion, void *contexto)
{
    callback = funcion;
    contexto_guardado = contexto;
}

ISR(TIMER2_OVF_vect)
{
    if (++contador_overflows >= OVERFLOWS_POR_SEGUNDO)
    {
        contador_overflows = 0;
        if (callback != NULL)
            callback(contexto_guardado);
    }
}