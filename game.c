/* ----------------------------------------------------------------
 * Bombermin
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
 * Rutina principal del juego
 *
 */
void main(void) {
  game_debug = 1;
  if (!game_debug) {
    in_wait_key();
    in_wait_nokey();
  }
  z80_delay_ms(25);

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

  // Configura direcciones para los btiles y blocks(UDGs)
  NIRVANAP_tiles(_btiles);
  NIRVANAP_chars(_blocks);

  // Configura los Atributos por defecto de nirvana para print_udg_nirv
  nirv_attribs[0] = nirv_attribs[1] = nirv_attribs[2] = nirv_attribs[3] = PAPER_BLACK | INK_CYAN;

  // Extrae Habitación 0

  // Enlaza la función "im2"  con la interrupción IM2 de Nirvana
  NIRVANAP_ISR_HOOK[0] = 205;
  z80_wpoke(&NIRVANAP_ISR_HOOK[1], (unsigned int)main_im2);

  // NIRVANAP_start();

  // btile_hup(BTILE_BEAKER, 16, 0);
  // btile_hdown(BTILE_BEAKER, 24, 0);

  // btile_hup(BTILE_BEAKER, 8, 2);
  // btile_hdown(BTILE_BEAKER, 16, 2);

  // btile_hup(BTILE_BEAKER, 184, 2);
  // btile_hdown(BTILE_BEAKER, 192, 2);

  // btile_draw(BTILE_BEAKER, 64, 8);
  // in_wait_key();
  // in_wait_nokey();

  // Bucle principal del Juego
  main_loop();
}

/*
 * Function:  main_loop
 * --------------------
 * Bucle principal del Juego
 *
 */
void main_loop() {
  // Inicializa la partida
  game_init();
  // Inicializa las entidades
  entity_init();
  // Crea Mapa
  map_create();
  // Inicializa Bombas
  bomb_init();
  // Imprime textos en Pantalla
  print_header();
  // Dibuja el mapa
  map_draw();
  // Imprime Footer
  print_footer();
  // Activa NIRVANA ENGINE
  NIRVANAP_start();
  // Activa rutinas im2
  im2_pause = OFF;
  for (;;) {
    // Imprime el tiempo restante de partida
    print_time();
    // Anima las Bombas TODO animación mas lenta
    bomb_anim();
    // Explota Bombas
    explode_check();
    // Colisiones con entidades
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
      print_char(sfx, 23, 0);
      beepSteve(sfx);
      ++sfx;
      if (sfx > 16) {
        sfx = 1;
      }
    }
  }
}

/*
 * Function:  main_im2
 * --------------------
 * Rutina enlazada a im2 Nirvana
 *
 */
void main_im2() {
  zx_border(INK_WHITE);
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
  zx_border(INK_WHITE);
}

/*
 * Function:  draw_entity
 * --------------------
 * Dibuja entidad, envuelve lógica para rotar entre bordes de pantalla
 * horizontalmente
 */
void draw_entity() {
  // Restaura Fondo
  draw_restore();
  // Restaura Fondo con el btile 0
  // btile_draw(0, lin0, col0 - scroll_min);
  // Tile Personaje
  btile_draw(tiles[entity] + *frame, *lin, *col - scroll_min);
  // Pintado especial si camina sobre ladrillos
  if (game_wallwalk) {
    draw_overwall();
  }
}

/*
 * Function:  draw_entity
 * --------------------
 * Dibuja entidad, envuelve lógica para rotar entre bordes de pantalla
 * horizontalmente
 */
