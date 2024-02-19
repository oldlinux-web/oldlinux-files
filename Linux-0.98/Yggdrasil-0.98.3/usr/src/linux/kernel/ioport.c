/*
 *	linux/kernel/ioport.c
 *
 * This contains the io-permission bitmap code - written by obz, with changes
 * by Linus.
 */

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/types.h>

#define _IODEBUG

#ifdef IODEBUG
static char * ios(unsigned long l)
{
	static char str[33] = { '\0' };
	int i;
	unsigned long mask;

	for (i = 0, mask = 0x80000000; i < 32; ++i, mask >>= 1)
		str[i] = (l & mask) ? '1' : '0';
	return str;
}

static void dump_io_bitmap(void)
{
	int i, j;
	int numl = sizeof(current->tss.io_bitmap) >> 2;

	for (i = j = 0; j < numl; ++i)
	{
		printk("%4d [%3x]: ", 64*i, 64*i);
		printk("%s ", ios(current->tss.io_bitmap[j++]));
		if (j < numl)
			printk("%s", ios(current->tss.io_bitmap[j++]));
		printk("\n");
	}
}
#endif

/*
 * this changes the io permissions bitmap in the current task.
 */
int sys_ioperm(unsigned long from, unsigned long num, int turn_on)
{
	unsigned long froml, lindex, tnum, numl, rindex, mask;
	unsigned long *iop;

	froml = from >> 5;
	lindex = from & 0x1f;
	tnum = lindex + num;
	numl = (tnum + 0x1f) >> 5;
	rindex = tnum & 0x1f;

	if (!suser())
		return -EPERM;
	if (froml * 32 + tnum > sizeof(current->tss.io_bitmap) * 8 - 8)
		return -EINVAL;

#ifdef IODEBUG
	printk("io: from=%d num=%d %s\n", from, num, (turn_on ? "on" : "off"));
#endif

	if (numl) {
		iop = (unsigned long *)current->tss.io_bitmap + froml;
		if (lindex != 0) {
			mask = (~0 << lindex);
			if (--numl == 0 && rindex)
				mask &= ~(~0 << rindex);
			if (turn_on)
				*iop++ &= ~mask;
			else
				*iop++ |= mask;
		}
		if (numl) {
			if (rindex)
				--numl;
			mask = (turn_on ? 0 : ~0);
			while (numl--)
				*iop++ = mask;
			if (numl && rindex) {
				mask = ~(~0 << rindex);
				if (turn_on)
					*iop++ &= ~mask;
				else
					*iop++ |= mask;
			}
		}
	}
	return 0;
}

unsigned int *stack;

/*
 * sys_iopl has to be used when you want to access the IO ports
 * beyond the 0x3ff range: to get the full 65536 ports bitmapped
 * you'd need 8kB of bitmaps/process, which is a bit excessive.
 *
 * Here we just change the eflags value on the stack: we allow
 * only the super-user to do it. This depends on the stack-layout
 * on system-call entry - see also fork() and the signal handling
 * code.
 */
int sys_iopl(long ebx,long ecx,long edx,
	     long esi, long edi, long ebp, long eax, long ds,
	     long es, long fs, long gs, long orig_eax,
	     long eip,long cs,long eflags,long esp,long ss)
{
	unsigned int level = ebx;

	if (level > 3)
		return -EINVAL;
	if (!suser())
		return -EPERM;
	*(&eflags) = (eflags & 0xffffcfff) | (level << 12);
	return 0;
}
