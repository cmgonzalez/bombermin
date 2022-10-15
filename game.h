#include "include/prisma.h"
#include "include/steeve.h"
#include <compress/zx7.h>
#include <string.h> // for memset()

//------------------------------------------------------------
// Pragmas
//------------------------------------------------------------

// No crear heap - do not create a heap
#pragma output CLIB_MALLOC_HEAP_SIZE = 0
// No cambiar sp - do not change sp
#pragma output REGISTER_SP = -1

uint16_t (*joyfunc1)(udk_t *);
udk_t k1;

//------------------------------------------------------------
//------------------------------------------------------------
// Definiciones
//------------------------------------------------------------
//------------------------------------------------------------
// Cantidad de Entidades
#define ENTITIES 8
// Máxima linea nirvana
#define MAX_LIN 168

// Mínima linea nirvana pseudo negativa 256 - 8
#define MIN_LIN 248

// Mapa de Pantalla 16 columnas por 11 filas, para btiles de 16x16 pixeles
#define MAP_WIDTH (16 * 3)
#define MAP_HEIGHT 11
#define MAP_SIZE MAP_WIDTH *MAP_HEIGHT

// Cuadros de Animación
#define FRAMES 3

#define ON 1
#define OFF 0

#define MAX_BOMBS 8

#define BLOCK_EMPTY 0
#define BLOCK_SOLID 1
#define BLOCK_WALL 2
#define BLOCK_OUT 3
#define BLOCK_BOMB 27

// Bombas
#define BOMB_ANIM_TIME 5
#define BOMB_FRAMES 10
#define BOMB_OFF 128
#define BOMB_EXPLODE3 252
#define BOMB_EXPLODE2 253
#define BOMB_EXPLODE1 254
#define BOMB_INITIAL 255

// Btiles asociados a las entidades
#define BTILE_EMPTY 0
#define BTILE_EXPLO 24
#define BTILE_BOMB 29
#define BTILE_WALL_EXP 37
#define BTILE_END_EXP 5

#define BTILE_PLAYER_HOR 8
#define BTILE_PLAYER_VER 16

#define BTILE_BALLON 44
#define BTILE_BEAKER 44 + 4
#define BTILE_LANTERN 44 + 8
#define BTILE_FACE 44 + 12
#define BTILE_JELLY 44 + 16
#define BTILE_GHOST 44 + 20
#define BTILE_BEAR 44 + 24
#define BTILE_COIN 44 + 28
#define BTILE_DIE 44 + 32

// definición de bits para flags0 (bits)
#define BIT_UP 1
#define BIT_DOWN 2
#define BIT_LEFT 4
#define BIT_RIGHT 8
#define BIT_ATTACK 16
#define BIT_BLOCK 32
#define BIT_BWALK 64
#define BIT_DISABLE 128

// definición de bits para flags1 (bits)
#define BIT_DUCK 1
#define BIT_CLIMB 2
#define BIT_JUMP 4
#define BIT_FALL 8
#define BIT_CANUP 16
#define BIT_CANDOWN 32
#define BIT_LOCK 64
#define BIT_DEAD 128

// sfx
#define SFX_BOMB_ADD 10
#define SFX_BOMB_EXPLO 12
#define SFX_BOMB_STEP 14
#define SFX_BAD_STEP 2

// Entidades
#define ENT_PLAYER 0
#define ENT_BALLON 1 // Valcom (Balloon)
#define ENT_BEAKER 2 // O'Neal (Beaker)
#define ENT_LANTERN 3 // Dahl (Lantern)
#define ENT_FACE 4 // Minvo (Face)
#define ENT_JELLY 5 // Ovape (Jelly)
#define ENT_GHOST 6 // Doria (Ghost)
#define ENT_BEAR 7 // Pass (Bear)
#define ENT_COIN 8 // Pontan(Coin)
#define ENT_ACTIVE 128

// Powerups
#define POW_FIRE 1 // 01 Fire
#define POW_BOMB 2 // 02 Bomb
#define POW_DETONATOR 3 // 03 Detonator
#define POW_SPEEDUP 4 // 04 Speedup
#define POW_BOMBWALK 5 // 05 Bomb-Walk
#define POW_WALLWALK 6 // 06 Wall-Walk
#define POW_MYSTERY 7 // 07 Mystery
#define POW_FIREPROOF 7 // 08 Fire-Proof

// Player
#define PLAYER_LIVES 3
#define PLAYER_TIME 200
#define PLAYER_FAST 3
#define PLAYER_NORMAL 7

#define ENT_EXPLODING 253
#define ENT_DIE 254
#define ENT_OFF 255

