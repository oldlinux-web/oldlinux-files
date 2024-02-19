/* bsect.h  -  Boot sector handling */

/* Written 1992 by Werner Almesberger */


#ifndef _BSECT_H_
#define _BSECT_H_

void bsect_open(char *boot_dev,char *map_file,char *install,int delay);

/* Loads the boot sector of the specified device and merges it with a new
   boot sector (if install != NULL). Sets the boot delay to 'delay' 1/10 sec.
   Creates a temporary map file. */

void bsect_image(char *spec);

/* Adds a new boot image. */

void bsect_update(char *backup_file,int force_backup);

/* Updates the boot sector and the map file. */

void bsect_cancel(void);

/* Cancels all changes. (Deletes the temporary map file and doesn't update
   the boot sector. */

#endif
