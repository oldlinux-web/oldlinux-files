/*
 *	scsi.c Copyright (C) 1992 Drew Eckhardt 
 *	generic mid-level SCSI driver by
 *		Drew Eckhardt 
 *
 *	<drew@colorado.edu>
 *
 *	Bug correction thanks go to : 
 *		Rik Faith <faith@cs.unc.edu>
 *		Tommy Thorn <tthorn>
 *		Thomas Wuensche <tw@fgb1.fgb.mw.tu-meunchen.de>
 */

#include <linux/config.h>

#ifdef CONFIG_SCSI
#include <asm/system.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/string.h>

#include "scsi.h"
#include "hosts.h"

#ifdef CONFIG_BLK_DEV_SD
#include "sd.h"
#endif

#ifdef CONFIG_BLK_DEV_ST
#include "st.h"
#endif

#ifdef CONFIG_BLK_DEV_SR
#include "sr.h"
#endif

/*
static const char RCSid[] = "$Header: /usr/src/linux/kernel/blk_drv/scsi/RCS/scsi.c,v 1.1 1992/07/24 06:27:38 root Exp root $";
*/

#define INTERNAL_ERROR (printk ("Internal error in file %s, line %d.\n", __FILE__, __LINE__), panic(""))

static void scsi_done (int host, int result);
static void update_timeout (void);
static void print_inquiry(unsigned char *data);

static int time_start;
static int time_elapsed;

/*
	global variables : 
	NR_SCSI_DEVICES is the number of SCSI devices we have detected, 
	scsi_devices an array of these specifing the address for each 
	(host, id, LUN)
*/
	
int NR_SCSI_DEVICES=0;
Scsi_Device scsi_devices[MAX_SCSI_DEVICE];

#define SENSE_LENGTH 255
/*
 *	As the scsi do command functions are inteligent, and may need to 
 *	redo a command, we need to keep track of the last command 
 *	executed on each one.
 */

#define WAS_RESET 	0x01
#define WAS_TIMEDOUT 	0x02
#define WAS_SENSE	0x04
#define IS_RESETTING	0x08

static Scsi_Cmnd last_cmnd[MAX_SCSI_HOSTS];
static int last_reset[MAX_SCSI_HOSTS];

/*
 *	This is the number  of clock ticks we should wait before we time out 
 *	and abort the command.  This is for  where the scsi.c module generates 
 *	the command, not where it originates from a higher level, in which
 *	case the timeout is specified there.
 *
 *	ABORT_TIMEOUT and RESET_TIMEOUT are the timeouts for RESET and ABORT
 *	respectively.
 */

#ifdef DEBUG
	#define SCSI_TIMEOUT 500
#else
	#define SCSI_TIMEOUT 100
#endif

#ifdef DEBUG
	#define SENSE_TIMEOUT SCSI_TIMEOUT
	#define ABORT_TIMEOUT SCSI_TIMEOUT
	#define RESET_TIMEOUT SCSI_TIMEOUT
#else
	#define SENSE_TIMEOUT 50
	#define RESET_TIMEOUT 50
	#define ABORT_TIMEOUT 50
	#define MIN_RESET_DELAY 25
#endif

/*
 *	As the actual SCSI command runs in the background, we must set up a 
 *	flag that tells scan_scsis() when the result it has is valid.  
 *	scan_scsis can set the_result to -1, and watch for it to become the 
 *	actual return code for that call.  the scan_scsis_done function() is 
 *	our user specified completion function that is passed on to the  
 *	scsi_do_cmd() function.
 */

volatile static int in_scan = 0;
static int the_result;
static unsigned char sense_buffer[SENSE_LENGTH];
static void scan_scsis_done (int host, int result)
	{
	
#ifdef DEBUG
	printk ("scan_scsis_done(%d, %06x)\n\r", host, result);
#endif	
	the_result = result;
	}
/*
 *	Detecting SCSI devices :	
 *	We scan all present host adapter's busses,  from ID 0 to ID 6.  
 *	We use the INQUIRY command, determine device type, and pass the ID / 
 *	lun address of all sequential devices to the tape driver, all random 
 *	devices to the disk driver.
 */

