
#ifndef _ST_H
	#define _ST_H
/*
	$Header: /usr/src/linux/kernel/blk_drv/scsi/RCS/st.h,v 1.1 1992/07/24 06:27:38 root Exp root $
*/

#ifndef _SCSI_H
#include "scsi.h"
#endif

#define MAX_ST 1

typedef struct 
	{
	/*
		Undecided goodies go here!!!
	*/
	Scsi_Device* device;	
	} Scsi_Tape;


extern int NR_ST;
extern Scsi_Tape scsi_tapes[MAX_ST];
unsigned long st_init(unsigned long, unsigned long);
#endif
