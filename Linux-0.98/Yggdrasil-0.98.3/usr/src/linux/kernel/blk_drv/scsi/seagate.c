/*
 *	seagate.c Copyright (C) 1992 Drew Eckhardt 
 *	low level scsi driver for ST01/ST02 by
 *		Drew Eckhardt 
 *
 *	<drew@colorado.edu>
 */

#include <linux/config.h>

#if defined(CONFIG_SCSI_SEAGATE) || defined(CONFIG_SCSI_FD_88x) 
#include <asm/io.h>
#include <asm/system.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include "seagate.h"
#include "scsi.h"
#include "hosts.h"


static int internal_command(unsigned char target, const void *cmnd,
			 void *buff, int bufflen, int reselect);

static int incommand;			/*
						set if arbitration has finished and we are 
						in some command phase.
					*/

static void *base_address = NULL;	/*
						Where the card ROM starts,
						used to calculate memory mapped
						register location.
					*/
static volatile int abort_confirm = 0;

static volatile void *st0x_cr_sr;       /*
						control register write,
						status register read.
						256 bytes in length.

						Read is status of SCSI BUS,
						as per STAT masks.

					*/


static volatile void *st0x_dr;         /*
						data register, read write
						256 bytes in length.
					*/


static volatile int st0x_aborted=0;	/* 
						set when we are aborted, ie by a time out, etc.
					*/

					/*
						In theory, we have a nice auto
						detect routine - but this 
						overides it. 
					*/

			
#define retcode(result) (((result) << 16) | (message << 8) | status) 			
#define STATUS (*(unsigned char *) st0x_cr_sr)
#define CONTROL STATUS 
#define DATA (*(unsigned char *) st0x_dr)

#ifndef OVERRIDE		
static const char *  seagate_bases[] = {(char *) 0xc8000, (char *) 0xca000, (char *) 0xcc000, (char *) 0xce000, (char *) 0xce000,
				        (char *) 0xdc000, (char *) 0xde000};
typedef struct 
	{
	char *signature ;
	unsigned offset;
	unsigned length;
	} Signature;
	
static const Signature signatures[] = {
#ifdef CONFIG_SCSI_SEAGATE
{"SCSI BIOS 2.00  (C) Copyright 1987 Seagate", 15, 40},

/*
	The following two lines are NOT mistakes.  One detects 
 	ROM revision 3.0.0, the other 3.2.  Since seagate
	has only one type of SCSI adapter, and this is not 
	going to change, the "SEAGATE" and "SCSI" together
	are probably "good enough"
*/

{"SEAGATE SCSI BIOS ",16, 17},
{"SEAGATE SCSI BIOS ",17, 17},
#endif

/*
	This is for the Future Domain 88x series.  I've been told that
	the Seagate controllers are just repackages of these, and seeing
	early seagate BIOS bearing the Future Domain copyright,
	I believe it.
*/

#ifdef CONFIG_SCSI_FD_88x
{"FUTURE DOMAIN CORP. (C) 1986-1989 V6.0A7/28/90", 5, 46},
#endif
}
;
/*
	Note that the last signature handles BIOS revisions 3.0.0 and 
	3.2 - the real ID's are 

SEAGATE SCSI BIOS REVISION 3.0.0
SEAGATE SCSI BIOS REVISION 3.2

*/

#define NUM_SIGNATURES (sizeof(signatures) / sizeof(Signature))
#endif

/*
 * hostno stores the hostnumber, as told to us by the init routine.
 */

static int hostno = -1;
static void seagate_reconnect_intr(int);

