#include "reloj.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>

// Variables privadas (Encapsulamiento - Ejer. 7g)
static volatile uint32_t tiempo_restante = 0;
static voidFuncPtr callback_finalizar = NULL;

void IniciarClock(void)
{
    TCNT2 = 0;                                         // Reiniciar contador
    TCCR2A = 0x00;                                     // Modo Normal
    TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20); // Prescaler 1024

    TIMSK2 |= (1 << TOIE2); // Habilitar interrupción local
    sei();
}

void PararClock(void)
{
    TCCR2B = 0;
    TIMSK2 &= ~(1 << TOIE2);
}

void SetearTiempoDeClock(uint32_t nuevo_tiempo)
{
    cli();
    tiempo_restante = nuevo_tiempo;
    sei();
}

uint32_t GetTiempoClock(void)
{
    return tiempo_restante;
}

void SumarTiempo(uint32_t segundos_a_sumar)
{
    cli();

    tiempo_restante += segundos_a_sumar;

    sei();
}

void SetFunc(voidFuncPtr funcion)
{
    callback_finalizar = funcion;
}

ISR(TIMER2_OVF_vect)
{
    if (tiempo_restante > 0)
    {
        tiempo_restante--;

        if (tiempo_restante == 0 && callback_finalizar != NULL)
        {
            callback_finalizar();
            PararClock();
        }
    }
}