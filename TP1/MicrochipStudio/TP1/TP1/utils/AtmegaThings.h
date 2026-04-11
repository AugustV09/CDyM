#ifndef ATMEGATHINGS_H
#define ATMEGATHINGS_H

// Se definiran MACROS para la maxima eficiencia
// No consume ciclos en el reloj

// set_bit: pone en el puerto "reg" el bit numero "bit" el valor 1
// clear_bit: Igual que el set_bit pero pone el valor 0

#define set_bit(reg, bit) (reg |= (1 << bit))
#define clear_bit(reg, bit) (reg &= ~(1 << bit))

// set_mask: pone la mascara que le llega. Ej mask = 1 << bit
// clear_mask: Igual que el set_mask pero se lo saca

#define set_mask(reg, mask) (reg |= (mask))
#define clear_mask(reg, mask) (reg &= ~(mask))

/*
// Uso en el código:
    set_bit(PORTB, PB0);

    set_mask(PORTB, 1 << PB0)
*/

#endif