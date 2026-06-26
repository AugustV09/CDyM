/**
 * @file rtc_DS3231.h
 * @brief Biblioteca para el control del tiempo (Hora, Min, Seg) en el RTC DS3231
 */

#ifndef RTC_DS3231_H
#define RTC_DS3231_H

#include <stdint.h>

/**
 * @struct rtc_time_t
 * @brief Estructura simplificada para almacenar únicamente los datos horarios.
 */
typedef struct
{
    uint8_t horas;
    uint8_t minutos;
    uint8_t segundos;
} rtc_time_t;

/**
 * @brief Inicializa la comunicación con el RTC. Setea hora basica.
 * @note Se debe inicializar el i2c antes de llamar a esta funcion.
 */
void rtc_init();

/**
 * @brief Configura una nueva hora en el RTC.
 * @param time Puntero a la estructura con horas, minutos y segundos deseados.
 */
void rtc_set_time(const rtc_time_t *time);

/**
 * @brief Lee la hora actual del RTC.
 * @param time Puntero a la estructura donde se guardarán los datos leídos.
 */
void rtc_get_time(rtc_time_t *time);

#endif /* RTC_H */