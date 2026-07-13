/**
 * @file adc.h
 * @brief Driver del conversor Analógico-Digital para el sensor de luz (LDR).
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 *
 * Conexionado (según enunciado TP4):
 *   PC3 = ADC3 = entrada del sensor LDR, formando un divisor de tensión
 *   con una resistencia de 100k a GND.
 *
 * ============================================================================
 * FUNCIONAMIENTO
 * ============================================================================
 *
 * El módulo configura el ADC en modo de disparo único por software
 * (Free Running deshabilitado), canal ADC3, referencia AVCC (5V) y
 * justificación a la derecha (10 bits en ADCL:ADCH).
 *
 * El muestreo periódico (~100 ms, igual que el ejercicio 2a de la práctica)
 * se obtiene reutilizando el mismo patrón de "tick por flag" que timer2/:
 * un contador de software, incrementado cada vez que el foreground consume
 * el tick de SW-PWM del Timer2 (~31250 Hz, ver timer2.h), dispara una nueva
 * conversión cada N ticks. La finalización de la conversión se señaliza con
 * la interrupción ADC_vect, que solo levanta una flag (adc_muestra_lista),
 * igual que hace timer2.c con sus propias flags.
 *
 * Jerarquía de llamadas en el foreground:
 *
 *   if (Timer2_HayTickPWM())
 *   {
 *       Timer2_LimpiarTickPWM();
 *       led_rgb_tick_sw_pwm();
 *       adc_tick();              <- cuenta ticks y dispara conversión cada 100ms
 *
 *       if (Timer2_HayTick())
 *       {
 *           Timer2_LimpiarTick();
 *           fade_tick();
 *       }
 *   }
 *
 *   if (adc_hay_muestra())
 *   {
 *       adc_limpiar_muestra();
 *       uint16_t ldr = adc_leer();
 *       uint16_t periodo_ms = adc_mapear_periodo(ldr);
 *       fade_set_periodo_ms(periodo_ms);
 *   }
 *
 * ============================================================================
 * MAPEO LDR → PERIODO DE FADE (inciso 4 del entregable)
 * ============================================================================
 *
 *   El enunciado pide que el periodo T del parpadeo varíe entre:
 *     6 s (mínima luz)   y   3 s (máxima iluminación)
 *
 *   Suponiendo que el divisor de tensión LDR–100k entrega mayor tensión
 *   (mayor lectura ADC) cuanto mayor es la luz ambiente (LDR baja su
 *   resistencia con más luz, y está conectado a AVCC con la 100k a GND):
 *     ldr alto  (mucha luz) → periodo corto (3000 ms)
 *     ldr bajo  (poca luz)  → periodo largo (6000 ms)
 *
 *   adc_mapear_periodo() implementa esta relación inversa y lineal.
 *
 *   ⚠ VERIFICAR CONTRA EL ESQUEMÁTICO REAL ANTES DE LA DEMO ⚠
 *   Este sentido depende de CÓMO ESTÉ ARMADO el divisor en Proteus / en
 *   el kit físico, no es algo que el software pueda saber por sí solo:
 *     - Si el LDR va del lado de AVCC y la resistencia de 100k del lado
 *       de GND (LDR arriba): más luz → menos R_LDR → el nodo intermedio
 *       (hacia PC3) sube de tensión → lectura ADC más alta.
 *       Este es el caso asumido por adc_mapear_periodo() tal como está.
 *     - Si está armado al revés (100k arriba, LDR abajo, hacia GND): más
 *       luz → menos R_LDR → el nodo intermedio BAJA de tensión → lectura
 *       ADC más baja. En ese caso el mapeo queda invertido y hay que
 *       invertir el sentido de la resta en adc_mapear_periodo().
 *   Para verificarlo en la práctica: iluminar el LDR con una linterna y
 *   observar con el debugger de Proteus (o imprimiendo adc_leer() por
 *   UART) si la lectura sube o baja. Si el fade se acelera con MENOS
 *   luz en vez de con más, el cableado está invertido respecto de lo
 *   asumido acá.
 */

#ifndef ADC_H
#define ADC_H

#include <stdint.h>

/* -------------------------------------------------------------------------
 * Inicialización
 * ---------------------------------------------------------------------- */

/**
 * @brief Inicializa el ADC para leer el canal ADC3 (PC3, sensor LDR).
 * @details Configura:
 *   - ADMUX:  REFS0=1 (referencia AVCC con capacitor en AREF), MUX[3:0]=0011
 *             (canal ADC3), ADLAR=0 (justificado a la derecha, 10 bits).
 *   - ADCSRA: prescaler = 128 → f_ADC = F_CPU / 128 = 125 kHz
 *             (dentro del rango recomendado de 50–200 kHz para 10 bits),
 *             ADEN=1 (habilita el ADC), ADIE=1 (habilita interrupción por
 *             fin de conversión).
 *   No dispara ninguna conversión: la primera se inicia en el primer
 *   vencimiento del contador de adc_tick().
 */
void adc_init(void);

/* -------------------------------------------------------------------------
 * Muestreo periódico (llamar desde el foreground, anidado al tick de
 * SW-PWM del Timer2, igual que fade_tick())
 * ---------------------------------------------------------------------- */

/**
 * @brief Cuenta ticks de SW-PWM del Timer2 y dispara una conversión ADC
 *   cada ~100 ms.
 * @details Debe llamarse una vez por cada tick de SW-PWM consumido en el
 *   foreground (~31250 Hz), de forma análoga a como main.c anida la llamada
 *   a fade_tick() dentro de Timer2_HayTick(). Internamente cuenta cuántos
 *   ticks pasaron y, al llegar a 100 ms, dispara una nueva conversión con
 *   ADCSRA |= (1 << ADSC). El resultado se recoge más tarde vía
 *   adc_hay_muestra()/adc_leer(), cuando la ISR ADC_vect haya terminado.
 */
void adc_tick(void);

/* -------------------------------------------------------------------------
 * Resultado de la conversión
 * ---------------------------------------------------------------------- */

/**
 * @brief Consulta si hay una muestra de ADC lista para leer.
 * @return 1 si la conversión disparada por adc_tick() ya finalizó, 0 si no.
 * @note La flag se levanta en la ISR ADC_vect al completarse la conversión.
 */
uint8_t adc_hay_muestra(void);

/**
 * @brief Limpia la flag de muestra lista.
 * @note Llamar desde el foreground antes de leer el valor con adc_leer().
 */
void adc_limpiar_muestra(void);

/**
 * @brief Devuelve el último valor de 10 bits leído del ADC3 (0–1023).
 * @return Valor de la conversión, ya combinado desde ADCL/ADCH.
 */
uint16_t adc_leer(void);

/* -------------------------------------------------------------------------
 * Mapeo LDR → periodo de fade
 * ---------------------------------------------------------------------- */

/**
 * @brief Mapea una lectura de ADC (0–1023) al rango de periodo de fade
 *   pedido por el enunciado (3000 ms a 6000 ms).
 * @param muestra_adc  Valor crudo del ADC3 (0–1023).
 * @return Periodo en ms, lineal entre 3000 y 6000 ms según el sentido
 *   configurado por la macro ADC_LDR_INVERTIDO en adc.c (ver comentario
 *   junto a esa macro, y la nota de verificación de cableado más arriba
 *   en este archivo).
 * @note El valor devuelto puede pasarse directamente a fade_set_periodo_ms(),
 *   que además satura el rango por las dudas.
 */
uint16_t adc_mapear_periodo(uint16_t muestra_adc);

#endif /* ADC_H */