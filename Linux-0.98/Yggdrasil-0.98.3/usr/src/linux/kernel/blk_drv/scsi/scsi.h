/*
 *	scsi.h Copyright (C) 1992 Drew Eckhardt 
 *	generic SCSI package header file by
 *		Drew Eckhardt 
 *
 *	<drew@colorado.edu>
 */

#ifndef _SCSI_H
	#define _SCSI_H
/*
	$Header: /usr/src/linux/kernel/blk_drv/scsi/RCS/scsi.h,v 1.1 1992/07/24 06:27:38 root Exp root $

	For documentation on the OPCODES, MESSAGES, and SENSE values,
	please consult the SCSI standard.

*/

/*
	SCSI opcodes
*/

#define TEST_UNIT_READY 	0x00
#define REZERO_UNIT		0x01
#define REQUEST_SENSE		0x03
#define FORMAT_UNIT		0x04
#define REASSIGN_BLOCKS		0x07
#define READ_6			0x08
#define WRITE_6			0x0a
#define SEEK_6			0x0b
#define INQUIRY			0x12
#define MODE_SELECT		0x15
#define RESERVE			0x16
#define RELEASE			0x17
#define COPY			0x18
#define MODE_SENSE		0x1a
#define START_STOP		0x1b
#define RECIEVE_DAIGNOSTIC	0x1c
#define SEND_DIAGNOSTIC		0x1d
#define ALLOW_MEDIUM_REMOVAL	0x1e

#define READ_CAPACITY		0x25
#define READ_10			0x28
#define WRITE_10		0x2a
#define SEEK_10			0x2b
#define WRITE_VERIFY		0x2e
#define VERIFY			0x2f
#define SEARCH_HIGH		0x30
#define SEARCH_EQUAL		0x31
#define SEARCH_LOW		0x32
#define SET_LIMITS		0x33
#define COMPARE			0x39
#define COPY_VERIFY		0x3a

#define COMMAND_SIZE(opcode) ((opcode) ? ((opcode) > 0x20 ? 10 : 6) : 0)

/*
	MESSAGE CODES
*/

#define COMMAND_COMPLETE	0x00
#define EXTENDED_MESSAGE	0x01
#define SAVE_POINTERS		0x02
#define RESTORE_POINTERS 	0x03
#define DISCONNECT		0x04
#define INITIATOR_ERROR		0x05
#define ABORT			0x06
#define MESSAGE_REJECT		0x07
#define NOP			0x08
#define MSG_PARITY_ERROR	0x09
#define LINKED_CMD_COMPLETE	0x0a
#define LINKED_FLG_CMD_COMPLETE	0x0b
#define BUS_DEVICE_RESET	0x0c
#define IDENTIFY_BASE		0x80
#define IDENTIFY(can_disconnect, lun)   (IDENTIFY_BASE |\
					 ((can_disconnect) ?  0x40 : 0) |\
					 ((lun) & 0x07)) 

				 
/*
	Status codes
*/

#define GOOD			0x00
#define CHECK_CONDITION		0x01
#define CONDITION_GOOD		0x02
#define BUSY			0x04
#define INTERMEDIATE_GOOD	0x08
#define INTERMEDIATE_C_GOOD	0x0a
#define RESERVATION_CONFLICT	0x0c

#define STATUS_MASK		0x1e
	
/*
	the return of the status word will be in the following format :
	The low byte is the status returned by the SCSI command, 
	with vendor specific bits masked.

	The next byte is the message which followed the SCSI status.
	This allows a stos to be used, since the Intel is a little
	endian machine.

	The final byte is a host return code, which is one of the following.

	IE 
	lsb		msb
	status	msg	host code	

        Our errors returned by OUR driver, NOT SCSI message.  Orr'd with
        SCSI message passed back to driver <IF any>.
*/

/* 	NO error							*/
#define DID_OK 			0x00
/* 	Couldn't connect before timeout period				*/
#define DID_NO_CONNECT		0x01
/*	BUS stayed busy through time out period				*/
#define DID_BUS_BUSY		0x02
/*	TIMED OUT for other reason					*/
#define DID_TIME_OUT		0x03
/*	BAD target.							*/
#define DID_BAD_TARGET		0x04
/*	Told to abort for some other reason				*/
#define	DID_ABORT		0x05
/*
	Parity error
*/
#define DID_PARITY		0x06
/*
	Internal error
*/
#define DID_ERROR 		0x07	
/*
	Reset by somebody.
*/
#define DID_RESET 		0x08
/*
	Got an interrupt we weren't expecting.
*/
#define	DID_BAD_INTR		0x09

