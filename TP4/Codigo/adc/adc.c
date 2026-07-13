/**
 * @file adc.c
 * @brief Implementación del driver ADC para el sensor de luz (LDR, ADC3).
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 */

#include "adc.h"
#include <avr/interrupt.h>
#include <avr/io.h>

/* =========================================================================
 * Constantes
 * ====================================================================== */

/*
 * Prescaler del ADC = 128 → f_ADC = F_CPU / 128 = 16.000.000 / 128 = 125 kHz
 * Dentro del rango recomendado (50–200 kHz) para conversiones de 10 bits
 * de precisión completa.
 */
#define ADC_PRESCALER_VAL ((1u << ADPS2) | (1u << ADPS1) | (1u << ADPS0))

/* Canal ADC3 (PC3): MUX[3:0] = 0011 */
#define ADC_CANAL_LDR 0x03u

/*
 * Periodo de muestreo pedido por el enunciado: 100 ms.
 * adc_tick() se llama desde el foreground una vez por cada tick de SW-PWM
 * del Timer2 (~31250 Hz, ver timer2.c), igual que fade_tick() se llama una
 * vez cada 313 de esos ticks (~10 ms). Para 100 ms se necesitan 10 veces
 * más ticks que para el tick de fade:
 *   N = 31250 × 0.100 = 3125
 */
#define TICKS_POR_MUESTREO 3125u

/*
 * ⚠ VERIFICAR CONTRA EL ESQUEMÁTICO REAL ANTES DE LA DEMO (ver adc.h) ⚠
 *
 * Sentido asumido por defecto (ADC_LDR_INVERTIDO = 0):
 *   LDR conectado entre AVCC y el nodo de PC3, con la resistencia de 100k
 *   entre ese nodo y GND. Más luz → menos R_LDR → nodo hacia PC3 sube de
 *   tensión → lectura ADC más alta → periodo de fade más corto.
 *
 * Si el divisor está armado al revés en el esquemático (100k entre AVCC
 * y PC3, LDR entre PC3 y GND), cambiar esta macro a 1: más luz seguiría
 * dando menos R_LDR, pero ahora el nodo hacia PC3 BAJA de tensión, así
 * que hay que invertir el sentido de adc_mapear_periodo() sin tocar el
 * resto del módulo.
 */
#define ADC_LDR_INVERTIDO 0

/* =========================================================================
 * Estado interno
 * ====================================================================== */

/** Contador descendente hasta la próxima conversión (análogo a contador_fade
 *  en timer2.c). */
static uint16_t contador_muestreo = TICKS_POR_MUESTREO;

/**
 * Bandera de muestra lista.
 * Se levanta en la ISR ADC_vect al finalizar la conversión disparada por
 * adc_tick(). La limpia el foreground con adc_limpiar_muestra(), siguiendo
 * el mismo patrón de flags binarias que timer2.c.
 */
static volatile uint8_t adc_muestra_lista = 0u;

/** Último resultado de conversión (0–1023), actualizado en la ISR. */
static volatile uint16_t adc_valor = 0u;

/* =========================================================================
 * API pública
 * ====================================================================== */

void adc_init(void)
{
    /*
     * ADMUX:
     *   REFS0=1, REFS1=0 → referencia = AVCC con capacitor externo en AREF
     *   ADLAR=0           → resultado justificado a la derecha (10 bits)
     *   MUX[3:0]=0011     → canal ADC3 (PC3, sensor LDR)
     */
    ADMUX = (1u << REFS0) | ADC_CANAL_LDR;

    /*
     * ADCSRA:
     *   ADEN=1  → habilita el ADC
     *   ADIE=1  → habilita la interrupción por fin de conversión
     *   ADPS[2:0] → prescaler 128 (f_ADC = 125 kHz)
     *   ADSC no se setea aquí: la primera conversión la dispara adc_tick()
     *   al vencer el primer periodo de muestreo.
     */
    ADCSRA = (1u << ADEN) | (1u << ADIE) | ADC_PRESCALER_VAL;

    contador_muestreo = TICKS_POR_MUESTREO;
    adc_muestra_lista = 0u;
    adc_valor = 0u;
}

void adc_tick(void)
{
    /*
     * Contador de 16 bits porque TICKS_POR_MUESTREO (3125) > 255.
     * Misma lógica que contador_fade en timer2.c, pero corrida en el
     * foreground en lugar de en una ISR.
     */
    if (--contador_muestreo == 0u)
    {
        contador_muestreo = TICKS_POR_MUESTREO;

        /* Dispara una nueva conversión; el resultado llega por ADC_vect. */
        ADCSRA |= (1u << ADSC);
    }
}

uint8_t adc_hay_muestra(void) { return adc_muestra_lista; }

void adc_limpiar_muestra(void) { adc_muestra_lista = 0u; }

uint16_t adc_leer(void) { return adc_valor; }

uint16_t adc_mapear_periodo(uint16_t muestra_adc)
{
    /*
     * Relación lineal entre luz y periodo, con el sentido controlado por
     * ADC_LDR_INVERTIDO (ver comentario junto a esa macro y en adc.h):
     *
     *   ADC_LDR_INVERTIDO = 0 (default):
     *     muestra_adc = 0    (mínima luz) → periodo = 6000 ms
     *     muestra_adc = 1023 (máxima luz) → periodo = 3000 ms
     *     periodo = 6000 - (muestra_adc * 3000) / 1023
     *
     *   ADC_LDR_INVERTIDO = 1 (divisor armado al revés):
     *     muestra_adc = 0    (máxima luz) → periodo = 3000 ms
     *     muestra_adc = 1023 (mínima luz) → periodo = 6000 ms
     *     periodo = 3000 + (muestra_adc * 3000) / 1023
     *
     * Se usa aritmética de 32 bits en el producto intermedio para evitar
     * desborde: 1023 × 3000 = 3.069.000, que no entra en uint16_t.
     */
    uint16_t delta_ms = (uint16_t)(((uint32_t)muestra_adc * 3000u) / 1023u);

#if ADC_LDR_INVERTIDO
    uint16_t periodo_ms = 3000u + delta_ms;
#else
    uint16_t periodo_ms = 6000u - delta_ms;
#endif

    return periodo_ms;
}

/* =========================================================================
 * ISR de fin de conversión del ADC
 * ====================================================================== */

ISR(ADC_vect)
{
    /*
     * Igual que en timer2.c: la ISR solo guarda el dato y levanta una flag.
     * El foreground decide qué hacer con la muestra (mapearla y pasársela
     * a fade_set_periodo_ms()).
     *
     * ADCL debe leerse antes que ADCH para que el hardware libere
     * correctamente el registro de resultado de 16 bits.
     */
    uint8_t lo = ADCL;
    uint8_t hi = ADCH;

    adc_valor = ((uint16_t)hi << 8u) | (uint16_t)lo;
    adc_muestra_lista = 1u;
}