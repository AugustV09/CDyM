/**
 * @file utils.c
 * @brief Implementación de utilidades genéricas de formateo.
 */

#include "utils.h"

void u8_to_str(uint8_t val, char *buf)
{
    char tmp[4];
    uint8_t i = 0u;

    if (val == 0u) { buf[0] = '0'; buf[1] = '\0'; return; }

    while (val > 0u) { tmp[i++] = '0' + (val % 10u); val /= 10u; }
    for (uint8_t j = 0u; j < i; j++) buf[j] = tmp[i - 1u - j];
    buf[i] = '\0';
}

void fmt_time(uint8_t horas, uint8_t minutos, uint8_t segundos, char *buf)
{
    buf[0] = '0' + horas    / 10u;
    buf[1] = '0' + horas    % 10u;
    buf[2] = ':';
    buf[3] = '0' + minutos  / 10u;
    buf[4] = '0' + minutos  % 10u;
    buf[5] = ':';
    buf[6] = '0' + segundos / 10u;
    buf[7] = '0' + segundos % 10u;
    buf[8] = '\0';
}
