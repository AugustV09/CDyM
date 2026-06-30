/**
 * @file dht11.h
 * @brief Interfaz del sensor DHT11 (temperatura y humedad relativa).
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 */

#ifndef DHT11_H
#define DHT11_H

#include <stdint.h>

typedef struct {
  uint8_t rh_int;
  uint8_t rh_dec;
  uint8_t temp_int;
  uint8_t temp_dec;
  uint8_t checksum;
} dht_data_t;

/** Configura PC0 como entrada con pull-up para el bus del DHT11. */
void dht11_init(void);

/**
 * @brief Lee humedad y temperatura del sensor.
 * @note Usar dht11_checksum_ok() para validar la trama antes de confiar en los datos.
 */
dht_data_t dht11_read(void);

/** Devuelve 1 si el checksum de la última lectura es válido. */
uint8_t dht11_checksum_ok(const dht_data_t *d);

#endif
