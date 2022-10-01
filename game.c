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
  printf("Bombermin");
  printAt(23, 14);
  printf("Demo");

  // Inicializa las entidades
  entity = 0;
  while (entity < ENTITIES) {
    dirs[entity] = BIT_RIGHT;
    cols[entity] = 2;
    lins[entity] = 32;
    frames[entity] = 2;
    timer[entity] = time + entity;
    ++entity;
  }

  // Configura direcciones para los btiles y blocks(UDGs)
  NIRVANAP_tiles(_btiles);
  NIRVANAP_chars(_blocks);

  // Configura los Atributos por defecto de nirvana para print_udg_nirv
  nirv_attribs[0] = nirv_attribs[1] = nirv_attribs[2] = nirv_attribs[3] =
      PAPER_BLACK | INK_CYAN;

  // Extrae Habitación 0
  scroll = 0;
  bomb = 0;
  map_create();
  bomb_init();
  map_draw();
  // Activa NIRVANA ENGINE
  NIRVANAP_start();
  // Enlaza la función "im2"  con la interrupción IM2 de Nirvana
  NIRVANAP_ISR_HOOK[0] = 205;
  z80_wpoke(&NIRVANAP_ISR_HOOK[1], (unsigned int)im2);

  // Atributos de fondo para pintar al limpiar
  attrs_back = &arr_back[0];
  attrs_fire_red = &arr_fire_red[0];
  attrs_fire_yellow = &arr_fire_yellow[0];

  player_radius = 4;
  // Bucle principal del Juego
  for (;;) {
    zx_border(INK_WHITE);
    // Anima Bombas
    bomb_anim();
    // Explota Bombas
    explode_check();
    // Pausa
    if (!in_explo)
      z80_delay_ms(25);
  }
}
/*
 * Function:  draw_2bt
 * --------------------
 * Dibuja 2 btiles, si c2 se dibuja el segundo,
 * Usado como función de ayuda de la restauración del mapa
 */
