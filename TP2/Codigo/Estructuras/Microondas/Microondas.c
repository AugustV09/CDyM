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

void Microondas_Init(void)
{
    // Ya no arrastra parámetros desde el main
    MEF_Microondas_Init();
}

void Microondas_Actualizar(void)
{
    uint8_t tecla;
    uint8_t error;

    if (!KEYPAD_Scan(&tecla))
    {
        tecla = 0;
    }

    error = MEF_ActualizarEstado(tecla);
    MEF_ActualizarSalidas();

    uint8_t mostrarDisplay[6];
    formatearTiempo(MEF_GetTiempoPantalla(), mostrarDisplay);

    LCDcursorOFF();
    LCDclearAndWriteString((char *)mostrarDisplay);

    uint8_t posCursor = MEF_GetPosTimerPantalla();
    uint8_t mostrarCursor = (MEF_GetEstadoMicroondas() == MEF_MICROONDAS_INGRESO) && posCursor <= 3;

    if (MEF_GetEstadoMicroondas() == MEF_MICROONDAS_FIN_COCCION)
    {
        LCDblank();
        return;
    }

    if (!mostrarCursor)
    {
        return;
    }

    if (posCursor >= 2)
    {
        posCursor++;
    }

    LCDGotoXY(posCursor, 0);
    LCDcursorOn();
}