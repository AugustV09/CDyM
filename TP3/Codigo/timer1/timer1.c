/**
 * @file timer1.c
 * @brief Implementación del módulo de temporización periódica con Timer 1.
 * @note Basado en el Timer/Counter 1 de 16 bits del ATmega328P a 16MHz.
 * Curso: Circuitos Digitales y Microcontroladores - UNLP.
 */
#include "timer1.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/**
 * Valor de comparación para generar exactamente 1 interrupción por segundo.
 * Cálculo: F_CPU / Prescaler - 1 = 16.000.000 / 1024 - 1 = 15624
 * El -1 es porque el timer cuenta de 0 a OCR1A inclusive (15625 pasos en total).
 */
#define OCR1A_1SEG 15624

/** Bandera que indica que transcurrieron T segundos. Seteada por la ISR, limpiada por el main. */
static volatile uint8_t evento_pendiente = 0;

/** Contador de segundos transcurridos desde el último evento. Incrementado por la ISR cada 1 segundo. */
static volatile uint8_t contador_seg = 0;

/** Intervalo de reporte configurado en segundos. Modificable en tiempo de ejecución con Timer1_SetIntervalo(). */
static volatile uint8_t intervalo_T = 5;

/**
 * @brief Inicializa el Timer 1 en modo CTC con prescaler 1024.
 */
void Timer1_Init(uint8_t intervalo_seg)
{
    intervalo_T = intervalo_seg;
    evento_pendiente = 0;
    contador_seg = 0;

    /* Resetea el contador del timer para evitar arrancar con un valor residual */
    TCNT1 = 0;

    /* Carga el valor de comparación: la ISR se dispara cuando TCNT1 alcanza este valor */
    OCR1A = OCR1A_1SEG;

    /* TCCR1A = 0: no se usa PWM ni salida por pin, solo modo CTC puro */
    TCCR1A = 0;

    /*
     * TCCR1B:
     *   WGM12 = 1 → Modo CTC: TCNT1 se resetea automáticamente al llegar a OCR1A
     *   CS12 | CS10 → Prescaler 1024: reduce el clock de 16MHz a 15.625 KHz
     */
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);

    /* Habilita la interrupción por Compare Match A del Timer 1 (TIMER1_COMPA_vect) */
    TIMSK1 |= (1 << OCIE1A);
}

/**
 * @brief Modifica el intervalo de reporte en tiempo de ejecución.
 */
void Timer1_SetIntervalo(uint8_t seg)
{
    /*
     * Sección crítica: intervalo_T y contador_seg son compartidos con la ISR.
     * Se guarda SREG para restaurar el estado de interrupciones exactamente como estaba,
     * en lugar de hacer un sei() ciego al final.
     */
    uint8_t sreg = SREG;
    cli();

    if (seg >= 2 && seg <= 60)
    {
        intervalo_T = seg;
        contador_seg = 0; /* Reinicia la cuenta para que el nuevo periodo empiece ahora */
    }

    SREG = sreg;
}

/**
 * @brief Consulta si hay un evento de temporización pendiente.
 */
uint8_t Timer1_HayEvento(void)
{
    return evento_pendiente;
}

/**
 * @brief Limpia la bandera de evento pendiente.
 */
void Timer1_LimpiarEvento(void)
{
    evento_pendiente = 0;
}

/**
 * @brief ISR del Timer 1 Canal A. Se ejecuta exactamente 1 vez por segundo.
 * @details Incrementa el contador de segundos y, al alcanzar el intervalo T
 * configurado, resetea el contador y levanta la bandera para el main loop.
 */
ISR(TIMER1_COMPA_vect)
{
    if (++contador_seg >= intervalo_T)
    {
        contador_seg = 0;
        evento_pendiente = 1;
    }
}