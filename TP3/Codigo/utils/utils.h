/**
 * @file utils.h
 * @brief Utilidades genéricas de formateo para bare metal (sin sprintf).
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

/**
 * @brief Convierte un uint8_t a string decimal terminado en '\0'.
 * @param val  Valor a convertir (0-255).
 * @param buf  Buffer destino (mínimo 4 bytes).
 */
void u8_to_str(uint8_t val, char *buf);

/**
 * @brief Escribe "HH:MM:SS\0" en buf a partir de horas, minutos y segundos.
 * @param horas    Valor de horas (0-23).
 * @param minutos  Valor de minutos (0-59).
 * @param segundos Valor de segundos (0-59).
 * @param buf      Buffer destino (mínimo 9 bytes).
 */
void fmt_time(uint8_t horas, uint8_t minutos, uint8_t segundos, char *buf);

#endif /* UTILS_H */
