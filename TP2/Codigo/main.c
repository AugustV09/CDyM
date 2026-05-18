#include "main.h"

#include <avr/sleep.h>

#include "MEF_Microondas/MEF_Microondas.h"
#include "Reloj/Reloj.h"

/*
    Cosas para hacer:

    - Borrar comentarios como este.
    - Ver que hacer con los "edge cases", ver si existen
    o ver que queremos hacer nosotros o que quieren ellos.

    - Ej: Cuando finaliza el microondas no puede poner en
    esos 5 segundos de espera un numero, debe apretar
    la B larga para volver al inicio.
    La consigna dice que podemos hacer lo que gustemos...

    - Mostrar el cursos para ver donde esta escribiendo el usuario? NO HACE FALTA PORQUE HAY MICRRONDAS QUE NO LO TIENEN POR EJEMPLO
        La posicion se encuentra en MEF_Microondas.posTimerPantalla

    -Borrar Errores si no los usamos! (Transformar funciones de uint8_t a void)

    Pensa otros ejemplos!!!
    Agarra el microondas de tu casa y hacelo explotar de pruebas
*/

int main(void)
{

    Reloj_IniciarClock();
    Microondas_Init();

    // Seteamos modo IDLE: apaga clock de CPU pero mantiene Timer2 corriendo
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();

    while (1)
    {
        // Se duerme acá. La interrupción de 5ms lo va a despertar.
        sleep_cpu();

        // Al despertar, verificamos si realmente pasaron los 25ms del tick
        if (Reloj_HayTick())
        {
            Reloj_LimpiarTick();

            // Actualiza la máquina de estados del microondas (lectura de teclado, LCD, etc.)
            Microondas_Actualizar();
        }
    }
}