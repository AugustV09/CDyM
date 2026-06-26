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

void dht11_read(dht_data_t *data);

#endif