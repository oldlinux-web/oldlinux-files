/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * $Header: /proj/X11/mit/server/ddx/x386/RCS/compiler.h,v 1.2 1991/06/27 00:01:11 root Exp $
 */


#ifndef _COMPILER_H
#define _COMPILER_H

#ifdef linux
static void inline outb(unsigned short port, char value)
{
__asm__ volatile ("outb %0,%1"
	::"a" ((char) value),"d" ((unsigned short) port));
}
static void inline outw(unsigned short port, short value)
{
__asm__ volatile ("outw %0,%1"
	::"a" ((short) value),"d" ((unsigned short) port));
}

static unsigned char inline inb(unsigned short port)
{
	unsigned char _v;
	__asm__ volatile ("inb %1,%0"
		:"=a" (_v):"d" ((unsigned short) port));
	return _v;
}
static unsigned short inline inw(unsigned short port)
{
	unsigned short _v;
	__asm__ volatile ("inw %1,%0"
		:"=a" (_v):"d" ((unsigned short) port));
	return _v;
}

#define intr_disable() /* should we do this?: asm("cli") */
#define intr_enable()  /* should we do this?: asm("sti") */

#else /* !linux */

#define usleep(usec) syscall(3112, usec / 1000)

#ifdef __GNUC__

static __inline__ void
outb(port, val)
     short port;
     char val;
{
  __asm__ volatile("out%B0 (%1)" : :"a" (val), "d" (port));
}

static __inline__ void
outw(port, val)
     short port;
     short val;
{
  __asm__ volatile("out%W0 (%1)" : :"a" (val), "d" (port));
}

static __inline__ unsigned int
inb(port)
     short port;
{
  unsigned int ret;
  __asm__ volatile("in%B0 (%1)" :
		   "=a" (ret) :
		   "d" (port));
  return ret;
}

static __inline__ unsigned int
inw(port)
     short port;
{
  unsigned int ret;
  __asm__ volatile("in%W0 (%1)" :
		   "=a" (ret) :
		   "d" (port));
  return ret;
}

static __inline__ void
intr_disable()
{
  __asm__ volatile("cli");
}

static __inline__ void
intr_enable()
{
  __asm__ volatile("sti");
}

#else /* __GNUC__ */
# if defined(__STDC__) && (__STDC__ == 1)
#  define asm __asm
# endif
#include <sys/inline.h>
#define intr_disable() asm("cli")
#define intr_enable()  asm("sti")
#endif

#endif /* linux */

#ifndef __STDC__
#define signed /**/
#define const /**/
#endif

#endif /* _COMPILER_H */
