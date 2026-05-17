#ifndef KEYPAD_H
#define KEYPAD_H
#include <stdint.h>
#include <avr/io.h>

//De base se va a hacer un teclado de 4x4 pero es extensible a más agregando las directivas necesarias según se comenta mas adelante en este mismo archivo y agregando la logica correspondiente en KeyPad.c (dicha logica tendría que seguir lo que ya va a estar escrito en KeyPad.c)
#define CANT_FILAS 4
#define CANT_COLUMNAS 4

#define FILA_0_PORT PORTB
#define FILA_1_PORT PORTB
#define FILA_2_PORT PORTB
#define FILA_3_PORT PORTD
/*Escribe a continuación los PORTX de las filas que quieras añadir:

*/

#define FILA_0_DD DDRB
#define FILA_1_DD DDRB
#define FILA_2_DD DDRB
#define FILA_3_DD DDRD
/*Escribe a continuación los DDRX de las filas que quieras añadir:

*/

#define FILA_0_ID PORTB4
#define FILA_1_ID PORTB3
#define FILA_2_ID PORTB0
#define FILA_3_ID PORTD7
/*Escribe a continuación los bits de los puertos a usar en las filas que quieras añadir:

*/

#define COLUMNA_0_PORT PORTD
#define COLUMNA_1_PORT PORTD
#define COLUMNA_2_PORT PORTD
#define COLUMNA_3_PORT PORTD
/*Escribe a continuación los PORTX de las columnas que quieras añadir:

*/

#define COLUMNA_0_DD DDRD
#define COLUMNA_1_DD DDRD
#define COLUMNA_2_DD DDRD
#define COLUMNA_3_DD DDRD
/*Escribe a continuación los DDRX de las columnas que quieras añadir:

*/

#define COLUMNA_0_PIN PIND
#define COLUMNA_1_PIN PIND
#define COLUMNA_2_PIN PIND
#define COLUMNA_3_PIN PIND
/*Escribe a continuación los PINX de las columnas que quieras añadir:

*/

#define COLUMNA_0_ID PORTD3
#define COLUMNA_1_ID PORTD5
#define COLUMNA_2_ID PORTD4
#define COLUMNA_3_ID PORTD2
/*Escribe a continuación los bits de los puertos a usar en las columnas que quieras añadir:

*/

uint8_t KEYPAD_Scan(uint8_t *); 

#endif