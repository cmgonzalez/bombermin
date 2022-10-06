/* ----------------------------------------------------------------
 * Rascan - The Simmerian
 * ----------------------------------------------------------------
   This file is part of Prisma Engine v2.0.

   Prisma Engine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Prisma Engine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with PRISMA ENGINE.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "game.h"
/*
 * Function:  main
 * --------------------
 * main game routine
 *
 */

void main(void) {
  // z80_delay_ms(250);
  // in_wait_key();
  // in_wait_nokey();

  // Inicializa controles
  k1.fire = IN_KEY_SCANCODE_SPACE;
  k1.left = IN_KEY_SCANCODE_o;
  k1.right = IN_KEY_SCANCODE_p;
  k1.up = IN_KEY_SCANCODE_q;
  k1.down = IN_KEY_SCANCODE_a;
  joyfunc1 = (uint16_t(*)(udk_t *))(in_stick_keyboard);

  // Inicializa pantalla
  zx_border(INK_WHITE);
  zx_cls(PAPER_WHITE);
  printPaper(INK_WHITE);
  printInk(INK_BLACK);
  printAt(0, 12);
  // printf("Bombermin");
  printAt(23, 14);
  printf("Demo");

  // Configura direcciones para los btiles y blocks(UDGs)
  NIRVANAP_tiles(_btiles);
  NIRVANAP_chars(_blocks);

  // Configura los Atributos por defecto de nirvana para print_udg_nirv
  nirv_attribs[0] = nirv_attribs[1] = nirv_attribs[2] = nirv_attribs[3] =
      PAPER_BLACK | INK_CYAN;

  // Extrae Habitación 0

  // Enlaza la función "im2"  con la interrupción IM2 de Nirvana
  NIRVANAP_ISR_HOOK[0] = 205;
  z80_wpoke(&NIRVANAP_ISR_HOOK[1], (unsigned int)im2);
  // Atributos de fondo para pintar al limpiar
  attrs_back = &arr_back[0];
  attrs_fire_red = &arr_fire_red[0];
  attrs_fire_yellow = &arr_fire_yellow[0];

  // Bucle principal del Juego
  entity = 0;
  bomb = 0;
  game_entities = 0;
  // Valores iniciales de la Partida
  player_lives = PLAYER_LIVES;
  player_radius = 1;
  player_bombs = 1;
  player_bomb_walk = 0;
  player_brick_walk = 0;
  player_speed = PLAYER_NORMAL;
  player_hit = 0;
  game_stage = 1;
  game_time = 200;
  // Inicializa las entidades
  entity_init();
  // Pantalla
  printAt(0, 1);
  printf("TIME");
  printAt(0, 24);
  printf("LEFT");
  // Crea Mapa
  map_create();
  // Inicializa Bombas
  bomb_init();
  // Dibuja el mapa
  map_draw();
  // Activa NIRVANA ENGINE
  NIRVANAP_start();
  // Activa rutinas im2
  im2_pause = 0;
  for (;;) {
    print_char(game_time, 0, 6);
    print_char(game_score, 0, 16);
    printAt(0, 28);
    printf("%2i", player_lives);
    // Anima Bombas
    bomb_anim();
    // Explota Bombas
    explode_check();
    // Pausa
    if (!in_explo)
      z80_delay_ms(25);

    entity_collision();
    if (player_hit) {
      beepSteve(14);
      player_hit = 0;
    }
    // Sonidos desde im2
    if (player_sound) {
      beepSteve(player_sound);
      player_sound = 0;
    }
    // Debug al apretar el Enter
    if (in_inkey() == 13) {
      printAt(0, 6);
      printf("L%3iC%3i", *lin, *col);
      z80_delay_ms(50);
      print_char(sfx, 1, 0);
      beepSteve(sfx);

      ++sfx;
      if (sfx > 16) {
        sfx = 1;
      }
    }
  }
}

/*
 * Function:  draw
 * --------------------
 * Dibuja entidad, envuelve lógica para rotar entre bordes de pantalla
 * horizontalmente
 */
