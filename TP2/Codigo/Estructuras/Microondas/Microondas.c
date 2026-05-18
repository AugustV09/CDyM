#include "Microondas.h"
#include "../Errores.h"
#include "../KeyPad/KeyPad.h"
#include "../LCD/lcd.h"
#include "../MEF_Microondas/MEF_Microondas.h"
#include <stdio.h>

static void formatearTiempo(uint16_t tiempo, uint8_t *out_str)
{
    uint8_t horas = tiempo / 100;
    uint8_t minutos = tiempo % 100;
    sprintf((char *)out_str, "%02d:%02d", horas, minutos);
}

void Microondas_Init(uint8_t ticksASegundos)
{
    // Inicializa la máquina de estados interna que ahora es estática
    MEF_Microondas_Init(ticksASegundos);
}

void Microondas_Actualizar(void)
{
    uint8_t tecla;
    uint8_t error;

    if (!KEYPAD_Scan(&tecla))
    {
        tecla = 0; // Si no hay tecla presionada de forma válida, se pasa 0
    }

    error = MEF_ActualizarEstado(tecla);
    MEF_ActualizarSalidas();

    uint8_t mostrarDisplay[6]; // Buffer para "MM:SS" + carácter nulo '\0'
    formatearTiempo(MEF_GetTiempoPantalla(), mostrarDisplay);

    LCDcursorOFF();
    LCDclearAndWriteString((char *)mostrarDisplay);

    uint8_t posCursor = MEF_GetPosTimerPantalla();
    uint8_t mostrarCursor = (MEF_GetEstadoMicroondas() == MEF_MICROONDAS_INGRESO) && posCursor <= 3;

    if (!mostrarCursor)
    {
        return;
    }

    // Si la posición del dígito pasa el separador de los dos puntos ':', saltamos ese espacio en el LCD
    if (posCursor >= 2)
    {
        posCursor++;
    }

    // Ubicar el cursor en la coordenada calculada (columna, fila)
    LCDGotoXY(posCursor, 0);
    LCDcursorOn();
}