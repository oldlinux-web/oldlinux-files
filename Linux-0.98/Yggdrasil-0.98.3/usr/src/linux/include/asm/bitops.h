#ifndef _ASM_BITOPS_H
/*
 * Copyright 1992, Linus Torvalds.
 */

#ifdef i386
/*
 * These have to be done with inline assembly: that way the bit-setting
 * is guaranteed to be atomic. Both set_bit and clear_bit return 0
 * if the bit-setting went ok, != 0 if the bit already was set/cleared.
 *
 * bit 0 is the LSB of addr; bit 32 is the LSB of (addr+1).
 */
extern inline int set_bit(int nr,int * addr)
{
	char ok;

	__asm__ __volatile__("btsl %1,%2\n\tsetb %0":
		"=q" (ok):"r" (nr),"m" (*(addr)));
	return ok;
}

extern inline int clear_bit(int nr, int * addr)
{
	char ok;

	__asm__ __volatile__("btrl %1,%2\n\tsetnb %0":
		"=q" (ok):"r" (nr),"m" (*(addr)));
	return ok;
}

/*
 * This routine doesn't need to be atomic, but it's faster to code it
 * this way.
 */
extern inline int test_bit(int nr, int * addr)
{
	char ok;

	__asm__ __volatile__("btl %1,%2\n\tsetb %0":
		"=q" (ok):"r" (nr),"m" (*(addr)));
	return ok;
}

#else
/*
 * For the benefit of those who are trying to port Linux to another
 * architecture, here are some C-language equivalents.  You should
 * recode these in the native assmebly language, if at all possible.
 * To guarantee atomicity, these routines call cli() and sti() to
 * disable interrupts while they operate.  (You have to provide inline
 * routines to cli() and sti().)
 *
 * Also note, these routines assume that you have 32 bit integers.
 * You will have to change this if you are trying to port Linux to the
 * Alpha architecture or to a Cray.  :-)
 * 
 * C language equivalents written by Theodore Ts'o, 9/26/92
 */

extern inline int set_bit(int nr,int * addr)
{
	int	mask, retval;

	addr += nr >> 5;
	mask = 1 << (nr & 0x1f);
	cli();
	retval = (mask & *addr) != 0;
	*addr |= mask;
	sti();
	return retval;
}

extern inline int clear_bit(int nr, int * addr)
{
	int	mask, retval;

	addr += nr >> 5;
	mask = 1 << (nr & 0x1f);
	cli();
	retval = (mask & *addr) == 0;
	*addr &= ~mask;
	sti();
	return retval;
}

extern inline int test_bit(int nr, int * addr)
{
	int	mask;

	addr += nr >> 5;
	mask = 1 << (nr & 0x1f);
	return ((mask & *addr) != 0);
}
#endif	/* i386 */
#endif /* _ASM_BITOPS_H */