void draw() {
  unsigned char t;
  // Dibuja un entity
  if (entity == 0) {
    switch (dirs[0]) {
    case BIT_RIGHT:
      t = BTILE_PLAYER_HOR + *frame;
      break;
    case BIT_LEFT:
      t = BTILE_PLAYER_HOR + FRAMES + *frame;
      break;
    case BIT_DOWN:
      t = BTILE_PLAYER_VER + *frame;
      break;
    case BIT_UP:
      t = BTILE_PLAYER_VER + FRAMES + *frame;
      break;
    }
  } else {
    t = tiles[entity] + *frame;
  }
  // Restaura Fondo
  draw_restore();
  // Tile Personaje
  btile_draw(t, *lin, *col - scroll_min);
}

/*
 * Function:  draw_restore
 * --------------------
 * Restaura fondo de una entidad, restauración completa
 * movimiento
 */
void draw_restore() {
  if (*col == col0 && *lin == lin0) {
    return;
  }
  // map_restore(lin0 >> 4 << 4, col0 >> 1 << 1);
  map_restore(lin0, col0);
}

/*
 * Function:  map_draw
 * --------------------
 * Dibuja la pantalla en base a los datos del arreglo screen
 */
void map_draw() {
  unsigned int i;
  // Limpia Sprites Nirvana para evitar problemas
  NIRVANAP_spriteT(0, 0, 0, 0);
  NIRVANAP_spriteT(1, 0, 0, 0);
  NIRVANAP_spriteT(2, 0, 0, 0);
  NIRVANAP_spriteT(3, 0, 0, 0);
  NIRVANAP_spriteT(4, 0, 0, 0);
  NIRVANAP_spriteT(5, 0, 0, 0);
  NIRVANAP_spriteT(6, 0, 0, 0);
  NIRVANAP_spriteT(7, 0, 0, 0);

  // Dibuja Mapa
  lin0 = 16;
  foo = 0;

  while (lin0 < 192) {
    col0 = 0;
    while (col0 < 31) {
      NIRVANAP_drawT_raw(screen[i + (scroll_min >> 1)], lin0, col0);
      ++i;
      col0 += 2;
    }
    ++foo;
    i = foo * (16 * 3);
    lin0 += 16;
  }
}

/*
 * Function:  frame_inc
 * --------------------
 * Aumenta el frames de animación, vuelve a cero al llegar al Máximo FRAMES (3)
 */
void frame_inc() {
  ++(*frame);
  if (*frame >= FRAMES) {
    *frame = 0;
  }
}

/*
 * Function:  entity_getnext
 * --------------------
 * Retorna la siguiente entidad que necesita ser dibujada,
 * si no hay candidata, retorna la próxima según contador entity_im2
 *
 */
unsigned char entity_getnext() {
  unsigned char e;

  ++entity_im2;
  if (entity_im2 >= ENTITIES) {
    entity_im2 = 1;
  }
  e = entity_im2;
  while (e) {
    if (time >= timers[e]) {
      entity_im2 = e;
      return e;
    }

    ++e;
    if (e >= ENTITIES) {
      e = 1;
    }
    if (e == entity_im2) {
      return e;
    }
  }
  return 1;
}

/*
 * Function:  entity_anim
 * --------------------
 * Anime entidades
 */
void entity_anim() {
  im2_free = 0;

  if ((time & 7) == 0) {
    // cada 4 frames se mueve el player
    entity = 0;
  } else {
    // Busco siguiente entidad para
    entity = entity_getnext(); // entity_im2;
  }

  if (time >= timers[entity] && types[entity] != ENT_OFF) {
    frame = &frames[entity];
    col = &cols[entity];
    lin = &lins[entity];
    col0 = *col;
    lin0 = *lin;
    switch (types[entity]) {
    case ENT_PLAYER:
      entity_move_player();
      break;
    case ENT_BALLON:
      entity_move_ballon();
      break;
    case ENT_BEAKER:
      entity_move_ballon();
      break;
    case ENT_EXPLODING:
      draw();
      ++*frame;
      if (*frame == 5) {
        // Elimina entidad
        types[entity] = ENT_OFF;
        map_restore(lin0, col0);
      } else {
        timers[entity] = time + 5;
      }
      break;
    case ENT_DIE:
      btile_draw(tiles[entity] + 3, *lin, *col - scroll_min);
      timers[entity] = time;
      --values[entity];
      if (values[entity] == 0) {
        types[entity] = ENT_EXPLODING;
        tiles[entity] = BTILE_DIE;
        break;
      }
    default:
      frame_inc();
      draw();
      // entity_move_ballon();
      break;
    }
  }

  im2_free = 1;
  zx_border(INK_WHITE);
}

