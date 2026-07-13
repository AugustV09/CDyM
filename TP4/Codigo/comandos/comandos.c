/**
 * @file comandos.c
 * @brief Implementación del parser de comandos UART para TP4.
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 */

#include "comandos.h"
#include "../UART/uart.h"
#include "../led_rgb/led_rgb.h"
#include <stdint.h>
#include <string.h>

#define CMD_BUF_SIZE 32u

/* -------------------------------------------------------------------------
 * Funciones privadas
 * ---------------------------------------------------------------------- */

/**
 * @brief Parsea un entero decimal sin signo desde un string hasta encontrar
 *   un delimitador que no sea dígito (',' o '\0').
 * @param s      Puntero al inicio del número en el string.
 * @param val    Puntero donde se guarda el valor parseado.
 * @param next   Puntero donde se deja el puntero después del número.
 * @return 1 si se parseó al menos un dígito y el valor cabe en uint8_t, 0 si
 * no.
 */
static uint8_t parsear_u8(const char *s, uint8_t *val, const char **next)
{
  uint16_t acum = 0u;
  uint8_t digitos = 0u;

  while (*s >= '0' && *s <= '9')
  {
    acum = (uint16_t)(acum * 10u + (uint16_t)(*s - '0'));
    s++;
    digitos++;
    if (acum > 255u)
      return 0u; /* desborde: inválido */
  }

  if (digitos == 0u)
    return 0u;

  *val = (uint8_t)acum;
  *next = s;
  return 1u;
}

/**
 * @brief Parsea y ejecuta SET_COLOR=R,G,B.
 * @param arg  Puntero al inicio del argumento (la parte después de
 * "SET_COLOR=").
 */
static void cmd_set_color(const char *arg)
{
  uint8_t r, g, b;
  const char *p = arg;

  /* Parsear R */
  if (!parsear_u8(p, &r, &p) || *p != ',')
  {
    uart_transfer("ERROR: Uso: SET_COLOR=R,G,B (0-255)\r\n");
    return;
  }
  p++; /* saltar la ',' */

  /* Parsear G */
  if (!parsear_u8(p, &g, &p) || *p != ',')
  {
    uart_transfer("ERROR: Uso: SET_COLOR=R,G,B (0-255)\r\n");
    return;
  }
  p++; /* saltar la ',' */

  /* Parsear B */
  if (!parsear_u8(p, &b, &p))
  {
    uart_transfer("ERROR: Uso: SET_COLOR=R,G,B (0-255)\r\n");
    return;
  }

  /*
   * Aplicar el color.
   * led_rgb_set_color() recibe los valores en el mismo rango que el usuario
   * indica (0 = apagado, 255 = máximo brillo). La inversión de polaridad
   * para el ánodo común ya está encapsulada dentro del módulo led_rgb.
   *
   * Ejemplo: SET_COLOR=0,255,255
   *   r=0   → canal rojo apagado
   *   g=255 → canal verde al máximo
   *   b=255 → canal azul al máximo
   */
  led_rgb_set_color(r, g, b);

  uart_transfer("OK: Color actualizado.\r\n");
}

/** Despacha el string de comando completo al handler correspondiente. */
static void despachar(const char *cmd)
{
  if (strncmp(cmd, "SET_COLOR=", 10u) == 0)
    cmd_set_color(cmd + 10u);
  else
    uart_transfer("ERROR: Comando desconocido.\r\n");
}

/* -------------------------------------------------------------------------
 * API pública
 * ---------------------------------------------------------------------- */

void procesar_comando(void)
{
  static char buf[CMD_BUF_SIZE];
  static uint8_t idx = 0u;
  /*
   * Flag de "línea descartada por desborde": se activa cuando el comando
   * entrante supera CMD_BUF_SIZE-1 caracteres sin encontrar un terminador.
   * Mientras esté activa, se siguen consumiendo bytes del UART (para no
   * dejar basura en el buffer circular) pero sin acumularlos en buf[],
   * hasta encontrar el próximo '\r' que cierra la línea inválida.
   */
  static uint8_t desbordado = 0u;
  uint8_t byte;

  while (uart_read(&byte))
  {
    if (byte == '\n' || byte == '\r')
    {
      if (desbordado)
      {
        /* Línea descartada por desborde: avisar y arrancar limpio. */
        uart_transfer("ERROR: Comando demasiado largo, descartado.\r\n");
        desbordado = 0u;
        idx = 0u;
      }
      else if (idx > 0)
      {
        buf[idx] = '\0';
        idx = 0u;
        despachar(buf);
      }
      return;
    }

    if (idx < (CMD_BUF_SIZE - 1u))
    {
      buf[idx++] = (char)byte;
    }
    else
    {
      /*
       * Desborde: en vez de resetear idx en silencio (lo que descartaría
       * el comando sin que el usuario se entere de por qué), se marca la
       * línea como inválida. Se sigue leyendo y descartando bytes hasta
       * el próximo terminador, momento en el que se informa el error.
       */
      desbordado = 1u;
    }
  }
}