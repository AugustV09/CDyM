/**
 * @file i2c.h
 * @brief Biblioteca para el manejo del periférico TWI (I2C) en AVR.
 * @note Diseñado para el microcontrolador ATmega328P a 16MHz
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 */

#ifndef I2C_H
#define I2C_H

#include "../main.h"
#include <stdint.h>

/**
 * @brief Inicializa el periférico TWI (I2C) del microcontrolador.
 * @details Configura la velocidad del reloj SCL (frecuencia estándar de 100 kHz)
 * y habilita el módulo de hardware.
 */
void i2c_init();

/**
 * @brief Genera la condición de INICIO (START) en el bus I2C.
 * @details Toma el control del bus para iniciar una nueva transacción.
 * Es una función bloqueante que espera hasta que el hardware confirme el START.
 */
void i2c_start();

/**
 * @brief Transmite un byte por el bus I2C.
 * @details Se utiliza tanto para enviar la dirección del esclavo (con el bit R/W)
 * como para enviar registros o datos de configuración.
 * @param data Byte que se desea transmitir.
 */
void i2c_write(uint8_t data);

/**
 * @brief Lee un byte recibido desde el dispositivo esclavo.
 * @details Permite lecturas consecutivas controlando la respuesta de ACK/NACK.
 * @param isLast Indica si es el último byte a leer.
 * - 0: Envía ACK (Acknowledge) para seguir leyendo más bytes.
 * - 1: Envía NACK (Not Acknowledge) para indicarle al esclavo que es el último byte.
 * @return uint8_t El byte de dato leído desde el bus.
 */
uint8_t i2c_read(uint8_t isLast);

/**
 * @brief Genera la condición de PARADA (STOP) en el bus I2C.
 * @details Finaliza la transacción activa y libera las líneas SDA y SCL
 * para que otros dispositivos puedan usar el bus.
 */
void i2c_stop();

#endif /* I2C_H */