void entity_collision() {
  unsigned char e = 1;
  // Todos
  while (e < ENTITIES) {
    if (types[e] < ENT_ACTIVE) {
      if ((abs(cols[e] - cols[0]) < 2) && (abs(lins[e] - lins[0]) < 16)) {
        zx_border(INK_RED);
        player_hit = 1;
        return;
      }
    }
    ++e;
  }
}
/*
 * Function:  entity_init
 * --------------------
 * Inicializa entidades
 *
 */
void entity_init() {
  entity = 0;
  // Todos
  while (entity < ENTITIES) {
    dir = &dirs[entity];
    entity_chdir();
    // dirs[entity] = BIT_LEFT;
    cols[entity] = 2 + (entity * 2); // (entity * 4) - 2;
    lins[entity] = 32;

    // types[entity] = 1 + (entity % 8);
    // tiles[entity] = BTILE_BALLON + (4 * ((entity - 1) % 8));
    types[entity] = 1;
    tiles[entity] = BTILE_BALLON;
    frames[entity] = 2;
    ++entity;
  }
  // Jugador
  types[0] = ENT_PLAYER;
  cols[0] = 2;
  lins[0] = 32;
  dirs[0] = BIT_RIGHT;
  tiles[0] = BTILE_PLAYER_HOR;
  // Valores iniciales para el control del scroll
  scroll_min = 0;
  scroll_max = 31;
}
/*
 * Function:  entity_move_player
 * --------------------
 * Mueve al protagonista
 *
 */
void entity_move_player() {
  unsigned inp;

  // Fijo la próxima llamada en 5 frames, TODO ponerla al final...

  inp = (joyfunc1)(&k1);

  if (inp & IN_STICK_FIRE) {
    bomb_add();
    // frame_inc();
    // draw();
    return;
  }
  if (inp & IN_STICK_LEFT) {
    dirs[0] = BIT_LEFT;
    move_left();
    if ((scroll_min > 0) && (*col - scroll_min) <= 8) {
      // Desplaza Mapa a la Izquierda
      map_scroll(BIT_LEFT);
    }
    frame_inc();
    draw();
    return;
  }
  if (inp & IN_STICK_RIGHT) {
    dirs[0] = BIT_RIGHT;
    move_right();
    if ((scroll_min < 64) && (*col - scroll_min) >= 24) {
      // Desplaza Mapa a la Derecha
      map_scroll(BIT_RIGHT);
    }
    frame_inc();
    draw();
    return;
  }
  if (inp & IN_STICK_DOWN) {
    dirs[0] = BIT_DOWN;
    move_down();
    frame_inc();
    draw();
    return;
  }
  if (inp & IN_STICK_UP) {
    dirs[0] = BIT_UP;
    move_up();
    frame_inc();
    draw();
    return;
  }
  if (inp == 0) {
    *frame = 0;
    draw();
    return;
  }
}
void entity_move_rc() {
  if (in_inkey() == 49) { // i
    if (move_cleft()) {
      move_left();
    }
  }
  if (in_inkey() == 50) { // d
    if (move_cright()) {
      move_right();
    }
  }
  if (in_inkey() == 51) { // arr
    if (move_cup()) {
      move_up();
    }
  }
  if (in_inkey() == 52) { // abajo
    if (move_cdown()) {
      move_down();
    }
  }
}
/*
 * Function:  entity_move_ballon
 * --------------------
 * Mueve a los enemigos
 *
 */
void entity_move_ballon() {

  switch (dirs[entity]) {
  case BIT_UP:
    if (move_cup()) {
      move_up();
    } else {
      entity_chdir();
    }
    break;
  case BIT_DOWN:
    if (move_cdown()) {
      move_down();
    } else {
      entity_chdir();
    }
    break;
  case BIT_LEFT:
    if (move_cleft()) {
      move_left();

    } else {
      entity_chdir();
      // dirs[entity] = BIT_RIGHT;
    }
    break;
  case BIT_RIGHT:
    if (move_cright()) {
      move_right();
    } else {
      // dirs[entity] = BIT_LEFT;
      entity_chdir();
    }
    break;
  }

  if (*col == (scroll_min + 32)) {
    map_restore(lin0, scroll_min + 30);
    return;
  }
  if (*col == (scroll_min - 1)) {
    map_restore(lin0, scroll_min);
    return;
  }

  // Determina si esta visible
  if (*col >= scroll_min && *col <= scroll_max) {
    // Visible
    frame_inc();
    // Borde Derecho
    if ((*col - scroll_min) == 0) {
      draw_restore();
      btile_half_h(0, tiles[entity] + *frame, *lin, 0);
      return;
    }
    // Borde Izquierdo
    if ((*col - scroll_min) == 31) {
      draw_restore();
      btile_half_h(1, tiles[entity] + *frame, *lin, 30);
      return;
    }

    // Normal
    draw();
    timers[entity] = time + 5;
  }
}

