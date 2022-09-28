/* Tool to import from Tiled JSON to a Compressed and compatible C struc
 * For Prisma Engine
 * Created by Noentiendo
 */

#include "funcs.h"
#include "parson/parson.h"
#include "zx7/zx7.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  JSON_Array *layers;
  JSON_Array *data;
  JSON_Array *objects;
  JSON_Array *properties;

  JSON_Object *map;
  JSON_Object *layer;
  JSON_Object *object;
  JSON_Object *property;

  JSON_Value *root_value;

  const char *name = NULL;
  const char *tipo = NULL;

  int tile_height;
  int tile_width;

  int height;
  int width;

  int screen_xi;
  int screen_yi;
  int screen_xe;
  int screen_ye;
  int screen_x;
  int screen_y;

  size_t i;
  size_t j;
  unsigned short k;
  unsigned short s;
  unsigned short x;
  unsigned short y;

  // Looping variables
  unsigned int index;
  unsigned short room;

  size_t input_size;
  size_t output_size;
  unsigned short map_zx7_size;
  size_t entities_packed_size;

  /* File Handling */
  FILE *file;
  char *output_name;

  verbose = 0;
  /* determine output filename */
  if (argc == 4) {
    output_name = "game_map.h";
  } else if (argc == 5) {
    output_name = argv[5];
  } else {
    fprintf(stderr,
            "Created by Noentiendo\nUsage: %s map.json map_width map_height\n",
            argv[0]);
    exit(1);
  }

  /* open input file */
  file = fopen(argv[1], "rb");
  if (!file) {
    fprintf(stderr, "Error: Cannot access input file %s\n", argv[1]);
    exit(1);
  }

  /* determine input size */
  fseek(file, 0L, SEEK_END);
  input_size = ftell(file);
  fseek(file, 0L, SEEK_SET);
  if (!input_size) {
    fprintf(stderr, "Error: Empty input file %s\n", argv[1]);
    exit(1);
  }

  /* close input file */
  fclose(file);
  root_value = json_parse_file(argv[1]);

  screen_width = atoi(argv[2]);
  screen_height = atoi(argv[3]);

  /* parsing json and validating output */

  /* parsing map to json object */
  map = json_value_get_object(root_value);
  tile_width = json_object_get_number(map, "tilewidth");
  tile_height = json_object_get_number(map, "tileheight");

  /* Process layers array */
  layers = json_object_get_array(map, "layers");
  if (layers == NULL) {
    printf("Error retrieving Layers.\n");
    return 0;
  }
  printf("Processing Map.\n");
  /* loop at layers */
  for (i = 0; i < json_array_get_count(layers); i++) {
    /* retrieve layer */
    layer = json_array_get_object(layers, i);
    name = json_object_get_string(layer, "name");
    tipo = json_object_get_string(layer, "type");

    /* Processing Background Layer - Tile map */
    if (strcmp(tipo, "tilelayer") == 0 && strcmp(name, "Background") == 0) {
      /* Get map dimensions*/
      printf("Processing Layer %s\n", name);
      map_width = json_object_get_number(layer, "width");
      map_height = json_object_get_number(layer, "height");

      // Calculating
      map_screens_h = map_width / screen_width;
      map_screens_v = map_height / screen_height;
      screen_total = map_screens_h * map_screens_v;

      /* Processing Background */
      data = json_object_get_array(layer, "data");
      tiled_size = json_array_get_count(data);
      if (verbose)
        printf("Size %i\n", tiled_size);
      for (j = 0; j < tiled_size; j++) {
        /*move to memory for processing */
        map_tiled[j] = json_array_get_number(data, j);
      }
    }

    /* Processing Objects Layer*/
    if (strcmp(tipo, "objectgroup") == 0 && strcmp(name, "Objects") == 0) {
      printf("Processing Layer %s\n", name);
      objects = json_object_get_array(layer, "objects");
      entity_count = 0;
      type_count = 0;
      for (j = 0; j < json_array_get_count(objects); j++) {
        // Retrieve Object
        object = json_array_get_object(objects, j);
        height = json_object_get_number(object, "height");
        width = json_object_get_number(object, "width");
        tipo = json_object_get_string(object, "type");

        /* Type Entities */
        name = json_object_get_string(object, "name");
        type[entity_count] = add_entity(name);

        x = json_object_get_number(object, "x");
        y = json_object_get_number(object, "y");

        k = (y / (screen_height * tile_height)) * map_screens_h;
        s = (x / (screen_width * tile_width));
        screen[entity_count] = k + s;
        col[entity_count] = (unsigned char)((x / tile_width) % screen_width);
        row[entity_count] = (unsigned char)((y / tile_height) % screen_height);
        id[entity_count] = json_object_get_number(object, "id");
        // Properties
        properties = json_object_get_array(object, "properties");
        for (k = 0; k < json_array_get_count(properties); k++) {
          // Values
          property = json_array_get_object(properties, k);
          name = json_object_get_string(property, "name");

          if (strcmp(name, "value_a") == 0) {
            x = json_object_get_number(property, "value");
            value_a[entity_count] = x;
          }
          if (strcmp(name, "value_b") == 0) {
            x = json_object_get_number(property, "value");
            value_b[entity_count] = x;
          }
          if (strcmp(name, "value_c") == 0) {
            x = json_object_get_number(property, "value");
            value_c[entity_count] = x;
          }
          if (strcmp(name, "value_d") == 0) {
            x = json_object_get_number(property, "value");
            value_d[entity_count] = x;
          }
        }
        ++entity_count;
        //}
      }
    }
  }

  /* Pack array - Generate output file */
  /* Processing Helper Layer - Screen distribution on map */
  if (verbose)
    printf("Checking Errors.\n");
  if ((map_width % screen_width) != 0) {
    printf("Error Map Width %i is not multiple of screen width %i.\n",
           map_width, screen_width);
    return 0;
  }
  if ((map_height % screen_height) != 0) {
    printf("Error Map Width %i is not multiple of screen width %i.\n",
           map_height, screen_height);
    return 0;
  }

  //////////////////////////////////////
  // Sort n Pack entities data for each screen
  //////////////////////////////////////
  if (verbose)
    printf("Packing Entities\n");
  room = 0;
  entity_data_total = 0;
  entity_index[0] = 0;

  while (room < screen_total) {
    entity_data_room = 0;
    i = 0;
    while (i < entity_count) {
      if (screen[i] == room) {
        // Pack Entities from current room
        entity_data[entity_data_total + 0] = type[i];
        entity_data[entity_data_total + 1] = row[i];
        entity_data[entity_data_total + 2] = col[i];
        entity_data[entity_data_total + 3] = id[i];
        entity_data[entity_data_total + 4] = value_a[i];
        entity_data[entity_data_total + 5] = value_b[i];
        entity_data[entity_data_total + 6] = value_c[i];
        entity_data[entity_data_total + 7] = value_d[i];

        entity_data_total += 8;
        entity_data_room += 8;
      }
      i++;
    }
    // EOF
    entity_size[room] = entity_data_room;
    room++;
    entity_index[room] = entity_data_total;
  }
  //////////////////////////////////////
  // Split map in rooms
  //////////////////////////////////////
  i = 0;
  room = 0;
  verbose = 0;
  while (i < map_width * map_height) {
    if (verbose)
      printf("room %d %d\n", room, (unsigned int)i);
    create_room(room, i);
    i = i + screen_width;
    ++room;
    if ((i % map_width) == 0) {
      i = room * screen_width * screen_height;
    }
  }
  //////////////////////////////////////
  // 7zx Compress Rooms
  //////////////////////////////////////
  if (verbose) {
    printf("7zx Compressing Rooms.\n");
  }

  i = 0;
  room = 0;
  input_size = screen_width * screen_height * sizeof(char);
  map_zx7_size = 0;
  map_index[room] = 0;

  while (room < screen_total) {
    p = &map_room[room][0];
    data_zx7 = compress(optimize(p, input_size), p, input_size, &output_size);
    memcpy(&map_zx7[map_zx7_size], data_zx7, output_size);

    if (verbose)
      printf("Room %d Size %d Bytes\n", room, (unsigned int)output_size);
    /* Add total compress size */
    ++room;
    map_zx7_size = map_zx7_size + output_size;
    map_index[room] = map_zx7_size;
  }

  printf("Rooms: %i\n", screen_total);
  printf("Unpack Size: %i Bytes\n",
         (int)input_size * map_screens_h * map_screens_v);
  printf("Packed Size: %i Bytes\n", (int)map_zx7_size);

  // Entities
  printf("Entities: %i\n", entity_count);
  if (entity_count) {
    printf("Entities Size: %i\n", (int)entity_data_total);
  }

  // file = fopen("map.tiled", "wb");
  // index = fwrite(map_tiled, sizeof(char), tiled_size, file);
  // if (index == 0) {
  //   printf("Error during writing to file !\n");
  // }

  file = fopen("map.pack", "wb");
  index = fwrite(map_zx7, sizeof(char), map_zx7_size, file);
  if (index == 0) {
    printf("Error during writing to map.pack\n");
  }
  fclose(file);
  file = fopen("map.index", "wb");
  index = fwrite(map_index, sizeof(unsigned short), screen_total, file);
  if (index == 0) {
    printf("Error during writing to map.index\n");
  }
  fclose(file);
  if (entity_data_total > 0) {
    file = fopen("entities.data", "wb");
    index = fwrite(entity_data, sizeof(unsigned char), entity_data_total, file);
    if (index == 0) {
      printf("Error during writing to entities.data\n");
    }
    fclose(file);

    file = fopen("entities.size", "wb");
    index = fwrite(entity_size, sizeof(unsigned char), screen_total, file);
    if (index == 0) {
      printf("Error during writing to entities.size\n");
    }
    fclose(file);
    file = fopen("entities.index", "wb");
    index = fwrite(entity_index, sizeof(unsigned short), screen_total, file);
    if (index == 0) {
      printf("Error during writing to entities.index\n");
    }
    fclose(file);
  }

  // file = fopen("map.blockset", "wb");
  // index = fwrite(map_blockset, sizeof(unsigned short), screen_total, file);
  // if (index == 0) {
  //   printf("Error during writing to map.blockset\n");
  // }
  // fclose(file);
  // Building .h file
  file = fopen("map.txt", "w");
  fprintf(file, "/*\n");
  fprintf(file, " * Map Data\n");
  fprintf(file, " * Tot Screens   : %i\n", screen_total);
  fprintf(file, " * Unpack Size   : %i\n",
          (unsigned short)input_size * map_screens_h * map_screens_v);
  fprintf(file, " * Packed Size   : %i\n", (unsigned short)map_zx7_size);
  fprintf(file, " * Map Width     : %i\n", map_screens_h);
  fprintf(file, " * Map Height    : %i\n", map_screens_v);
  fprintf(file, " * Screen Width  : %i\n", screen_width);
  fprintf(file, " * Screen Height : %i\n", screen_height);
  fprintf(file, "*/\n");
  fprintf(file, "\n");
  fprintf(file, "#define MAP_WIDTH %i\n", map_screens_h);
  fprintf(file, "#define MAP_HEIGHT %i\n", map_screens_v);
  fprintf(file, "#define MAX_ENTITIES %i\n", entity_count);
  fprintf(file, "#define MAX_TYPES %i\n", type_count);
  fprintf(file, "#define MAX_ROOM %i\n", screen_total);
  i = 0;
  while (i < type_count) {
    unsigned char *s = (unsigned char *)entities_types[i];
    while (*s) {
      *s = toupper((unsigned char)*s);
      s++;
    }
    fprintf(file, "#define %s %d\n", (const char *)entities_types[i],
            (unsigned int)i);
    ++i;
  }
  fclose(file);

  return 0;
}
