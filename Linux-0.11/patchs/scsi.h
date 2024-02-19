
#ifndef __SCSI_H__
	#define __SCSI_H__
/*
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
#define SEEK			0x0b
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
#define MESAGE_REJECT		0x07
#define NOP			0x08
#define MSG_PARITY_ERROR	0x09
#define LINKED_CMD_COMPLETE	0x0a
#define LINKED_FLG_CMD_COMPLETE	0x0b
#define BUS_DEVICE_RESET	0x0c
#define IDENTIFY		0x80
 
/*
        Our errors returned by OUR driver, NOT SCSI message.  Orr'd with
        SCSI message passed back to driver <IF any>.
*/
 
/* 	NO error							*/
#define DID_OK 			0x0000
/* 	Couldn't connect before timeout period				*/
#define DID_NO_CONNECT		0x0100
/*	BUS stayed busy through time out period				*/
#define DID_BUS_BUSY		0x0200
/*	TIMED OUT for other reason					*/
#define DID_TIME_OUT		0x0300
/*	ERROR from TARGET						*/
#define DID_TERROR		0x0400
/*	TARGET was busy							*/
#define DID_TBUSY		0x0500
/*	TARGET disconnected prematurely					*/
#define DID_TDISCONNECT		0x0600
/*	TARGET was off line						*/
#define DID_TOFFLINE		0x0700
/*      TARGET wants US to send IT a message				*/
#defibe DID_TREQ_MSG_OUT	0x0800
/*	TARGET parity error						*/
#define DID_TPARITY		0x0900
/*	TARGET requested reselect					*/
#define DID_TRESELECT		0x0A00
/*	TARGET was not in the range 0-6 inlclusive			*/
#define DID_BAD_TARGET		0x0B00
 
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
 
 
#endif