void entity_chdir() {
  unsigned char r = rand() % 4;
  // zx_border(INK_RED);
  switch (r) {
  case 0:
    dirs[entity] = BIT_UP;
    break;
  case 1:
    dirs[entity] = BIT_DOWN;
    break;
  case 2:
    dirs[entity] = BIT_LEFT;
    break;
  case 3:
    dirs[entity] = BIT_RIGHT;
    break;
  }
}
/*
 * Function:  entity_die
 * --------------------
 * Fija una entidad en agonizante
 *
 */
void entity_die(unsigned char e) {
  zx_border(INK_RED);
  if (e) {
    // Enemigo
    types[e] = ENT_DIE;
    btile_draw_halt(tiles[entity] + 3, *lin, *col - scroll_min);
    timers[e] = time;
    values[e] = 3;
    ++game_score;
  } else {
    // Player
  }
}
/*
 * Function:  move_cup
 * --------------------
 * Comprueba si se puede mover a arriba
 *
 */
unsigned char move_cup() {
  // Puede ir hacia arriba?
  return map_empty(*lin - 8, *col);
}
/*
 * Function:  move_cdown
 * --------------------
 * Comprueba si se puede mover a abajo
 *
 */
unsigned char move_cdown() {
  // Puede ir a la abajo?
  return map_empty(*lin + 16, *col);
}
/*
 * Function:  move_cright
 * --------------------
 * Se puede mover a la derecha?
 *
 */
unsigned char move_cright() {
  // Puede ir a la derecha
  return map_empty(*lin, *col + 2);
}
/*
 * Function:  move_cleft
 * --------------------
 * Se puede mover a la izquierda!?
 *
 */
unsigned char move_cleft() {
  // Puede ir a la izquierda
  return map_empty(*lin, *col - 1);
}
/*
 * Function:  move_right
 * --------------------
 * Mueve la entidad a la derecha
 *
 */
void move_right() {
  if ((*lin % 16 == 0) && move_cright()) {
    ++(*col);
  }
}

/*
 * Function:  move_left
 * --------------------
 * Mueve la entidad a la izquierda
 *
 */
void move_left() {
  if ((*lin % 16 == 0) && move_cleft()) {
    --(*col);
  }
}

/*
 * Function:  move_up
 * --------------------
 * Mueve entidad arriba
 *
 */
void move_up() {
  if (move_cup()) {
    if (*col & 1) {
      return;
    }
    *lin -= 8;
    if (*lin > 192) {
      *lin = MAX_LIN;
    }
  }
}

/*
 * Function:  move_down
 * --------------------
 * Mueve entidad abajo
 *
 */
void move_down() {
  if (move_cdown()) {
    if (*col & 1) {
      return;
    }
    *lin += 8;
    if (*lin > MAX_LIN) {
      *lin = MIN_LIN; // 256 - 8 pseudo negativo
    }
  }
}
/*
 * Function:  map_create
 * --------------------
 * Crea el mapa
 *
 */
