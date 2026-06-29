#ifndef DHT11_H
#define DHT11_H

#include <stdint.h>
typedef struct
{
    uint8_t rh_int;
    uint8_t rh_dec;
    uint8_t temp_int;
    uint8_t temp_dec;
    uint8_t checksum;
} dht_data_t;

void dht11_init(void);
dht_data_t dht11_read(void);

/** Devuelve 1 si el checksum del DHT11 es correcto. */
uint8_t dht11_checksum_ok(const dht_data_t *d);

#endif