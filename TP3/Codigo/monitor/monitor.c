/**
 * @file monitor.c
 * @brief Implementación de la lógica de monitoreo del invernadero.
 */

#include "monitor.h"
#include "../utils/utils.h"
#include "../rtc_DS3231/rtc_DS3231.h"
#include "../DHT11/dht11.h"
#include "../UART/uart.h"
#include <string.h>

/* -------------------------------------------------------------------------
 * Estado interno
 * ---------------------------------------------------------------------- */
static uint8_t tramas_desde_alerta = 0u;

/* -------------------------------------------------------------------------
 * Funciones privadas
 * ---------------------------------------------------------------------- */

/** Devuelve 1 si la hora cae en horario diurno (07:00-18:59). */
static uint8_t es_dia(uint8_t horas)
{
    return (horas >= 7u && horas <= 18u) ? 1u : 0u;
}

/** Devuelve 1 si el checksum del DHT11 es correcto. */
static uint8_t dht11_checksum_ok(const dht_data_t *d)
{
    uint8_t suma = d->rh_int + d->rh_dec + d->temp_int + d->temp_dec;
    return (suma == d->checksum) ? 1u : 0u;
}

/** Envía un mensaje de alerta por UART para temperatura o humedad. */
static void enviar_alerta(const char *hms, uint8_t diurno,
                          const char *variable, uint8_t valor, uint8_t es_temp)
{
    char buf[90];
    char num[4];

    buf[0] = '\0';
    strcat(buf, "[ALERTA] ["); strcat(buf, hms); strcat(buf, "] ");
    strcat(buf, variable);
    strcat(buf, diurno ? "diurno! Valor: " : "nocturno! Valor: ");
    u8_to_str(valor, num); strcat(buf, num);
    strcat(buf, es_temp ? "\xB0\x43\r\n" : "%\r\n");

    uart_transfer(buf);
}

/* -------------------------------------------------------------------------
 * API pública
 * ---------------------------------------------------------------------- */

void enviar_telemetria(void)
{
    rtc_time_t t;
    dht_data_t s;
    char buf[90];
    char hms[9];
    char num[4];

    rtc_get_time(&t);
    s = dht11_read();
    fmt_time(t.horas, t.minutos, t.segundos, hms);

    /* Sensor ausente o lectura inválida */
    if (!dht11_checksum_ok(&s))
    {
        buf[0] = '\0';
        strcat(buf, "["); strcat(buf, hms); strcat(buf, "] ");
        strcat(buf, "T: --\xB0\x43 | H: --% | Estado: SIN_SENSOR\r\n");
        uart_transfer(buf);
        return;
    }

    uint8_t temp   = s.temp_int;
    uint8_t hum    = s.rh_int;
    uint8_t diurno = es_dia(t.horas);

    uint8_t temp_ok = diurno ? (temp >= DIA_TEMP_MIN   && temp <= DIA_TEMP_MAX)
                              : (temp >= NOCHE_TEMP_MIN && temp <= NOCHE_TEMP_MAX);
    uint8_t hum_ok  = diurno ? (hum  >= DIA_HUM_MIN    && hum  <= DIA_HUM_MAX)
                              : (hum  >= NOCHE_HUM_MIN  && hum  <= NOCHE_HUM_MAX);
    uint8_t normal  = temp_ok && hum_ok;

    /* Trama de telemetría */
    buf[0] = '\0';
    strcat(buf, "["); strcat(buf, hms); strcat(buf, "] T: ");
    u8_to_str(temp, num); strcat(buf, num);
    strcat(buf, "\xB0\x43 | H: ");
    u8_to_str(hum, num);  strcat(buf, num);
    strcat(buf, "% | Estado: ");
    strcat(buf, normal ? "NORMAL\r\n" : "ALERTA\r\n");
    uart_transfer(buf);

    /* Lógica de alarma repetida cada TRAMAS_POR_ALERTA */
    if (!normal)
    {
        if (++tramas_desde_alerta >= TRAMAS_POR_ALERTA)
        {
            tramas_desde_alerta = 0u;

            if (!temp_ok)
                enviar_alerta(hms, diurno, "Temperatura fuera de rango ", temp, 1u);
            if (!hum_ok)
                enviar_alerta(hms, diurno, "Humedad fuera de rango ", hum, 0u);
        }
    }
    else
    {
        tramas_desde_alerta = 0u;
    }
}
