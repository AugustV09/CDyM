/**
 * @file monitor.h
 * @brief Lógica de monitoreo del invernadero: rangos, ventana horaria y telemetría.
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 *
 * Rangos según Tabla del TP:
 *   Día   (07:00-18:59): Temp 20-30°C | Hum 50-70%
 *   Noche (19:00-06:59): Temp 15-22°C | Hum 60-80%
 */

#ifndef MONITOR_H
#define MONITOR_H

#include <stdint.h>

/* -------------------------------------------------------------------------
 * Rangos óptimos segun la tabla del enunciado
 * ---------------------------------------------------------------------- */
#define DIA_TEMP_MIN 20u
#define DIA_TEMP_MAX 30u
#define DIA_HUM_MIN 50u
#define DIA_HUM_MAX 70u

#define NOCHE_TEMP_MIN 15u
#define NOCHE_TEMP_MAX 22u
#define NOCHE_HUM_MIN 60u
#define NOCHE_HUM_MAX 80u

/** Cantidad de tramas de telemetría entre mensajes de ALERTA repetidos. */
#define TRAMAS_POR_ALERTA 2u

/**
 * @brief Lee el RTC y el DHT11, formatea y envía la trama de telemetría por UART.
 *
 * Trama normal:
 *   [HH:MM:SS] T: XX°C | H: XX% | Estado: NORMAL/ALERTA\r\n
 *
 *   Si el sensor no responde:
 *   [HH:MM:SS] T: --°C | H: --% | Estado: SIN_SENSOR\r\n
 *
 *   Cada TRAMAS_POR_ALERTA tramas en ALERTA, además envía:
 *   [ALERTA] [HH:MM:SS] Temperatura/Humedad fuera de rango diurno/nocturno! Valor: XX\r\n
 */
void enviar_telemetria(void);

#endif /* MONITOR_H */
