#include "config.h"

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
        MicroondasTecladoPresionado(&microondas, teclaPrecionada);
        MicroondasAplicarCambiosDeEstado(&microondas);

        // void actualizarDisplay(microondas.tiempo); -> NO EXISTE TODAVIA

        /*
            Ideas del timer:
            Si esta encendido, es decir se le dio a IniciarClock();
            Deberia ejecutar cada 1 segundo el ActualizarDisplay();
            Y cuando detecte que se volvio 0 -> Microondas
        */
    }
}