void draw_2bt(unsigned int i, unsigned char l, unsigned char c,
              unsigned char c2) {

  // chequeo si el indice i esta en el mapa
  if (i < MAP_SIZE) {
    draw_rst_add(screen[scroll + i + 00], l, c);
    if (c2) {
      if (c == 30) {
        draw_rst_add(screen[scroll + i - 15], l, 0);
      } else {
        draw_rst_add(screen[scroll + i + 01], l, c + 2);
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
  // Dibuja un entity
  switch (*dir) {
  case BIT_RIGHT:
    foo = PLAYER_HORIZONTAL + *frame;
    break;
  case BIT_LEFT:
    foo = PLAYER_HORIZONTAL + FRAMES + *frame;
    break;
  case BIT_DOWN:
    foo = PLAYER_VERTICAL + *frame;
    break;
  case BIT_UP:
    foo = PLAYER_VERTICAL + FRAMES + *frame;
    break;
  default:
    break;
  }
  // Restaura Fondo
  draw_restore();
  // Tile Personaje
  btile_draw(foo, *lin, *col);
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
  map_restore(lin0, col0);
}

/*
 * Function:  map_draw
 * --------------------
 * Dibuja la pantalla en base a los datos del arreglo screen
 */
void map_draw() {
  unsigned int i;
  // Limpia Sprites Nirvana
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
      NIRVANAP_drawT_raw(screen[i + scroll], lin0, col0);
      ++i;
      col0 += 2;
    }
    ++foo;
    i = foo * (16 * 3);
    lin0 += 16;
  }
}
/*
 * Function:  entity_set
 * --------------------
 * Asigna Punteros
 */
void entity_anim() {
  if ((time % 6) == 0) {
    entity = 0;
    frame = &frames[0];
    dir = &dirs[0];
    col = &cols[0];
    lin = &lins[0];
    // flag0 = &flags0[0];
    // flag1 = &flags1[0];
    dir0 = *dir;
    col0 = *col;
    lin0 = *lin;
    move_player();
  }
}
/*
 * Function:  frame_inc
 * --------------------
 * Aumenta el frames de animación, vuelve a cero al llegar al Máximo FRAMES
 */
void frame_inc() {
  ++(*frame);
  if (*frame >= FRAMES) {
    *frame = 0;
  }
}

/*
 * Function:  move_player
 * --------------------
 * Mueve al protagonista
 *
 */
void move_player() {
  unsigned inp;
  // Debug al apretar el Enter
  if (in_inkey() == 13) {
    printAt(0, 6);
    printf("L%3iC%3iF%3iF%3i", *lin, *col, *flag0, *flag1);
    z80_delay_ms(50);
  }
  // Fijo la próxima llamada en 5 frames, TODO ponerla al final...

  inp = (joyfunc1)(&k1);

  if (inp & IN_STICK_FIRE) {
    move_fire();
    return;
  }
  if (inp & IN_STICK_LEFT) {
    move_left();
    return;
  }
  if (inp & IN_STICK_RIGHT) {
    move_right();
    return;
  }
  if (inp & IN_STICK_DOWN) {
    move_down();
    return;
  }
  if (inp & IN_STICK_UP) {
    move_up();
    return;
  }
  if (inp == 0) {
    move_noinput();
    return;
  }
}

/*
 * Function:  move_enemies
 * --------------------
 * Mueve a los enemigos
 *
 */
void move_enemies() {
  unsigned char ndir;
  timer[entity] = time + 5;
  foo = rand() & 0b00000111;
  switch (foo) {
  case 0:
    // Detención
    ndir = 0;
    break;
  case 1:
    // Cambio ndir
    if (bitCheck(*flag0, BIT_RIGHT)) {
      ndir = 1;
    } else {
      ndir = 2;
    }
    break;
  default:
    // Continuar Marcha
    if (bitCheck(*flag0, BIT_RIGHT)) {
      ndir = 2;
    } else {
      ndir = 1;
    }

    break;
  }

  switch (ndir) {
  case 1:
    /* Izquierda */
    move_left();
    break;
  case 2:
    /* Derecha */
    move_right();
    break;
  default:
    /* Nada */
    *frame = 0;
    draw();
    break;
  }
}

/*
 * Function:  move_noinput
 * --------------------
 * Sin input del usuario vuelve a posición inicial
 *
 */
void move_noinput() {
  *frame = 0;
  draw();
}
/*
 * Function:  move_fire
 * --------------------
 * Pone una bomba
 *
 */
void move_fire() { bomb_add(); }

/*
 * Function:  move_right
 * --------------------
 * Mueve la entidad a la derecha
 *
 */
void move_right() {
  *dir = BIT_RIGHT;
  if ((*lin % 16 == 0) && map_empty(*lin, *col + 2)) {

    ++(*col);
    if (*col >= 24) {
      if (entity == 0) {
        // Scroll
        map_scroll(BIT_RIGHT);
      }
    }
    if (*col > 30) {
      *col = 30;
    }
  }
  frame_inc();
  draw();
}

/*
 * Function:  move_left
 * --------------------
 * Mueve la entidad a la izquierda
 *
 */
void move_left() {
  *dir = BIT_LEFT;
  if ((*lin % 16 == 0) && map_empty(*lin, *col - 1)) {

    --(*col);
    if (*col <= 8) {
      if (entity == 0) {
        // Scroll
        map_scroll(BIT_LEFT);
      }
    }
    if (*col > 30) {
      *col = 0;
    }
  }
  frame_inc();
  draw();
}

/*
 * Function:  move_up
 * --------------------
 * Mueve entidad arriba
 *
 */
void move_up() {
  *dir = BIT_UP;
  if (map_empty(*lin - 8, *col)) {
    if (*col & 1) {
      return;
    }
    *lin -= 8;
    if (*lin > 192) {
      *lin = MAX_LIN;
    }
  }
  frame_inc();
  draw();
}

/*
 * Function:  move_down
 * --------------------
 * Mueve entidad abajo
 *
 */
void move_down() {
  *dir = BIT_DOWN;

  if (map_empty(*lin + 16, *col)) {

    if (*col & 1) {
      return;
    }
    *lin += 8;
    if (*lin > MAX_LIN) {
      *lin = MIN_LIN; // 256 - 8 pseudo negativo
    }
  }
  frame_inc();
  draw();
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
      if (rand() & 0b00000001 && screen[i] == BLOCK_EMPTY && lin0 > 4) {
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
  // Limpia Area del Player
  screen[MAP_WIDTH + 1] = 0;
  screen[MAP_WIDTH + 2] = 0;
  screen[2 * MAP_WIDTH + 1] = 0;

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
    /* code */
    if (scroll < 31) {
      bomb_scroll(8);
      scroll += 8;
      *col = 8;
      map_update();
    }
    break;
  case BIT_LEFT:
    /* code */
    if (scroll > 0) {
      bomb_scroll(-8);
      scroll -= 8;
      *col = 32 - 8;
      map_update();
    }
    break;
  }
  // Descomprime y Dibuja la habitación
}

void map_update() {
  intrinsic_di();
  memset((void *)(22528 + 32), PAPER_GREEN | INK_GREEN, (768 - 64));
  map_draw();
  intrinsic_ei();
}

unsigned int map_calc(unsigned char l, unsigned char c) {
  return ((l - 16 >> 4) * MAP_WIDTH) + (c >> 1);
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
    return screen[scroll + i];
  } else {
    return T_ER;
  }
}

