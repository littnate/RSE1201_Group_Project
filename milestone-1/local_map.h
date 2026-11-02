#ifndef LOCAL_MAP_H
#define LOCAL_MAP_H

#include <stdbool.h>

#define LOCAL_MAP_TERRAIN_UNKNOWN '?'

typedef struct {
    char terrain;
    unsigned scanned;
    unsigned explored;
} LocalMapTerrainCount;

void local_map_init(void);
void local_map_reset(void);

void local_map_mark_explored_here(void);

void local_map_move_success(int dx, int dy);

void local_map_record_scan(int dx, int dy, char terrain);

char local_map_peek_terrain(int x, int y);
char local_map_peek_terrain_at_offset(int dx, int dy);

void local_map_mark_blocked(int x, int y);
void local_map_mark_blocked_at_offset(int dx, int dy);

unsigned local_map_terrain_counts(LocalMapTerrainCount *out, unsigned capacity);

void local_map_print_explored(void);
void local_map_print_scanned(void);

#endif
