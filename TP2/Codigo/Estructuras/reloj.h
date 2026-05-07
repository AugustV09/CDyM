#ifndef RELOJ_H_
#define RELOJ_H_

#include <stdint.h>

typedef void (*voidFuncPtr)(void);

void IniciarClock(void);
void PararClock(void);
void SetearTiempoDeClock(uint32_t nuevo_tiempo);
uint32_t GetTiempoClock(void);

void SumarTiempo(uint32_t segundos_a_sumar);

void SetFunc(voidFuncPtr funcion);

#endif