/*
 *  kernel/chr_drv/vt.c
 *
 *  Copyright (C) 1992 obz under the linux copyright
 */

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/kd.h>
#include <linux/vt.h>

#include <asm/io.h>
#include <asm/segment.h>

#include "vt_kern.h"

/*
 * console (vt and kd) routines, as defined by usl svr4 manual
 */

struct vt_cons vt_cons[NR_CONSOLES];

extern unsigned char kleds;
extern unsigned char kraw;
extern unsigned char ke0;

extern int sys_ioperm(unsigned long from, unsigned long num, int on);
extern void set_leds(void);

/*
 * these are the valid i/o ports we're allowed to change. they map all the
 * video ports
 */
#define GPFIRST 0x3b4
#define GPLAST 0x3df
#define GPNUM (GPLAST - GPFIRST + 1)

/*
 * turns on sound of some freq. 0 turns it off.
 * stolen from console.c, so i'm not sure if its the correct interpretation
 */
static int
kiocsound(unsigned int freq)
{
	if (freq == 0) {
		/* disable counter 2 */
		outb(inb_p(0x61)&0xFC, 0x61);
	}
	else {
		/* enable counter 2 */
		outb_p(inb_p(0x61)|3, 0x61);
		/* set command for counter 2, 2 byte write */
		outb_p(0xB6, 0x43);
		/* select desired HZ */
		outb_p(freq & 0xff, 0x42);
		outb((freq >> 8) & 0xff, 0x42);
	}
	return 0;
}

/*
 * We handle the console-specific ioctl's here.  We allow the
 * capability to modify any console, not just the fg_console. 
 */
int vt_ioctl(struct tty_struct *tty, struct file * file,
	     unsigned int cmd, unsigned int arg)
{
	int console;
	unsigned char ucval;

	console = tty->line - 1;

	if (console < 0 || console >= NR_CONSOLES)
		return -EINVAL;

	switch (cmd) {
	case KIOCSOUND:
		return kiocsound((unsigned int)arg);

	case KDGKBTYPE:
		/*
		 * this is naive.
		 */
		verify_area((void *) arg, sizeof(unsigned char));
		put_fs_byte(KB_101, (unsigned char *) arg);
		return 0;

	case KDADDIO:
	case KDDELIO:
		/*
		 * KDADDIO and KDDELIO may be able to add ports beyond what
		 * we reject here, but to be safe...
		 */
		if (arg < GPFIRST || arg > GPLAST)
			return -EINVAL;
		return sys_ioperm(arg, 1, (cmd == KDADDIO)) ? -ENXIO : 0;

	case KDENABIO:
	case KDDISABIO:
		return sys_ioperm(GPFIRST, GPNUM,
				  (cmd == KDENABIO)) ? -ENXIO : 0;

	case KDSETMODE:
		/*
		 * currently, setting the mode from KD_TEXT to KD_GRAPHICS
		 * doesn't do a whole lot. i'm not sure if it should do any
		 * restoration of modes or what...
		 */
		switch (arg) {
		case KD_GRAPHICS:
			break;
		case KD_TEXT0:
		case KD_TEXT1:
			arg = KD_TEXT;
		case KD_TEXT:
			break;
		default:
			return -EINVAL;
		}
		if (vt_cons[console].vt_mode == (unsigned char) arg)
			return 0;
		vt_cons[console].vt_mode = (unsigned char) arg;
		if (console != fg_console)
			return 0;
		if (arg == KD_TEXT)
			unblank_screen();
		else {
			timer_active &= 1<<BLANK_TIMER;
			blank_screen();
		}
		return 0;
	case KDGETMODE:
		verify_area((void *) arg, sizeof(unsigned long));
		put_fs_long(vt_cons[console].vt_mode, (unsigned long *) arg);
		return 0;

	case KDMAPDISP:
	case KDUNMAPDISP:
		/*
		 * these work like a combination of mmap and KDENABIO.
		 * this could be easily finished.
		 */
		return -EINVAL;

	case KDSKBMODE:
		if (arg == K_RAW) {
			if (console == fg_console) {
				kraw = 1;
				ke0 = 0;
			} else {
				vt_cons[console].vc_kbdraw = 1;
				vt_cons[console].vc_kbde0 = 0;
			}
		}
		else if (arg == K_XLATE) {
			if (console == fg_console)
				kraw = 0;
			else
				vt_cons[console].vc_kbdraw = 0;
		}
		else
			return -EINVAL;
		flush_input(tty);
		return 0;
	case KDGKBMODE:
		verify_area((void *) arg, sizeof(unsigned long));
		ucval = (console == fg_console) ? kraw :
			vt_cons[console].vc_kbdraw;
		put_fs_long(ucval ? K_RAW : K_XLATE, (unsigned long *) arg);
		return 0;

	case KDGETLED:
		verify_area((void *) arg, sizeof(unsigned char));
		ucval = (console == fg_console) ? kleds :
			vt_cons[console].vc_kbdleds;
		put_fs_byte((((ucval & 1) ? LED_SCR : 0) |
			     ((ucval & 2) ? LED_NUM : 0) |
			     ((ucval & 4) ? LED_CAP : 0)),
			    (unsigned char *) arg);
		return 0;
	case KDSETLED:
		if (arg & ~7)
			return -EINVAL;
		ucval = (((arg & LED_SCR) ? 1 : 0) |
			 ((arg & LED_NUM) ? 2 : 0) |
			 ((arg & LED_CAP) ? 4 : 0));
		if (console == fg_console) {
			kleds = ucval;
			set_leds();
		}
		else
			vt_cons[console].vc_kbdleds = ucval;
		return 0;

	default:
		return -EINVAL;
	}
}