void map_create() {
  unsigned int i;
  col0 = 0;
  lin0 = 0;
  while (lin0 < 11) {
    col0 = 0;
    while (col0 < 16 * 3) {
      // Clear
      screen[i] = BLOCK_EMPTY;

      // Bordes
      if (lin0 == 0 || lin0 == 10 || col0 == 0 || col0 == MAP_WIDTH - 2) {
        screen[i] = BLOCK_SOLID;
      }
      // Cuadricula
      if (((col0 & 1) == 0) && ((lin0 & 1) == 0)) {
        screen[i] = BLOCK_SOLID;
      }
      // Ladrillos al Azar
      if (rand() & 0b00000001 && screen[i] == BLOCK_EMPTY) {
        //      if (rand() & 0b00000001 && screen[i] == BLOCK_EMPTY && lin0 > 2)
        screen[i] = BLOCK_BRICK;
      }

      // Fin derecho del mapa para simetría
      if (col0 == MAP_WIDTH - 1) {
        screen[i] = BLOCK_OUT;
      }

      ++i;
      ++col0;
    }
    ++lin0;
  }
  // Despeja Area del Player
  screen[MAP_WIDTH + 1] = 0;
  screen[MAP_WIDTH + 2] = 0;
  screen[2 * MAP_WIDTH + 1] = 0;

  // Posiciona Entidades
  entity = 1;
  while (entity < ENTITIES) {
    col0 = rand() % (16 * 3);
    lin0 = rand() % 11;
    col0 = (col0 >> 1) << 1;
    lin0 = lin0 << 4;
    if (map_get(lin0, col0) == BLOCK_EMPTY) {
      cols[entity] = col0;
      lins[entity] = lin0;
      ++entity;
    }
  }

  // // Marco bordes para debug
  // screen[0] = 3;
  // screen[MAP_WIDTH - 1] = 3;
  // screen[MAP_WIDTH * (MAP_HEIGHT - 1)] = 3;
  // screen[MAP_WIDTH * MAP_HEIGHT - 1] = 3;
}
/*
 * Function:  map_scroll
 * --------------------
 * Calcula la siguiente habitación según la dirección, descomprime y dibuja
 *
 */
void map_scroll(unsigned char d) {

  switch (d) {
  case BIT_RIGHT:
    scroll_min += 16;
    break;
  case BIT_LEFT:
    scroll_min -= 16;
    break;
  }
  // re dibuja la pantalla considerando el scroll
  intrinsic_di();
  memset((void *)(22528 + 32), PAPER_GREEN | INK_GREEN, (768 - 64));
  map_draw();
  intrinsic_ei();
  scroll_max = scroll_min + 32;
}

/*
 * Function:  map_calc
 * --------------------
 * calcula un indice del mapa a partir de las coordenadas l,c , la columna es
 * relativa al mapa no la pantalla.
 */
unsigned int map_calc(unsigned char l, unsigned char c) {
  return ((l - 16 >> 4) * MAP_WIDTH) + (c >> 1);
}

/*
 * Function:  map_cexplode
 * --------------------
 * Retorna si un bloque en l, c puede explotarse
 */
unsigned char map_cexplode(unsigned char l, unsigned char c) {
  unsigned char v;
  v = map_get(l, c);
  if (v == BLOCK_EMPTY || v == BLOCK_BRICK || v == BLOCK_BOMB) {
    return 1;
  } else {
    return 0;
  }
}
/*
 * Function:  map_get
 * --------------------
 * Retorna el bloque en base a una linea y columna de la pantalla
 * Retorna T_ER si esta fuera del mapa
 */
unsigned char map_get(unsigned char l, unsigned char c) {
  unsigned int i;
  i = map_calc(l, c);
  if (i < MAP_SIZE) {
    return screen[i];
  } else {
    return T_ER;
  }
}

void map_set(unsigned char v, unsigned char l, unsigned char c) {
  screen[map_calc(l, c)] = v;
}

/*
 * Function:  map_empty
 * --------------------
 * Retorna si la coordenada permite desplazamiento
 */
unsigned char map_empty(unsigned char l, unsigned char c) {
  unsigned char v = map_get(l, c);
  if (v == BLOCK_EMPTY) { // || v == BLOCK_BRICK) {
    return 1;
  } else {
    return 0;
  }
}
/*
 * Function:  map_restore
 * --------------------
 * Restaura dibujo de un casillero del mapa, según las coordenadas
 */
void map_restore(unsigned char l, unsigned char c) {
  btile_draw(screen[map_calc(l, c)], l, c - scroll_min);
}

/*
 * Function:  move_stair_lin
 * --------------------
 * Chequea la linea para permitir si un sprite 32x16 se existe escalera
 * linea l
 *
 */
unsigned char move_stair_lin(unsigned char l) {
  if (map_get(l, *col) == T_PL) {
    return 1;
  }
  return 0;
}

/*
 * Function:  move_bloc_col
 * --------------------
 * Chequea la columna para permitir si un sprite 32x16 se puede posicionar en la
 * columna c
 *
 */
