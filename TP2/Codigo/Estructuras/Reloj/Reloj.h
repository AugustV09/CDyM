#ifndef RELOJ_H
#define RELOJ_H
#include <stdint.h>

// El callback ahora recibe un contexto genérico
typedef void (*clockCallback)(void *contexto);

void Reloj_IniciarClock(void);
void Reloj_PararClock(void);
void Reloj_SetFunc(clockCallback funcion, void *contexto);

#endif