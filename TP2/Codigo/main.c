#include "main.h"

#include "Microondas.h"

int main(void)
{

    uint8_t teclaPrecionada;
    struct Microondas microondas;
    // Definición de pines según consigna
    struct PuertoPin mag = {&PORTB, PB5};
    struct PuertoPin luz = {&PORTC, PC4};
    struct PuertoPin alm = {&PORTC, PC5};

    MicroondasInicializarMicroondas(&microondas, mag, luz, alm);

    while (1)
    {
        /*
            Idea simple:

            1) leer teclado
            2) ver si se apreto algo
            3) si se apreto algo, llamar a pulso teclado microondas
            4) actualizar display
        */

        // void leerTeclado(); -> NO EXISTE TODAVIA
        // El leer teclado es del teclado...

        // Hacer un IF que sea si hubo pulsacion entonces esto:

        if (MicroondasTecladoPresionado(&microondas, teclaPrecionada) == SIN_ERROR)
            MicroondasAplicarCambiosDeEstado(&microondas);

        // void actualizarDisplay(microondas.tiempo); -> NO EXISTE TODAVIA
        // El actualizar display NO es del microondas

        // El actualizar display puede ser con el tiempo...
        // Si salto algun error... mostrarlo hasta que se vuelva a presionar una tecla!!!
        // Que no sea con timer..., que no sea bueno mostremos el error por 1 segundo

        // Desaparecer ese error del display si:
        // -> Se presiona cualquier tecla.
        // -> Si cambio el tiempo del microondas
        // -> Tipo si el error pasa cuando quedan 00:01 segs cuando pasa a 00:00 el error debe desaparecer
        // -> Si cambia el estado. Si el error pasa cuando se esta pausado y se pasa a ingeso, el error debe desaparecer

        // CUALQUIER COSA PREGUNTAME!!!
    }
}
