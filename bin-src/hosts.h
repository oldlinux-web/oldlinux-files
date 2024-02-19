#ifndef __HOST_H__
	#define __HOSTS_H__
 
/*
	File hosts.h
	SCSI host adapter include file.
*/
 
#define MAX_SCSI_HOSTS 1
 
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
		SCSI driver.
	*/
 
	int (* detect)(void); 
 
	/*
		The info function will return whatever useful
		information the developer sees fit.
	*/
 
	char * (* info)(void);
 
	/*
		The command function shall return the SCSI return
		code in the low 8 bits, a driver error in the high 8
		bits.  Target is the target ID, IN normal numbers - not a
		bit.  The cmnd is the variable length command to
		be sent, 
		buff a pointer to the buffer which will be read from / 
		written to, and bufflen the length of that buffer
	*/
 
	int (* command)(unsigned char target, const void *cmnd, 
			     void *buff, int bufflen);
 
	/*
		present contains a flag as to weather we are present -
		so we don't have to call detect multiple times.
	*/
 
	unsigned char present;	
	} Scsi_Host;
 
/*
	The scsi_hosts array is	the array containing the data for all 
	possible <supported> scsi hosts.   
*/
 
extern Scsi_Host scsi_hosts[MAX_SCSI_HOSTS];
 
/*
	scsi_init initializes the scsi hosts.
*/
 
void scsi_init(void);
#endif