void draw_player() {
  unsigned char t;
  // Dibuja al protagonista

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
  if (*col != col0 || *lin != lin0) {
    // Restaura Fondo
    draw_restore();
  }
  // Tile Personaje
  btile_draw(t, *lin, *col - scroll_min);
  // Pintado especial si camina sobre ladrillos
  if (game_wallwalk) {
    draw_overwall();
  }
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
  if (*col != col0) {
    // Movimiento Horizontal
    if (*col > col0) {
      // Derecha
      map_restore((lin0 >> 4) << 4, ((col0) >> 1) << 1);
    } else {
      // Izquierda
      map_restore((lin0 >> 4) << 4, ((col0 + 1) >> 1) << 1);
    }
  } else {
    // Movimiento Vertical
    if (*lin > lin0) {
      // Arriba
      map_restore(((lin0) >> 4) << 4, (col0 >> 1) << 1);
    } else {
      // Abajo
      map_restore(((lin0 + 8) >> 4) << 4, (col0 >> 1) << 1);
    }
  }
}

/*
 * Function:  draw_overwall
 * --------------------
 * Efecto de dibujo sobre ladrillos, pinta con atributos y hace las lineas de los ladrillos
 *
 */
void draw_overwall() {

  if (screen[map_calc(*lin, *col)] >= BLOCK_WALL) {
    NIRVANAP_paintC(attrs_wall, *lin, *col - scroll_min);
  }
  if (screen[map_calc(*lin, 1 + *col)] >= BLOCK_WALL) {
    NIRVANAP_paintC(attrs_wall, *lin, 1 + *col - scroll_min);
  }
  if (screen[map_calc(8 + *lin, *col)] >= BLOCK_WALL) {
    NIRVANAP_paintC(attrs_wall, *lin + 8, *col - scroll_min);
  }
  if (screen[map_calc(8 + *lin, 1 + *col)] >= BLOCK_WALL) {
    NIRVANAP_paintC(attrs_wall, *lin + 8, 1 + *col - scroll_min);
  }
}

/*
 * Function:  map_draw
 * --------------------
 * Dibuja la pantalla en base a los datos del arreglo screen
 */
void map_draw() {
  unsigned int i;
  unsigned int t;
  // Limpia Sprites Nirvana para evitar problemas
  sprite_reset();

  // Dibuja Mapa
  lin0 = 16;
  foo = 0;

  while (lin0 < 192) {
    col0 = 0;
    while (col0 < 31) {
      t = screen[i + (scroll_min >> 1)];
      if (t > 16) {
        t = BLOCK_WALL;
      }
      NIRVANAP_drawT_raw(t, lin0, col0);
      ++i;
      col0 += 2;
    }
    ++foo;
    // i = foo * (16 * 3);
    i = (foo << 5) + (foo << 4);
    lin0 += 16;
  }
}

/*
 * Function:  game_init
 * --------------------
 * Inicializa una partida
 */
