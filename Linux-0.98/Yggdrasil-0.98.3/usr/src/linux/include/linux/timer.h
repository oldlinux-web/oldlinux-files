#ifndef _LINUX_TIMER_H
#define _LINUX_TIMER_H

/*
 * DON'T CHANGE THESE!! Most of them are hardcoded into some assembly language
 * as well as being defined here.
 */

/*
 * The timers are:
 *
 * BLANK_TIMER		console screen-saver timer
 *
 * BEEP_TIMER		console beep timer
 *
 * RS_TIMER		timer for the RS-232 ports
 * 
 * HD_TIMER		harddisk timer
 *
 * FLOPPY_TIMER		floppy disk timer (not used right now)
 * 
 * SCSI_TIMER		scsi.c timeout timer
 *
 * NET_TIMER		tcp/ip timeout timer
 */

#define BLANK_TIMER	0
#define BEEP_TIMER	1
#define RS_TIMER	2

#define HD_TIMER	16
#define FLOPPY_TIMER	17
#define SCSI_TIMER 	18
#define NET_TIMER	19

struct timer_struct {
	unsigned long expires;
	void (*fn)(void);
};

extern unsigned long timer_active;
extern struct timer_struct timer_table[32];

#endif