int seagate_st0x_detect (int hostnum)
	{
#ifndef OVERRIDE
	int i,j;
#endif
static struct sigaction seagate_sigaction = {
	&seagate_reconnect_intr,
	0,
	SA_INTERRUPT,
	NULL
};

/*
 *	First, we try for the manual override.
 */
#ifdef DEBUG 
	printk("Autodetecting seagate ST0x\n");
#endif
	
	if (hostno != -1)
		{
		printk ("ERROR : seagate_st0x_detect() called twice.\n");
		return 0;
		}
	
	base_address = NULL;
#ifdef OVERRIDE
	base_address = (void *) OVERRIDE;	
#ifdef DEBUG
	printk("Base address overridden to %x\n", base_address);
#endif
#else	
/*
 *	To detect this card, we simply look for the signature
 *	from the BIOS version notice in all the possible locations
 *	of the ROM's.  This has a nice sideeffect of not trashing
 * 	any register locations that might be used by something else.
 */

	for (i = 0; i < (sizeof (seagate_bases) / sizeof (char  * )); ++i)
		for (j = 0; !base_address && j < NUM_SIGNATURES; ++j)
		if (!memcmp ((void *) (seagate_bases[i] +
		    signatures[j].offset), (void *) signatures[j].signature,
		    signatures[j].length))
			base_address = (void *) seagate_bases[i];
 #endif
 
	if (base_address)
		{
		st0x_cr_sr =(void *) (((unsigned char *) base_address) + 0x1a00); 
		st0x_dr = (void *) (((unsigned char *) base_address )+ 0x1c00);
#ifdef DEBUG
		printk("ST0x detected. Base address = %x, cr = %x, dr = %x\n", base_address, st0x_cr_sr, st0x_dr);
#endif
/*
 *	At all times, we will use IRQ 5.  
 */
		hostno = hostnum;
		if (irqaction(5, &seagate_sigaction)) {
			printk("Unable to allocate IRQ5 for ST0x driver\n");
			return 0;
		}
		return -1;
		}
	else
		{
#ifdef DEBUG
		printk("ST0x not detected.\n");
#endif
		return 0;
		}
	}
	 
char *seagate_st0x_info(void)
{
	static char buffer[] = "Seagate ST-0X SCSI driver by Drew Eckhardt \n"
"$Header: /usr/src/linux/kernel/blk_drv/scsi/RCS/seagate.c,v 1.1 1992/07/24 06:27:38 root Exp root $\n";
	return buffer;
}

/*
 * These are our saved pointers for the outstanding command that is 
 * waiting for a reconnect
 */

static unsigned char current_target;
static unsigned char *current_cmnd, *current_data;
static int current_bufflen;
static void (*done_fn)(int, int) = NULL;

/*
 * These control whether or not disconnect / reconnect will be attempted,
 * or are being attempted.
 */

#define NO_RECONNECT 	0
#define RECONNECT_NOW 	1
#define CAN_RECONNECT	2

/*
 * This determines if we are expecting to reconnect or not.
 */

static int should_reconnect = 0;

/*
 * The seagate_reconnect_intr routine is called when a target reselects the 
 * host adapter.  This occurs on the interrupt triggered by the target 
 * asserting SEL.
 */

static void seagate_reconnect_intr (int unused)
	{
	int temp;

/* enable all other interrupts. */	
	sti();
#if (DEBUG & PHASE_RESELECT)
	printk("scsi%d : seagate_reconnect_intr() called\n", hostno);
#endif

	if (!should_reconnect)
	    printk("scsi%d: unexpected interrupt.\n", hostno);
	else
		{
		should_reconnect = 0;

#if (DEBUG & PHASE_RESELECT)
		printk("scsi%d : internal_command("
		       "%d, %08x, %08x, %d, RECONNECT_NOW\n", hostno, 
			current_target, current_data, current_bufflen);
#endif
	
		temp =  internal_command (current_target, 
			current_cmnd, current_data, current_bufflen,
			RECONNECT_NOW);

		if (msg_byte(temp) != DISCONNECT)
			{
			if (done_fn)
				{
#if (DEBUG & PHASE_RESELECT)
				printk("scsi%d : done_fn(%d,%08x)", hostno, 
				hostno, temp);
#endif
				done_fn (hostno, temp);
				}
			else
				printk("done_fn() not defined.\n");
			}
		}
	} 

