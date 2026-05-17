#include <avr/io.h>
#include <cstdint>
#include <stdint.h>
#include "KeyPad.h"

static uint8_t KEYPAD_Update(void) 
{ 
    static volatile uint8_t * const fila_dd[CANT_FILAS] = {
        &FILA_0_DD, &FILA_1_DD, &FILA_2_DD, &FILA_3_DD
    };

    static volatile uint8_t * const fila_port[CANT_FILAS] = {
        &FILA_0_PORT, &FILA_1_PORT, &FILA_2_PORT, &FILA_3_PORT
    };

    static const uint8_t fila_id[CANT_FILAS] = {
        FILA_0_ID, FILA_1_ID, FILA_2_ID, FILA_3_ID
    };

    static volatile uint8_t * const columna_pin[CANT_COLUMNAS] = {
        &COLUMNA_0_PIN, &COLUMNA_1_PIN, &COLUMNA_2_PIN, &COLUMNA_3_PIN
    };

    static volatile uint8_t * const columna_dd[CANT_COLUMNAS] = {
        &COLUMNA_0_DD, &COLUMNA_1_DD, &COLUMNA_2_DD, &COLUMNA_3_DD
    };

    static volatile uint8_t * const columna_port[CANT_COLUMNAS] = {
        &COLUMNA_0_PORT, &COLUMNA_1_PORT, &COLUMNA_2_PORT, &COLUMNA_3_PORT
    };

    static const uint8_t columna_id[CANT_COLUMNAS] = {
        COLUMNA_0_ID, COLUMNA_1_ID, COLUMNA_2_ID, COLUMNA_3_ID
    };

    uint8_t f, c, i;

    for (i=0; i< CANT_FILAS; i++) {
    
        *fila_port[i] &= ~(1 << fila_id[i]);
    }

    for (i=0; i< CANT_COLUMNAS; i++) {
    
        *columna_port[i] |= (1 << columna_id[i]);
        *columna_dd[i] &= ~(1 << columna_id[i]);
    }

    for (f=0; f<CANT_FILAS; f++) {

        for (i = 0; i < CANT_FILAS; i++) {
            if (i == f) {
                *fila_dd[i] |= (1 << fila_id[i]);
            } else {
                *fila_dd[i] &= ~(1 << fila_id[i]);
            }
        }


        _delay_us(1);
        for (c=0; c<CANT_COLUMNAS; c++) {
    
            if ( !( (*columna_pin[c]) & (1u << columna_id[c]) ) ) {
                
                return (f*CANT_FILAS + c);
            }
        }
    }
    return 0xFF;
}

uint8_t KEYPAD_Scan(uint8_t *pkey) 
{
    static uint8_t Old_key = 0xFF, Last_valid_key = 0xFF; // no hay tecla presionada;
    uint8_t Key = KEYPAD_Update();

    if (Key == 0xFF) {
        Old_key = 0xFF; // no hay tecla presionada
        Last_valid_key = 0xFF;
        return 0;
    }

    if (Key == Old_key) {          // 2da verificación
        if (Key != Last_valid_key) { // evita múltiple detección
            *pkey = Key;
            Last_valid_key = Key;
            return 1;
        }
    }

    Old_key = Key; // 1era verificación
    return 0;
}