/**
 * @file main.c
 * @brief TP4 - Control de LED RGB con sensor de luz, UART y PWM.
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 *
 * Arquitectura Background / Foreground:
 *   BACKGROUND (ISRs):
 *     TIMER2_COMPA_vect  -> acumula tick_sw_pwm_count (cada ~32 us, sin
 *                          perder ticks si el foreground se demora)
 *                          + tick_fade_flag (cada ~10 ms / 313 interrupciones)
 *     USART_UDRE_vect    -> transmisión UART (buffer circular)
 *     USART_RX_vect      -> recepción UART (buffer circular)
 *
 *   FOREGROUND (loop principal):
 *     - Drena TODOS los ticks de SW-PWM pendientes (while) ->
 *       led_rgb_tick_sw_pwm() + adc_tick() por cada uno
 *     - Consume tick de fade (~10 ms) -> llama a fade_tick()
 *     - Procesa comandos UART -> llama a procesar_comando()
 *     - Muestrea el ADC -> adc_tick() dispara conversión c/100ms,
 *       adc_hay_muestra()/adc_leer() consumen el resultado
 *
 * Módulos del proyecto:
 *   led_rgb/   -> driver PWM (Timer1 modo 1, Phase Correct 8-bit, invertido
 *                + software PWM PB5)
 *   timer2/    -> tick ISR a 31250 Hz: contador de SW-PWM (sin pérdida)
 *                + flag de fade (~10 ms)
 *   fade/      -> máquina de estados de desvanecimiento
 *   comandos/  -> parser UART (SET_COLOR)
 *   adc/       -> muestreo del LDR (ADC3) y mapeo a periodo T
 *   UART/      -> driver UART0 con buffers circulares
 *   utils/     -> formateo sin sprintf
 *
 * Estado de implementación:
 *   Inciso 1 (PWM):        OK completo
 *   Inciso 2 (SET_COLOR):  OK completo (comandos.c parseado, led_rgb aplica)
 *   Inciso 3 (fade):       OK completo (máquina de estados en fade/fade.c)
 *   Inciso 4 (ADC->T):      OK completo (adc/adc.c muestrea cada 100 ms y
 *                                  llama a fade_set_periodo_ms())
 */

#include "main.h"
#include "UART/uart.h"
#include "adc/adc.h"
#include "comandos/comandos.h"
#include "fade/fade.h"
#include "led_rgb/led_rgb.h"
#include "timer2/timer2.h"

/*
 * Periodo inicial del fade en ms.
 * Se elegirá dinámicamente por el ADC;
 * por ahora se usa el punto medio del rango (4500 ms).
 */
#define FADE_PERIODO_INICIAL_MS 4500u

int main(void) {
  /* --- Inicialización de periféricos --- */
  uart_init();

  /*
   * Inicializar el driver PWM del LED RGB.
   * led_rgb_init() configura el Timer1 en modo 1 (Phase Correct PWM 8-bit,
   * invertido) y el pin PB5 como salida para el software PWM.
   * Timer2_Init() arranca el tick a 31250 Hz que alimenta las flags de
   * SW-PWM y de fade.
   */
  led_rgb_init();
  Timer2_Init();

  /*
   * Inicializar el fade con el periodo por defecto.
   * fade_init() arranca la máquina de estados en FADE_SUBIENDO
   * con intensidad 0 y el periodo indicado.
   */
  fade_init(FADE_PERIODO_INICIAL_MS);

  /*
   * Inicializar el ADC.
   * adc_init() configura ADC3 (PC3, sensor LDR) con referencia AVCC,
   * prescaler 128 (10 bits) y habilita la interrupción de fin de
   * conversión. El muestreo periódico (~100 ms) lo controla adc_tick(),
   * llamado desde el loop principal en cada tick de SW-PWM del Timer2
   * (ver adc/adc.h para el detalle del mecanismo).
   */
  adc_init();

  /* Habilitar interrupciones globales */
  sei();

  uart_transfer("TP4 - LED RGB iniciado.\r\n");
  uart_transfer("Comando: SET_COLOR=R,G,B (ej: SET_COLOR=0,255,128)\r\n");

  /* --- Loop principal (foreground) --- */
  while (1) {
    /*
     * DRENADO DE TICKS DE SW-PWM (Timer2):
     *
     * Timer2_ConsumirTickPWM() devuelve 1 y descuenta uno del contador
     * acumulador por cada tick de SW-PWM pendiente (ver timer2.h). Se usa
     * en un while() en lugar de un if() para no perder ticks: si el
     * foreground tardó en volver a esta parte del loop (por ejemplo
     * mientras procesaba un comando UART), pueden haberse acumulado
     * varios ticks, y hay que recuperarlos todos antes de seguir, o el
     * PWM por software y el muestreo del ADC se atrasarían respecto del
     * tiempo real.
     *
     * Dentro del while:
     *   - led_rgb_tick_sw_pwm() avanza el PWM por software del canal
     *     rojo (PB5), una vez por cada tick de Timer2 (~31250 Hz).
     *   - adc_tick() cuenta ticks para disparar una conversión ADC cada
     *     ~100 ms (ver adc/adc.c).
     *
     * La flag de fade (Timer2_HayTick()) se sigue consultando por
     * separado, ya que corre a menor frecuencia (~10 ms) y no necesita
     * el mismo mecanismo de recuperación de ticks perdidos.
     */
    while (Timer2_ConsumirTickPWM()) {
      led_rgb_tick_sw_pwm();
      adc_tick();
    }

    if (Timer2_HayTick()) /* cada ~10 ms */
    {
      Timer2_LimpiarTick();
      fade_tick(); /* avanza la máquina de estados */
    }

    /*
     * Procesar comando UART.
     * Si el usuario envía "SET_COLOR=R,G,B\r\n", procesar_comando() lo parsea
     * y llama a led_rgb_set_color(r, g, b). La intensidad actual (del fade)
     * se aplica automáticamente gracias a la capa color x intensidad del
     * módulo.
     */
    if (uart_has_message()) {
      uart_clear_message();
      procesar_comando();
    }

    /*
     * Muestrear el ADC y actualizar el periodo del fade.
     * adc_tick() (llamado más arriba) dispara una conversión cada ~100 ms;
     * acá se consume el resultado apenas la ISR ADC_vect lo deja listo.
     */
    if (adc_hay_muestra()) {
      adc_limpiar_muestra();
      uint16_t ldr = adc_leer();

      /* Mapea la lectura (0-1023) al rango de periodo pedido (3000-6000 ms):
       *   más luz (ldr alto)   -> periodo corto (3000 ms)
       *   menos luz (ldr bajo) -> periodo largo (6000 ms) */
      uint16_t periodo_ms = adc_mapear_periodo(ldr);
      fade_set_periodo_ms(periodo_ms);
    }
  }

  return 0;
}