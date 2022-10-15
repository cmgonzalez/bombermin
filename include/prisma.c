/*
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
/*
 Technical notes

A quick note about the "btile" format:

  Each "btile" represents a 16x16 image, using 8x2 attributes and 48 Bytes long.


  Byte 1: bitmap value for 1st pixel line, 1st column
  Byte 2: bitmap value for 1st pixel line, 2nd column
  Byte 3: bitmap value for 2nd pixel line, 1st column
  Byte 4: bitmap value for 2nd pixel line, 2nd column
  Byte 5: bitmap value for 3rd pixel line, 1st column
  Byte 6: bitmap value for 3rd pixel line, 2nd column
  Byte 31: bitmap value at 16th pixel line, 1st column
  Byte 32: bitmap value at 16th pixel line, 2nd column

  Byte 33: attribute value for 1st and 2nd pixel line, 1st column
  Byte 34: attribute value for 3rd and 4th pixel line, 1st column
  ...
  Byte 35: attribute value for 5th and 6th pixel line, 1st column
  ...
  Byte 40: attribute value for 15th and 16th pixel line, 1st column
  Byte 41: attribute value for 1st and 2nd pixel line, 2nd column
  Byte 42: attribute value for 3rd and 4th pixel line, 2nd column
  ...
  Byte 48: attribute value for 15th and 16th pixel line, 2nd column

  The ordering above looks somewhat un conventional, but there's a reason: it's
  more convenient for the NIRVANA ENGINE this way, so it can be more compact
  and efficient.


 */
#include "prisma.h"

// Variables
extern unsigned char nirv_attribs[] = {
    PAPER_BLACK | INK_WHITE,
    PAPER_BLACK | INK_WHITE,
    PAPER_BLACK | INK_WHITE,
    PAPER_BLACK | INK_WHITE,
};

unsigned char nirv_sp = 0;
unsigned char draw_sp = 0;

unsigned char *prisma_screen_map;

// Restore buff
unsigned char rst_t[MAX_DRAW];
unsigned char rst_l[MAX_DRAW];
unsigned char rst_c[MAX_DRAW];
unsigned char rst_i;

/*
 * Function:  print_udg
 * --------------------
 * Imprime un caracter o UDG en la pantalla
 *
 */
void print_udg(unsigned char ch, unsigned char r, unsigned char c) {
  if (c <= 31) {
    printAt(r, c);
    printf("%c", ch);
  }
}

/*
 * Function:  print_char
 * --------------------
 * Imprime un char en la pantalla, 3 digitos
 *
 */
void print_char(unsigned char ch, unsigned char r, unsigned char c) {
  if (c <= 31) {
    printAt(r, c);
    printf("%3i", ch);
  }
}

/*
 * Function:  print_int
 * --------------------
 * Imprime un char en la pantalla, 3 digitos
 *
 */
void print_int(unsigned int i, unsigned char r, unsigned char c) {
  if (c <= 31) {
    printAt(r, c);
    printf("%6i", i);
  }
}

/*
 * Function:  print_udg_nirv
 * --------------------
 * Imprime un caracter o UDG en la pantalla
 *
 */
void print_udg_nirv(unsigned char ch, unsigned char l, unsigned char c) {
  if (c > 31) {
    c -= 32;
  }
  if (c < 32 && l < 192) {
    NIRVANAP_printC(ch, nirv_attribs, l, c);
  }
}

/*
 * Function:  btile_hleft
 * --------------------
 * Dibuja un medio btile horizontal (8px)
 *
 */
