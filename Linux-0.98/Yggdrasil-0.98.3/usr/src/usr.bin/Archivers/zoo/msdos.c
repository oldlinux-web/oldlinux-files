/* msdos.c */

/* Highly system-dependent routines go here */

/* settime() */

/* Accepts a date/time in DOS format and sets the file time. Returns 1
if OK, 0 if error */

#include "options.h"
#include "zoo.h"
#include "zooio.h"		/* to satisfy declarations in zoofns.h */
#include "zoofns.h"
#include "errors.i"
#include <stdio.h>		/* to get fileno() */

/* register definitions specific for Turbo C */
union	REGS	{
	struct { unsigned ax, bx, cx, dx, si, di, carry, flags; } x;
	struct { unsigned char al, ah, bl, bh, cl, ch, dl, dh; }  h;
};

int settime (file,date,time)
ZOOFILE file;
unsigned date, time;
{
	extern intdos();
	union REGS regs;
	regs.h.ah = 0x57;						/* DOS FileTimes call */
	regs.h.al = 0x01;					/* set date/time request */
	regs.x.bx = fileno (file);		/* get handle */
	regs.x.cx = time;
	regs.x.dx = date;

	/* first flush file so later write won't occur on close */
	fflush (file);

	intdos (&regs, &regs);
	if (regs.x.carry != 0)
		return (0);
	else
		return (1);
} /* settime */

/* gets date and time of file */
gettime (file,date,time)
ZOOFILE file;
unsigned *date, *time;
{
	union REGS regs;
	regs.h.ah = 0x57;						/* DOS FileTimes call */
	regs.h.al = 0x00;						/* get date/time request */
	regs.x.bx = fileno (file);		/* get handle */
	intdos (&regs, &regs);
	*time = regs.x.cx;
	*date = regs.x.dx;
	if (regs.x.carry != 0)
		return (0);
	else
		return (1);
} /* settime */


/* space() */

/* Returns free space in bytes on disk n (0 = default, 1 = A, etc.).  Returns
	0 if drive number is invalid.  Before getting disk space, the function
	requests DOS to flush its internal buffers */

unsigned long space (drive, alloc_size)
int drive;
int *alloc_size;
{
	unsigned long free_space;
	union REGS regs;

	regs.h.ah = 0x0d;										/* disk reset DOS call */
	intdos (&regs, &regs);

	regs.h.ah = 0x36;										/* GetFreeSpace DOS call */
	regs.h.dl = drive;
	intdos (&regs, &regs);

	/* space = clusters * sectors/cluster * bytes/sector.  */
	/* ax=0xFFFF on error */

	/* cluster size = sectors/cluster * bytes/sector */
	*alloc_size = regs.x.ax * regs.x.cx;

	/* space = cluster * alloc_size */
	if (regs.x.ax == 0xffff)
		return (0L);			/* invalid drive */
	else {
		free_space = ((unsigned long) regs.x.bx) * *alloc_size;
		return (free_space);
	}
}
