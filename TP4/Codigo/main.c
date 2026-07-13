/**
 * @file main.c
 * @brief TP4 – Control de LED RGB con sensor de luz, UART y PWM.
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 *
 * Arquitectura Background / Foreground:
 *   BACKGROUND (ISRs):
 *     TIMER2_COMPA_vect  → levanta tick_sw_pwm_flag (cada ~32 µs)
 *                          + tick_fade_flag (cada ~10 ms / 313 interrupciones)
 *     USART_UDRE_vect    → transmisión UART (buffer circular)
 *     USART_RX_vect      → recepción UART (buffer circular)
 *
 *   FOREGROUND (loop principal – anidamiento de flags):
 *     - Consume tick de SW-PWM → llama a led_rgb_tick_sw_pwm()
 *       └─ Anidado: consume tick de fade → llama a fade_tick()
 *     - Procesa comandos UART → llama a procesar_comando()
 *     - Muestrea el ADC → adc_tick() dispara conversión c/100ms,
 *       adc_hay_muestra()/adc_leer() consumen el resultado  [inciso 4 ✓]
 *
 * Módulos del proyecto:
 *   led_rgb/   → driver PWM (Timer1 modo 1, Phase Correct 8-bit, invertido
 *                + software PWM PB5)                             [inciso 1 ✓]
 *   timer2/    → tick ISR a 31250 Hz con dos flags desacopladas  [inciso 1 ✓]
 *   fade/      → máquina de estados de desvanecimiento           [inciso 3 ✓]
 *   comandos/  → parser UART (SET_COLOR)                         [inciso 2 ✓]
 *   adc/       → muestreo del LDR (ADC3) y mapeo a periodo T     [inciso 4 ✓]
 *   UART/      → driver UART0 con buffers circulares             [reutilizado]
 *   utils/     → formateo sin sprintf                            [reutilizado]
 *
 * Estado de implementación:
 *   Inciso 1 (PWM):        ✓ completo
 *   Inciso 2 (SET_COLOR):  ✓ completo (comandos.c parseado, led_rgb aplica)
 *   Inciso 3 (fade):       ✓ completo (máquina de estados en fade/fade.c)
 *   Inciso 4 (ADC→T):      ✓ completo (adc/adc.c muestrea cada 100 ms y
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
 * Se elegirá dinámicamente por el ADC en el inciso 4;
 * por ahora se usa el punto medio del rango (4500 ms).
 */
#define FADE_PERIODO_INICIAL_MS 4500u

int main(void)
{
  /* --- Inicialización de periféricos --- */
  uart_init();

  /*
   * INCISO 1: Inicializar el driver PWM del LED RGB.
   * led_rgb_init() configura el Timer1 en modo 1 (Phase Correct PWM 8-bit,
   * invertido) y el pin PB5 como salida para el software PWM.
   * Timer2_Init() arranca el tick a 31250 Hz que alimenta las flags de
   * SW-PWM y de fade.
   */
  led_rgb_init();
  Timer2_Init();

  /*
   * INCISO 3: Inicializar el fade con el periodo por defecto.
   * fade_init() arranca la máquina de estados en FADE_SUBIENDO
   * con intensidad 0 y el periodo indicado.
   */
  fade_init(FADE_PERIODO_INICIAL_MS);

  /*
   * INCISO 4: Inicializar el ADC.
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
  while (1)
  {
    /*
     * ANIDAMIENTO DE FLAGS (Timer2):
     *
     * Flag externa (TickPWM): se levanta en cada ISR del Timer2
     * (~31250 Hz). Al consumirla, se ejecuta led_rgb_tick_sw_pwm()
     * para avanzar el PWM por software del canal rojo (PB5).
     *
     * Flag interna (Tick): se levanta cada ~313 ISRs (~10 ms).
     * Al consumirla, se ejecuta fade_tick() para avanzar la
     * máquina de estados del fade.
     *
     * Este diseño desacopla el módulo Timer2 de led_rgb y fade:
     * la ISR solo levanta flags, y el foreground decide a qué
     * módulo llamar.
     */
    if (Timer2_HayTickPWM())
    {
      Timer2_LimpiarTickPWM();
      led_rgb_tick_sw_pwm(); /* siempre al entrar a la primera flag */

      /*
       * INCISO 4: cuenta ticks de SW-PWM y dispara una conversión ADC
       * cada ~100 ms (ver adc/adc.c). Se anida en el mismo nivel que
       * fade_tick(), ya que ambos dependen del tick de SW-PWM del Timer2.
       */
      adc_tick();

      if (Timer2_HayTick()) /* cada ~10 ms */
      {
        Timer2_LimpiarTick();
        fade_tick(); /* avanza la máquina de estados */
      }
    }

    /*
     * INCISO 2: Procesar comando UART.
     * Si el usuario envía "SET_COLOR=R,G,B\r\n", procesar_comando() lo parsea
     * y llama a led_rgb_set_color(r, g, b). La intensidad actual (del fade)
     * se aplica automáticamente gracias a la capa color × intensidad del
     * módulo.
     */
    if (uart_has_message())
    {
      uart_clear_message();
      procesar_comando();
    }

    /*
     * INCISO 4: Muestrear el ADC y actualizar el periodo del fade.
     * adc_tick() (llamado más arriba) dispara una conversión cada ~100 ms;
     * acá se consume el resultado apenas la ISR ADC_vect lo deja listo.
     */
    if (adc_hay_muestra())
    {
      adc_limpiar_muestra();
      uint16_t ldr = adc_leer();

      /* Mapea la lectura (0-1023) al rango de periodo pedido (3000-6000 ms):
       *   más luz (ldr alto)   → periodo corto (3000 ms)
       *   menos luz (ldr bajo) → periodo largo (6000 ms) */
      uint16_t periodo_ms = adc_mapear_periodo(ldr);
      fade_set_periodo_ms(periodo_ms);
    }
  }

  return 0;
}