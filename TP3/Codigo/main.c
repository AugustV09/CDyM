#include "timer1/timer1.h"

int main(void)
{
    // UART_Init();
    // I2C_Init();
    // RTC_Init();
    // DHT11_Init();
    // Timer1_Init(5); // T = 5 segundos por defecto

    while (1)
    {
        // FOREGROUND: chequeo de eventos sin bloquear

        /*
                if (Timer1_HayEvento())
                {
                    Timer1_LimpiarEvento();
                    // enviar_telemetria(); // lee RTC + DHT11, formatea y manda por UART
                }

                if (UART_HayComando())
                {
                    // procesar_comando(); // SET_TIME o SET_TM
                }
        */
    }
}