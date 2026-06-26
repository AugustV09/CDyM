/**
 * @file rtc_DS3231.c
 * @brief Implementación de funciones simplificadas para el RTC DS3231
 */

#include "rtc_DS3231.h"
#include "../i2c/i2c.h"

#define RTC_ADDR_WRITE 0b11010000 // Dirección I2C para escritura
#define RTC_ADDR_READ 0b11010001  // Dirección I2C para lectura
#define RTC_REG_SEGUNDOS 0x00     // Registro inicial del RTC (Segundos)

/**
 * @brief Convierte decimal estándar a BCD (Binary Coded Decimal).
 */
static uint8_t decimal_to_bcd(uint8_t val)
{
    return ((val / 10) << 4) | (val % 10);
}

/**
 * @brief Convierte BCD a decimal estándar.
 */
static uint8_t bcd_to_decimal(uint8_t val)
{
    return ((val >> 4) * 10) + (val & 0x0F);
}

void rtc_init()
{

    // 2. Carga de valores iniciales completos en el RTC
    i2c_start();
    i2c_write(RTC_ADDR_WRITE);
    i2c_write(RTC_REG_SEGUNDOS); // Apuntamos al registro 0x00 (Segundos)

    // Escribimos toda la ráfaga de registros de corrido (el RTC incrementa el puntero automáticamente)
    i2c_write(decimal_to_bcd(0));  // Reg 0x00: Segundos (00)
    i2c_write(decimal_to_bcd(0));  // Reg 0x01: Minutos (00)
    i2c_write(decimal_to_bcd(17)); // Reg 0x02: Horas (17 - Formato 24h)
    i2c_write(decimal_to_bcd(3));  // Reg 0x03: Día de la semana (1=Dom, 2=Lun... seteamos Martes)
    i2c_write(decimal_to_bcd(14)); // Reg 0x04: Día del mes (14)
    i2c_write(decimal_to_bcd(6));  // Reg 0x05: Mes (06 - Junio)
    i2c_write(decimal_to_bcd(05)); // Reg 0x06: Año (05 - representa 2005)

    i2c_stop();
}

void rtc_set_time(const rtc_time_t *time)
{
    i2c_start();
    i2c_write(RTC_ADDR_WRITE);
    i2c_write(RTC_REG_SEGUNDOS); // Apuntamos al primer registro (0x00)

    // Al escribir secuencialmente, el RTC avanza automáticamente:
    // Registro 0x00 -> Segundos, Registro 0x01 -> Minutos, Registro 0x02 -> Horas
    i2c_write(decimal_to_bcd(time->segundos));
    i2c_write(decimal_to_bcd(time->minutos));
    i2c_write(decimal_to_bcd(time->horas));

    i2c_stop();
}

void rtc_get_time(rtc_time_t *time)
{
    // 1. Escritura esclava ficticia para decirle al RTC que apunte al registro 0x00
    i2c_start();
    i2c_write(RTC_ADDR_WRITE);
    i2c_write(RTC_REG_SEGUNDOS);

    // 2. Reinicio del bus (Repeated Start) en modo lectura
    i2c_start();
    i2c_write(RTC_ADDR_READ);

    // 3. Leemos de corrido los 3 registros que nos importan
    time->segundos = bcd_to_decimal(i2c_read(0)); // Lee Reg 0x00 y envía ACK (quiere más datos)
    time->minutos = bcd_to_decimal(i2c_read(0));  // Lee Reg 0x01 y envía ACK (quiere más datos)
    time->horas = bcd_to_decimal(i2c_read(1));    // Lee Reg 0x02 y envía NACK (es el último que queremos)

    i2c_stop();
}