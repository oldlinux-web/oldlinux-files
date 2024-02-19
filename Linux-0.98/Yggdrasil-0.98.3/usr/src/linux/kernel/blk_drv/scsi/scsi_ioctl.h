#ifndef _SCSI_IOCTL_H
#define _SCSI_IOCTL_H 

#ifndef _CONFIG_H
#include <linux/config.h>
#endif

#define SCSI_IOCTL_PROBE_HOST 0
#define SCSI_IOCTL_SEND_COMMAND 1
#define SCSI_IOCTL_TEST_UNIT_READY 2
/* The door lock/unlock constants are compatible with Sun constants for
   the cdrom */
#define SCSI_IOCTL_DOORLOCK 0x5380		/* lock the eject mechanism */
#define SCSI_IOCTL_DOORUNLOCK 0x5381		/* unlock the mechanism	  */

#define	SCSI_REMOVAL_PREVENT	1
#define	SCSI_REMOVAL_ALLOW	0

#ifdef CONFIG_BLK_DEV_SD
/* 	Should start at 128	*/
#endif

#ifdef CONFIG_BLK_DEV_SD
/* 	Should start at 256	*/
#endif

#endif


