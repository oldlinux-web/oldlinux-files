/* geometry.h  -  Device and file geometry computation */

/* Written 1992 by Werner Almesberger */


#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include "lilo.h"


typedef struct {
    int device,heads,cylinders,sectors;
    int start; /* partition offset */
    int spb; /* sectors per block */
    int fd,file;
} GEOMETRY;

void geo_init(char *name);

/* Loads the disk geometry table. */

int geo_open(GEOMETRY *geo,char *name,int flags);

/* Opens the specified file or block device, obtains the necessary geometry
   information and returns the file descriptor. If the name contains a BIOS
   device specification (xxx:yyy), it is removed and stored in the geometry
   descriptor. */

void geo_close(GEOMETRY *geo);

/* Closes a file or device that has previously been opened by geo_open. */

int geo_comp_addr(GEOMETRY *geo,int offset,SECTOR_ADDR *addr);

/* Determines the address of the disk sector that contains the offset'th
   byte of the specified file or device. Returns a non-zero value if such
   a sector exists, zero if it doesn't. */

#endif
