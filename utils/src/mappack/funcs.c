#include "funcs.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parseInt(char *chars) {
  int sum = 0;
  int len = strlen(chars);
  for (int x = 0; x < len; x++) {
    int n = chars[len - (x + 1)] - '0';
    sum = sum + powInt(n, x);
  }
  return sum;
}

int powInt(int x, int y) {
  for (int i = 0; i < y; i++) {
    x *= 10;
  }
  return x;
}

void create_room(unsigned short room, unsigned short start) {
  unsigned short j;
  unsigned short k;
  unsigned short i;

  unsigned short col;

  unsigned char blockset0;
  unsigned char blockset1;

  k = start;
  j = 0;
  col = 0;

  if (verbose) {
    printf("Room %i\n", room);
  }
  while (j < screen_width * screen_height) {
    // Tiled CSM!!! why -1
    map_room[room][j] = map_tiled[(j % screen_width) + k] - 1;

    if (verbose) {
      printf("%02X ", map_room[room][j]);
    }
    ++j;
    ++col;
    if (j % screen_width == 0) {
      if (verbose)
        printf("\n");
      k = k + map_width;
      col = 0;
    }
  }
}

void to_upper(char *f_string) {
  unsigned char c;
  for (c = 0; f_string[c] != '\0'; c++) {
    if (f_string[c] >= 'a' && f_string[c] <= 'z') {
      f_string[c] = f_string[c] - 32;
    } else {
      // f_string[c] = f_string[c];
    }
  }
}

unsigned char add_entity(const char *f_name) {
  unsigned char count;
  count = 0;
  // Search Existing
  while (count < type_count) {
    if (strcmp(f_name, (const char *)entities_types[count]) == 0) {
      return count;
    }
    ++count;
  }
  // Append to list
  strcpy((char *)entities_types[type_count], f_name);
  if (verbose) {
    printf("f_name %s\n", f_name);
    printf("array %s\n", (char *)entities_types[type_count]);
  }
  count = type_count;
  ++type_count;
  return count;
}