/*
	Driver status
*/ 
#define DRIVER_OK		0x00

/*
	These indicate the error that occured, and what is available.
*/

#define DRIVER_BUSY		0x01
#define DRIVER_SOFT		0x02
#define DRIVER_MEDIA		0x03
#define DRIVER_ERROR		0x04	

#define DRIVER_INVALID		0x05
#define DRIVER_TIMEOUT		0x06
#define DRIVER_HARD		0x07

#define SUGGEST_RETRY		0x08
#define SUGGEST_ABORT		0x09 
#define SUGGEST_REMAP		0x0a
#define SUGGEST_DIE		0x0b

#define DRIVER_SENSE		0x10

#define DRIVER_MASK 0x0f
#define SUGGEST_MASK 0xf0

/*

	SENSE KEYS
*/

#define NO_SENSE 		0x00
#define RECOVERED_ERROR		0x01
#define NOT_READY		0x02
#define MEDIUM_ERROR		0x03
#define	HARDWARE_ERROR		0x04
#define ILLEGAL_REQUEST		0x05
#define UNIT_ATTENTION		0x06
#define DATA_PROTECT		0x07
#define BLANK_CHECK		0x08
#define COPY_ABORTED		0x0a
#define ABORTED_COMMAND		0x0b
#define	VOLUME_OVERFLOW		0x0d
#define MISCOMPARE		0x0e


/*
	DEVICE TYPES

*/

#define TYPE_DISK	0x00
#define TYPE_TAPE	0x01
#define TYPE_WORM	0x04	/* Treated as ROM by our system */
#define TYPE_ROM	0x05
#define TYPE_NO_LUN	0x7f
/*
	Every SCSI command starts with a one byte OP-code.
	The next byte's high three bits are the LUN of the
	device.  Any multi-byte quantities are stored high byte
	first, and may have a 5 bit MSB in the same byte
	as the LUN.
*/


/*
	The scsi_device struct contains what we know about each given scsi
	device.
*/

typedef struct scsi_device {
	unsigned char host_no, id, lun;
	int access_count;	/* Count of open channels/mounts */
	unsigned writeable:1;
	unsigned removable:1; 
	unsigned random:1;
	unsigned changed:1;	/* Data invalid due to media change */
	unsigned busy:1;	/* Used to prevent races */
} Scsi_Device;
/*
	Use these to separate status msg and our bytes
*/

#define status_byte(result) (((result) >> 1) & 0xf)
#define msg_byte(result) (((result) >> 8) & 0xff)
#define host_byte(result) (((result) >> 16) & 0xff)
#define driver_byte(result) (((result) >> 24) & 0xff)
#define sugestion(result) (driver_byte(result) & SUGGEST_MASK)

#define sense_class(sense) (((sense) >> 4) & 0x7)
#define sense_error(sense) ((sense) & 0xf)
#define sense_valid(sense) ((sense) & 0x80);

/*
	These are the SCSI devices available on the system.
*/

#define MAX_SCSI_DEVICE 4
extern int NR_SCSI_DEVICES;
extern Scsi_Device scsi_devices[MAX_SCSI_DEVICE];
/*
	scsi_abort aborts the current command that is executing on host host.
	The error code, if non zero is returned in the host byte, otherwise 
	DID_ABORT is returned in the hostbyte.
*/

extern int scsi_abort (int host, int code);

/*
	Initializes all SCSI devices.  This scans all scsi busses.
*/

extern unsigned long scsi_dev_init (unsigned long, unsigned long);

/*
	You guesed it.  This sends a command to the selected SCSI host 

extern void print_inquiry(unsigned char *data);

*/



extern void scsi_do_cmd (int host,  unsigned char target, const void *cmnd ,
                  void *buffer, unsigned bufflen, void (*done)(int,int),
                  int timeout, unsigned  char *sense_buffer, int retries);

extern int scsi_reset (int host);
#endif
