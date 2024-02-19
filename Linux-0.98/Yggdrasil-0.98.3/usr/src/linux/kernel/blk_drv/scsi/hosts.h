/*
 *	hosts.h Copyright (C) 1992 Drew Eckhardt 
 *	mid to low-level SCSI driver interface header by	
 *		Drew Eckhardt 
 *
 *	<drew@colorado.edu>
 */

#ifndef _HOSTS_H
	#define _HOSTS_H

#ifndef MAX_SCSI_HOSTS
	#include "max_hosts.h"
#endif

/*
	$Header: /usr/src/linux/kernel/blk_drv/scsi/RCS/hosts.h,v 1.1 1992/07/24 06:27:38 root Exp root $
*/

/*
	The Scsi_Cmnd structure is used by scsi.c internally, and for communication with
	low level drivers that support multiple outstanding commands.
*/

typedef struct scsi_cmnd {
	int host;
	unsigned char target, lun;
	unsigned char cmnd[10];
	unsigned bufflen;
	void *buffer;
	
	unsigned char sense_cmnd[6];
	unsigned char *sense_buffer;	

	unsigned flags;
		
	int retries;
	int allowed;
	int timeout_per_command, timeout_total, timeout;
	
	void (*done)(int,int);
	struct scsi_cmnd *next, *prev;	
	} Scsi_Cmnd;		 

/*
	The Scsi_Host type has all that is needed to interface with a SCSI
	host in a device independant matter.  
*/

typedef struct     
	{
	/*
		The name pointer is a pointer to the name of the SCSI
		device detected.
	*/

	char *name;

	/*
		The detect function shall return non zero on detection,
		and initialize all data necessary for this particular
		SCSI driver.  It is passed the host number, so this host
		knows where it is in the hosts array
	*/

	int (* detect)(int); 

	/*
		The info function will return whatever useful
		information the developer sees fit.              
	*/

        char *(* info)(void);

	/*
		The command function takes a target, a command (this is a SCSI 
		command formatted as per the SCSI spec, nothing strange), a 
		data buffer pointer, and data buffer length pointer.  The return
		is a status int, bit fielded as follows : 
		Byte	What
		0	SCSI status code
		1	SCSI 1 byte message
		2 	host error return.
		3	mid level error return
	*/

	int (* command)(unsigned char target, const void *cmnd, 
			     void *buff, int bufflen);

        /*
                The QueueCommand function works in a similar manner
                to the command function.  It takes an additional parameter,
                void (* done)(int host, int code) which is passed the host 
		# and exit result when the command is complete.  
		Host number is the POSITION IN THE hosts array of THIS
		host adapter.
        */

        int (* queuecommand)(unsigned char target, const void *cmnd,
                             void *buff, int bufflen, void (*done)(int,int));

	
	/*
		Since the mid level driver handles time outs, etc, we want to 
		be able to abort the current command.  Abort returns 0 if the 
		abortion was successful.  If non-zero, the code passed to it 
		will be used as the return code, otherwise 
		DID_ABORT  should be returned.

		Note that the scsi driver should "clean up" after itself, 
		resetting the bus, etc.  if necessary. 
	*/

	int (* abort)(int);

	/*
		The reset function will reset the SCSI bus.  Any executing 
		commands should fail with a DID_RESET in the host byte.
	*/ 

	int (* reset)(void);
	
	/*
		This determines if we will use a non-interrupt driven
		or an interrupt driven scheme,  It is set to the maximum number
		of simulataneous commands a given host adapter will accept.
	*/

	int can_queue;

	/*
		In many instances, especially where disconnect / reconnect are 
		supported, our host also has an ID on the SCSI bus.  If this is 
		the case, then it must be reserved.  Please set this_id to -1 if 		your settup is in single initiator mode, and the host lacks an 
		ID.
	*/
	
	int this_id;

	/*
		present contains a flag as to weather we are present -
		so we don't have to call detect multiple times.
	*/

	unsigned present:1;	
	/*
	  true if this host adapter uses unchecked DMA onto an ISA bus.
	*/
	unsigned unchecked_isa_dma:1;

	} Scsi_Host;

/*
	The scsi_hosts array is	the array containing the data for all 
	possible <supported> scsi hosts.   
*/

extern Scsi_Host scsi_hosts[];

/*
	This is our semaphore array, used by scsi.c, sd.c, st.c.
	Other routines SHOULD NOT mess with it.  Your driver should NOT mess with it.
	This is used to protect against contention by disk and tape drivers.
*/

extern volatile unsigned char host_busy[];
extern volatile int host_timeout[];

/*
	This is the queue of currently pending commands for a given
	SCSI host.
*/

extern volatile Scsi_Cmnd *host_queue[];

/*
	scsi_init initializes the scsi hosts.
*/


void scsi_init(void);

#define BLANK_HOST {"", 0, 0, 0, 0, 0, 0, 0, 0, 0}
#endif