unsigned char move_bloc_col(unsigned char c) {

  if (c > 31) {
    // Permite loopear horizontalmente en la pantalla
    return 0;
  }

  if (*lin & 15) {
    // lin0 % 16 bitwise
    // Linea no modulo 16, debo chequear 3 bloques
    if (map_get(*lin + 00, c) == T_SO || //
        map_get(*lin + 16, c) == T_SO || //
        map_get(*lin + 32, c) == T_SO    //
    ) {
      return 1;
    }
  } else {
    // Linea modulo 16, debo chequear solo 2 bloques
    if (map_get(*lin, c) == T_SO || map_get(*lin + 16, c) == T_SO) {
      return 1;
    }
  }
  return 0;
}

/*
 * Function:  im2
 * --------------------
 * Rutina enlazada a im2 Nirvana
 *
 */
void im2() {
  // Aumenta "Reloj"
  ++time;
  // Contador de Segundos
  ++time_sec;
  if (time_sec == 50) {
    --game_time;
    time_sec = 0;
  }
  // Animación entidades
  if (im2_free && !im2_pause) {
    entity_anim();
  }
}

/*
 * Function:  bomb_init
 * --------------------
 * Inicializa las bombas
 *
 */
void bomb_init() {
  unsigned char b;
  b = 0;
  while (b < MAX_BOMBS) {
    bomb_frame[b] = BOMB_OFF;
    ++b;
  }
}

/*
 * Function:  bomb_get
 * --------------------
 * Busca la próxima bomba disponible, o retorna MAX_BOMBS si no hay disponibles
 *
 */
unsigned char bomb_get() {
  unsigned char b;
  b = 0;
  while (b < MAX_BOMBS) {
    if (bomb_frame[b] == BOMB_OFF) {
      return b;
    }
    ++b;
  }
  return MAX_BOMBS;
}

/*
 * Function:  bomb_add
 * --------------------
 * Agrega (Si es posible) a los arreglos de bombas
 *
 */
void bomb_add() {
  unsigned char l;
  unsigned char c;
  unsigned char b;
  c = (*col >> 1) << 1;
  l = (*lin >> 4) << 4;
  if (map_get(l, c) == BLOCK_EMPTY) {
    // Busco Siguiente slot de bomba disponible
    b = bomb_get();
    // Seteo la bomba
    if (b < MAX_BOMBS) {
      map_set(BLOCK_BOMB, l, c);
      // player_sound = SFX_BOMB_ADD;
      bomb_lin[b] = l;
      bomb_col[b] = c;
      bomb_timer[b] = time + 200;
      bomb_frame[b] = BOMB_FRAMES;
    }
  }
}

/*
 * Function:  bomb_anim
 * --------------------
 * Animación de las bombas
 *
 */
void bomb_anim() {
  unsigned char b;
  b = 0;
  while (b < MAX_BOMBS) {
    if (bomb_frame[b] < BOMB_OFF) {
      // Animación Bomba
      sprite_draw(b, BTILE_BOMB + (bomb_frame[b] % 3), bomb_lin[b],
                  bomb_col[b] - scroll_min);

      --bomb_frame[b];
    }
    ++b;
  }
}

/*
 * Function:  bomb_explode
 * --------------------
 * Explota una bomba
 *
 */
