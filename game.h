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
#define MAP_WIDTH 16 * 3
#define MAP_HEIGHT 11
#define MAP_SIZE MAP_WIDTH *MAP_HEIGHT

// Cuadros de Animación
#define FRAMES 3

#define MAX_BOMBS 8

#define BLOCK_EMPTY 0
#define BLOCK_SOLID 1
#define BLOCK_BRICK 2
#define BLOCK_OUT 3
#define BLOCK_BOMB 255

// Bombas
#define BOMB_FRAMES 10
#define BOMB_OFF 128
#define BOMB_EXPLODE3 252
#define BOMB_EXPLODE2 253
#define BOMB_EXPLODE1 254
#define BOMB_EXPLODE 255

// Btiles asociados a las entidades
#define BTILE_EMPTY 0
#define BTILE_EXPLO 24
#define BTILE_BOMB 29
#define BTILE_BRICK_EXP 37

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
#define SFX_BOMB_ADD 2
#define SFX_BOMB_EXPLO 12
#define SFX_BOMB_STEP 14

// Entidades
#define ENT_PLAYER 0
#define ENT_BALLON 1
#define ENT_BEAKER 2
#define ENT_LANTERN 3
#define ENT_FACE 4
#define ENT_JELLY 5
#define ENT_GHOST 6
#define ENT_BEAR 7
#define ENT_COIN 8
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
unsigned char *addr;
unsigned char foo;
unsigned char fanim;
unsigned char entity;

// Ubicación inicial de la entidad
unsigned char dir0;
unsigned char col0;
unsigned char lin0;

// Para los sprites nirvana de primer plano
unsigned char nirv_sprite;

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

unsigned char tiles[ENTITIES];

unsigned char frames[ENTITIES];

// Mapa de Pantalla
unsigned char screen[MAP_SIZE];
unsigned char scroll_min;
unsigned char scroll_max;

// Atributos Nirvanisticos
unsigned char *attrs;
unsigned char *attrs_back;
unsigned char *attrs_fire_red;
unsigned char *attrs_fire_yellow;

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

// Bombas
unsigned char bomb_col[MAX_BOMBS];
unsigned char bomb_lin[MAX_BOMBS];
unsigned char bombf[MAX_BOMBS];
unsigned char bomb;

// Explosiones
unsigned char explo_up[MAX_BOMBS];
unsigned char explo_down[MAX_BOMBS];
unsigned char explo_left[MAX_BOMBS];
unsigned char explo_right[MAX_BOMBS];

unsigned char explo_cu[MAX_BOMBS];
unsigned char explo_cd[MAX_BOMBS];
unsigned char explo_cl[MAX_BOMBS];
unsigned char explo_cr[MAX_BOMBS];

// Valores del Player
unsigned char player_radius;
unsigned char player_bombs;
unsigned char player_ghost;
unsigned char player_hit;

unsigned char im2_pause = 1;
unsigned char im2_free = 1;
unsigned char in_explo;

unsigned char sfx = 1;

//------------------------------------------------------------
//------------------------------------------------------------
// Definiciones de Funciones
//------------------------------------------------------------
//------------------------------------------------------------

// Interrupciones
void im2();

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
void map_update();

// Entities
void entity_move_ballon();
void entity_move_player();
void entity_chdir();

// Bombas
void bomb_init();
unsigned char bomb_get();
void bomb_add();
void bomb_anim();
void bomb_explode(unsigned char b);
void bomb_activate(unsigned char l, unsigned char c);

// Explosiones
void explode_draw(unsigned char b, unsigned char p);
void explode_paint(unsigned char b, unsigned char *a);
void explode_explode(unsigned char b);
void explode_kill(unsigned char b);
void explode_cell(unsigned char b, unsigned char l, unsigned char c);
void explode_check();
void explode_restore(unsigned char b);

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