// Tipos de Bloques del mapa, 48 bloques max
#define T_BG 0 // Fondos
#define T_FG 1 // 1er Plano usa sprites nirvana z-index superior
#define T_PL 2 // Plataformas/Escaleras
#define T_SO 3 // Sólidos
#define T_ER 255 // Fuera de Pantalla

//------------------------------------------------------------
//------------------------------------------------------------
// Variables
//------------------------------------------------------------
//------------------------------------------------------------

// Uso general
unsigned int time;
unsigned char time_sec;
unsigned char *addr;
unsigned char foo;
unsigned char fanim;
unsigned char entity;

unsigned char entity_im2 = 0;

// Ubicación inicial de la entidad
unsigned char dir0;
unsigned char col0;
unsigned char lin0;

// Punteros para asignar a al entidad actual del bucle principal
unsigned char *frame;
unsigned char *dir;
unsigned char *col;
unsigned char *lin;

// Variables de las Entidades
unsigned char types[ENTITIES];
unsigned char dirs[ENTITIES];
unsigned char lins[ENTITIES];
unsigned char cols[ENTITIES];

unsigned int timers[ENTITIES];

unsigned char tiles[ENTITIES];

unsigned char frames[ENTITIES];
unsigned char speeds[ENTITIES];
unsigned char seeds[ENTITIES];
unsigned char values[ENTITIES];

// Mapa de Pantalla
unsigned char screen[MAP_SIZE];
unsigned char scroll_min;
unsigned char scroll_max;

// Arreglos para Atributos Nirvanisticos
unsigned char att[4];
unsigned char arr_back[4] = {
    PAPER_GREEN | INK_GREEN,
    PAPER_GREEN | INK_GREEN,
    PAPER_GREEN | INK_GREEN,
    PAPER_GREEN | INK_GREEN,
};
unsigned char arr_fire_red[4] = {
    PAPER_YELLOW | INK_RED,
    PAPER_YELLOW | INK_RED,
    PAPER_YELLOW | INK_RED,
    PAPER_YELLOW | INK_RED,
};
unsigned char arr_fire_yellow[4] = {
    PAPER_RED | INK_YELLOW,
    PAPER_RED | INK_YELLOW,
    PAPER_RED | INK_YELLOW,
    PAPER_RED | INK_YELLOW,
};

unsigned char arr_wall[4] = {
    PAPER_WHITE | INK_BLACK,
    PAPER_WHITE | INK_BLACK | BRIGHT,
    PAPER_WHITE | INK_BLACK | BRIGHT,
    PAPER_WHITE | INK_BLACK,
};

// Punteros para Atributos Nirvanisticos
unsigned char *attrs;
unsigned char *attrs_back = &arr_back[0];
unsigned char *attrs_fire_red = &arr_fire_red[0];
unsigned char *attrs_fire_yellow = &arr_fire_yellow[0];
unsigned char *attrs_wall = &arr_wall[0];

// Bombas
unsigned char bomb_col[MAX_BOMBS];
unsigned char bomb_lin[MAX_BOMBS];
unsigned char bomb_frame[MAX_BOMBS];
unsigned int bomb_timer[MAX_BOMBS];
unsigned int bomb_mode[MAX_BOMBS];
unsigned char bomb;
unsigned int bombs_time;

// Explosiones
unsigned char explo_up[MAX_BOMBS];
unsigned char explo_down[MAX_BOMBS];
unsigned char explo_left[MAX_BOMBS];
unsigned char explo_right[MAX_BOMBS];

unsigned char explo_max_up[MAX_BOMBS];
unsigned char explo_max_down[MAX_BOMBS];
unsigned char explo_max_left[MAX_BOMBS];
unsigned char explo_max_right[MAX_BOMBS];

unsigned char explo_trigger[MAX_BOMBS];

// Valores del Player
unsigned char player_lives;
unsigned char player_radius;
unsigned char player_bombs;
unsigned char player_bombwalk;
unsigned char player_wallwalk;
unsigned char player_mystery;
unsigned char player_fireproof;
unsigned char player_speed;
unsigned char player_hit;
unsigned char player_sound;

// Valores del Juego
unsigned char game_debug;

unsigned char game_entities;
unsigned int game_score;
unsigned char game_stage;
unsigned char game_stage_print;

unsigned char game_time;
unsigned char game_wallwalk;
unsigned char game_bombwalk;

// Valores para controla el dibujado en im2
unsigned char im2_pause = 1;
unsigned char im2_free = 1;
unsigned char explo_sound;

unsigned char sfx = 1;

// Etapas
// clang-format off
// 01 Valcom (Balloon)
// 02 O'Neal (Beaker)
// 03 Dahl (Lantern)
// 04 Minvo (Face)
// 05 Ovape (Jelly)
// 06 Doria (Ghost)
// 07 Pass (Bear)
// 08 Pontan(Coin)
//Powerups
//01 Fire
//02 Bomb
//03 Detonator
//04 Speedup
//05 Bomb-Walk
//06 Wall-Walk
//07 Mystery

