/**
 * @file fade.c
 * @brief Implementación de la lógica de fade del LED RGB.
 * @author Rodriguez Fausto, Octavio Tomas Isabella Valenzi
 *
 * Máquina de estados con 4 fases:
 *   SUBIENDO -> MESETA -> BAJANDO -> PAUSA -> SUBIENDO ...
 *
 * La "entrada" de la máquina es un contador de ticks que se incrementa
 * con cada llamada a fade_tick() (cada ~10 ms, cadenciado por Timer2).
 * Las transiciones ocurren cuando el contador alcanza la duración de
 * la fase actual en ticks.
 *
 * Forma de onda de intensidad:
 *
 *   255 |      ___________
 *       |     /           \
 *       |    /             \
 *     0 |___/               \___
 *       |<1s><---1s---><1s->|
 *       |<--------T-------->|
 *
 * T = periodo_total_ms (3000-6000 ms), modificable en tiempo de ejecución.
 * Las tres fases activas (subiendo, meseta, bajando) duran 1 s cada una.
 * La pausa dura T - 3 s (0 s cuando T = 3000 ms).
 */

#include "fade.h"
#include "../led_rgb/led_rgb.h"

/* =========================================================================
 * Constantes de timing
 * ====================================================================== */

/**
 * Ticks por fase de 1 segundo.
 * Con tick de ~10 ms (Timer2 cada 313 interrupciones):
 *   1000 ms / 10 ms ~ 100 ticks por segundo.
 */
#define TICKS_POR_FASE 100u

/**
 * Duración total de las tres fases fijas (subiendo + meseta + bajando)
 * en milisegundos.
 */
#define DURACION_FASES_FIJAS_MS 3000u

/**
 * Periodo del tick de fade en milisegundos (aprox. 10 ms).
 * Usado para convertir milisegundos a ticks:
 *   ticks = milisegundos / MS_POR_TICK
 */
#define MS_POR_TICK 10u

/* =========================================================================
 * Máquina de estados - tipos y variables
 * ====================================================================== */

/**
 * Estados de la máquina de fade.
 * Definidos como enum privado: solo fade.c los necesita.
 */
typedef enum {
  FADE_SUBIENDO, /**< Intensidad sube de 0 a 255 en ~1 s.        */
  FADE_MESETA,   /**< Intensidad permanece en 255 durante ~1 s.  */
  FADE_BAJANDO,  /**< Intensidad baja de 255 a 0 en ~1 s.        */
  FADE_PAUSA     /**< Intensidad en 0 durante (T - 3 s).         */
} fade_estado_t;

/** Estado actual de la máquina. */
static fade_estado_t estado = FADE_SUBIENDO;

/**
 * Contador de ticks transcurridos en la fase actual.
 * Se reinicia a 0 en cada transición de estado.
 */
static uint16_t contador = 0u;

/** Periodo total en ms (modificable en tiempo de ejecución). */
static uint16_t periodo_total_ms = 3000u;

/* =========================================================================
 * API pública
 * ====================================================================== */

void fade_init(uint16_t periodo_ms) {
  fade_set_periodo_ms(periodo_ms);

  estado = FADE_SUBIENDO;
  contador = 0u;

  /* Arrancar con intensidad 0 (la máquina la irá subiendo). */
  led_rgb_set_intensidad(0u);
}

void fade_tick(void) {
  /*
   * La máquina se estructura como un switch sobre el estado actual.
   * Dentro de cada caso, el contador de ticks (incrementado en cada
   * llamada) actúa como la "entrada" que determina la transición.
   *
   * Para la intensidad en subida/bajada se usa aritmética de 16 bits:
   *   intensidad = (contador * 255) / TICKS_POR_FASE
   * El producto máximo es 100 x 255 = 25500, que cabe en uint16_t.
   */
  switch (estado) {
  case FADE_SUBIENDO:
    contador++;
    if (contador >= TICKS_POR_FASE) {
      /* Llegó al final de la subida: fijar 255 explícitamente
       * para evitar error de redondeo en la división entera. */
      led_rgb_set_intensidad(255u);
      estado = FADE_MESETA;
      contador = 0u;
    } else {
      /* Rampa lineal ascendente:
       *   tick 1   -> (1 x 255) / 100 = 2
       *   tick 50  -> (50 x 255) / 100 = 127
       *   tick 99  -> (99 x 255) / 100 = 252  */
      uint8_t intensidad =
          (uint8_t)(((uint16_t)contador * 255u) / TICKS_POR_FASE);
      led_rgb_set_intensidad(intensidad);
    }
    break;

  case FADE_MESETA:
    contador++;
    if (contador >= TICKS_POR_FASE) {
      /* Fin de la meseta: transición a bajada. */
      estado = FADE_BAJANDO;
      contador = 0u;
    }
    /* Intensidad se mantiene en 255 (fijada al entrar a MESETA). */
    break;

  case FADE_BAJANDO:
    contador++;
    if (contador >= TICKS_POR_FASE) {
      /* Llegó al final de la bajada: fijar 0 explícitamente. */
      led_rgb_set_intensidad(0u);
      estado = FADE_PAUSA;
      contador = 0u;
    } else {
      /* Rampa lineal descendente:
       *   tick 1   -> 255 - (1 x 255) / 100 = 253
       *   tick 50  -> 255 - (50 x 255) / 100 = 128
       *   tick 99  -> 255 - (99 x 255) / 100 = 3    */
      uint8_t intensidad =
          255u - (uint8_t)(((uint16_t)contador * 255u) / TICKS_POR_FASE);
      led_rgb_set_intensidad(intensidad);
    }
    break;

  case FADE_PAUSA: {
    /*
     * Duración de la pausa en ticks:
     *   ticks_pausa = (periodo_total_ms - 3000) / 10
     *
     * Cuando T = 3000 ms, ticks_pausa = 0 -> transición inmediata
     * (el LED nunca se queda apagado, sube de nuevo enseguida).
     *
     * Se lee periodo_total_ms en cada tick para que un cambio
     * dinámico del periodo (vía fade_set_periodo_ms()) tome efecto
     * sin esperar al próximo ciclo completo.
     */
    uint16_t pausa_ms = 0u;
    if (periodo_total_ms > DURACION_FASES_FIJAS_MS)
      pausa_ms = periodo_total_ms - DURACION_FASES_FIJAS_MS;

    uint16_t ticks_pausa = pausa_ms / MS_POR_TICK;

    if (contador >= ticks_pausa) {
      /* Fin de la pausa: reiniciar ciclo. */
      estado = FADE_SUBIENDO;
      contador = 0u;
      led_rgb_set_intensidad(0u);
    } else {
      contador++;
    }
    /* Intensidad se mantiene en 0 (fijada al entrar a PAUSA). */
    break;
  }
  }
}

void fade_set_periodo_ms(uint16_t periodo_ms) {
  if (periodo_ms < 3000u)
    periodo_ms = 3000u;
  if (periodo_ms > 6000u)
    periodo_ms = 6000u;
  periodo_total_ms = periodo_ms;
}
