#include "main.h"

#include "Microondas.h"

/*
    Cosas para hacer:

    - Borrar comentarios como este.
    - Ver que hacer con los "edge cases", ver si existen
    o ver que queremos hacer nosotros o que quieren ellos.

    - Ej: Cuando finaliza el microondas no puede poner en
    esos 5 segundos de espera un numero, debe apretar
    la B larga para volver al inicio.
    La consigna dice que podemos hacer lo que gustemos...

    - Mostrar el cursos para ver donde esta escribiendo el usuario?
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

    Microondas_Init(&microondas, 8, mag, luz, alm); // El numero 8 es {K}

    while (1)
    {

        // Idea
        /*
            Que el sleep se haga y se llame al update en un N tiempo constante
            {K} <- Numero que esta arriba!
            Que cada {K} N tiempos signifique un segundo
        */

        Microondas_Actualizar(&microondas);
    }
}
