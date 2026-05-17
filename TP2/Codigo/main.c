#include "main.h"

#include <avr/sleep.h>

#include "Estructuras/LCD/lcd.h"
#include "Microondas.h"
#include "lcd.h"
#include "Reloj.h"

#define TICKS_A_SEGUNDO 8 //{K} es 8


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

    uint8_t teclaPrecionada;
    struct Microondas microondas;
    // Definición de pines según consigna
    struct PuertoPin mag = {&PORTB, PB5};
    struct PuertoPin luz = {&PORTC, PC4};
    struct PuertoPin alm = {&PORTC, PC5};

    Microondas_Init(&microondas, TICKS_A_SEGUNDO, mag, luz, alm);
    LCD_Init();
    Reloj_IniciarClock();
    Reloj_SetTicksPorSegundo(TICKS_A_SEGUNDO); //Nuevo

    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();

    while (1)
    {
        
        // Idea
        /*
            Que el sleep se haga y se llame al update en un N tiempo constante
            {K} <- Numero que esta arriba!
            Que cada {K} N tiempos signifique un segundo
        */
        while (!Reloj_HayTick()) //Nuevo
            sleep_cpu();

        Reloj_LimpiarTick(); //Nuevo
        Microondas_Actualizar(&microondas);
    }
}