void explode_anim(unsigned char b) {
  unsigned char t;

  switch (bomb_frame[b]) {
  case BOMB_EXPLODE:

    // Limpia sprite Nirvana de la bomba
    NIRVANAP_spriteT(b, 0, 0, 0);

    // Parámetros iniciales de Explosión
    explo_down[b] = bomb_lin[b];
    explo_up[b] = bomb_lin[b];
    explo_left[b] = bomb_col[b];
    explo_right[b] = bomb_col[b];
    // Calcula margenes

    // Margen Abajo Tope 160
    t = bomb_lin[b] + (player_radius << 4); //(player_radius * 16);
    if (t > 160) {
      t = 160;
    }
    while (explo_down[b] < t && //
           map_cexplode(explo_down[b] + 16, bomb_col[b])) {
      explo_down[b] += 16;
    }

    // Margen Arriba Tope 8
    t = bomb_lin[b] - (player_radius << 4); //(player_radius * 16);
    if (t > 160) {
      // Overflow
      t = 8;
    }
    while (explo_up[b] > t && //
           map_cexplode(explo_up[b] - 16, bomb_col[b])) {
      explo_up[b] -= 16;
    }

    // Margen Izquierda Tope 2
    t = bomb_col[b] - (player_radius << 1); //(player_radius * 2);
    if (t > 96) {                           // MAP_WIDTH*2
      // Overflow
      t = 2;
    }
    while (explo_left[b] > t && //
           map_cexplode(bomb_lin[b], explo_left[b] - 2)) {
      explo_left[b] -= 2;
    }

    // Margen Derecha Tope ??
    t = bomb_col[b] + (player_radius << 1); //(player_radius * 2);
    // if (t > 30) {
    //   // Overflow
    //   t = 30;
    // }
    while (explo_right[b] < t && //
           map_cexplode(bomb_lin[b], explo_right[b] + 2)) {
      explo_right[b] += 2;
    }
    // Fijo Siguiente Estado
    bomb_frame[b] = BOMB_EXPLODE1;
    // Mata
    explode_kill(b);
    // Dibuja
    explode_draw(b, 0);
    // Explota Bordes
    explode_edges(b);
    NIRVANAP_halt();
    beepSteve(SFX_BOMB_EXPLO);
    explode_paint(b, attrs_fire_yellow);
    explode_paint(b, attrs_fire_red);
    break;
  case BOMB_EXPLODE1:
    // Fijo Siguiente Estado
    bomb_frame[b] = BOMB_EXPLODE2;
    // Mata
    explode_kill(b);
    // Dibuja
    explode_draw(b, 8);
    // Explota Bordes
    explode_edges(b);
    break;
  case BOMB_EXPLODE2:
    // Fijo Siguiente Estado
    bomb_frame[b] = BOMB_EXPLODE3;
    // Mata
    explode_kill(b);
    // Dibuja
    explode_draw(b, 16);
    // Explota Bordes
    explode_edges(b);
    // Siguiente Animación
    break;
  case BOMB_EXPLODE3:
    // Fijo Siguiente Estado (No mover)
    bomb_frame[b] = BOMB_OFF;
    // Restauro
    explode_paint(b, attrs_back);
    explode_edges(b);
    // Limpia Mapa para no confundir a la función explode
    map_set(BLOCK_EMPTY, bomb_lin[b], bomb_col[b]);
    // Limpia Pantalla
    NIRVANAP_halt();
    // Desactiva Bomba
    bomb_frame[b] = BOMB_OFF;
    bomb_col[b] = BOMB_OFF;
    bomb_lin[b] = BOMB_OFF;
    break;
  default:
    break;
  }
}

void explode_kill(unsigned char b) {
  unsigned char e = 0;
  // Todos
  while (e < ENTITIES) {
    if (types[e] < ENT_ACTIVE) {
      if ((explo_left[b] <= cols[e]) &&     //
          (explo_right[b] >= cols[e]) &&    //
          (abs(bomb_lin[b] - lins[e]) < 16) //
      ) {
        entity_die(e);
      }
      if ((explo_up[b] <= lins[e]) &&      //
          (explo_down[b] >= lins[e]) &&    //
          (abs(bomb_col[b] - cols[e]) < 2) //
      ) {
        entity_die(e);
      }
    }
    ++e;
  }
}
void explode_edges(unsigned char b) {
  if ((bomb_lin[b] - (player_radius << 4)) <= explo_up[b]) {
    explode_cell(b, explo_up[b], bomb_col[b]);
  }
  if ((bomb_lin[b] + (player_radius << 4)) >= explo_down[b]) {
    explode_cell(b, explo_down[b], bomb_col[b]);
  }

  if ((bomb_col[b] + (player_radius << 1)) <= explo_left[b]) {
    explode_cell(b, bomb_lin[b], explo_left[b]);
  }

  if ((bomb_col[b] + (player_radius << 1)) >= explo_right[b]) {
    explode_cell(b, bomb_lin[b], explo_right[b]);
  }
}

void explode_cell(unsigned char b, unsigned char l, unsigned char c) {
  switch (screen[map_calc(l, c)]) {
  case BLOCK_BRICK:
    // Explota los ladrillos
    if (bomb_frame[b] >= BOMB_EXPLODE3) {
      btile_draw_halt(                                       //
          BTILE_BRICK_EXP + (BOMB_EXPLODE1 - bomb_frame[b]), // Tile
          l,                                                 // Linea
          c - scroll_min                                     // Columna
      );
    } else {
      btile_draw_halt(BTILE_EMPTY, l, c - scroll_min);
      map_set(BLOCK_EMPTY, l, c);
    }
    break;
  case BLOCK_BOMB:
    // Activa otras bombas
    bomb_activate(l, c);
    break;
  default:
    break;
  }
}

