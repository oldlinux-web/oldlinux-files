/* map.h  -  Map file creation */

/* Written 1992 by Werner Almesberger */


#ifndef _MAP_H_
#define _MAP_H_

#include "common.h"
#include "geometry.h"


void map_create(char *name);

/* Create and initialize the specified map file. */

void map_done(DESCR_SECTOR *descr,SECTOR_ADDR *addr);

/* Updates and closes the map file. */

void map_add_sector(void *sector);

/* Adds the specified sector to the map file and registers it in the map
   section. */

void map_begin_section(void);

/* Begins a map section. Note: maps can also be written to memory with 
   map_write. Thus, the map routines can be used even without map_create. */

void map_add(GEOMETRY *geo,int from,int num_sect);

/* Adds pointers to sectors from the specified file to the map file. */

void map_compact(void);

/* Compacts the current map section. */

int map_end_section(SECTOR_ADDR *addr);

/* Writes a map section to the map file and returns the address of the first
   sector of that section. Returns the number of sectors that have been
   mapped. */

int map_write(SECTOR_ADDR *list,int max_len);

/* Writes a map section to an array. If the section (including the terminating
   zero entry) exceeds max_len, map_write dies. */

#endif
