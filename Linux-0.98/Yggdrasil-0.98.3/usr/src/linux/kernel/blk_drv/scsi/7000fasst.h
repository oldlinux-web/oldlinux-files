#ifndef _WD7000FASST_H

/* $Id: 7000fasst.h,v 1.1 1992/07/24 06:27:38 root Exp root $
 *
 * Header file for the WD 7000-FASST driver for Linux
 *
 * $Log: 7000fasst.h,v $
 * Revision 1.1  1992/07/24  06:27:38  root
 * Initial revision
 *
 * Revision 1.1  1992/07/05  08:32:32  root
 * Initial revision
 *
 * Revision 1.1  1992/05/15  18:38:05  root
 * Initial revision
 *
 * Revision 1.1  1992/04/02  03:23:13  drew
 * Initial revision
 *
 * Revision 1.3  1992/01/27  14:46:29  tthorn
 * *** empty log message ***
 *
 */

#include <linux/types.h>

#undef STATMASK
#undef CONTROL

#define io_base		0x350
#define intr_chan	15
#define dma_chan	6
#define OGMB_CNT	8
#define ICMB_CNT	8

/* I/O Port interface 4.2 */
/* READ */
#define ASC_STAT io_base
#define INT_IM	0x80		/* Interrupt Image Flag */
#define CMD_RDY	0x40		/* Command Port Ready */
#define CMD_REJ	0x20		/* Command Port Byte Rejected */
#define ASC_INI	0x10		/* ASC Initialized Flag */
#define STATMASK 0xf0		/* The lower 4 Bytes are reserved */

/* This register saves two purposes
 * Diagnostics error
 * Interrupt Status
 */
#define INTR_STAT ASC_STAT+1
#define ANYINTR	0x80		/* Mailbox Service possible/required */
#define IMB	0x40		/* 1 Incoming / 0 Outgoing */
#define MBMASK 0x3f
/* if MSB is zero, the content of the lower ones keeps Diagnostic State *
 * 00	Power-on, no diagnostics executed
 * 01	No diagnostic Error Occured
 * 02	RAM Failed
 * 03	FIFO R/W Failed
 * ...
*/

/* WRITE */
#define COMMAND ASC_STAT

#define INTR_ACK ASC_STAT+1


#define CONTROL ASC_STAT+2
#define INT_EN	0x08		/* Interrupt Enable	*/
#define DMA_EN	0x04		/* DMA Enable		*/
#define SCSI_RES	0x02	/* SCSI Reset		*/
#define ASC_RES	0x01		/* ASC Reset		*/

/* The DMA-Controller */
#define DMA_MODE_REG	0xd6
#define DMA_MASK_REG	0xd4
#define S_DMA_MSK	0x04
#define DMA_CH		0x02
#define	CASCADE		0xc0

/* Mailbox Definition 5.3 */

/* These belong in scsi.h also */
#undef any2scsi
#define any2scsi(up, p)			\
(up)[0] = (((long)(p)) >> 16);	\
(up)[1] = ((long)(p)) >> 8;		\
(up)[2] = ((long)(p));

#define scsi2int(up) ( (((long)*(up)) << 16) + (((long)(up)[1]) << 8) + ((long)(up)[2]) )

#define xany2scsi(up, p)	\
(up)[0] = ((long)(p)) >> 24;	\
(up)[1] = ((long)(p)) >> 16;	\
(up)[2] = ((long)(p)) >> 8;	\
(up)[3] = ((long)(p));

#define xscsi2int(up) ( (((long)(up)[0]) << 24) + (((long)(up)[1]) << 16) \
		      + (((long)(up)[2]) <<  8) +  ((long)(up)[3]) )

#define MAX_CDB 12
#define MAX_SENSE 14

struct scb {			/* Command Control Block 5.4.1 */
  unchar op;			/* Command Control Block Operation Code */
  unchar idlun;			/* op=0,2:Target Id, op=1:Initiator Id */
				/* Outbound data transfer, length is checked*/
				/* Inbound data transfer, length is checked */
				/* Logical Unit Number */
  unchar scbdata[12];		/* SCSI Command Block */
  unchar sretstat;		/* SCSI Return Status */
  unchar vue;			/* Vendor Unique Error Code */
  unchar maxdata[3];		/* Maximum Data Transfer Length */
  unchar dataptr[3];		/* SCSI Data Block Pointer */
  unchar linkptr[3];		/* Next Command Link Pointer */
  unchar direc;			/* Transfer Direction */
  unchar reserved2[6];		/* SCSI Command Descriptor Block */
				/* REQUEST SENSE */
};

int wd7000fasst_detect(int);
int wd7000fasst_command(unsigned char target, const void *cmnd, void *buff, int bufflen);
int wd7000fasst_queuecommand(unchar target, const void *cmnd, void *buff, int bufflen, void (*done)(int,int));
int wd7000fasst_abort(int);
char *wd7000fasst_info(void);
int wd7000fasst_reset(void);

#ifndef NULL
	#define NULL 0
#endif

#define WD7000FASST {"Western Digital 7000FASST", wd7000fasst_detect,	\
		wd7000fasst_info, wd7000fasst_command,		\
		wd7000fasst_queuecommand,		\
		wd7000fasst_abort,				\
		wd7000fasst_reset,				\
		1, 7, 0, 1}
#endif
