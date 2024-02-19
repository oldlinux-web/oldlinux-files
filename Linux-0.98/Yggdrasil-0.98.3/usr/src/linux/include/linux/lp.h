#ifndef _LINUX_LP_H
#define _LINUX_LP_H

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/sched.h>

#include <asm/io.h>
#include <asm/segment.h>

/*
 * usr/include/linux/lp.h c.1991-1992 James Wiegand
 * many modifications copyright (C) 1992 Michael K. Johnson
 */

/*
 * caveat: my machine only has 1 printer @ lpt2 so lpt1 & lpt3 are 
 * implemented but UNTESTED
 *
 * My machine (Michael K. Johnson) has only lpt1...  dupla caveat...
 */

/*
 * Per POSIX guidelines, this module reserves the LP and lp prefixes
 * These are the lp_table[minor].flags flags...
 */
#define LP_EXIST 0x0001
#define LP_SELEC 0x0002
#define LP_BUSY	 0x0004
#define LP_OFFL	 0x0008
#define LP_NOPA  0x0010
#define LP_ERR   0x0020

/* timeout for each character  (This is a good case 50 Mhz computer
   at a poor case 10 KBS xfer rate to the printer, as best as I can
   tell.)  This is in instruction cycles, kinda -- it is the count
   in a busy loop.  THIS IS THE VALUE TO CHANGE if you have extremely
   slow printing, or if the machine seems to slow down a lot when you
   print.  If you have slow printing, increase this number and recompile,
   and if your system gets bogged down, decrease this number.*/
#define LP_TIME_CHAR 5000

/* timeout for printk'ing a timeout, in jiffies (100ths of a second).
   If your printer isn't printing at least one character every five seconds,
   you have worse problems than a slow printer driver and lp_timeout printed
   every five seconds while trying to print. */
#define LP_TIMEOUT 5000

#define LP_B(minor)	lp_table[(minor)].base		/* IO address */
#define LP_F(minor)	lp_table[(minor)].flags		/* flags for busy, etc. */
#define LP_S(minor)	inb(LP_B((minor)) + 1)		/* status port */
#define LP_C(minor)	(lp_table[(minor)].base + 2)	/* control port */
#define LP_COUNT(minor)	lp_table[(minor)].count		/* last count */
#define LP_TIME(minor)	lp_table[(minor)].time		/* last time */

/* 
since we are dealing with a horribly slow device
I don't see the need for a queue
*/
struct lp_struct {
	int base;
	int flags;
	int count;
	int time;
};

/* This is the starting value for the heuristic algorithm.  If you
 * want to tune this and have a fast printer (i.e. HPIIIP), decrease
 * this number, and if you have a slow printer, increase this number.
 * This is not stricly necessary, as the algorithm should be able to
 * adapt to your printer relatively quickly.
 * this is in hundredths of a second, the default 50 being .5 seconds.
 */

#define LP_INIT_TIME 50

/* This is our first guess at the size of the buffer on the printer,
 * in characters.  I am assuming a 4K buffer because most newer printers
 * have larger ones, which will be adapted to.  At this time, it really
 * doesn't matter, as this value isn't used.
 */

#define LP_INIT_COUNT 4096

/* the BIOS manuals say there can be up to 4 lpt devices
 * but I have not seen a board where the 4th address is listed
 * if you have different hardware change the table below 
 * please let me know if you have different equipment
 * if you have more than 3 printers, remember to increase LP_NO
 */
struct lp_struct lp_table[] = {
	{ 0x3bc, 0, LP_INIT_COUNT, LP_INIT_TIME, },
	{ 0x378, 0, LP_INIT_COUNT, LP_INIT_TIME, },
	{ 0x278, 0, LP_INIT_COUNT, LP_INIT_TIME, }
}; 
#define LP_NO 3

/* 
 * bit defines for 8255 status port
 * base + 1
 * accessed with LP_S(minor), which gets the byte...
 */
#define LP_PBUSY	0x80 /* active low */
#define LP_PACK		0x40 /* active low */
#define LP_POUTPA	0x20
#define LP_PSELECD	0x10
#define LP_PERRORP	0x08 /* active low*/

/* 
 * defines for 8255 control port
 * base + 2 
 * accessed with LP_C(minor)
 */
#define LP_PSELECP	0x08
#define LP_PINITP	0x04  /* active low */
#define LP_PAUTOLF	0x02
#define LP_PSTROBE	0x01

/* 
 * the value written to ports to test existence. PC-style ports will 
 * return the value written. AT-style ports will return 0. so why not
 * make them the same ? 
 */
#define LP_DUMMY	0x00

/*
 * This is the port delay time.  Your mileage may vary.
 * It is used only in the lp_init() routine.
 */
#define LP_DELAY 	150000

/*
 * function prototypes
 */

extern long lp_init(long);

#endif
