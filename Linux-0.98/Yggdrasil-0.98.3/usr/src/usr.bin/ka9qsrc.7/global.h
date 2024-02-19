
/* Global definitions used by every source file.
 * Some may be compiler dependent.
 */
#define MK_FP(seg,ofs)	((void far *) \
			   (((unsigned long)(seg) << 16) | (unsigned)(ofs)))
/*wa6ngo* Switch "a" and "b" below as these functions are called with
	  the offset as the first param and the segment as the second*/
#define pokew(a,b,c)	(*((int  far*)MK_FP((b),(a))) = (int)(c))
#define pokeb(a,b,c)	(*((char far*)MK_FP((b),(a))) = (char)(c))
#define peekw(a,b)	(*((int  far*)MK_FP((b),(a))))
#define peekb(a,b)	(*((char far*)MK_FP((b),(a))))
#define movblock(so,ss,do,ds,c)	movedata(ss,so,ds,do,c)

#define outportw outport
#define inportw inport
#define index strchr
#define rindex strrchr

/* Indexes into binmode in files.c; hook for compilers that have special
 * open modes for binary files
 */
#define	READ_BINARY	0
#define	WRITE_BINARY	1
#define APPEND_BINARY   2
extern char *binmode[];

/* not all compilers grok defined() */
#ifdef NODEFINED
#define defined(x) (x)
#endif

/* These two lines assume that your compiler's longs are 32 bits and
 * shorts are 16 bits. It is already assumed that chars are 8 bits,
 * but it doesn't matter if they're signed or unsigned.
 */
typedef long int32;		/* 32-bit signed integer */
typedef unsigned short int16;	/* 16-bit unsigned integer */
#define	uchar(x) ((unsigned char)(x))
#define	MAXINT16 65535		/* Largest 16-bit integer */

/* Since not all compilers support structure assignment, the ASSIGN()
 * macro is used. This controls how it's actually implemented.
 */
#ifdef	NOSTRUCTASSIGN	/* Version for old compilers that don't support it */
#define	ASSIGN(a,b)	memcpy((char *)&(a),(char *)&(b),sizeof(b));
#else			/* Version for compilers that do */
#define	ASSIGN(a,b)	((a) = (b))
#endif

/* Define null object pointer in case stdio.h isn't included */
#ifndef	NULL
/* General purpose NULL pointer */
#ifdef ATARI_ST
#define NULL (char *)0		/* MW does not like funny typecasts on void */
#else
#define NULL (void *)0
#endif
#endif
#define	NULLCHAR (char *)0	/* Null character pointer */
#define	NULLFP	 (int (*)())0	/* Null pointer to function returning int */
#define	NULLVFP	 (void (*)())0	/* Null pointer to function returning void */
#define	NULLFILE (FILE *)0	/* Null file pointer */

/* General purpose function macros */
#define	min(x,y)	((x)<(y)?(x):(y))	/* Lesser of two args */
#define	max(x,y)	((x)>(y)?(x):(y))	/* Greater of two args */

/* Convert an address to a LONG value for printing */
#ifdef MSDOS
long	ptr2long();			/* for fuzzy segment addresses */
#else
#define ptr2long(x)	((long) (x))	/* typecast suffices for others */
#endif

#ifdef	MPU8080	/* Assembler routines are available */
int16 hinibble(),lonibble(),hibyte(),lobyte(),hiword(),loword();

#else

/* Extract a short from a long */
/* According to my docs, this bug is fixed in MWC 3.0. (from 3.0.6 release
   notes.) -- hyc */
#if	(ATARI_ST && (MWC < 306))
extern int Sixteen;
#define hiword(x)	((int16)((x) >> Sixteen)) /* hide compiler bug.. */
#else
#define hiword(x)	((int16)((x) >> 16))
#endif
#define	loword(x)	((int16)(x))

/* Extract a byte from a short */
#define	hibyte(x)	(((x) >> 8) & 0xff)
#define	lobyte(x)	((x) & 0xff)

/* Extract nibbles from a byte */
#define	hinibble(x)	(((x) >> 4) & 0xf)
#define	lonibble(x)	((x) & 0xf)

#endif

#if	(defined(SYS5) || (defined(ATARI_ST) && defined(LATTICE)))
#define rindex	strrchr
#define index	strchr
#endif

/* Heavily used functions from the standard library */
char *malloc(),*calloc(),*ctime(),*tmpnam();

#if	ATARI_ST && MICRORTX
/* the 68000 processor won't let you disable interrupts in user mode
 * therefore, a trap handler has been defined to do this, and it's
 * installed as trap #5. The Mark Williams compiler generates a trap #5
 * when you call the magic function "micro_rtx" (their multi-tasking
 * executive).
 * the trap handler returns the previous processor level as a char.
 */

#define disable()	micro_rtx(6)
#define restore(state)	micro_rtx(state)

/* a quick, non-checking free() function as a macro */
#ifdef QFREE
#define free(p)		((char *) (p))[-1] |= 1;
#endif
#endif











