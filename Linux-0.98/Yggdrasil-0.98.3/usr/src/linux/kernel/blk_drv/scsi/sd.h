/*
 *	sd.h Copyright (C) 1992 Drew Eckhardt 
 *	SCSI disk driver header file by
 *		Drew Eckhardt 
 *
 *	<drew@colorado.edu>
 */
#ifndef _SD_H
	#define _SD_H
/*
	$Header: /usr/src/linux/kernel/blk_drv/scsi/RCS/sd.h,v 1.1 1992/07/24 06:27:38 root Exp root $
*/

#ifndef _SCSI_H
#include "scsi.h"
#endif

#ifndef _GENDISK_H
#include <linux/genhd.h>
#endif

/*
	This is an arbitrary constant, and may be changed to whatever
	suits your purposes.  Note that smaller will get you a few bytes
	more in kernel space if that is your thing.
*/

#define MAX_SD 4
extern int NR_SD;

extern struct hd_struct sd[MAX_SD << 4];

typedef struct {
		unsigned capacity;		/* size in blocks */
		unsigned sector_size;		/* size in bytes */
		Scsi_Device  *device;		
		unsigned char sector_bit_size;	/* sector_size = 2 to the  bit size power */
		unsigned char sector_bit_shift;	/* power of 2 sectors per FS block */
		unsigned ten:1;			/* support ten byte read / write */
		unsigned remap:1;		/* support remapping  */
		} Scsi_Disk;
	
extern Scsi_Disk rscsi_disks[MAX_SD];

unsigned long sd_init(unsigned long, unsigned long);

#define HOST (rscsi_disks[DEVICE_NR(CURRENT->dev)].device->host_no)
#define ID (rscsi_disks[DEVICE_NR(CURRENT->dev)].device->id)
#define LUN (rscsi_disks[DEVICE_NR(CURRENT->dev)].device->lun)
#endif
