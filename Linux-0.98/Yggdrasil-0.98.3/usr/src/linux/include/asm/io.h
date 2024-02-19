#ifndef _ASM_IO_H
#define _ASM_IO_H

/*
 * Thanks to James van Artsdalen for a better timing-fix than
 * the two short jumps: using outb's to a nonexistent port seems
 * to guarantee better timings even on fast machines.
 *
 * On the other hand, I'd like to be sure of a non-existent port:
 * I feel a bit unsafe abou using 0x80.
 *
 *		Linus
 */

#ifdef SLOW_IO_BY_JUMPING
#define __SLOW_DOWN_IO __asm__ __volatile__("jmp 1f\n1:\tjmp 1f\n1:")
#else
#define __SLOW_DOWN_IO __asm__ __volatile__("outb %al,$0x80")
#endif

#ifdef REALLY_SLOW_IO
#define SLOW_DOWN_IO { __SLOW_DOWN_IO; __SLOW_DOWN_IO; __SLOW_DOWN_IO; __SLOW_DOWN_IO; }
#else
#define SLOW_DOWN_IO __SLOW_DOWN_IO
#endif

extern void inline outb(char value, unsigned short port)
{
__asm__ __volatile__ ("outb %%al,%%dx"
		::"a" ((char) value),"d" ((unsigned short) port));
}

extern unsigned int inline inb(unsigned short port)
{
	unsigned int _v;
__asm__ __volatile__ ("inb %%dx,%%al"
		:"=a" (_v):"d" ((unsigned short) port),"0" (0));
	return _v;
}

extern void inline outb_p(char value, unsigned short port)
{
__asm__ __volatile__ ("outb %%al,%%dx"
		::"a" ((char) value),"d" ((unsigned short) port));
	SLOW_DOWN_IO;
}

extern unsigned int inline inb_p(unsigned short port)
{
	unsigned int _v;
__asm__ __volatile__ ("inb %%dx,%%al"
		:"=a" (_v):"d" ((unsigned short) port),"0" (0));
	SLOW_DOWN_IO;
	return _v;
}

#endif