static void scan_scsis (void)
	{
        int host_nr , dev, lun, type, maxed, slave;
	static unsigned char scsi_cmd [12];
	static unsigned char scsi_result [256];

	++in_scan;

        for (slave = host_nr = 0; host_nr < MAX_SCSI_HOSTS; ++host_nr, 
	     slave = 0)
                if (scsi_hosts[host_nr].present)
			{
			for (dev = 0; dev < 7; ++dev)
				if (scsi_hosts[host_nr].this_id != dev)
                                #ifdef MULTI_LUN
				for (lun = 0; lun < 8; ++lun)
					{
				#else
					{
					lun = 0;
				#endif
/*
 * Build an INQUIRY command block.  
 */

					scsi_cmd[0] = INQUIRY;
					scsi_cmd[1] = (lun << 5) & 0xe0;
					scsi_cmd[2] = 0;
					scsi_cmd[3] = 0;
					scsi_cmd[4] = 255;
					scsi_cmd[5] = 0;
					the_result = -1;	
#ifdef DEBUG
					memset ((void *) scsi_result , 0, 255);
#endif 
					scsi_do_cmd (host_nr, dev, (void *)  scsi_cmd, (void *) 							    
						 scsi_result, 256,  scan_scsis_done, 
						 SCSI_TIMEOUT, sense_buffer, 3);
					
/*
 * 	Wait for valid result 
 */

					while (the_result < 0);

					if (!the_result)
						{
						scsi_devices[NR_SCSI_DEVICES].
							host_no = host_nr;
						scsi_devices[NR_SCSI_DEVICES].
							id = dev;
						scsi_devices[NR_SCSI_DEVICES].
							lun = lun;
						scsi_devices[NR_SCSI_DEVICES].
							removable = (0x80 & 
							scsi_result[1]) >> 7;
						scsi_devices[NR_SCSI_DEVICES].
						        changed = 0;
						scsi_devices[NR_SCSI_DEVICES].
						        access_count = 0;
						scsi_devices[NR_SCSI_DEVICES].
						        busy = 0;
/* 
 *	Currently, all sequential devices are assumed to be tapes,
 *	all random devices disk, with the appropriate read only 
 *	flags set for ROM / WORM treated as RO.
 */ 

                                                switch (type = scsi_result[0])
                                                {
                                                case TYPE_TAPE :
                                                case TYPE_DISK :
                                                	scsi_devices[NR_SCSI_DEVICES].writeable = 1;
                                                     	break;
                                                case TYPE_WORM :
                                                case TYPE_ROM :
                                                	scsi_devices[NR_SCSI_DEVICES].writeable = 0;
                                                	break;
                                                default :
                                                	type = -1;
                                                }

                                                scsi_devices[NR_SCSI_DEVICES].random = (type == TYPE_TAPE) ? 0 : 1;

                                                maxed = 0;
                                                switch (type)
						{
                                                case -1 :
                                                	break;
                                                case TYPE_TAPE :
#ifdef DEBUG
							printk("Detected scsi tape at host %d, ID  %d, lun %d \n", host_nr, dev, lun);
#endif
#ifdef CONFIG_BLK_DEV_ST
                                                        if (!(maxed = (NR_ST == MAX_ST)))
                                                        	scsi_tapes[NR_ST].device = &scsi_devices[NR_SCSI_DEVICES];
#endif
							break;
							case TYPE_ROM:
								printk("Detected scsi CD-ROM at host %d, ID  %d, lun %d \n", host_nr, dev, lun);
#ifdef CONFIG_BLK_DEV_SR
                                                               	if (!(maxed = (NR_SR >= MAX_SR)))
										scsi_CDs[NR_SR].device = &scsi_devices[NR_SCSI_DEVICES];
#endif
								break;
                                                default :
#ifdef DEBUG
							printk("Detected scsi disk at host %d, ID  %d, lun %d \n", host_nr, dev, lun);
#endif
#ifdef CONFIG_BLK_DEV_SD
                                                        if (!(maxed = (NR_SD >= MAX_SD)))
								rscsi_disks[NR_SD].device = &scsi_devices[NR_SCSI_DEVICES];
#endif
						}

					        print_inquiry(scsi_result);

                                                if (maxed)
                                                	{
                                                                printk ("Already have detected "
									"maximum number of SCSI "
									"%ss Unable to \n"
                                                                        "add drive at SCSI host "
									"%s, ID %d, LUN %d\n\r", 
									(type == TYPE_TAPE) ?
                                                                             "tape" : 
									(type == TYPE_DISK) ?
									     "disk" : "CD-ROM", 
									scsi_hosts[host_nr].name,
                                                                dev, lun);
                                                        type = -1;
                                                        break;
                                                        }

                                                 else if (type != -1)
                                                        {
							char *p;
							char str[25]; 
memcpy((void *) str, (void *) &scsi_result[8], 8);
for (p = str; (p < (str  + 8)) && (*p != ' '); ++p);
*p++ = ' ';
memcpy((void *) p, (void *) &scsi_result[16], 16);
for (; *p != ' '; ++p);
*p = 0;

printk("s%c%d at scsi%d, id %d, lun %d : %s\n",
	(type == TYPE_TAPE) ? 't' : ((type == TYPE_ROM) ? 'r' : 'd'),
	(type == TYPE_TAPE) ? 
#ifdef CONFIG_BLK_DEV_ST
	NR_ST  
#else 
	-1
#endif
	: 
       (type == TYPE_ROM ? 
#ifdef CONFIG_BLK_DEV_SR
	NR_SR
#else
	-1	
#endif
	:
#ifdef CONFIG_BLK_DEV_SD
	NR_SD
#else
	-1	
#endif
	)

	,host_nr , dev, lun, p); 
                                                        if (type == TYPE_TAPE)
#ifdef CONFIG_BLK_DEV_ST
                                                        	++NR_ST;
#else
;
#endif

                                                  else if (type == TYPE_DISK)
#ifdef CONFIG_BLK_DEV_SD
                                                        	++NR_SD;
#else
;
#endif
								else
#ifdef CONFIG_BLK_DEV_SR
								        ++NR_SR;
#else
;
#endif
                                                                }
							++slave;
							++NR_SCSI_DEVICES;
                                                        }       /* if result == DID_OK ends */
                                        }       /* for lun ends */
                        }      	/* if present */  

	printk("scsi : detected "
#ifdef CONFIG_BLK_DEV_SD
	"%d SCSI disk%s "
#endif

#ifdef CONFIG_BLK_DEV_ST
	"%d tape%s "
#endif

#ifdef CONFIG_BLK_DEV_SR
"%d CD-ROM drive%s "
#endif

	"total.\n"  

#ifdef CONFIG_BLK_DEV_SD
	, NR_SD, (NR_SD != 1) ? "s" : ""
#endif

#ifdef CONFIG_BLK_DEV_ST
	, NR_ST, (NR_ST != 1) ? "s" : ""
#endif

#ifdef CONFIG_BLK_DEV_SR
        , NR_SR, (NR_SR != 1) ? "s" : ""
#endif
	);
	in_scan = 0;
        }       /* scan_scsis  ends */

/*
 *	We handle the timeout differently if it happens when a reset, 
 *	abort, etc are in process. 
 */

static unsigned char internal_timeout[MAX_SCSI_HOSTS];

/*
 *	Flag bits for the internal_timeout array 
 */

#define NORMAL_TIMEOUT 0
#define IN_ABORT 1
#define IN_RESET 2
/*
	This is our time out function, called when the timer expires for a 
	given host adapter.  It will attempt to abort the currently executing 
	command, that failing perform a kernel panic.
*/ 

static void scsi_times_out (int host)
	{
	
 	switch (internal_timeout[host] & (IN_ABORT | IN_RESET))
		{
		case NORMAL_TIMEOUT:
			if (!in_scan)
			      printk("SCSI host %d timed out - aborting command \r\n",
				host);
			
			if (!scsi_abort	(host, DID_TIME_OUT))
				return;				
		case IN_ABORT:
			printk("SCSI host %d abort() timed out - reseting \r\n",
				host);
			if (!scsi_reset (host)) 
				return;
		case IN_RESET:
		case (IN_ABORT | IN_RESET):
			printk("Unable to reset scsi host %d\r\n",host);
			panic("");
		default:
			INTERNAL_ERROR;
		}
					
	}

/*
	This is inline because we have stack problemes if we recurse to deeply.
*/
			 
static void internal_cmnd (int host,  unsigned char target, const void *cmnd , 
		  void *buffer, unsigned bufflen, void (*done)(int,int))
	{
	int temp;

#ifdef DEBUG_DELAY	
	int clock;
#endif

	if ((host < 0) ||  (host > MAX_SCSI_HOSTS))
		panic ("Host number in internal_cmnd() is out of range.\n");


/*
	We will wait MIN_RESET_DELAY clock ticks after the last reset so 
	we can avoid the drive not being ready.
*/ 
temp = last_reset[host];
while (jiffies < temp);

host_timeout[host] = last_cmnd[host].timeout_per_command;
update_timeout();

/*
	We will use a queued command if possible, otherwise we will emulate the
	queing and calling of completion function ourselves. 
*/
#ifdef DEBUG
	printk("internal_cmnd (host = %d, target = %d, command = %08x, buffer =  %08x, \n"
		"bufflen = %d, done = %08x)\n", host, target, cmnd, buffer, bufflen, done);
#endif

        if (scsi_hosts[host].can_queue)
		{
#ifdef DEBUG
	printk("queuecommand : routine at %08x\n", 
		scsi_hosts[host].queuecommand);
#endif
                scsi_hosts[host].queuecommand (target, cmnd, buffer, bufflen, 
					       done);
		}
	else
		{

#ifdef DEBUG
	printk("command() :  routine at %08x\n", scsi_hosts[host].command);
#endif
		temp=scsi_hosts[host].command (target, cmnd, buffer, bufflen);

#ifdef DEBUG_DELAY
	clock = jiffies + 400;
	while (jiffies < clock);
	printk("done(host = %d, result = %04x) : routine at %08x\n", host, temp, done);
#endif
		done(host, temp);
		}	
#ifdef DEBUG
	printk("leaving internal_cmnd()\n");
#endif
	}	

static void scsi_request_sense (int host, unsigned char target, 
					unsigned char lun)
	{
	cli();
	host_timeout[host] = SENSE_TIMEOUT;
	update_timeout();
	last_cmnd[host].flags |= WAS_SENSE;
	sti();
	
	last_cmnd[host].sense_cmnd[1] = lun << 5;	

	internal_cmnd (host, target, (void *) last_cmnd[host].sense_cmnd, 
		       (void *) last_cmnd[host].sense_buffer, SENSE_LENGTH,
		       scsi_done);
	}





/*
	scsi_do_cmd sends all the commands out to the low-level driver.  It 
	handles the specifics required for each low level driver - ie queued 
	or non queud.  It also prevents conflicts when different high level 
	drivers go for the same host at the same time.
*/

void scsi_do_cmd (int host,  unsigned char target, const void *cmnd , 
		  void *buffer, unsigned bufflen, void (*done)(int,int),
		  int timeout, unsigned  char *sense_buffer, int retries 
		   )
        {
	int ok = 0;

#ifdef DEBUG
	int i;	
	printk ("scsi_do_cmd (host = %d, target = %d, buffer =%08x, "
		"bufflen = %d, done = %08x, timeout = %d, retries = %d)\n"
		"command : " , host, target, buffer, bufflen, done, timeout, retries);
	for (i = 0; i < 10; ++i)
		printk ("%02x  ", ((unsigned char *) cmnd)[i]); 
	printk("\n");
#endif
	
	if ((host  >= MAX_SCSI_HOSTS) || !scsi_hosts[host].present)
		{
		printk ("Invalid or not present host number. %d\n", host);
		panic("");
		}

	
/*
	We must prevent reentrancy to the lowlevel host driver.  This prevents 
	it - we enter a loop until the host we want to talk to is not busy.   
	Race conditions are prevented, as interrupts are disabled inbetween the
	time we check for the host being not busy, and the time we mark it busy
	ourselves.
*/

	do 	{
		cli();
		if (host_busy[host])
			{
			sti();
#ifdef DEBUG
			printk("Host %d is busy.\n", host);
#endif
			while (host_busy[host]);
#ifdef DEBUG
			printk("Host %d is no longer busy.\n", host);
#endif
			}
		else
			{
			host_busy[host] = 1;
			ok = 1;
			sti();
			}
		} while (!ok);
		

/*
	Our own function scsi_done (which marks the host as not busy, disables 
	the timeout counter, etc) will be called by us or by the 
	scsi_hosts[host].queuecommand() function needs to also call
	the completion function for the high level driver.

*/

	memcpy ((void *) last_cmnd[host].cmnd , (void *) cmnd, 10);
	last_cmnd[host].host = host;
	last_cmnd[host].target = target;
	last_cmnd[host].lun = (last_cmnd[host].cmnd[1] >> 5);
	last_cmnd[host].bufflen = bufflen;
	last_cmnd[host].buffer = buffer;
	last_cmnd[host].sense_buffer = sense_buffer;
	last_cmnd[host].flags=0;
	last_cmnd[host].retries=0;
	last_cmnd[host].allowed=retries;
	last_cmnd[host].done = done;
	last_cmnd[host].timeout_per_command = timeout;
				
	/* Start the timer ticking.  */

	internal_timeout[host] = 0;
	internal_cmnd (host,  target, cmnd , buffer, bufflen, scsi_done);

#ifdef DEBUG
	printk ("Leaving scsi_do_cmd()\n");
#endif
        }


/*
	The scsi_done() function disables the timeout timer for the scsi host, 
	marks the host as not busy, and calls the user specified completion 
	function for that host's current command.
*/

static void reset (int host)
	{
	#ifdef DEBUG
		printk("reset(%d)\n", host);
	#endif

	last_cmnd[host].flags |= (WAS_RESET | IS_RESETTING);
	scsi_reset(host);

	#ifdef DEBUG
		printk("performing request sense\n");
	#endif

	scsi_request_sense (host, last_cmnd[host].target, last_cmnd[host].lun);
	}
	
	

static int check_sense (int host)
	{
	if (((last_cmnd[host].sense_buffer[0] & 0x70) >> 4) == 7)
		switch (last_cmnd[host].sense_buffer[2] & 0xf)
		{
		case NO_SENSE:
		case RECOVERED_ERROR:
			return 0;

		case ABORTED_COMMAND:
		case NOT_READY:
			return SUGGEST_RETRY;	
		case UNIT_ATTENTION:
			return SUGGEST_ABORT;

		/* these three are not supported */	
		case COPY_ABORTED:
		case VOLUME_OVERFLOW:
		case MISCOMPARE:
	
		case MEDIUM_ERROR:
			return SUGGEST_REMAP;
		case BLANK_CHECK:
		case DATA_PROTECT:
		case HARDWARE_ERROR:
		case ILLEGAL_REQUEST:
		default:
			return SUGGEST_ABORT;
		}
	else
		return SUGGEST_RETRY;	
	}	

/* This function is the mid-level interrupt routine, which decides how
 *  to handle error conditions.  Each invocation of this function must
 *  do one and *only* one of the following:
 *
 *  (1) Call last_cmnd[host].done.  This is done for fatal errors and
 *      normal completion, and indicates that the handling for this
 *      request is complete.
 *  (2) Call internal_cmnd to requeue the command.  This will result in
 *      scsi_done being called again when the retry is complete.
 *  (3) Call scsi_request_sense.  This asks the host adapter/drive for
 *      more information about the error condition.  When the information
 *      is available, scsi_done will be called again.
 *  (4) Call reset().  This is sort of a last resort, and the idea is that
 *      this may kick things loose and get the drive working again.  reset()
 *      automatically calls scsi_request_sense, and thus scsi_done will be
 *      called again once the reset is complete.
 *
 *      If none of the above actions are taken, the drive in question
 * will hang. If more than one of the above actions are taken by
 * scsi_done, then unpredictable behavior will result.
 */
static void scsi_done (int host, int result)
	{
	int status=0;
	int exit=0;
	int checked;
	int oldto;
	oldto = host_timeout[host];
	host_timeout[host] = 0;
	update_timeout();

#define FINISHED 0
#define MAYREDO  1
#define REDO	 3
#define PENDING  4

#ifdef DEBUG
	printk("In scsi_done(host = %d, result = %06x)\n", host, result);
#endif
	if (host > MAX_SCSI_HOSTS || host  < 0) 
		{
		host_timeout[host] = 0;
		update_timeout();
		panic("scsi_done() called with invalid host number.\n");
		}

	switch (host_byte(result))	
	{
	case DID_OK:
		if (last_cmnd[host].flags & IS_RESETTING)
			{
			last_cmnd[host].flags &= ~IS_RESETTING;
			status = REDO;
			break;
			}

		if (status_byte(result) && (last_cmnd[host].flags & 
		    WAS_SENSE))	
			{
			last_cmnd[host].flags &= ~WAS_SENSE;
			cli();
			internal_timeout[host] &= ~SENSE_TIMEOUT;
			sti();

			if (!(last_cmnd[host].flags & WAS_RESET))
				{
				reset(host);
				return;
				}
			else
				{
				exit = (DRIVER_HARD | SUGGEST_ABORT);
				status = FINISHED;
				}
			}
		else switch(msg_byte(result))
			{
			case COMMAND_COMPLETE:
			switch (status_byte(result))
			{
			case GOOD:
				if (last_cmnd[host].flags & WAS_SENSE)
					{
#ifdef DEBUG
	printk ("In scsi_done, GOOD status, COMMAND COMPLETE, parsing sense information.\n");
#endif

					last_cmnd[host].flags &= ~WAS_SENSE;
					cli();
					internal_timeout[host] &= ~SENSE_TIMEOUT;
					sti();
	
					switch (checked = check_sense(host))
					{
					case 0: 
#ifdef DEBUG
	printk("NO SENSE.  status = REDO\n");
#endif

						host_timeout[host] = oldto;
						update_timeout();
						status = REDO;
						break;
					case SUGGEST_REMAP:			
					case SUGGEST_RETRY: 
#ifdef DEBUG
	printk("SENSE SUGGEST REMAP or SUGGEST RETRY - status = MAYREDO\n");
#endif

						status = MAYREDO;
						exit = SUGGEST_RETRY;
						break;
					case SUGGEST_ABORT:
#ifdef DEBUG
	printk("SENSE SUGGEST ABORT - status = FINISHED");
#endif

						status = FINISHED;
						exit =  DRIVER_SENSE;
						break;
					default:
						printk ("Internal error %s %s \n", __FILE__, 
							__LINE__);
					}			   
					}	
				else
					{
#ifdef DEBUG
	printk("COMMAND COMPLETE message returned, status = FINISHED. \n");
#endif

					exit =  DRIVER_OK;
					status = FINISHED;
					}
				break;	

			case CHECK_CONDITION:

#ifdef DEBUG
	printk("CHECK CONDITION message returned, performing request sense.\n");
#endif

				scsi_request_sense (host, last_cmnd[host].target, last_cmnd[host].lun);
				status = PENDING;
				break;       	
			
			case CONDITION_GOOD:
			case INTERMEDIATE_GOOD:
			case INTERMEDIATE_C_GOOD:
#ifdef DEBUG
	printk("CONDITION GOOD, INTERMEDIATE GOOD, or INTERMEDIATE CONDITION GOOD recieved and ignored. \n");
#endif
				break;
				
			case BUSY:
#ifdef DEBUG
	printk("BUSY message returned, performing REDO");
#endif
				host_timeout[host] = oldto;
				update_timeout();
				status = REDO;
				break;

			case RESERVATION_CONFLICT:
				reset(host);
				return;
#if 0
				exit = DRIVER_SOFT | SUGGEST_ABORT;
				status = MAYREDO;
				break;
#endif
			default:
				printk ("Internal error %s %s \n"
					"status byte = %d \n", __FILE__, 
					__LINE__, status_byte(result));
				
			}
			break;
			default:
				panic ("unsupported message byte recieved.");
			}
			break;
	case DID_TIME_OUT:	
#ifdef DEBUG
	printk("Host returned DID_TIME_OUT - ");
#endif

		if (last_cmnd[host].flags & WAS_TIMEDOUT)	
			{
#ifdef DEBUG
	printk("Aborting\n");
#endif	
			exit = (DRIVER_TIMEOUT | SUGGEST_ABORT);
			}		
		else 
			{
#ifdef DEBUG
			printk ("Retrying.\n");
#endif
			last_cmnd[host].flags  |= WAS_TIMEDOUT;
			status = REDO;
			}
		break;
	case DID_BUS_BUSY:
	case DID_PARITY:
		status = REDO;
		break;
	case DID_NO_CONNECT:
#ifdef DEBUG
		printk("Couldn't connect.\n");
#endif
		exit  = (DRIVER_HARD | SUGGEST_ABORT);
		break;
	case DID_ERROR:	
		status = MAYREDO;
		exit = (DRIVER_HARD | SUGGEST_ABORT);
		break;
	case DID_BAD_TARGET:
	case DID_ABORT:
		exit = (DRIVER_INVALID | SUGGEST_ABORT);
		break;	
	default : 		
		exit = (DRIVER_ERROR | SUGGEST_DIE);
	}

	switch (status) 
		{
		case FINISHED:
		case PENDING:
			break;
		case MAYREDO:

#ifdef DEBUG
	printk("In MAYREDO, allowing %d retries, have %d\n\r",
	       last_cmnd[host].allowed, last_cmnd[host].retries);
#endif

			if ((++last_cmnd[host].retries) < last_cmnd[host].allowed)
			{
			if ((last_cmnd[host].retries >= (last_cmnd[host].allowed >> 1))
			    && !(last_cmnd[host].flags & WAS_RESET))
			        {
					reset(host);
					break;
			        }

			}
			else
				{
				status = FINISHED;
				break;
				}
			/* fall through to REDO */

		case REDO:
			if (last_cmnd[host].flags & WAS_SENSE)			
				scsi_request_sense (host, last_cmnd[host].target,	
			       last_cmnd[host].lun); 	
			else	
				internal_cmnd (host, last_cmnd[host].target,	
			        last_cmnd[host].cmnd,  
				last_cmnd[host].buffer,   
				last_cmnd[host].bufflen, scsi_done);			
			break;	
		default: 
			INTERNAL_ERROR;
		}

	if (status == FINISHED) 
		{
		#ifdef DEBUG
			printk("Calling done function - at address %08x\n", last_cmnd[host].done);
		#endif
		host_busy[host] = 0;
		last_cmnd[host].done (host, (result | ((exit & 0xff) << 24)));
		}


#undef FINISHED
#undef REDO
#undef MAYREDO
#undef PENDING
	}

/*
	The scsi_abort function interfaces with the abort() function of the host
	we are aborting, and causes the current command to not complete.  The 
	caller should deal with any error messages or status returned on the 
	next call.
	
	This will not be called rentrantly for a given host.
*/
	
/*
	Since we're nice guys and specified that abort() and reset()
	can be non-reentrant.  The internal_timeout flags are used for
	this.
*/


int scsi_abort (int host, int why)
	{
	int temp, oldto;
	
	while(1)	
		{
		cli();
		if (internal_timeout[host] & IN_ABORT) 
			{
			sti();
			while (internal_timeout[host] & IN_ABORT);
			}
		else
			{	
			oldto = host_timeout[host];
			internal_timeout[host] |= IN_ABORT;
			host_timeout[host] = ABORT_TIMEOUT;	
			update_timeout();

			
			sti();
			if (!host_busy[host] || !scsi_hosts[host].abort(why))
				temp =  0;
			else
				temp = 1;
			
			cli();
			internal_timeout[host] &= ~IN_ABORT;
			host_timeout[host]=oldto;
			update_timeout();
			sti();
			return temp;
			}
		}	
	}

int scsi_reset (int host)
	{
	int temp, oldto;
	
	while (1) {
		cli();	
		if (internal_timeout[host] & IN_RESET)
			{
			sti();
			while (internal_timeout[host] & IN_RESET);
			}
		else
			{
			oldto = host_timeout[host];	
			host_timeout[host] = RESET_TIMEOUT;	
			update_timeout();	
			internal_timeout[host] |= IN_RESET;
					
			if (host_busy[host])
				{	
				sti();
				if (!(last_cmnd[host].flags & IS_RESETTING) && !(internal_timeout[host] & IN_ABORT))
					scsi_abort(host, DID_RESET);

				temp = scsi_hosts[host].reset();			
				}				
			else
				{
				host_busy[host]=1;
	
				sti();
				temp = scsi_hosts[host].reset();
				last_reset[host] = jiffies;
				host_busy[host]=0;
				}
	
			cli();
			host_timeout[host] = oldto;		
			update_timeout();
			internal_timeout[host] &= ~IN_RESET;
			sti();
			return temp;	
			}
		}
	}
			 

static void scsi_main_timeout(void)
	{
	/*
		We must not enter update_timeout with a timeout condition still pending.
	*/

	int i, timed_out;

	do 	{	
		cli();

	/*
		Find all timers such that they have 0 or negative (shouldn't happen)
		time remaining on them.
	*/
			
		for (i = timed_out = 0; i < MAX_SCSI_HOSTS; ++i)
			if (host_timeout[i] != 0 && host_timeout[i] <= time_elapsed)
				{
				sti();
				host_timeout[i] = 0;
				scsi_times_out(i);
				++timed_out; 
				}

		update_timeout();				
	   	} while (timed_out);	
	sti();
	}

/*
	These are used to keep track of things. 
*/

static int time_start, time_elapsed;

/*
	The strategy is to cause the timer code to call scsi_times_out()
	when the soonest timeout is pending.  
*/
	
static void update_timeout(void)
	{
	unsigned int i, least, used;

	cli();

/* 
	Figure out how much time has passed since the last time the timeouts 
   	were updated 
*/
	used = (time_start) ? (jiffies - time_start) : 0;

/*
	Find out what is due to timeout soonest, and adjust all timeouts for
	the amount of time that has passed since the last time we called 
	update_timeout. 
*/
	
	for (i = 0, least = 0xffffffff; i < MAX_SCSI_HOSTS; ++i)	
		if (host_timeout[i] > 0 && (host_timeout[i] -= used) < least)
			least = host_timeout[i]; 

/*
	If something is due to timeout again, then we will set the next timeout 
	interrupt to occur.  Otherwise, timeouts are disabled.
*/
	
	if (least != 0xffffffff)
		{
		time_start = jiffies;	
		timer_table[SCSI_TIMER].expires = (time_elapsed = least) + jiffies;	
		timer_active |= 1 << SCSI_TIMER;
		}
	else
		{
		timer_table[SCSI_TIMER].expires = time_start = time_elapsed = 0;
		timer_active &= ~(1 << SCSI_TIMER);
		}	
	sti();
	}		
/*
	scsi_dev_init() is our initialization routine, which inturn calls host 
	initialization, bus scanning, and sd/st initialization routines.  It 
	should be called from main().
*/

static unsigned char generic_sense[6] = {REQUEST_SENSE, 0,0,0, 255, 0};		
unsigned long scsi_dev_init (unsigned long memory_start,unsigned long memory_end)
	{
	int i;
#ifdef FOO_ON_YOU
	return;
#endif	
	timer_table[SCSI_TIMER].fn = scsi_main_timeout;
	timer_table[SCSI_TIMER].expires = 0;

	scsi_init();            /* initialize all hosts */
/*
 *	Set up sense command in each host structure.
 */

	for (i = 0; i < MAX_SCSI_HOSTS; ++i)
		{
		memcpy ((void *) last_cmnd[i].sense_cmnd, (void *) generic_sense,
			6);
		last_reset[i] = 0;
		}
				
        scan_scsis();           /* scan for scsi devices */

#ifdef CONFIG_BLK_DEV_SD
	memory_start = sd_init(memory_start, memory_end);              /* init scsi disks */
#endif

#ifdef CONFIG_BLK_DEV_ST
        memory_start = st_init(memory_start, memory_end);              /* init scsi tapes */
#endif

#ifdef CONFIG_BLK_DEV_SR
	memory_start = sr_init(memory_start, memory_end);
#endif
	return memory_start;
	}
#endif

static void print_inquiry(unsigned char *data)
{
        int i;

	printk("  Vendor:");
	for (i = 8; i < 15; i++)
	        {
	        if (data[i] >= 20)
		        printk("%c", data[i]);
	        else
		        printk(" ");
	        }

	printk("  Model:");
	for (i = 16; i < 31; i++)
	        {
	        if (data[i] >= 20)
		        printk("%c", data[i]);
	        else
		        printk(" ");
	        }

	printk("  Rev:");
	for (i = 32; i < 35; i++)
	        {
	        if (data[i] >= 20)
		        printk("%c", data[i]);
	        else
		        printk(" ");
	        }

	printk("\n");

	i = data[0] & 0x1f;

	printk("  Type: %s ", 	i == 0x00 ? "Direct-Access    " :
				i == 0x01 ? "Sequential-Access" :
				i == 0x02 ? "Printer          " :
				i == 0x03 ? "Processor        " :
				i == 0x04 ? "WORM             " :
				i == 0x05 ? "CD-ROM           " :
				i == 0x06 ? "Scanner          " :
				i == 0x07 ? "Optical Device   " :
				i == 0x08 ? "Medium Changer   " :
				i == 0x09 ? "Communications   " :
				            "Unknown          " );
	printk("ANSI SCSI revision: %02x\n", data[2] & 0x07);
}

