/*
 *      sr.h by David Giller
 *      CD-ROM disk driver header file
 *      
 *      adapted from:
 *	sd.h Copyright (C) 1992 Drew Eckhardt 
 *	SCSI disk driver header file by
 *		Drew Eckhardt 
 *
 *	<drew@colorado.edu>
 */

#ifndef _SR_H
#define _SR_H

#include "scsi.h"

#define MAX_SR 2
extern int NR_SR;

typedef struct
	{
	unsigned 	capacity;		/* size in blocks 			*/
	unsigned 	sector_size;		/* size in bytes 			*/
	Scsi_Device  	*device;		
	unsigned char	sector_bit_size;	/* sector size = 2^sector_bit_size	*/
	unsigned char	sector_bit_shift;	/* sectors/FS block = 2^sector_bit_shift*/
	unsigned 	ten:1;			/* support ten byte commands		*/
	unsigned 	remap:1;		/* support remapping			*/
	unsigned 	use:1;			/* is this device still supportable	*/
	} Scsi_CD;
	
extern Scsi_CD scsi_CDs[MAX_SR];

unsigned long sr_init(unsigned long, unsigned long);

#define SR_HOST (scsi_CDs[DEVICE_NR(CURRENT->dev)].device->host_no)
#define SR_ID (scsi_CDs[DEVICE_NR(CURRENT->dev)].device->id)
#define SR_LUN (scsi_CDs[DEVICE_NR(CURRENT->dev)].device->lun)
#endif