void game_init() {
  // Valores iniciales de la Partida
  im2_pause = ON;
  entity = 0;
  bomb = 0;

  game_entities = 0;
  game_stage = 0;
  game_time = PLAYER_TIME;

  player_lives = PLAYER_LIVES;
  player_speed = PLAYER_NORMAL;
  if (!game_debug) {
    player_radius = 1;
    player_bombs = 1;
    player_wallwalk = OFF;
    player_bombwalk = OFF;
    player_mystery = OFF;
    player_fireproof = OFF;
  } else {
    player_radius = 4;
    player_bombs = 8;
    player_wallwalk = ON;
    player_bombwalk = ON;
    player_mystery = ON;
    player_fireproof = ON;
  }

  player_hit = OFF;
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
 * Function:  entity_add
 * --------------------
 * Agrega entidades al mapa
 *
 */
void entity_add(unsigned char t, unsigned char c) {
  unsigned char c0 = 0;
  // Posiciona Entidades
  while (c0 < c) {
    col0 = rand() % (16 * 3);
    lin0 = rand() % 11;
    col0 = (col0 >> 1) << 1;
    lin0 = lin0 << 4;
    if (map_get(lin0, col0) == BLOCK_EMPTY) {
      cols[entity] = col0;
      lins[entity] = lin0;
      types[entity] = t;
      switch (t) {
      case ENT_BALLON:
        tiles[entity] = BTILE_BALLON;
        frames[entity] = 2;
        speeds[entity] = 9;
        seeds[entity] = 9; // Tonto
        wallwakers[entity] = OFF;
        scores[entity] = 1;
        break;
      case ENT_BEAKER:
        tiles[entity] = BTILE_BEAKER;
        frames[entity] = 2;
        speeds[entity] = 5;
        seeds[entity] = 1;
        wallwakers[entity] = OFF;
        scores[entity] = 2;
        break;
      case ENT_LANTERN:
        tiles[entity] = BTILE_LANTERN;
        frames[entity] = 2;
        speeds[entity] = 3;
        seeds[entity] = 1;
        wallwakers[entity] = OFF;
        scores[entity] = 4;
        break;
      case ENT_FACE:
        tiles[entity] = BTILE_FACE;
        frames[entity] = 2;
        speeds[entity] = 2;
        seeds[entity] = 9; // Tonto
        wallwakers[entity] = OFF;
        scores[entity] = 8;
        break;
      case ENT_JELLY:
        tiles[entity] = BTILE_JELLY;
        frames[entity] = 2;
        speeds[entity] = 16;
        seeds[entity] = 9;
        wallwakers[entity] = ON;
        scores[entity] = 10;
        break;
      case ENT_GHOST:
        tiles[entity] = BTILE_GHOST;
        frames[entity] = 2;
        speeds[entity] = 5;
        seeds[entity] = 9;
        wallwakers[entity] = ON;
        scores[entity] = 20;
        break;
      case ENT_BEAR:
        tiles[entity] = BTILE_BEAR;
        frames[entity] = 2;
        speeds[entity] = 5;
        seeds[entity] = 9;
        wallwakers[entity] = OFF;
        scores[entity] = 40;
        break;
      case ENT_COIN:
        tiles[entity] = BTILE_COIN;
        frames[entity] = 2;
        speeds[entity] = 5;
        seeds[entity] = 9;
        wallwakers[entity] = ON;
        scores[entity] = 80;
        break;
      default:
        break;
      }

      ++entity;
      ++c0;
    }
  }
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
      // Jugador
      game_wallwalk = player_wallwalk;
      game_bombwalk = player_bombwalk;
      entity_move_player();
      game_wallwalk = 0;
      game_bombwalk = 0;
      break;
    case ENT_EXPLODING:
      // Entidad Explotando
      // Determina si esta visible
      if (*col >= scroll_min && *col <= scroll_max) {
        draw_entity();
        ++*frame;
        if (*frame == 5) {
          // Elimina entidad
          types[entity] = ENT_OFF;
          map_restore(lin0, col0);
        } else {
          timers[entity] = time + 5;
        }
      } else {
        types[entity] = ENT_OFF;
      }
      break;
    case ENT_DIE:
      // Entidad Agonizando
      btile_draw(tiles[entity] + 3, *lin, *col - scroll_min);
      timers[entity] = time;
      --values[entity];
      if (values[entity] == 0) {
        types[entity] = ENT_EXPLODING;
        tiles[entity] = BTILE_DIE;
        break;
      }
    default:
      // Defecto
      game_wallwalk = wallwakers[entity];
      entity_move();
      game_wallwalk = 0;
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
    cols[entity] = 2 + (entity * 2); // (entity * 4) - 2;
    lins[entity] = 32;
    types[entity] = ENT_OFF;
    tiles[entity] = BTILE_COIN;
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
    draw_player();
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
    draw_player();
    return;
  }
  if (inp & IN_STICK_DOWN) {
    dirs[0] = BIT_DOWN;
    move_down();
    frame_inc();
    draw_player();
    return;
  }
  if (inp & IN_STICK_UP) {
    dirs[0] = BIT_UP;
    move_up();
    frame_inc();
    draw_player();
    return;
  }
  if (inp == 0) {
    *frame = 0;
    draw_player();
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
 * Function:  entity_move
 * --------------------
 * Mueve a los enemigos
 *
 */
void entity_move() {

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
  // Limpia orillas al dejar la pantalla
  // Borde Derecho

  if (*col == (scroll_min + 31)) {
    map_restore(*lin, scroll_min + 30);
    btile_hleft(tiles[entity] + *frame, *lin, 31);
    return;
  }

  if (*col == (scroll_min + 32)) {
    map_restore(lin0, scroll_min + 30);
    return;
  }
  // Borde Izquierdo
  if (*col == (scroll_min - 1)) {
    // Dibuja Mitad
    map_restore(lin0, scroll_min);
    btile_hright(tiles[entity] + *frame, *lin, 0);
    return;
  }
  if (*col == (scroll_min - 2)) {
    // Limpia
    map_restore(lin0, scroll_min);
    return;
  }
  // Determina si esta visible
  if (*col >= scroll_min && *col <= scroll_max) {
    // Visible
    frame_inc();

    // Normal
    draw_entity();
    timers[entity] = time + speeds[entity];
  }
}

/*
 * Function:  entity_chdir
 * --------------------
 * Cambia la dirección de una entidad
 *
 */
void entity_chdir() {
  unsigned char r;
  r = rand() % 10; // 0 a 9
  if (r < seeds[entity]) {
    // Vuelvo en dirección contraria
    switch (dirs[entity]) {
    case BIT_DOWN:
      dirs[entity] = BIT_UP;
      break;
    case BIT_UP:
      dirs[entity] = BIT_DOWN;
      break;
    case BIT_LEFT:
      dirs[entity] = BIT_RIGHT;
      break;
    case BIT_RIGHT:
      dirs[entity] = BIT_LEFT;
      break;
    }
  } else {
    // Cambio dirección al azar
    r = rand() % 4;
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
}

/*
 * Function:  entity_die
 * --------------------
 * Fija una entidad en agonizante
 *
 */
void entity_die(unsigned char e) {
  if (e) {
    // Enemigo
    types[e] = ENT_DIE;
    btile_draw_halt(tiles[entity] + 3, *lin, *col - scroll_min);
    timers[e] = time;
    values[e] = 3;
    game_score += scores[entity];
    print_score();
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
        // if (rand() & 0b00000001 && screen[i] == BLOCK_EMPTY && lin0 > 2) {
        screen[i] = BLOCK_WALL;
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
  // El primer enemigo empieza con el indice 1
  entity = 1;
  entity_add(ENT_BALLON, 1);
  entity_add(ENT_BEAKER, 1);
  entity_add(ENT_LANTERN, 1);
  entity_add(ENT_FACE, 1);
  entity_add(ENT_JELLY, 1);
  entity_add(ENT_GHOST, 1);
  entity_add(ENT_BEAR, 1);
  entity_add(ENT_COIN, 1);

  map_add(POW_FIRE);
  map_add(POW_BOMB);
  map_add(POW_BOMBWALK);
  map_add(POW_DETONATOR);
  map_add(POW_SPEEDUP);
  map_add(POW_MYSTERY);
  map_add(POW_FIREPROOF);
  map_add(POW_DOOR);
}

/*
 * Function:  map_add
 * --------------------
 * agrega un powerup mapa
 */
void map_add(unsigned char p) {
  unsigned char c0 = 0;
  // Posiciona Entidades
  while (c0 < 1) {
    col0 = rand() % (16 * 3);
    lin0 = rand() % 11;
    col0 = (col0 >> 1) << 1;
    lin0 = lin0 << 4;
    if (map_get(lin0, col0) == BLOCK_WALL) {
      map_set(16 + p, lin0, col0);
      ++c0;
    }
  }
}

/*
 * Function:  map_calc
 * --------------------
 * calcula un indice del mapa a partir de las coordenadas l,c , la columna es
 * relativa al mapa no la pantalla.
 */
unsigned int map_calc(unsigned char l, unsigned char c) {
  //
  // return ((l - 16 >> 4) * MAP_WIDTH) + (c >> 1);
  unsigned v = (l - 16 >> 4);
  return (v << 5) + (v << 4) + (c >> 1);
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

/*
 * Function:  map_set
 * --------------------
 * fija el valor v en la casilla (l,c) del mapa
 *
 */
void map_set(unsigned char v, unsigned char l, unsigned char c) {
  // TODO MACRO
  screen[map_calc(l, c)] = v;
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
 * Function:  map_empty
 * --------------------
 * Retorna si la coordenada permite desplazamiento
 */
unsigned char map_empty(unsigned char l, unsigned char c) {
  unsigned char v = map_get(l, c);
  if (v > 16) {
    v = BLOCK_WALL;
  }

  if ((v < BLOCK_BOMB) || (game_bombwalk && v < BLOCK_WALL) || (game_wallwalk && v < BLOCK_SOLID)) {
    return 1;
  } else {
    return 0;
  }
}
/*
 * Function:  map_empty_fast
 * --------------------
 * Retorna si la coordenada permite desplazamiento
 */
unsigned char map_empty_fast(unsigned char l, unsigned char c) {
  unsigned char v = map_get(l, c);
  if ((v == BLOCK_EMPTY)) {
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
  unsigned char t;
  // TODO MACRO
  t = screen[map_calc(l, c)];
  if (t > 16) {
    t = BLOCK_WALL;
  }

  btile_draw(t, l, c - scroll_min);
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
 * Function:  bomb_anim
 * --------------------
 * Animación de las bombas
 *
 */
void bomb_anim() {
  unsigned char b;
  if (time > bombs_time) {
    b = 0;
    // Para prevenir que Nirvana dibuje incorrectamente si le pilla una
    // interrupción
    NIRVANAP_halt();
    while (b < MAX_BOMBS) {
      // Esta la bomba activa
      if (bomb_mode[b] == BOMB_INITIAL) {
        // Animación Bomba
        sprite_draw(                    //
            b,                          //
            BTILE_BOMB + bomb_frame[b], //
            bomb_lin[b],                //
            bomb_col[b] - scroll_min    //
        );
        ++bomb_frame[b];
        if (bomb_frame[b] > 2) {
          bomb_frame[b] = 0;
        }
      }
      ++b;
    }
    bombs_time = time + BOMB_ANIM_TIME;
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
    bomb_timer[b] = 0;
    bomb_mode[b] = BOMB_OFF;
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
    if (bomb_timer[b] == 0) {
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
      bomb_frame[b] = 0;
      bomb_mode[b] = BOMB_INITIAL;
      player_sound = SFX_BOMB_ADD;
    }
  }
}

/*
 * Function:  bomb_activate
 * --------------------
 * Activa una bomba en las coordenadas (l,c) llamado desde la explosión
 * busca una bomba en esa posición y la explota.
 *
 */
void bomb_activate(unsigned char d, unsigned char l, unsigned char c) {
  unsigned char b;
  b = 0;
  while (b < MAX_BOMBS) {
    if (bomb_mode[b] == BOMB_INITIAL && bomb_lin[b] == l && bomb_col[b] == c) {
      explo_trigger[b] = d;
      bomb_timer[b] = time;
      explode_anim(b);
    }
    ++b;
  }
}

/*
 * Function:  explode_anim
 * --------------------
 * Animación de las explosiones, llamada principal
 *
 */
void explode_anim(unsigned char b) {

  switch (bomb_mode[b]) {
  case BOMB_INITIAL:

    // Limpia sprite Nirvana de la bomba
    NIRVANAP_spriteT(b, 0, 0, 0);

    // Calcula Explosión
    explode_calc(b);
    // Fijo Siguiente Estado
    bomb_mode[b] = BOMB_EXPLODE1;
    // Mata
    explode_kill(b);
    // Fijo el frame para la animación de los ladrillos
    bomb_frame[b] = 0;
    im2_pause = 1;
    // Dibuja
    explode_draw(b, 0);
    // Explota Bordes
    explode_edges(b);
    im2_pause = 0;
    break;
  case BOMB_EXPLODE1:
    if (explo_sound == 0) {
      player_sound = SFX_BOMB_EXPLO;
      explo_sound = 1;
    }
    // Fijo Siguiente Estado
    bomb_mode[b] = BOMB_EXPLODE2;
    // Mata
    explode_kill(b);
    // Fijo el frame para la animación de los ladrillos
    bomb_frame[b] = 1;
    // Dibuja
    im2_pause = 1;
    explode_draw(b, 3);
    // Explota Bordes
    explode_edges(b);
    im2_pause = 0;
    break;
  case BOMB_EXPLODE2:
    // Fijo Siguiente Estado
    bomb_mode[b] = BOMB_EXPLODE3;
    // Mata
    explode_kill(b);
    // Fijo el frame para la animación de los ladrillos
    bomb_frame[b] = 2;
    im2_pause = 1;
    // Dibuja
    explode_draw(b, 6);
    // Explota Bordes
    explode_edges(b);
    // Siguiente Animación
    im2_pause = 0;
    break;
  case BOMB_EXPLODE3:
    // Fijo Siguiente Estado
    // Restauro
    im2_pause = 1;
    explode_restore(b);
    explode_edges(b);
    im2_pause = 0;
    // Limpia Mapa para no confundir a la función explode
    map_set(BLOCK_EMPTY, bomb_lin[b], bomb_col[b]);
    // Limpia Pantalla
    // NIRVANAP_halt();
    // Desactiva Bomba
    bomb_mode[b] = BOMB_OFF;
    bomb_col[b] = 0;
    bomb_lin[b] = 0;
    explo_trigger[b] = 0;
    bomb_timer[b] = 0;
    break;
  default:
    break;
  }
}

/*
 * Function:  explode_edges
 * --------------------
 * Animación de las explosiones, llamada principal
 *
 */
void explode_edges(unsigned char b) {
  // Abajo
  if (explo_down[b] + 16 <= explo_max_down[b]) {
    explode_cell(b, BIT_UP, explo_down[b] + 16, bomb_col[b]);
  }
  // Arriba
  if (explo_up[b] - 16 >= explo_max_up[b]) {
    explode_cell(b, BIT_DOWN, explo_up[b] - 16, bomb_col[b]);
  }
  // Izquierda
  if (explo_left[b] - 2 >= explo_max_left[b]) {
    explode_cell(b, BIT_RIGHT, bomb_lin[b], explo_left[b] - 2);
  }
  // Derecha
  if (explo_right[b] + 2 <= explo_max_right[b]) {
    explode_cell(b, BIT_LEFT, bomb_lin[b], explo_right[b] + 2);
  }
}

/*
 * Function:  explode_calc
 * --------------------
 * Calcula rangos abajo arriba izquierda derecha de la explosion, ademas del
 * máximo de rango para explotar los ladrillos
 *
 */
void explode_calc(unsigned char b) {

  // Calcula margenes arriba, abajo, izquierda y derecha de la explosion
  // Parámetros iniciales de Explosión
  explo_down[b] = bomb_lin[b];
  explo_up[b] = bomb_lin[b];
  explo_left[b] = bomb_col[b];
  explo_right[b] = bomb_col[b];

  // Margen Abajo
  if (explo_trigger[b] == BIT_DOWN) {
    explo_max_down[b] = bomb_lin[b];
  } else {
    explo_max_down[b] = bomb_lin[b] + (player_radius << 4);
  }
  // t = bomb_lin[b] + (player_radius * 16);
  if (explo_max_down[b] > 160) {
    // Máximo 160
    explo_max_down[b] = 160;
  }
  while (explo_down[b] < explo_max_down[b] && map_get(explo_down[b] + 16, bomb_col[b]) == BLOCK_EMPTY) {
    explo_down[b] += 16;
  }

  // Margen Arriba
  if (explo_trigger[b] == BIT_UP) {
    explo_max_up[b] = bomb_lin[b];
  } else {
    explo_max_up[b] = bomb_lin[b] - (player_radius << 4);
  }
  // t = bomb_lin[b] - (player_radius * 16);
  if (explo_max_up[b] > 160 || explo_max_up[b] < 32) {
    // Mínimo 8
    explo_max_up[b] = 32;
  }
  while (explo_up[b] > explo_max_up[b] && map_get(explo_up[b] - 16, bomb_col[b]) == BLOCK_EMPTY) {
    explo_up[b] -= 16;
  }

  // Margen Izquierda
  if (explo_trigger[b] == BIT_LEFT) {
    explo_max_left[b] = bomb_col[b];
  } else {
    explo_max_left[b] = bomb_col[b] - (player_radius << 1);
  }
  // t = bomb_col[b] - (player_radius * 2);
  if (explo_max_left[b] > 96) { // MAP_WIDTH*2*3
    // Mínimo
    explo_max_left[b] = 0;
  }
  while (explo_left[b] > explo_max_left[b] && map_get(bomb_lin[b], explo_left[b] - 2) == BLOCK_EMPTY) {
    explo_left[b] -= 2;
  }
  // Margen Derecha
  if (explo_trigger[b] == BIT_RIGHT) {
    explo_max_right[b] = bomb_col[b];
  } else {
    explo_max_right[b] = bomb_col[b] + (player_radius << 1);
  }
  // t = bomb_col[b] + (player_radius * 2);
  if (explo_max_right[b] > 96) { // MAP_WIDTH*2*3
    // Máximo
    explo_max_right[b] = 96;
  }
  while (explo_right[b] < explo_max_right[b] && map_get(bomb_lin[b], explo_right[b] + 2) == BLOCK_EMPTY) {
    explo_right[b] += 2;
  }
}

/*
 * Function:  explode_cell
 * --------------------
 * Explota la casilla en (l,c) del mapa por la bomba b en la dirección d
 * la dirección se ocupa para cuando hay explosiones en cascada,
 * para no volver a dibujar en la dirección de la bomba que la explotó
 */
void explode_cell(unsigned char b, unsigned char d, unsigned char l, unsigned char c) {
  unsigned char t = screen[map_calc(l, c)];
  switch (t) {
  case BLOCK_WALL:
    // Explota los ladrillos
    if (bomb_mode[b] != BOMB_EXPLODE3) {
      btile_draw_halt(                    //
          BTILE_WALL_EXP + bomb_frame[b], // Tile
          l,                              // Linea
          c - scroll_min                  // Columna
      );
    } else {
      btile_draw_halt(BTILE_EMPTY, l, c - scroll_min);
      map_set(BLOCK_EMPTY, l, c);
    }
    break;
  case BLOCK_BOMB:
    // Activa otras bombas
    // btile_draw_halt(BTILE_BEAR, l, c - scroll_min);
    bomb_activate(d, l, c);
    break;
  default:
    if (t > 16) {
      map_set(t - 16, l, c);
      map_restore(l, c);
    }

    break;
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
  unsigned char c;
  b = 0;
  while (b < MAX_BOMBS) {
    if (bomb_timer[b] != 0 && time > bomb_timer[b]) {
      explo_trigger[b] = 0;
      explode_anim(b);
      ++c;
    }
    ++b;
  }
  if (c == 0) {
    explo_sound = 0;
  }
}
/*
 * Function:  explode_draw
 * --------------------
 * Dibuja la explosion de la bomba b, con la potencia p
 * se llama 3 veces por explosión
 */
void explode_draw(unsigned char b, unsigned char p) {
  unsigned char i;
  // Vertical
  i = explo_down[b];
  NIRVANAP_halt();
  // Explosión Centro
  btile_drawA(BTILE_EXPLO + p, bomb_lin[b], bomb_col[b] - scroll_min);
  // Explosión Arriba
  i = bomb_lin[b] - 16;
  while (i >= explo_up[b]) {
    btile_drawA(BTILE_EXPLO + p + 1, i, bomb_col[b] - scroll_min);
    i -= 16;
  }
  // Explosión Abajo
  i = bomb_lin[b] + 16;
  while (i <= explo_down[b]) {
    btile_drawA(BTILE_EXPLO + p + 1, i, bomb_col[b] - scroll_min);
    i += 16;
  }
  // Explosión Izquierda
  i = explo_left[b];
  if (i == bomb_col[b]) {
    i += 2;
  }
  while (i < bomb_col[b]) {
    btile_drawA(BTILE_EXPLO + p + 2, bomb_lin[b], i - scroll_min);
    i += 2;
  }
  // Explosión Derecha
  i = bomb_col[b] + 2;
  while (i <= explo_right[b]) {
    btile_drawA(BTILE_EXPLO + p + 2, bomb_lin[b], i - scroll_min);
    i += 2;
  }

  NIRVANAP_halt();
  if (p == 0) {
    // Dibuja bordes de explosion solo para el primer frame de la animacion de la explosión
    // Dibuja Abajo
    if (explo_down[b] == explo_max_down[b] && explo_down[b] != bomb_lin[b]) {
      btile_hdown(BTILE_END_EXP, explo_down[b] + 8, bomb_col[b] - scroll_min);
    }
    // Dibuja Arriba
    if (explo_up[b] == explo_max_up[b] && explo_up[b] != bomb_lin[b]) {
      btile_hup(BTILE_END_EXP, explo_up[b], bomb_col[b] - scroll_min);
    }
    // Dibuja Izquierda
    if (explo_left[b] == explo_max_left[b] && explo_left[b] != bomb_col[b]) {
      btile_hleft(BTILE_END_EXP, bomb_lin[b], explo_left[b] - scroll_min);
    }
    // Dibuja Derecha
    if (explo_right[b] == explo_max_right[b] && explo_right[b] != bomb_col[b]) {
      btile_hright(BTILE_END_EXP, bomb_lin[b], explo_right[b] + 1 - scroll_min);
    }
  }
}

/*
 * Function:  explode_kill
 * --------------------
 * Mata a entidades afectadas por la explosión
 *
 */
void explode_kill(unsigned char b) {
  unsigned char e = 0;
  // Todos
  while (e < ENTITIES) {
    if (types[e] < ENT_ACTIVE) {
      if ((explo_left[b] <= cols[e]) &&      //
          (explo_right[b] >= cols[e]) &&     //
          (abs(bomb_lin[b] - lins[e]) <= 16) //
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

/*
 * Function:  explode_restore
 * --------------------
 * Restaura la pantalla despues de una explosion, con el btile 0
 *
 */
void explode_restore(unsigned char b) {
  unsigned char i;

  // Vertical
  i = explo_down[b];
  while (i >= explo_up[b]) {
    btile_draw_halt(0, i, bomb_col[b] - scroll_min);
    i -= 16;
  }
  // Horizontal
  i = explo_left[b];

  while (i <= explo_right[b]) {
    btile_draw_halt(0, bomb_lin[b], i - scroll_min);
    i += 2;
  }
}

/*
 * Function:  print_header
 * --------------------
 * Imprime textos fijos en la parte superior de la pantalla
 *
 */
void print_header() {
  // Tiempo
  printAt(0, 1);
  printf("TIME");
  // Puntaje
  print_score();
  // Vidas
  printAt(0, 24);
  printf("LEFT");
  print_lives();
}

/*
 * Function:  print_header
 * --------------------
 * Imprime textos fijos en la parte superior de la pantalla
 *
 */
void print_footer() {
  if (!game_debug) {
    printAt(23, 9);
    printf("Bombermin Demo");
  } else {
    printAt(23, 24);
    printf("DEBUG");
  }
}
/*
 * Function:  print_time
 * --------------------
 * Imprime el tiempo restante de partida
 *
 */
void print_time() {
  printAt(0, 6);
  printf("%3i", game_time);
}

/*
 * Function:  print_score
 * --------------------
 * Imprime el puntaje del jugador
 *
 */
void print_score() {
  printAt(0, 16);
  printf("%2i00", game_score);
}

/*
 * Function:  print_lives
 * --------------------
 * Imprime las vidas del jugador
 *
 */
void print_lives() {
  printAt(0, 28);
  printf("%2i", player_lives);
}
