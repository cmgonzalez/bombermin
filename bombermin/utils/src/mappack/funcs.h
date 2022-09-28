
#define MAX_ENTITIES 256 // Max of classes
#define MAX_STR_LEN 32   //	Max Class name Length
#define MAX_WIDTH 256
#define MAX_HEIGHT 256
#define MAX_ROOMS 256
#define MAX_SCR_WIDTH 32
#define MAX_SCR_HEIGHT 24 // Spectrum!

unsigned char verbose;

// Maps data arrays
unsigned char map_tiled[MAX_WIDTH * MAX_HEIGHT];
unsigned char map_room[MAX_ROOMS][MAX_SCR_WIDTH * MAX_SCR_HEIGHT];
unsigned char map_zx7[MAX_WIDTH * MAX_HEIGHT];
unsigned short map_index[MAX_ROOMS];
unsigned char map_blockset[MAX_ROOMS];
unsigned char map_blocks;
unsigned short tiled_size;

// Entities data arrays
unsigned char *entities_types[MAX_ENTITIES][MAX_STR_LEN];
unsigned char type[MAX_ENTITIES];
unsigned char screen[MAX_ENTITIES];
unsigned char row[MAX_ENTITIES];
unsigned char col[MAX_ENTITIES];
unsigned char value_a[MAX_ENTITIES];
unsigned char value_b[MAX_ENTITIES];
unsigned char value_c[MAX_ENTITIES];
unsigned char value_d[MAX_ENTITIES];
unsigned char id[MAX_ENTITIES];

// Entities data arrays
unsigned char entity_data[MAX_ENTITIES * 8];
unsigned char entity_size[MAX_ENTITIES];
unsigned short entity_index[MAX_ENTITIES];
unsigned char entity_7zx[MAX_ENTITIES * 8];

unsigned char entity_data_sizes[MAX_ENTITIES];

unsigned char entity_data_room;
unsigned short entity_data_total;
unsigned short entity_count;
unsigned short type_count;

// Map variables
int map_height;
int map_width;
int map_screens_h;
int map_screens_v;
int screen_height;
int screen_width;
int screen_total;

unsigned char *p;
unsigned char *data_zx7;

void create_room(unsigned short room, unsigned short start);
void to_upper(char *f_string);
unsigned char add_entity(const char *f_name);
int parseInt(char *chars);
int powInt(int x, int y);
