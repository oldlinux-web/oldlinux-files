/*
 *	hosts.c Copyright (C) 1992 Drew Eckhardt 
 *	mid to lowlevel SCSI driver interface by
 *		Drew Eckhardt 
 *
 *	<drew@colorado.edu>
 */


/*
 *	This file contains the medium level SCSI
 *	host interface initialization, as well as the scsi_hosts array of SCSI
 *	hosts currently present in the system. 
 */

#include <linux/config.h>

#ifdef CONFIG_SCSI
#include <linux/kernel.h>
#include "scsi.h"

#ifndef NULL 
	#define NULL 0L
#endif

#ifdef FIGURE_MAX_SCSI_HOSTS
	#define MAX_SCSI_HOSTS
#endif

#include "hosts.h"

#ifdef CONFIG_SCSI_AHA1542
#include "aha1542.h"
#endif

#ifdef CONFIG_SCSI_FUTURE_DOMAIN
#include "fdomain.h"
#endif

#ifdef CONFIG_SCSI_SEAGATE
#include "seagate.h"
#endif

#ifdef CONFIG_SCSI_ULTRASTOR
#include "ultrastor.h"
#endif

#ifdef CONFIG_SCSI_7000FASST
#include "7000fasst.h"
#endif

/*
static const char RCSid[] = "$Header: /usr/src/linux/kernel/blk_drv/scsi/RCS/hosts.c,v 1.1 1992/07/24 06:27:38 root Exp root $";
*/

/*
 *	The scsi host entries should be in the order you wish the 
 *	cards to be detected.  A driver may appear more than once IFF
 *	it can deal with being detected (and therefore initialized) 
 *	with more than one simulatenous host number, can handle being
 *	rentrant, etc.
 *
 *	They may appear in any order, as each SCSI host  is told which host number it is
 *	during detection.
 */

/*
 *	When figure is run, we don't want to link to any object code.  Since 
 *	the macro for each host will contain function pointers, we cannot 
 *	use it and instead must use a "blank" that does no such 
 *	idiocy.
 */

#ifdef FIGURE_MAX_SCSI_HOSTS
	#define BLANKIFY(what) BLANK_HOST
#else
	#define BLANKIFY(what) what
#endif

Scsi_Host scsi_hosts[] =
	{
#ifdef CONFIG_SCSI_AHA1542
	BLANKIFY(AHA1542),
#endif

#ifdef CONFIG_SCSI_FUTURE_DOMAIN
	BLANKIFY(FDOMAIN_16X0),
#endif

#ifdef CONFIG_SCSI_SEAGATE
	BLANKIFY(SEAGATE_ST0X),
#endif
#ifdef CONFIG_SCSI_ULTRASTOR
	BLANKIFY(ULTRASTOR_14F),
#endif
#ifdef CONFIG_SCSI_7000FASST
	BLANKIFY(WD7000FASST),
#endif
	};

#ifdef FIGURE_MAX_SCSI_HOSTS
	#undef MAX_SCSI_HOSTS
	#define  MAX_SCSI_HOSTS  (sizeof(scsi_hosts) / sizeof(Scsi_Host))
#endif

#ifdef FIGURE_MAX_SCSI_HOSTS
#include <stdio.h>
void main (void)
{	
	printf("%d", MAX_SCSI_HOSTS);
}
#else
/*
 *	Our semaphores and timeout counters, where size depends on MAX_SCSI_HOSTS here. 
 */

volatile unsigned char host_busy[MAX_SCSI_HOSTS];
volatile int host_timeout[MAX_SCSI_HOSTS];
volatile Scsi_Cmnd *host_queue[MAX_SCSI_HOSTS]; 

void scsi_init(void)
	{
	static int called = 0;
	int i, count;	
	if (!called)
		{
		called = 1;	
		for (count = i = 0; i < MAX_SCSI_HOSTS; ++i)
			{
/*
 * Initialize our semaphores.  -1 is interpreted to mean 
 * "inactive" - where as 0 will indicate a time out condition.
 */ 

			host_busy[i] = 0;
			host_timeout[i] = 0;
			host_queue[i] = NULL;	
			
			if ((scsi_hosts[i].detect) &&  (scsi_hosts[i].present = scsi_hosts[i].detect(i)))
				{		
				printk ("scsi%d : %s.\n\r",
				         count, scsi_hosts[i].name);
				printk ("%s", scsi_hosts[i].info());
				++count;
				}
			}
		printk ("scsi : %d hosts. \n\r", count);
		}

	}

#endif
#else
void main(void) {
	printf("0\n");
	}
#endif	
