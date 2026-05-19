Fun facts del microondas

- Si se pausa y luego aprieta la C se hace un +30 y arranca (obviamente puerta cerrada)

CONTINUAR!

Comando para subir al arduino

sudo avrdude -v -p m328p -c arduino -P /dev/ttyUSB0 -b 115200 -D -U flash:w:"/home/fausto/CDyM/TP2/Codigo/main.hex":i

Aclaraciones Diagrama de Estado (revisar antes la primera pagina del PDF del mismo):
- Los casos con t=0 y t=1 que harían que un estado transicione a si mismo debido a la ausencia de las entradas especificas para transicionar a un estado diferente se omitieron para no sobrecargar el diagrama.

Aclaraciones Conceptuales (aquello dejado a nuestro criterio por el enunciado):
- El usuario puede ingresar 60 o más segundos en el par de digitos dedicado a ello en el display pero el reloj de dicho display cuando disminuye por primera vez lo hace a 59 segundos y así sigue (probar en el Proteus para comprenderlo mejor).