void btile_hleft(unsigned char bt, unsigned char l, unsigned char c) {
  unsigned char *d0; // Destino
  unsigned char *d1; // Destino
  unsigned char *s;  // Origen
  if (l < 192 && c < 32) {
    // Cambia coordenada nirvana a pantalla normal
    l = l - 8;
    // btile Origen
    // s = &btiles[0] + (t * 48); => t*16 + t*8
    s = &btiles[((bt << 5) + (bt << 4))];
    // Destino pantalla optimizado para mod 8
    d0 = zx_py2saddr(l) + c;
    d1 = zx_saddrcdown(d0);

    // Atributos
    NIRVANAP_paintC(s + 40, l + 8, c);
    NIRVANAP_paintC(s + 44, l + 16, c);
    // Pixeles
    *(d0) = *(s);             // b0
    *(d0 += 256) = *(s += 2); // b1
    *(d0 += 256) = *(s += 2); // b2
    *(d0 += 256) = *(s += 2); // b3
    *(d0 += 256) = *(s += 2); // b4
    *(d0 += 256) = *(s += 2); // b5
    *(d0 += 256) = *(s += 2); // b6
    *(d0 += 256) = *(s += 2); // b7

    *(d1) = *(s += 2);        // b8
    *(d1 += 256) = *(s += 2); // b9
    *(d1 += 256) = *(s += 2); // b10
    *(d1 += 256) = *(s += 2); // b11
    *(d1 += 256) = *(s += 2); // b12
    *(d1 += 256) = *(s += 2); // b13
    *(d1 += 256) = *(s += 2); // b14
    *(d1 += 256) = *(s += 2); // b15
  }
}

/*
 * Function:  btile_hleft
 * --------------------
 * Dibuja un medio btile horizontal (8px)
 *
 */
void btile_hright(unsigned char bt, unsigned char l, unsigned char c) {
  unsigned char *d0; // Destino
  unsigned char *d1; // Destino
  unsigned char *s;  // Origen
  if (l < 192 && c < 32) {
    // Cambia coordenada nirvana a pantalla normal
    l = l - 8;
    // btile Origen
    // s = &btiles[0] + (t * 48); => t*16 + t*8
    s = &btiles[((bt << 5) + (bt << 4))];
    // Destino pantalla optimizado para mod 8
    d0 = zx_py2saddr(l) + c;
    d1 = zx_saddrcdown(d0);

    NIRVANAP_paintC(s + 32, l + 8, c);
    NIRVANAP_paintC(s + 36, l + 16, c);
    // Pixeles
    *(d0) = *(++s);           // b0
    *(d0 += 256) = *(s += 2); // b1
    *(d0 += 256) = *(s += 2); // b2
    *(d0 += 256) = *(s += 2); // b3
    *(d0 += 256) = *(s += 2); // b4
    *(d0 += 256) = *(s += 2); // b5
    *(d0 += 256) = *(s += 2); // b6
    *(d0 += 256) = *(s += 2); // b7

    *(d1) = *(s += 2);        // b8
    *(d1 += 256) = *(s += 2); // b9
    *(d1 += 256) = *(s += 2); // b10
    *(d1 += 256) = *(s += 2); // b11
    *(d1 += 256) = *(s += 2); // b12
    *(d1 += 256) = *(s += 2); // b13
    *(d1 += 256) = *(s += 2); // b14
    *(d1 += 256) = *(s += 2); // b15
  }
}
/*
 * Function:  btile_hup
 * --------------------
 * Dibuja un medio btile vertical (8px)
 *
 */
void btile_hup(unsigned char bt, unsigned char l, unsigned char c) {
  unsigned char *d; // Destino

  unsigned char *s; // Origen
  if (l >= 8 && l <= 192 && c < 31) {
    // Cambia coordenada nirvana a pantalla normal
    l = l - 8;
    // btile Origen
    // s = &btiles[0] + (t * 48); => t*16 + t*8
    s = &btiles[((bt << 5) + (bt << 4))];
    // Destino pantalla optimizado para mod 8
    d = zx_py2saddr(l) + c;

    // Atributos
    NIRVANAP_paintC(s + 32, l + 8, c);
    NIRVANAP_paintC(s + 40, l + 8, c + 1);
    // Pixeles
    *(d) = *(s);          // b0
    *(d + 1) = *(++s);    // b1
    *(d += 256) = *(++s); // b2
    *(d + 1) = *(++s);    // b3
    *(d += 256) = *(++s); // b4
    *(d + 1) = *(++s);    // b5
    *(d += 256) = *(++s); // b6
    *(d + 1) = *(++s);    // b7
    *(d += 256) = *(++s); // b8
    *(d + 1) = *(++s);    // b9
    *(d += 256) = *(++s); // b10
    *(d + 1) = *(++s);    // b11
    *(d += 256) = *(++s); // b12
    *(d + 1) = *(++s);    // b13
    *(d += 256) = *(++s); // b14
    *(d + 1) = *(++s);    // b15
  }
}
/*
 * Function:  btile_hdown
 * --------------------
 * Dibuja un medio btile vertical (8px)
 * h = 0 derecha  h = 1 izquierda
 *
 */
