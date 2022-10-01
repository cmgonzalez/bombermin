/*
   This file is part of Prisma Engine v2.0.

   Chimuelo Engine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Chimuelo Engine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with PRISMA ENGINE.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <arch/zx.h>
#include <arch/zx/nirvana+.h>
#include <input.h>
#include <stdio.h>
#include <stdlib.h>
#include <z80.h>

// Pragmas
#pragma printf = "%c %u %i"

// Definiciones
#define MAX_DRAW 16

// Macros
#define printInk(k) printf("\x10%c", '0' + (k))
#define printPaper(k) printf("\x11%c", '0' + (k))
#define printAt(r, c) printf("\x16%c%c", (c) + 1, (r) + 1)
#define bitCheck(a, b) (unsigned char)(a & b)
#define bitSet(a, b) (a |= b)
#define bitClear(a, b) (a &= ~b)

// Variables

void draw_rst_draw();
void draw_rst_add(unsigned char, unsigned char, unsigned char);
void draw_rst_init();

// Definiciones de Funciones
void print_udg(unsigned char, unsigned char, unsigned char);
void print_char(unsigned char ch, unsigned char r, unsigned char c);
void print_int(unsigned int, unsigned char, unsigned char);
void print_udg_nirv(unsigned char, unsigned char, unsigned char);
void btile_draw(unsigned char, unsigned char, unsigned char);
void btile_paint(unsigned char *a, unsigned char l, unsigned char c);

void btile_draw_halt(unsigned char, unsigned char, unsigned char);
void sprite_draw(unsigned char s, unsigned char t, unsigned char l,
                 unsigned char c);
void btile_half(unsigned char, unsigned char);
void btile_half_h(unsigned char h, unsigned char bt, unsigned char l,
                  unsigned char c);
void btile_half_v(unsigned char h, unsigned char bt, unsigned char l,
                  unsigned char c);
// Definiciones de Variables
extern unsigned char nirv_attribs[];

// btiles definidos en assets.asm
extern unsigned char btiles[];
// blocks definidos en assets.asm
extern unsigned char blocks[];

extern unsigned char rst_i;
