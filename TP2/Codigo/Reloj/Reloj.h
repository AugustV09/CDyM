#ifndef RELOJ_H
#define RELOJ_H

#include <stdint.h>

void Reloj_IniciarClock(void);
void Reloj_PararClock(void);
uint8_t Reloj_HayTick(void);
void Reloj_LimpiarTick(void);

#endif