void map_set(unsigned char v, unsigned char l, unsigned char c) {
  screen[scroll + map_calc(l, c)] = v;
}

/*
 * Function:  map_empty
 * --------------------
 * Retorna si la coordenada permite desplazamiento
 */
unsigned char map_empty(unsigned char l, unsigned char c) {
  unsigned char v;
  v = map_get(l, c);
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
  btile_draw(screen[scroll + map_calc(l, c)], l, c);
  // unsigned char v = screen[scroll + map_calc(l, c)];
  // switch (v) {
  // case BLOCK_EMPTY:
  //   btile_paint(attrs_back, l, c);
  //   break;
  // default:
  //   btile_draw(screen[scroll + map_calc(l, c)], l, c);
  //   break;
  // }
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
  zx_border(INK_WHITE);
  ++time;
  if (!im2_pause) {
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
    bombf[b] = BOMB_OFF;
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
    if (bombf[b] == BOMB_OFF) {
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
  b = 0;
  // Chequeo si ya hay una bomba en esa posición
  while (b < MAX_BOMBS) {
    if (bomb_lin[b] == l && bomb_col[b] == c) {
      return;
    }
    ++b;
  }
  // Busco Siguiente slot de bomba disponible
  b = bomb_get();
  // Seteo la bomba
  if (b < MAX_BOMBS) {
    bomb_lin[b] = l;
    bomb_col[b] = c;
    bombf[b] = BOMB_FRAMES;
    map_set(BLOCK_BOMB, l, c);
  }
}

/*
 * Function:  bomb_anim
 * --------------------
 * Animación de las bombas llamada desde im2
 *
 */
void bomb_anim() {
  unsigned char b;
  b = 0;
  while (b < MAX_BOMBS) {
    if (bombf[b] < BOMB_OFF) {
      // Animación Bomba
      sprite_draw(b, BTILE_BOMB + (bombf[b] % 3), bomb_lin[b], bomb_col[b]);
      --bombf[b];
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
void bomb_explode(unsigned char b) {
  unsigned char v;
  switch (bombf[b]) {
  case BOMB_EXPLODE:
    /* code */
    // Limpia sprite Nirvana de la bomba
    NIRVANAP_spriteT(b, 0, 0, 0);

    // Parametros inciales de Explosión
    explo_down[b] = bomb_lin[b];
    explo_up[b] = bomb_lin[b];
    explo_left[b] = bomb_col[b];
    explo_right[b] = bomb_col[b];
    // Calcula margenes

    // Abajo Tope 160
    foo = bomb_lin[b] + (player_radius << 4); //(player_radius * 16);
    if (foo > 160) {
      foo = 160;
    }
    while (explo_down[b] < foo &&
           map_get(explo_down[b] + 16, bomb_col[b]) == BLOCK_EMPTY) {
      explo_down[b] += 16;
    }

    // Arriba Tope 8
    foo = bomb_lin[b] - (player_radius << 4); //(player_radius * 16);
    if (foo > 160) {
      // Overflow
      foo = 8;
    }

    while (explo_up[b] > foo &&
           map_get(explo_up[b] - 16, bomb_col[b]) == BLOCK_EMPTY) {
      explo_up[b] -= 16;
    }

    // Izquierda Tope 2
    foo = bomb_col[b] - (player_radius << 1); //(player_radius * 2);
    if (foo > 30) {
      // Overflow
      foo = 2;
    }
    while (explo_left[b] > foo &&
           map_get(bomb_lin[b], explo_left[b] - 2) == BLOCK_EMPTY) {
      explo_left[b] -= 2;
    }

    // Derecha Tope 30
    foo = bomb_col[b] + (player_radius << 1); //(player_radius * 2);
    if (foo > 30) {
      // Overflow
      foo = 30;
    }
    while (explo_right[b] < foo &&
           map_get(bomb_lin[b], explo_right[b] + 2) == BLOCK_EMPTY) {
      explo_right[b] += 2;
    }
    // Animación Explosion
    bombf[b] = BOMB_EXPLODE1;
    explode_draw(b, 0);
    explode_explode(b);
    explode_paint(b, attrs_fire_yellow);
    explode_paint(b, attrs_fire_red);
    break;
  case BOMB_EXPLODE1:
    bombf[b] = BOMB_EXPLODE2;
    explode_draw(b, 8);
    explode_explode(b);
    break;
  case BOMB_EXPLODE2:
    bombf[b] = BOMB_EXPLODE3;
    explode_draw(b, 16);
    explode_explode(b);
    break;
  case BOMB_EXPLODE3:
    // Restauro
    bombf[b] = BOMB_OFF;
    explode_paint(b, attrs_back);
    explode_explode(b);
    // Limpia Mapa para no confundir a la función explode
    map_set(BLOCK_EMPTY, bomb_lin[b], bomb_col[b]);
    // Limpia Pantalla
    NIRVANAP_halt();
    // btile_draw(BTILE_EMPTY, bomb_lin[b], bomb_col[b]);
    // Desactiva Bomba
    bomb_col[b] = BOMB_OFF;
    bomb_lin[b] = BOMB_OFF;
    break;
  default:
    break;
  }
}

void explode_explode(unsigned char b) {
  explode_cell(b, explo_up[b] - 16, bomb_col[b]);
  explode_cell(b, explo_down[b] + 16, bomb_col[b]);
  explode_cell(b, bomb_lin[b], explo_left[b] - 2);
  explode_cell(b, bomb_lin[b], explo_right[b] + 2);
}

void explode_cell(unsigned char b, unsigned char l, unsigned char c) {
  switch (screen[scroll + map_calc(l, c)]) {
  case BLOCK_BRICK:
    /* code */
    if (bombf[b] >= BOMB_EXPLODE3) {
      btile_draw_halt(BTILE_BRICK_EXP + (BOMB_EXPLODE1 - bombf[b]), l, c);
    } else {
      btile_draw_halt(BTILE_EMPTY, l, c);
      map_set(BLOCK_EMPTY, l, c);
    }
    break;
  case BLOCK_BOMB:
    /* code */
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
  btile_draw(BTILE_EXPLO + p, bomb_lin[b], bomb_col[b]);
  if (i == bomb_lin[b]) {
    i -= 16;
  }
  while (i >= explo_up[b]) {
    if (!(j & 3)) {
      NIRVANAP_halt();
    }
    ++j;
    btile_draw(BTILE_EXPLO + p + 1, i, bomb_col[b]);
    i -= 16;
    if (i == bomb_lin[b]) {
      i -= 16;
    }
  }
  // Horizontal
  i = explo_left[b];
  if (i == bomb_col[b]) {
    i += 2;
  }
  while (i <= explo_right[b]) {
    if (!(j & 3)) {
      NIRVANAP_halt();
    }
    ++j;
    btile_draw(BTILE_EXPLO + p + 2, bomb_lin[b], i);
    i += 2;
    if (i == bomb_col[b]) {
      i += 2;
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
    btile_paint(a, i, bomb_col[b]);
    i -= 16;
  }
  // Horizontal
  i = explo_left[b];

  NIRVANAP_halt();
  while (i <= explo_right[b]) {
    btile_paint(a, bomb_lin[b], i);
    i += 2;
  }
  im2_pause = 0;
}

/*
 * Function:  bomb_scroll
 * --------------------
 * Mueve bombas al hacer scroll
 *
 */
void bomb_scroll(signed char s) {
  unsigned char b;
  b = 0;
  while (b < MAX_BOMBS) {
    if (bombf[b] != BOMB_EXPLODE) {
      bomb_col[b] = bomb_col[b] - (s << 1); //*2
    }
    ++b;
  }
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
    if (bombf[b] > BOMB_OFF) {
      bomb_explode(b);
      in_explo = 1;
    }
    ++b;
  }
}

void bomb_activate(unsigned char l, unsigned char c) {
  unsigned char b;
  b = 0;
  while (b < MAX_BOMBS) {
    if (bombf[b] < BOMB_OFF && bomb_lin[b] == l && bomb_col[b] == c) {
      zx_border(INK_RED);
      bombf[b] = BOMB_EXPLODE;
      bomb_explode(b);
    }
    ++b;
  }
}
void explode_restore(unsigned char b) {
  unsigned char i = explo_down[b];

  while (i < explo_up[b]) {
    btile_paint(attrs_back, i, bomb_col[b]);
    i += 16;
  }
  i = explo_left[b] + 2;
  while (i < explo_right[b] - 2) {
    btile_paint(attrs_back, bomb_lin[b], i);
    i += 2;
  }
}