void explode_draw(unsigned char b, unsigned char p) {
  unsigned char i;
  unsigned char j;
  im2_pause = 1;
  // Vertical
  i = explo_down[b];
  j = 0;
  NIRVANAP_halt();
  // Centro Explosión
  btile_draw(BTILE_EXPLO + p, bomb_lin[b], bomb_col[b] - scroll_min);
  if (i == bomb_lin[b]) {
    i -= 16;
  }
  // Explosión Vertical
  while (i >= explo_up[b] &&
         map_get(i, bomb_col[b] - scroll_min) == BLOCK_EMPTY) {
    if (!(j & 3)) {
      NIRVANAP_halt();
    }
    ++j;
    btile_draw(BTILE_EXPLO + p + 1, i, bomb_col[b] - scroll_min);
    i -= 16;
    if (i == bomb_lin[b]) {
      i -= 16;
    }
  }
  // Explosión Horizontal
  i = explo_left[b];
  if (i == bomb_col[b]) {
    i += 2;
  }
  while (i <= explo_right[b] &&
         map_get(bomb_lin[b], i - scroll_min) == BLOCK_EMPTY) {
    if (!(j & 3)) {
      NIRVANAP_halt();
    }
    ++j;
    btile_draw(BTILE_EXPLO + p + 2, bomb_lin[b], i - scroll_min);
    i += 2;
    if (i == bomb_col[b]) {
      i += 2;
    }
  }
  if (p == 0) {
    // Bordes de explosion solo para el primer dibujado
    // Derecha
    if (map_get(bomb_lin[b], explo_right[b] + 1 - scroll_min) == BLOCK_EMPTY) {
      btile_half_h(0, BTILE_END_EXP, bomb_lin[b],
                   explo_right[b] + 1 - scroll_min);
    }
    // Izquierda
    if (map_get(bomb_lin[b], explo_left[b] - 1 - scroll_min) == BLOCK_EMPTY) {
      btile_half_h(1, BTILE_END_EXP, bomb_lin[b],
                   explo_left[b] - 1 - scroll_min);
    }
    // Arriba
    if (map_get(explo_up[b], bomb_col[b] - scroll_min) == BLOCK_EMPTY) {

      btile_half_v(0, BTILE_END_EXP, explo_up[b], bomb_col[b] - scroll_min);
    }
    // Abajo
    if (map_get(explo_down[b] + 8, bomb_col[b] - scroll_min) == BLOCK_EMPTY) {
      btile_half_v(1, BTILE_END_EXP, explo_down[b] + 8,
                   bomb_col[b] - scroll_min);
    }
  }
  im2_pause = 0;
}

void explode_paint(unsigned char b, unsigned char *a) {
  unsigned char i;

  im2_pause = 1;
  // Vertical
  i = explo_down[b];
  NIRVANAP_halt();
  while (i >= explo_up[b]) {
    btile_paint(a, i, bomb_col[b] - scroll_min);
    i -= 16;
  }
  // Horizontal
  i = explo_left[b];

  NIRVANAP_halt();
  while (i <= explo_right[b]) {
    btile_paint(a, bomb_lin[b], i - scroll_min);
    i += 2;
  }
  im2_pause = 0;
}

/*
 * Function:  explode_check
 * --------------------
 * Explota las bombas, llamado desde bucle principal
 *
 */
void explode_check() {
  unsigned char b;
  in_explo = 0;
  b = 0;
  while (b < MAX_BOMBS) {
    if (bomb_frame[b] > BOMB_OFF) {
      explode_anim(b);
      in_explo = 1;
    }
    ++b;
  }
}

void bomb_activate(unsigned char l, unsigned char c) {
  unsigned char b;
  b = 0;
  while (b < MAX_BOMBS) {
    if (bomb_frame[b] < BOMB_OFF && bomb_lin[b] == l && bomb_col[b] == c) {
      bomb_frame[b] = BOMB_EXPLODE;
      explode_anim(b);
    }
    ++b;
  }
}