unsigned char stages[] = {
    //VA ON BE DA MI OV DO PA PO BO PW 
       6, 0, 0, 0, 0, 0, 0, 0, 0,01, //01 [6 Valcoms / Fire]
       3, 3, 0, 0, 0, 0, 0, 0, 0,02, //02 [3 Valcoms, 3 O'Neals / Bomb]
       2, 2, 2, 0, 0, 0, 0, 0, 0,03, //03 [2 Valcoms, 2 O'Neals, 2 Dahls / Detonator]
       0, 4, 3, 0, 0, 0, 0, 0, 0,04, //04 [1 Valcom, 1 O'Neal, 2 Dahls, 2 Minvos / Speedup]
       0, 4, 3, 0, 0, 0, 0, 0, 0,02,// 05 [4 O'Neals, 3 Dahls / Bomb]
       1, 0, 0, 0, 0, 0, 0, 0, 1,00,// BONUS STAGE [Valcoms]
       0, 2, 0, 3, 2, 0, 0, 0, 0,02,// 06 [2 O'Neals, 3 Dahls, 2 Minvos / Bomb]
       0, 2, 0, 3, 0, 2, 0, 0, 0,01,// 07 [2 O'Neals, 3 Dahls, 2 Ovapes / Fire]       
       0, 1, 0, 2, 4, 0, 0, 0, 0,03,// 08 [1 O'Neal, 2 Dahls, 4 Minvos / Detonator]
       0, 1, 0, 1, 4, 0, 1, 0, 0,05,// 09 [1 O'Neal, 1 Dahl, 4 Minvos, 1 Doria / Bomb-Walk]
       0, 1, 0, 1, 1, 1, 0, 0, 0,06,// 10 [1 O'Neal, 1 Dahl, 1 Minvo, 1 Ovape, 3 Dorias / Wall-Walker]
       0, 1, 0, 0, 0, 0, 0, 0, 1,00,// BONUS STAGE [O'Neals]
// 11 [1 O'Neal, 2 Dahls, 3 Minvos, 1 Ovape, 1 Doria / Bomb]
// 12 [1 O'Neal, 1 Dahl, 1 Minvo, 1 Ovape, 4 Dorias / Bomb]
// 13 [3 Dahls, 3 Minvos, 2 Dorias / Detonator]
// 14 [7 Ovapes, 1 Pass / Bomb-Walk]
// 15 [1 Dahl, 3 Minvos, 3 Dorias, 1 Pass / Fire]
// BONUS STAGE [Dahls]
// 16 [3 Minvos, 4 Dorias, 1 Pass / Wall-Walk]
// 17 [5 Dahls, 2 Dorias, 1 Pass / Bomb]
// 18 [3 Valcoms, 3 O'Neals, 2 Passes / Bomb-Walk]
// 19 [1 Valcom, 1 Ovape, 1 O'Neal, 3 Dahls, 2 Passes / Bomb]
// 20 [1 O'Neal, 1 Dahl, 1 Minvo, 1 Ovape, 2 Dorias, 2 Passes / Detonator]
// BONUS STAGE [Minvos]
// 21 [3 Ovapes, 4 Dorias, 2 Passes / Bomb-Walk]
// 22 [4 Dahls, 3 Minvos, 1 Doria, 1 Pass / Detonator]
// 23 [2 Dahls, 2 Minvos, 2 Ovapes, 2 Dorias, 1 Pass / Bomb]
// 24 [1 Dahl, 1 Minvo, 2 Ovapes, 4 Dorias, 1 Pass / Detonator]
// 25 [2 O'Neals, 1 Dahl, 1 Minvo, 2 Ovapes, 2 Dorias, 1 Pass / Bomb-Walk]
// BONUS STAGE [Ovapes]
// 26 [1 Valcom, 1 O'Neal, 1 Dahl, 1 Minvo, 1 Ovape, 2 Dorias, 1 Pass / Mystery]
// 27 [1 Valcom, 1 O'Neal, 1 Ovape, 5 Dorias, 1 Pass / Fire]
// 28 [1 O'Neal, 3 Dahls, 3 Minvos, 1 Doria, 1 Pass / Bomb]
// 29 [5 Ovapes, 2 Dorias, 2 Passes / Detonator]
// 30 [3 Dahls, 2 Minvos, 2 Ovapes, 1 Doria, 1 Pass / Flame-Proof]
// BONUS STAGE [Dorias]
// 31 [2 O'Neals, 2 Dahls, 2 Minvos, 2 Ovapes, 2 Dorias / Wall-Walker]
// 32 [1 O'Neal, 1 Dahl, 3 Minvos, 4 Dorias, 1 Pass / Bomb]
// 33 [2 Dahls, 2 Minvos, 1 Ovape, 3 Dorias, 2 Passes / Detonator]
// 34 [2 Dahls, 3 Minvos, 3 Dorias, 2 Passes / Mystery]
// 35 [2 Dahls, 1 Minvo, 1 Ovape, 3 Dorias, 2 Passes / Bomb-Walk]
// BONUS STAGE [Passes]
// 36 [2 Dahls, 2 Minvos, 3 Dorias, 3 Passes / Flame-Proof]
// 37 [2 Dahls, 1 Minvo, 1 Ovape, 3 Dorias, 3 Passes / Detonator]
// 38 [2 Dahls, 2 Minvos, 3 Dorias, 3 Passes / Fire]
// 39 [1 Dahl, 1 Minvo, 2 Ovapes, 2 Dorias, 4 Passes / Wall-Walker]
// BONUS STAGE [Pontans]
// 40 [1 Dahl, 2 Minvos, 3 Dorias, 4 Passes / Mystery]
// 41 [1 Dahl, 1 Minvo, 1 Ovape, 3 Dorias, 4 Passes / Detonator]
// 42 [1 Minvo, 1 Ovape, 3 Dorias, 5 Passes / Wall-Walker]
// 43 [1 Minvo, 1 Ovape, 2 Dorias, 6 Passes / Bomb-Walk]
// 44 [1 Minvo, 1 Ovape, 2 Dorias, 6 Passes / Detonator]
// BONUS STAGE [Pontans]
// 45 [2 Ovapes, 2 Dorias, 6 Passes / Mystery]
// 46 [2 Ovapes, 2 Dorias, 6 Passes / Wall-Walker]
// 47 [2 Ovapes, 2 Dorias, 6 Passes / Bomb-Walk]
// 48 [1 Ovape, 2 Dorias, 6 Passes, 1 Pontan / Detonator]
// 49 [2 Ovapes, 1 Doria, 6 Passes, 1 Pontan / Fire-Proof]
// BONUS STAGE [Pontans]
// 50 [2 Ovapes, 1 Doria, 5 Passes, 2 Pontans / Mystery]

};
// clang-format on
//------------------------------------------------------------
//------------------------------------------------------------
// Definiciones de Funciones
//------------------------------------------------------------
//------------------------------------------------------------

