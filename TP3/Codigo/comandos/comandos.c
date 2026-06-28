/**
 * @file comandos.c
 * @brief Implementación del parser de comandos UART.
 */

#include "comandos.h"
#include "../UART/uart.h"
#include "../rtc_DS3231/rtc_DS3231.h"
#include "../timer1/timer1.h"
#include <string.h>
#include <stdint.h>

#define CMD_BUF_SIZE 32u

/* -------------------------------------------------------------------------
 * Funciones privadas
 * ---------------------------------------------------------------------- */

/** Parsea y ejecuta SET_TIME=HH:MM:SS. */
static void cmd_set_time(const char *arg)
{
    if (strlen(arg) != 8u || arg[2] != ':' || arg[5] != ':')
    {
        uart_transfer("ERROR: Uso: SET_TIME=HH:MM:SS\r\n");
        return;
    }

    rtc_time_t nuevo;
    nuevo.horas    = (uint8_t)((arg[0] - '0') * 10u + (arg[1] - '0'));
    nuevo.minutos  = (uint8_t)((arg[3] - '0') * 10u + (arg[4] - '0'));
    nuevo.segundos = (uint8_t)((arg[6] - '0') * 10u + (arg[7] - '0'));

    if (nuevo.horas > 23u || nuevo.minutos > 59u || nuevo.segundos > 59u)
    {
        uart_transfer("ERROR: Hora fuera de rango.\r\n");
        return;
    }

    rtc_set_time(&nuevo);
    uart_transfer("OK: Hora actualizada.\r\n");
}

/** Parsea y ejecuta SET_TM=SS. */
static void cmd_set_tm(const char *arg)
{
    uint8_t val   = 0u;
    uint8_t valido = 1u;

    for (uint8_t i = 0u; arg[i] != '\0'; i++)
    {
        if (arg[i] < '0' || arg[i] > '9') { valido = 0u; break; }
        val = (uint8_t)(val * 10u + (arg[i] - '0'));
    }

    if (!valido || val < 2u || val > 60u)
    {
        uart_transfer("ERROR: Uso: SET_TM=SS (2-60)\r\n");
        return;
    }

    Timer1_SetIntervalo(val);
    uart_transfer("OK: Intervalo actualizado.\r\n");
}

/** Despacha el string de comando completo al handler correspondiente. */
static void despachar(const char *cmd)
{
    if (strncmp(cmd, "SET_TIME=", 9u) == 0)
        cmd_set_time(cmd + 9u);
    else if (strncmp(cmd, "SET_TM=", 7u) == 0)
        cmd_set_tm(cmd + 7u);
    else
        uart_transfer("ERROR: Comando desconocido.\r\n");
}

/* -------------------------------------------------------------------------
 * API pública
 * ---------------------------------------------------------------------- */

void procesar_comando(void)
{
    static char    buf[CMD_BUF_SIZE];
    static uint8_t idx = 0u;

    uint8_t byte;

    while (uart_read(&byte))
    {
        if (byte == '\r') continue;

        if (byte == '\n')
        {
            buf[idx] = '\0';
            idx = 0u;
            despachar(buf);
            return;
        }

        if (idx < (CMD_BUF_SIZE - 1u)) buf[idx++] = (char)byte;
        else                            idx = 0u;   /* overflow: descartar */
    }
}
