#include "IncisoA.h"

void LEDsRotar(volatile uint8_t *port)
{
	*port = ((*port) << 1) | ((*port) >> 7);
}