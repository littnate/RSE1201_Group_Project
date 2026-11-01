#ifndef LOCAL_MAP_H
#define LOCAL_MAP_H

#include <stdbool.h>

/* Relative map grown from successful moves and scans.
 * Spawn starts at (0, 0); North decreases y and East increases x. */

#define LOCAL_MAP_TERRAIN_UNKNOWN '?'

typedef struct {
    char terrain;
    unsigned scanned;
    unsigned explored;
} LocalMapTerrainCount;

void local_map_init(void);
void local_map_reset(void);

int local_map_get_x(void);
int local_map_get_y(void);

/* Mark current cell explored (for example, at spawn). */
void local_map_mark_explored_here(void);

/* Step vehicle position by (dx, dy) and mark the destination explored. */
void local_map_move_success(int dx, int dy);

/* Store scanned terrain symbol at offset (dx, dy) from the vehicle. */
void local_map_record_scan(int dx, int dy, char terrain);

/* Return known terrain at (x, y) or offset (dx, dy), or '?'. */
char local_map_peek_terrain(int x, int y);
char local_map_peek_terrain_at_offset(int dx, int dy);

bool local_map_is_explored(int x, int y);
bool local_map_is_blocked(int x, int y);
void local_map_mark_blocked(int x, int y);
void local_map_mark_blocked_at_offset(int dx, int dy);

void local_map_get_bounds(int *min_x, int *max_x, int *min_y, int *max_y);
bool local_map_has_grid(void);

unsigned local_map_count_scanned(char terrain);

/* Fill out[] with scanned and explored counts per discovered terrain symbol. */
unsigned local_map_terrain_counts(LocalMapTerrainCount *out, unsigned capacity);

/* Print the explored ('x') or scanned ('c') map view. */
void local_map_print_explored(void);
void local_map_print_scanned(void);

#endif /* LOCAL_MAP_H */
