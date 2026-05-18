#ifndef RELOJ_H
#define RELOJ_H

#include <stdint.h>

// Inicialización básica del hardware (Timer2 a 5ms)
void Reloj_Init(void);

// Control de la cuenta de segundos
void Reloj_StartCountingSeconds(void);
void Reloj_StopCountingSeconds(void);
void Reloj_RestartSeconds(void);

// Gestión de Ticks (25ms) para el loop principal / MEF
uint8_t Reloj_HayTick(void);
void Reloj_LimpiarTick(void);

// Gestión de Segundos
uint8_t Reloj_HaySegundo(void);
void Reloj_LimpiarSegundo(void);

#endif // RELOJ_H