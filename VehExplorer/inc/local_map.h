#ifndef LOCAL_MAP_H
#define LOCAL_MAP_H

#include <stddef.h>
#include <stdbool.h>

/* Relative grid grown from successful moves and scans.
 * Origin (0,0) = spawn. North decreases y; East increases x. */

#define LM_TERRAIN_UNKNOWN '?'

typedef struct {
    char terrain; /* known symbol, or LM_TERRAIN_UNKNOWN */
    unsigned long scanned;
    unsigned long explored;
} LocalMapTerrainCount;

void local_map_init(void);
void local_map_reset(void);

int local_map_get_x(void);
int local_map_get_y(void);

/* Mark current cell explored (e.g. at spawn). */
void local_map_mark_explored_here(void);

/* On successful move: step by (dx,dy) and mark the new cell explored. */
void local_map_move_success(int dx, int dy);

/* On successful scan: store terrain at offset (dx,dy) from vehicle. */
void local_map_record_scan(int dx, int dy, char terrain);

/* Known scanned terrain at absolute (x,y), or LM_TERRAIN_UNKNOWN. */
char local_map_peek_terrain(int x, int y);

/* Known scanned terrain at offset (dx,dy) from vehicle. */
char local_map_peek_terrain_at_offset(int dx, int dy);

/*
 * Fill out[] with per-terrain scanned/explored tallies (dynamic symbols).
 * Explored cells with no scanned symbol use LM_TERRAIN_UNKNOWN.
 * Returns number of distinct entries written (0 if out is NULL / capacity 0
 * or map empty). Truncates silently if more than capacity entries exist.
 */
size_t local_map_terrain_counts(LocalMapTerrainCount *out, size_t capacity);

/* Print explored / scanned views to match tutor TerrainMap style:
 * dashed green header, cell field-width 3, cyan bold+underline vehicle,
 * tight bounds over known cells (no fixed empty window / box border). */
void local_map_print_explored(void);
void local_map_print_scanned(void);

#endif /* LOCAL_MAP_H */