void btile_hdown(unsigned char bt, unsigned char l, unsigned char c) {
  unsigned char *d; // Destino

  unsigned char *s; // Origen
  if (l >= 8 && l <= 192 && c < 31) {
    // Cambia coordenada nirvana a pantalla normal
    l = l - 8;
    // btile Origen
    // s = &btiles[0] + (t * 48); => t*16 + t*8
    s = &btiles[((bt << 5) + (bt << 4))];
    // Destino pantalla optimizado para mod 8
    d = zx_py2saddr(l) + c;

    // Atributos
    NIRVANAP_paintC(s + 36, l + 8, c);
    NIRVANAP_paintC(s + 44, l + 8, c + 1);
    // Pixeles
    *(d) = *(s += 16);    // b0
    *(d + 1) = *(++s);    // b1
    *(d += 256) = *(++s); // b2
    *(d + 1) = *(++s);    // b3
    *(d += 256) = *(++s); // b4
    *(d + 1) = *(++s);    // b5
    *(d += 256) = *(++s); // b6
    *(d + 1) = *(++s);    // b7
    *(d += 256) = *(++s); // b8
    *(d + 1) = *(++s);    // b9
    *(d += 256) = *(++s); // b10
    *(d + 1) = *(++s);    // b11
    *(d += 256) = *(++s); // b12
    *(d + 1) = *(++s);    // b13
    *(d += 256) = *(++s); // b14
    *(d + 1) = *(++s);    // b15
  }
}
/*
 * Function:  btile_draw
 * --------------------
 * Dibuja un btile con chequeos
 *
 */
void btile_draw(unsigned char til, unsigned char lin, unsigned char col) {
  if (lin < 192 && col < 31) {
    NIRVANAP_drawT_raw(til, lin, col);
  }
}

void btile_drawA(unsigned char til, unsigned char lin, unsigned char col) {
  if (lin < 192 && col < 31) {
    NIRVANAP_drawT_raw(til, lin, col);
    ++draw_sp;
    if (draw_sp == 4) {
      draw_sp = 0;
      NIRVANAP_halt();
    }
  }
}

void btile_draw_halt(unsigned char til, unsigned char lin, unsigned char col) {
  if (lin < 192 && col < 31) {
    NIRVANAP_halt();
    NIRVANAP_drawT_raw(til, lin, col);
  }
}
/*
 * Function:  btile_draw_l
 * --------------------
 * Dibuja un btile con chequeo y scroll en el horinzontal
 *
 */
void btile_draw_l(unsigned char til, unsigned char lin, unsigned char col) {
  if (lin < 192) {
    if (col < 31) {
      // Dibuja btile Normal
      NIRVANAP_drawT_raw(til, lin, col);
    } else {
      // Dibuja btile en la columna 31 y 0 para efecto de loop
      btile_hright(til, lin, 0);
      btile_hleft(til, lin, 31);
    }
  }
}

void btile_paint(unsigned char *a, unsigned char l, unsigned char c) {
  if (l < 192 && c < 31) {
    NIRVANAP_fillT_raw(*a, l, c);
  }
}

/*
 * Function:  sprite_draw
 * --------------------
 * Dibuja un btile usando los sprites nirvana "gratis"
 * el dibujo se realiza en el próximo frame y sobrescribe la pantalla.
 */