/* 
 * The seagate_st0x_queue_command() function provides a queued interface
 * to the seagate SCSI driver.  Basically, it just passes control onto the
 * seagate_command() function, after fixing it so that the done_fn()
 * is set to the one passed to the function.
 */

int seagate_st0x_queue_command (unsigned char target, const void *cmnd,
				void *buff, int bufflen, void (*fn)(int, 
				 int))
	{
	int result;

	done_fn = fn;
	current_target = target;
	(const void *) current_cmnd = cmnd;
	current_data = buff;
	current_bufflen = bufflen;

	result = internal_command (target, cmnd, buff, bufflen, 
				   CAN_RECONNECT);
	if (msg_byte(result) == DISCONNECT)
		return 0;
	else 
		{
		done_fn (hostno, result); 
		return 1; 
		}
	}

int seagate_st0x_command (unsigned char target, const void *cmnd, 
			void *buff, int bufflen)
	{
	return internal_command (target, cmnd, buff, bufflen, 
				 (int) NO_RECONNECT);
	}
	
static int internal_command(unsigned char target, const void *cmnd,
			 void *buff, int bufflen, int reselect)
	{
	int len;			
	unsigned char *data;	
	int clock;			
	int temp;


#if ((DEBUG & PHASE_ETC) || (DEBUG & PRINT_COMMAND) || (DEBUG & PHASE_EXIT))	
	int i;
#endif

#if (DEBUG & PHASE_ETC)
	int phase=0, newphase;
#endif

	int done = 0;
	unsigned char status = 0;	
	unsigned char message = 0;
	register unsigned char status_read;

	len=bufflen;
	data=(unsigned char *) buff;

	incommand = 0;
	st0x_aborted = 0;

#if (DEBUG & PRINT_COMMAND)
	printk ("scsi%d : target = %d, command = ", hostno, target);
	for (i = 0; i < COMMAND_SIZE(((unsigned char *)cmnd)[0]); ++i)
		printk("%02x ",  ((unsigned char *) cmnd)[i]);
	printk("\n");
#endif

#if (DEBUG & PHASE_RESELECT)
	switch (reselect)
	{
	case RECONNECT_NOW :
		printk("scsi%d : reconnecting\n", hostno);
		break;
	case CAN_RECONNECT :
		printk("scsi%d : allowed to reconnect\n", hostno);
		break;
	default :
		printk("scsi%d : not allowed to reconnect\n", hostno);
	}
#endif
	

	if (target > 6)
		return DID_BAD_TARGET;

/*
 *	We work it differently depending on if this is is "the first time,"
 *	or a reconnect.  If this is a reselct phase, then SEL will 
 *	be asserted, and we must skip selection / arbitration phases.
 */

	if (reselect == RECONNECT_NOW) 
		{
#if (DEBUG & PHASE_RESELECT)
		printk("scsi%d : phase RESELECT \n", hostno);
#endif

/*
 *	At this point, we should find the logical or of our ID and the original
 *	target's ID on the BUS, with BSY, SEL, and I/O signals asserted.
 *
 *	After ARBITRATION phase is completed, only SEL, BSY, and the 
 *	target ID are asserted.  A valid initator ID is not on the bus
 *	until IO is asserted, so we must wait for that.
 */
		
		for (clock = jiffies + 10, temp = 0; (jiffies < clock) &&
		     !(STATUS & STAT_IO););
		
		if (jiffies >= clock)
			{
#if (DEBUG & PHASE_RESELECT)
			printk("scsi%d : RESELECT timed out while waiting for IO .\n",
				hostno);
#endif
			return (DID_BAD_INTR << 16);
			}

/* 
 * 	After I/O is asserted by the target, we can read our ID and its
 *	ID off of the BUS.
 */
 
		if (!((temp = DATA) & 0x80))
			{
#if (DEBUG & PHASE_RESELECT)
			printk("scsi%d : detected reconnect request to different target.\n" 
			       "\tData bus = %d\n", hostno, temp);
#endif
			return (DID_BAD_INTR << 16);
			}

		if (!(temp & (1 << current_target)))
			{
			printk("scsi%d : Unexpected reselect interrupt.  Data bus = %d\n",
				hostno, temp);
			return (DID_BAD_INTR << 16);
			}
                data=current_data;      /* WDE add */
                cmnd=current_cmnd;      /* WDE add */
                len=current_bufflen;    /* WDE add */

/*
 * 	We have determined that we have been selected.  At this point, 
 *	we must respond to the reselection by asserting BSY ourselves
 */

		CONTROL = (BASE_CMD | CMD_DRVR_ENABLE | CMD_BSY);

/*
 *	The target will drop SEL, and raise BSY, at which time we must drop
 *	BSY.
 */

		for (clock = jiffies + 10; (jiffies < clock) &&  (STATUS & STAT_SEL););

		if (jiffies >= clock)
			{ 
			CONTROL = (BASE_CMD | CMD_INTR);
#if (DEBUG & PHASE_RESELECT)
			printk("scsi%d : RESELECT timed out while waiting for SEL.\n",
				hostno);
#endif
			return (DID_BAD_INTR << 16);				 
			}

		CONTROL = BASE_CMD;

/*
 *	At this point, we have connected with the target and can get 
 *	on with our lives.
 */	 
		}  	
	else
 		{	
#if (DEBUG & PHASE_BUS_FREE)
		printk ("scsi%d : phase = BUS FREE \n", hostno);
#endif

/*
 *	BUS FREE PHASE
 *
 * 	On entry, we make sure that the BUS is in a BUS FREE
 *	phase, by insuring that both BSY and SEL are low for
 *	at least one bus settle delay.  Several reads help
 *	eliminate wire glitch.
 */

		clock = jiffies + ST0X_BUS_FREE_DELAY;	

		while (((STATUS |  STATUS | STATUS) & 
		         (STAT_BSY | STAT_SEL)) && 
			 (!st0x_aborted) && (jiffies < clock));

		if (jiffies > clock)
			return retcode(DID_BUS_BUSY);
		else if (st0x_aborted)
			return retcode(st0x_aborted);

/*
 *	Bus free has been detected, within BUS settle.  I used to 
 *	support an arbitration phase - however, on the Seagate, this 
 *	degraded performance by a factor > 10 - so it is no more.
 */

/*
 *	SELECTION PHASE
 *
 *	Now, we select the disk, giving it the SCSI ID at data
 *	and a command of PARITY if necessary, and we raise SEL.
 */

#if (DEBUG & PHASE_SELECTION)
		printk("scsi%d : phase = SELECTION\n", hostno);
#endif

		clock = jiffies + ST0X_SELECTION_DELAY;

/*
 *	If we wish to disconnect, we should request a MESSAGE OUT
 *	at this point.  Technically, ATTN should be raised before 
 *	SEL = true and BSY = false (from arbitration), but I think this 
 *	should do.
 */
		if (reselect)
			CONTROL = BASE_CMD | CMD_DRVR_ENABLE |
				CMD_ATTN;
		
/*
 *	We must assert both our ID and our target's ID on the bus.
 */
		DATA = (unsigned char) ((1 << target) | 0x80);

/*
 *	If we are allowing ourselves to reconnect, then I will keep 
 *	ATTN raised so we get MSG OUT. 
 */ 
		CONTROL =  BASE_CMD | CMD_DRVR_ENABLE | CMD_SEL | 
			(reselect ? CMD_ATTN : 0);

/*
 *	When the SCSI device decides that we're gawking at it, it will 
 *	respond by asserting BUSY on the bus.
 */
		while (!((status_read = STATUS) & STAT_BSY) && 
			(jiffies < clock) && !st0x_aborted)

#if (DEBUG & PHASE_SELECTION)
		{
		temp = clock - jiffies;

		if (!(jiffies % 5))
			printk("seagate_st0x_timeout : %d            \r",temp);
	
		}
		printk("Done.                                             \n\r");
		printk("scsi%d : status = %02x, seagate_st0x_timeout = %d, aborted = %02x \n", 
			hostno, status_read, temp, st0x_aborted);
#else
		;
#endif
	

		if ((jiffies > clock)  || (!st0x_aborted && 
			!(status_read & STAT_BSY)))
			{
#if (DEBUG & PHASE_SELECT)
			printk ("scsi%d : NO CONNECT with target %d, status = %x \n", 
				hostno, target, STATUS);
#endif
			return retcode(DID_NO_CONNECT);
			}

/*
 *	If we have been aborted, and we have a command in progress, IE the 
 *	target still has BSY asserted, then we will reset the bus, and 
 * 	notify the midlevel driver to expect sense.
 */

		if (st0x_aborted)
			{
			CONTROL = BASE_CMD;
			if (STATUS & STAT_BSY)
				{
				seagate_st0x_reset();
				return retcode(DID_RESET);
				}
			return retcode(st0x_aborted);
			}	
		}

	CONTROL = BASE_CMD | CMD_DRVR_ENABLE | 
		((reselect == CAN_RECONNECT) ? CMD_ATTN : 0) ;
	
/*
 * 	INFORMATION TRANSFER PHASE
 *
 *	The nasty looking read / write inline assembler loops we use for 
 *	DATAIN and DATAOUT phases are approximately 4-5 times as fast as 
 *	the 'C' versions - since we're moving 1024 bytes of data, this
 *	really adds up.
 */

#if (DEBUG & PHASE_ETC)
	printk("scsi%d : phase = INFORMATION TRANSFER\n", hostno);
#endif  

	incommand = 1;


/*
 * 	Now, we poll the device for status information,
 *	and handle any requests it makes.  Note that since we are unsure of 
 *	how much data will be flowing across the system, etc and cannot 
 *	make reasonable timeouts, that we will instead have the midlevel
 * 	driver handle any timeouts that occur in this phase.
 */

	while (((status_read = STATUS) & STAT_BSY) && !st0x_aborted && !done) 
		{
#ifdef PARITY
		if (status_read & STAT_PARITY)
			{
			done = 1;
			st0x_aborted = DID_PARITY;
			}	
#endif

		if (status_read & STAT_REQ)
			{
#if (DEBUG & PHASE_ETC)
			if ((newphase = (status_read & REQ_MASK)) != phase)
				{
				phase = newphase;
				switch (phase)
				{
				case REQ_DATAOUT: 
					printk("scsi%d : phase = DATA OUT\n",
						hostno); 
					break;
				case REQ_DATAIN : 
					printk("scsi%d : phase = DATA IN\n",
						hostno); 
					break;
				case REQ_CMDOUT : 
					printk("scsi%d : phase = COMMAND OUT\n",
						hostno); 
					break;
				case REQ_STATIN :
					 printk("scsi%d : phase = STATUS IN\n",
						hostno); 
					break;
				case REQ_MSGOUT :
					printk("scsi%d : phase = MESSAGE OUT\n",
						hostno); 
					break;
				case REQ_MSGIN :
					printk("scsi%d : phase = MESSAGE IN\n",
						hostno);
					break;
				default : 
					printk("scsi%d : phase = UNKNOWN\n",
						hostno); 
					st0x_aborted = 1; 
					done = 1;
				}	
				}
#endif

		switch (status_read & REQ_MASK)
		{			
		case REQ_DATAOUT : 

/*
 * 	We loop as long as we are in a data out phase, there is data to send, 
 *	and BSY is still active.
 */
		__asm__ ("

/*
	Local variables : 
	len = ecx
	data = esi
	st0x_cr_sr = ebx
	st0x_dr =  edi

	Test for any data here at all.
*/
	movl %0, %%esi		/* local value of data */
	movl %1, %%ecx		/* local value of len */	
	orl %%ecx, %%ecx
	jz 2f

	cld

	movl _st0x_cr_sr, %%ebx
	movl _st0x_dr, %%edi
	
1:	movb (%%ebx), %%al
/*
	Test for BSY
*/

	test $1, %%al 
	jz 2f

/*
	Test for data out phase - STATUS & REQ_MASK should be REQ_DATAOUT, which is 0.
*/
	test $0xe, %%al
	jnz 2f	
/*
	Test for REQ
*/	
	test $0x10, %%al
	jz 1b
	lodsb
	movb %%al, (%%edi) 
	loop 1b

2: 
	movl %%esi, %2
	movl %%ecx, %3
									":
/* output */
"=r" (data), "=r" (len) :
/* input */
"0" (data), "1" (len) :
/* clobbered */
"ebx", "ecx", "edi", "esi"); 

			break;

		case REQ_DATAIN : 
/*
 * 	We loop as long as we are in a data in phase, there is room to read, 
 * 	and BSY is still active
 */
 
			__asm__ ("
/*
	Local variables : 
	ecx = len
	edi = data
	esi = st0x_cr_sr
	ebx = st0x_dr

	Test for room to read
*/

	movl %0, %%edi		/* data */
	movl %1, %%ecx		/* len */
	orl %%ecx, %%ecx
	jz 2f

	cld
	movl _st0x_cr_sr, %%esi
	movl _st0x_dr, %%ebx

1:	movb (%%esi), %%al
/*
	Test for BSY
*/

	test $1, %%al 
	jz 2f

/*
	Test for data in phase - STATUS & REQ_MASK should be REQ_DATAIN, = STAT_IO, which is 4.
*/
	movb $0xe, %%ah	
	andb %%al, %%ah
	cmpb $0x04, %%ah
	jne 2f
		
/*
	Test for REQ
*/	
	test $0x10, %%al
	jz 1b

	movb (%%ebx), %%al	
	stosb	
	loop 1b

2: 	movl %%edi, %2	 	/* data */
	movl %%ecx, %3 		/* len */
									":
/* output */
"=r" (data), "=r" (len) :
/* input */
"0" (data), "1" (len) :
/* clobbered */
"ebx", "ecx", "edi", "esi"); 
			break;

		case REQ_CMDOUT : 
			while (((status_read = STATUS) & STAT_BSY) && 
			       ((status_read & REQ_MASK) == REQ_CMDOUT))
				if (status_read & STAT_REQ)
					DATA = *(unsigned char *) cmnd ++;
			break;
	
		case REQ_STATIN : 
			status = DATA;
			break;
				
		case REQ_MSGOUT : 
/*
 *	We can only have sent a MSG OUT if we requested to do this 
 *	by raising ATTN.  So, we must drop ATTN.
 */

			CONTROL = BASE_CMD | CMD_DRVR_ENABLE;
/*
 * 	If we are reconecting, then we must send an IDENTIFY message in 
 *	 response  to MSGOUT.
 */
			if (reselect)
				{
				DATA = IDENTIFY(1,0);
#if (DEBUG & (PHASE_RESELECT | PHASE_MSGOUT)) 
				printk("scsi%d : sent IDENTIFY message.\n", hostno);
#endif
				}
			else  
				{
				DATA = MESSAGE_REJECT;
				
#if (DEBUG & PHASE_MSGOUT)
				printk("scsi%d : sent MESSAGE REJECT message.\n", hostno);
#endif
				}
			break;
					
		case REQ_MSGIN : 
			switch (message = DATA)
			{
			case DISCONNECT :
				should_reconnect = 1;
                                current_data = data;    /* WDE add */
                                current_bufflen = len;  /* WDE add */
#if (DEBUG & (PHASE_RESELECT | PHASE_MSGIN))
				printk("scsi%d : disconnected.\n", hostno);
				done=1;
				break;
#endif
			case COMMAND_COMPLETE :
#if (DEBUG & PHASE_MSGIN)	
				printk("scsi%d : command complete.\n", hostno);
				done=1;
				break;
#endif
			case ABORT :
#if (DEBUG & PHASE_MSGIN)
				printk("scsi%d : abort message.\n", hostno);
#endif
				done=1;
				break;
			case SAVE_POINTERS :
				current_data = data;	/* WDE mod */
                                current_bufflen = len;  /* WDE add */
#if (DEBUG & PHASE_MSGIN)
				printk("scsi%d : pointers saved.\n", hostno);
#endif 
				break;
			case RESTORE_POINTERS:
				data=current_data;	/* WDE mod */
				cmnd=current_cmnd;
#if (DEBUG & PHASE_MSGIN)
				printk("scsi%d : pointers restored.\n", hostno);
#endif
				break;
			default:

/*
 * 	IDENTIFY distinguishes itself from the other messages by setting the
 *	high byte.
 */

				if (message & 0x80)
					{
#if (DEBUG & PHASE_MSGIN)
					printk("scsi%d : IDENTIFY message received from id %d, lun %d.\n",
						hostno, target, message & 7);
#endif
					}	
				else
					{
			
#if (DEBUG & PHASE_MSGIN)
					printk("scsi%d : unknown message %d from target %d.\n",
						hostno,  message,   target);
#endif	
					}
			}
			break;

		default : 
			printk("scsi%d : unknown phase.\n", hostno); 
			st0x_aborted = DID_ERROR; 
		}	
		} /* while ends */
		} /* if ends */

#if (DEBUG & (PHASE_DATAIN | PHASE_DATAOUT | PHASE_EXIT))
	printk("Transfered %d bytes, allowed %d additional bytes\n", (bufflen - len), len);
#endif

#if (DEBUG & PHASE_EXIT)
	printk("Buffer : \n");
	for (i = 0; i < 20; ++i) 
		printk ("%02x  ", ((unsigned char *) data)[i]);	/* WDE mod */
	printk("\n");
	printk("Status = %02x, message = %02x\n", status, message);
#endif

	
	if (st0x_aborted)
		{
		if (STATUS & STAT_BSY)
			{
			seagate_st0x_reset();
			st0x_aborted = DID_RESET;
			}
		abort_confirm = 1;
		}		

	if (should_reconnect)
		{
#if (DEBUG & PHASE_RESELECT)
		printk("scsi%d : exiting seagate_st0x_queue_command() with reconnect enabled.\n",
			hostno);
#endif
		CONTROL = BASE_CMD | CMD_INTR ;
		}
	else
		CONTROL = BASE_CMD;

	return retcode (st0x_aborted);
	}

int seagate_st0x_abort (int code)
	{
	if (code)
		st0x_aborted = code;
	else
		st0x_aborted = DID_ABORT;

		return 0;
	}

/*
	the seagate_st0x_reset function resets the SCSI bus
*/
	
int seagate_st0x_reset (void)
	{
	unsigned clock;
	/*
		No timeouts - this command is going to fail because 
		it was reset.
	*/

#ifdef DEBUG
	printk("In seagate_st0x_reset()\n");
#endif


	/* assert  RESET signal on SCSI bus.  */
		
	CONTROL = BASE_CMD  | CMD_RST;
	clock=jiffies+2;

	
	/* Wait.  */
	
	while (jiffies < clock);

	CONTROL = BASE_CMD;
	
	st0x_aborted = DID_RESET;

#ifdef DEBUG
	printk("SCSI bus reset.\n");
#endif
	return 0;
	}

#endif	

