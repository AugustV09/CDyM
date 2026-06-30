/**
 * @file i2c.c
 * @brief Implementación de las funciones de control para el periférico TWI (I2C).
 * @note Basado en las especificaciones del registro TWI del ATmega328P.
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 */

#include "i2c.h"

/**
 * @brief Inicializa el periférico TWI (I2C) del microcontrolador.
 * @details Configura los registros de tasa de bits (Bit Rate) y prescaler
 * para establecer la frecuencia de SCL a 100 kHz (Modo Estándar).
 */
void i2c_init()
{
    // Configura el prescaler en 1 (TWPS1 = 0 y TWPS0 = 0 en el registro TWSR)
    TWSR = 0x00;

    // Configura el valor del Bit Rate Register (TWBR)
    TWBR = VALOR_TWBR;

    // Configura el registro de control inicial de la interfaz TWI
    TWCR = 0x04;
}

/**
 * @brief Genera la condición de INICIO (START) en el bus I2C.
 * @details Envía la señal de START al bus y espera mediante polling a que
 * el hardware tome el control de las líneas SDA y SCL.
 */
void i2c_start()
{
    // Se escribe un 1 en TWINT para limpiarlo, se solicita START (TWSTA) y se mantiene activo el módulo (TWEN)
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    // Espera bloqueante hasta que el hardware finalice la condición de START (TWINT se pone en 1)
    while ((TWCR & (1 << TWINT)) == 0)
        ;
}

/**
 * @brief Genera la condición de PARADA (STOP) en el bus I2C.
 * @details Envía la señal de STOP para liberar el bus.
 */
void i2c_stop()
{
    // Se escribe un 1 en TWINT para limpiarlo, se solicita STOP (TWSTO) y se mantiene activo el módulo (TWEN)
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}

/**
 * @brief Transmite un byte por el bus I2C (Dirección o Datos).
 * @details Carga el dato en el registro de transmisión y limpia el flag de
 * interrupción para iniciar el envío físico de los bits.
 * @param data Byte a transmitir.
 */
void i2c_write(uint8_t data)
{
    // Almacena el byte a transmitir en el registro de datos TWI (TWDR)
    TWDR = data;

    // Limpia TWINT para iniciar la transmisión por hardware manteniendo el módulo habilitado
    TWCR = (1 << TWINT) | (1 << TWEN);

    // Espera mediante polling hasta que el byte y su respectivo ACK/NACK hayan sido transferidos
    while ((TWCR & (1 << TWINT)) == 0)
        ;
}

/**
 * @brief Lee un byte recibido desde el esclavo y define la respuesta (ACK/NACK).
 * @details Configura el periférico para responder con un ACK si se esperan más datos,
 * o con un NACK si es el último byte solicitado, protegiendo el flujo de lectura.
 * @param isLast Flag de control (0 para solicitar más bytes con ACK, 1 para finalizar con NACK).
 * @return uint8_t Byte de datos leído desde el registro TWDR.
 */
uint8_t i2c_read(uint8_t isLast)
{
    if (isLast == 0)
        // Quedan bytes por leer: Limpia TWINT y habilita la generación automática de ACK (TWEA)
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    else
        // Último byte: Limpia TWINT sin habilitar TWEA para que el máster responda con NACK
        TWCR = (1 << TWINT) | (1 << TWEN);

    // Espera bloqueante hasta que el byte del esclavo haya sido recibido por completo
    while ((TWCR & (1 << TWINT)) == 0)
        ;

    // Retorna el dato capturado en el registro de datos
    return TWDR;
}