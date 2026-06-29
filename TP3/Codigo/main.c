/**
 * @file main.c
 * @brief Monitor de Invernadero - TP3 Circuitos Digitales y Microcontroladores
 *
 * Arquitectura Background / Foreground:
 *   BACKGROUND (ISRs): TIMER1_COMPA_vect, USART_UDRE_vect, USART_RX_vect
 *   FOREGROUND (loop): chequeo de banderas, sin esperas activas
 *
 * Módulos de aplicación:
 *   monitor/   -> lógica de rangos, ventana horaria y telemetría
 *   comandos/  -> parser UART (SET_TIME, SET_TM)
 *   utils/     -> formateo de enteros y tiempo sin sprintf
 */

#include "main.h"
#include "UART/uart.h"
#include "i2c/i2c.h"
#include "rtc_DS3231/rtc_DS3231.h"
#include "DHT11/dht11.h"
#include "timer1/timer1.h"
#include "monitor/monitor.h"
#include "comandos/comandos.h"

#define T_DEFAULT_SEG 5u

int main(void)
{
    uart_init();
    i2c_init();
    rtc_init();
    dht11_init();
    Timer1_Init(T_DEFAULT_SEG);

    /* Habilita las interrupciones globales */
    sei();

    uart_transfer("Monitor de Invernadero iniciado.\r\n");

    while (1)
    {
        if (Timer1_HayEvento())
        {
            Timer1_LimpiarEvento();
            enviar_telemetria();
        }

        if (uart_has_message())
        {
            uart_clear_message();
            procesar_comando();
        }
    }

    return 0;
}