void sprite_draw(unsigned char s, unsigned char t, unsigned char l, unsigned char c) {
  if (l < 192 && c < 31) {
    // Dibuja un sprite nirvana en el próximo frame
    NIRVANAP_spriteT(s, t, l, c);
  }
}

// /*
//  * Function:  sprite_drawA
//  * --------------------
//  * Dibuja un btile usando los sprites nirvana "gratis"
//  * el dibujo se realiza en el próximo frame y sobrescribe la pantalla.
//  */
// void sprite_drawA(unsigned char t, unsigned char l, unsigned char c) {
//   if (l < 192 && c < 31) {
//     // Dibuja un sprite nirvana en el próximo frame
//     NIRVANAP_spriteT(nirv_sp, t, l, c);
//     ++nirv_sp;
//     if (nirv_sp == 8) {
//       nirv_sp = 0;
//       NIRVANAP_halt();
//     }
//   }
// }

/*
 * Function:  sprite_reset
 * --------------------
 * Resete los sprites nirvana
 */
void sprite_reset() {
  NIRVANAP_spriteT(0, 0, 0, 0);
  NIRVANAP_spriteT(1, 0, 0, 0);
  NIRVANAP_spriteT(2, 0, 0, 0);
  NIRVANAP_spriteT(3, 0, 0, 0);
  NIRVANAP_spriteT(4, 0, 0, 0);
  NIRVANAP_spriteT(5, 0, 0, 0);
  NIRVANAP_spriteT(6, 0, 0, 0);
  NIRVANAP_spriteT(7, 0, 0, 0);
  nirv_sp = 0;
  // NIRVANAP_halt();
}

// void btile_paint(unsigned char t, unsigned char c) {
//   // colores
//   unsigned int si;
//   unsigned int se;
//   //   // Byte 33: attribute value for 1st and 2nd pixel line, 1st column
//   //   // Byte 34: attribute value for 3rd and 4th pixel line, 1st column
//   //   // Byte 35: attribute value for 5th and 6th pixel line, 1st column
//   //   // ...
//   //   // Byte 40: attribute value for 15th and 16th pixel line, 1st column
//   //   // Byte 41: attribute value for 1st and 2nd pixel line, 2nd column
//   //   // Byte 42: attribute value for 3rd and 4th pixel line, 2nd column
//   //   // ...
//   //   // Byte 48: attribute value for 15th and 16th pixel line, 2nd column

//   if (btiles_color[t] != c) {
//     foo = 0;
//     si = (t * 48) + 32;
//     se = si + 16;
//     while (si < se) {
//       if ((btiles[si] & 0b00000111) == btiles_color[t]) {
//         btiles[si] = c;
//       }
//       ++si;
//     }
//     btiles_color[t] = c;
//   }
// }

// void draw_rst_init() {
//   // Reinicia contador
//   rst_i = 0;
// }

// void draw_rst_add(unsigned char t, unsigned char l, unsigned char c) {
//   unsigned char j;
//   if (c < 32 && l < 192) {

//     j = 0;
//     while (j < rst_i) {
//       if (l == rst_l[j] && c == rst_c[j]) {
//         rst_t[j] = t;
//         rst_l[j] = l;
//         rst_c[j] = c;
//         return;
//       }
//       ++j;
//     }

//     rst_t[rst_i] = t;
//     rst_l[rst_i] = l;
//     rst_c[rst_i] = c;
//     ++rst_i;
//     if (rst_i >= MAX_DRAW) {
//       rst_i = 0;
//     }
//   }
// }

// void draw_rst_draw() {
//   unsigned char i;
//   i = 0;
//   while (i < rst_i) {
//     if (rst_c[i] < 31) {
//       NIRVANAP_drawT_raw(rst_t[i], rst_l[i], rst_c[i]);
//     } else {
//       btile_half_v(0, rst_t[i], rst_l[i], 31);
//       btile_half_v(1, rst_t[i], rst_l[i], 0);
//     }
//     ++i;
//   }
// }
