/*$Source: /usr/home/dhesi/zoo/RCS/io.c,v $*/
/*$Id: io.c,v 1.14 91/07/09 01:39:54 dhesi Exp $*/
/***********************************************************
	io.c -- input/output

Adapted from "ar" archiver written by Haruhiko Okumura.
***********************************************************/
#ifdef ANSI_HDRS
# include <stdlib.h>
# include <string.h>
#endif

#include "options.h"
#include "zoo.h"
#include "ar.h"
#include "lzh.h"
#include "zooio.h"	/* for NULLFILE */
#include "portable.h"

extern void prterror();

#include "errors.i"

#define JUST_LZH		/* for stand-alone compression */

#if 0
# define CRCPOLY  0xA001  /* ANSI CRC-16 */ /* CCITT: 0x8408 */
# define UPDATE_CRC(c) \
	crc = crctable[(crc ^ (c)) & 0xFF] ^ (crc >> CHAR_BIT)
static ushort crctable[UCHAR_MAX + 1];
t_uint16 crc;
#endif

extern FILE *arcfile, *lzh_outfile;
t_uint16 bitbuf;
int unpackable;

ulong compsize, origsize;

static uint  subbitbuf;
static int   bitcount;

#if 0
void make_crctable()
{
	uint i, j, r;

	for (i = 0; i <= UCHAR_MAX; i++) {
		r = i;
		for (j = 0; j < CHAR_BIT; j++)
			if (r & 1) r = (r >> 1) ^ CRCPOLY;
			else       r >>= 1;
		crctable[i] = r;
	}
}
#endif

void fillbuf(n)  /* Shift bitbuf n bits left, read n bits */
int n;
{
	bitbuf <<= n;
	while (n > bitcount) {
		bitbuf |= subbitbuf << (n -= bitcount);
#ifdef JUST_LZH
		if (feof(arcfile))
			subbitbuf = 0;
		else
			subbitbuf = (uchar) zgetc(arcfile);
#else
		if (compsize != 0) {
			compsize--;  subbitbuf = (uchar) zgetc(arcfile);
		} else subbitbuf = 0;
#endif /* JUST_LZH */
		bitcount = CHAR_BIT;
	}
	bitbuf |= subbitbuf >> (bitcount -= n);
}

uint getbits(n)
int n;
{
	uint x;

	x = bitbuf >> (BITBUFSIZ - n);  fillbuf(n);
	return x;
}

void putbits(n, x)  /* Write rightmost n bits of x */
int n;
uint x;
{
	if (n < bitcount) {
		subbitbuf |= x << (bitcount -= n);
	} else {
#ifdef JUST_LZH
		(void) putc((int) (subbitbuf | (x >> (n -= bitcount))), lzh_outfile);
		compsize++;
#else
		if (compsize < origsize) {
			(void) zputc((int) (subbitbuf | (x >> (n -= bitcount))), lzh_outfile);
			compsize++;
		} else unpackable = 1;
#endif /* JUST_LZH */

		if (n < CHAR_BIT) {
			subbitbuf = x << (bitcount = CHAR_BIT - n);
		} else {
#ifdef JUST_LZH
			(void) putc((int) (x >> (n - CHAR_BIT)), lzh_outfile);
			compsize++;
#else
			if (compsize < origsize) {
				(void) zputc((int) (x >> (n - CHAR_BIT)), lzh_outfile);
				compsize++;
			} else unpackable = 1;
#endif /* JUST_LZH */
			subbitbuf = x << (bitcount = 2 * CHAR_BIT - n);
		}
	}
}

extern void addbfcrc();

int fread_crc(p, n, f)
uchar *p;
int n;
FILE *f;
{
	int i;

	i = n = fread((char *) p, 1, n, f);  origsize += n;
	addbfcrc(p, i);
	return n;
}

void fwrite_crc(p, n, f)
uchar *p;
int n;
FILE *f;
{
	if (f != NULLFILE) {
		if (fwrite((char *) p, 1, n, f) < n) 
			prterror('f', disk_full);
	}
	addbfcrc(p, n);
}

void init_getbits()
{
	bitbuf = 0;  subbitbuf = 0;  bitcount = 0;
	fillbuf(BITBUFSIZ);
}

void init_putbits()
{
	bitcount = CHAR_BIT;  subbitbuf = 0;
}