// Interrupciones
void main_im2();
void main_loop();

// Entidades

// Mover Entidades
void move_left();
void move_right();
void move_down();
void move_up();

unsigned char move_cleft();
unsigned char move_cright();
unsigned char move_cdown();
unsigned char move_cup();

void move_fire();

// Mapas
unsigned int map_calc(unsigned char l, unsigned char c);
unsigned char map_get(unsigned char l, unsigned char c);
void map_set(unsigned char v, unsigned char l, unsigned char c);
void map_restore(unsigned char l, unsigned char c);
unsigned char map_empty(unsigned char l, unsigned char c);
void map_scroll(unsigned char d);

// Entities
void entity_move();
void entity_move_player();
void entity_chdir();

// Juego
void game_init();

// Bombas
void bomb_init();
unsigned char bomb_get();
void bomb_add();
void bomb_anim();
void explode_anim(unsigned char b);
void bomb_activate(unsigned char d, unsigned char l, unsigned char c);

// Explosiones
void explode_calc(unsigned char b);
void explode_draw(unsigned char b, unsigned char p);
void explode_restore(unsigned char b);
void explode_edges(unsigned char b);
void explode_kill(unsigned char b);
void explode_cell(unsigned char b, unsigned char d, unsigned char l, unsigned char c);
void explode_check();

// Testing
void plot(unsigned char x, unsigned char y);

/*
************************************************************
Notas Técnicas - Zx Spectrum
************************************************************

Mapa de Memoria

      #FF58	#FFFF	168	Reserved
      #5CCB	#FF57	41,612	Free memory
      #5CC0	#5CCA	11	Reserved
      #5C00	#5CBF	192	System variables
      #5B00	#5BFF	256	Printer buffer
22528 #5800	#5AFF	768	Attributes
16384 #4000	#57FF	6,144	Pixel data
00000 #0000	#3FFF	16,384	Basic ROM


Colores
Decimal	Binario Color
0	    000	    Black
1	    001	    Blue
2	    010	    Red
3	    011	    Magenta
4	    100	    Green
5	    101	    Cyan
6	    110	    Yellow
7	    111	    White

Ejemplo
Flash	Bright	Paper	Ink	Value	    Decimal	Description
0	    1	    100	    000	01100000	96 Bright - black on green

*/