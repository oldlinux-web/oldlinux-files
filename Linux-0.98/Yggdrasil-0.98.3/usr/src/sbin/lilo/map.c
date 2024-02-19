/* map.c  -  Map file creation */

/* Written 1992 by Werner Almesberger */


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "lilo.h"
#include "common.h"
#include "geometry.h"
#include "map.h"


typedef struct _map_entry {
    SECTOR_ADDR addr;
    struct _map_entry *next;
} MAP_ENTRY;


static MAP_ENTRY *map,*last;
static GEOMETRY map_geo;
static SECTOR_ADDR zero_addr;
static int map_file;


void map_create(char *name)
{
    char buffer[SECTOR_SIZE*2];
    int fd;

    if ((fd = creat(name,0600)) < 0) die("creat %s: %s",name,strerror(errno));
    (void) close(fd);
    memset(buffer,0,SECTOR_SIZE*2);
    map_file = geo_open(&map_geo,name,O_RDWR);
    if (write(map_file,buffer,SECTOR_SIZE*2) != SECTOR_SIZE*2)
	die("write %s: %s",name,strerror(errno));
    if (!geo_comp_addr(&map_geo,SECTOR_SIZE,&zero_addr))
	die("Hole found in map file (zero sector)");
}


void map_done(DESCR_SECTOR *descr,SECTOR_ADDR *addr)
{
    struct stat st;

    if (lseek(map_file,0,0) < 0) pdie("lseek map file");
    if (write(map_file,(char *) descr,SECTOR_SIZE) != SECTOR_SIZE)
	pdie("write map file");
    if (!geo_comp_addr(&map_geo,0,addr))
	die("Hole found in map file (descr. sector)");
    if (verbose > 1) {
	if (fstat(map_file,&st) < 0) pdie("fstat map file");
	printf("Map file size: %d bytes.\n",st.st_size);
    }
    geo_close(&map_geo);
}


void map_register(SECTOR_ADDR *addr)
{
    MAP_ENTRY *new;

    new = alloc(sizeof(MAP_ENTRY));
    memcpy(&new->addr,addr,sizeof(SECTOR_ADDR));
    new->next = NULL;
    if (last) last->next = new;
    else map = new;
    last = new;
}


void map_add_sector(void *sector)
{
    int here;
    SECTOR_ADDR addr;

    if ((here = lseek(map_file,0L,1)) < 0) pdie("lseek map file");
    if (write(map_file,sector,SECTOR_SIZE) != SECTOR_SIZE)
	pdie("write map file");
    if (!geo_comp_addr(&map_geo,here,&addr))
	die("Hole found in map file (app. sector)");
    map_register(&addr);
}


void map_begin_section(void)
{
    map = last = NULL;
}


void map_add(GEOMETRY *geo,int from,int num_sect)
{
    int count;
    SECTOR_ADDR addr;

    for (count = 0; count < num_sect; count++) {
	if (geo_comp_addr(geo,SECTOR_SIZE*(count+from),&addr))
	    map_register(&addr);
	else {
	    map_register(&zero_addr);
	    if (verbose > 3) printf("Covering hole at sector %d.\n",count);
	}
    }
}


void map_compact(void)
{
    MAP_ENTRY *walk,*next;
    int count,removed,offset;

    removed = 0;
    walk = map;
    for (count = 0; walk && count < SETUPSECS; count++) walk = walk->next;
    offset = 0;
    while (walk && walk->next)
	if (walk->addr.device != walk->next->addr.device || walk->addr.track !=
	  walk->next->addr.track || walk->addr.head != walk->next->addr.head ||
	  walk->addr.sector+walk->addr.num_sect != walk->next->addr.sector ||
	  offset >> 16 != (offset+SECTOR_SIZE*walk->addr.num_sect) >> 16) {
	    offset += SECTOR_SIZE*walk->addr.num_sect;
	    walk = walk->next;
	}
	else {
	    walk->addr.num_sect++;
	    next = walk->next->next;
	    free(walk->next);
	    removed++;
	    walk->next = next;
	}
    if (verbose > 1)
	printf("Compaction removed %d BIOS call%s.\n",removed,removed == 1 ?
	  "" : "s");
}


static void map_alloc_page(int offset,SECTOR_ADDR *addr)
{
    int here;

    if ((here = lseek(map_file,offset,1)) < 0) pdie("lseek map file");
    if (write(map_file,"",1) != 1) pdie("write map file");
    if (!geo_comp_addr(&map_geo,here,addr))
	die("Hole found in map file (alloc_page)");
    if (lseek(map_file,-offset-1,1) < 0) pdie("lseek map file");
}


int map_end_section(SECTOR_ADDR *addr)
{
    int first,offset,sectors;
    char buffer[SECTOR_SIZE];
    MAP_ENTRY *walk,*next;

    first = 1;
    memset(buffer,0,SECTOR_SIZE);
    offset = sectors = 0;
    if (compact) map_compact();
    if (map == NULL) die("Empty map section");
    for (walk = map; walk; walk = next) {
	next = walk->next;
	if (first) {
	    first = 0;
	    map_alloc_page(0,addr);
	}
	if (offset+sizeof(SECTOR_ADDR)*2 > SECTOR_SIZE) {
	    map_alloc_page(SECTOR_SIZE,(SECTOR_ADDR *) (buffer+offset));
    	    if (write(map_file,buffer,SECTOR_SIZE) != SECTOR_SIZE)
		pdie("write map file");
	    memset(buffer,0,SECTOR_SIZE);
	    offset = 0;
	}
	memcpy(buffer+offset,&walk->addr,sizeof(SECTOR_ADDR));
	offset += sizeof(SECTOR_ADDR);
	sectors += walk->addr.num_sect;
	free(walk);
    }
    if (offset)
	if (write(map_file,buffer,SECTOR_SIZE) != SECTOR_SIZE)
	    pdie("write map file");
    return sectors;
}


int map_write(SECTOR_ADDR *list,int max_len)
{
    MAP_ENTRY *walk,*next;
    int sectors;

    sectors = 0;
    for (walk = map; walk; walk = next) {
	next = walk->next;
	if (--max_len <= 0) die("Map segment is too big.");
	memcpy(list++,&walk->addr,sizeof(SECTOR_ADDR));
	free(walk);
	sectors++;
    }
    memset(list,0,sizeof(SECTOR_ADDR));
    return sectors;
}
