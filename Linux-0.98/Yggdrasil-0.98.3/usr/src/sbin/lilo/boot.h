/* boot.h  -  Boot image composition */

/* Written 1992 by Werner Almesberger */


#ifndef _BOOT_H_
#define _BOOT_H_

#include "lilo.h"


void boot_image(char *spec,IMAGE_DESCR *descr);

/* Maps a "classic" boot image. */

void boot_device(char *spec,char *range,IMAGE_DESCR *descr);

/* Maps sectors from a device as the boot image. Can be used to boot raw-written
   disks. */

void boot_unstripped(char *setup,char *kernel,IMAGE_DESCR *descr);

/* Maps an unstripped kernel image as the boot image. The setup (without the
   header) is prepended. */

void boot_other(char *loader,char *boot,char *part,IMAGE_DESCR *descr);

/* Merges a loader with a partition table and appends a boot sector. This mix
   is used to boot non-Linux systems. */

#endif
