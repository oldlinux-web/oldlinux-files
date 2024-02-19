/*
 *   bin86/as/genbin.c
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* genbin.c - binary code generation routines for assembler */

#include "const.h"
#include "type.h"
#include "address.h"
#include "file.h"
#include "globvar.h"

PRIVATE char *asmbeg;		/* beginning of assembler code */
				/* for overwrite check */
				/* bss-init to zero = NULL and not changed */
PRIVATE offset_t binfbuf;	/* binary code buffer for file (counter) */
PRIVATE offset_t binmax;	/* maximum value of binmbuf for pass 1 */
PRIVATE offset_t binmin;	/* minimum value of binmbuf for pass 1 */

FORWARD void putbinoffset P((offset_t offset, count_t size));

/* write header to binary file */

PUBLIC void binheader()
{
    if ((innum = binfil) != 0x0)
    {
	writec(0x0);		/* binary header byte */
#ifdef LONG_BINHEADER
	writeoff(binmax - binmin);	/* program length */
	writeoff(binfbuf = binmin);	/* program start */
#else
	writew((unsigned) (binmax - binmin));	/* program length */
	writew((unsigned) (binfbuf = binmin));	/* program start */
#endif
    }
}

/* write trailer to binary file */

PUBLIC void bintrailer()
{
    if ((innum = binfil) != 0x0)
    {
	writec(0xFF);		/* binary trailer byte */
	writew(0x0);		/* further trailer bytes */
#ifdef LONG_BINHEADER
	writeoff(pedata & UNDBIT ? binmin : progent);	/* entry point */
#else
	writew(pedata & UNDBIT ? (unsigned) binmin : (unsigned) progent);
#endif
    }
}

/* generate binary code for current line */

PUBLIC void genbin()
{
    struct address_s *adrptr;
    char *bufptr;
    unsigned char remaining;

    if (binaryg && mcount != 0x0)
    {
	if (popflags)
	{
	    if (fcflag)
	    {
		bufptr = databuf.fcbuf;
		remaining = mcount;
		do
		    putbin(*bufptr++);
		while (--remaining != 0x0);
	    }
	    if (fdflag)
	    {
		adrptr = databuf.fdbuf;
		remaining = mcount;
		do
		{
		    putbinoffset(adrptr->offset, 0x2);
		    ++adrptr;
		}
		while ((remaining -= 0x2) != 0x0);
	    }
#if SIZEOF_OFFSET_T > 0x2
	    if (fqflag)
	    {
		adrptr = databuf.fqbuf;
		remaining = mcount;
		do
		{
		    putbinoffset(adrptr->offset, 0x4);
		    ++adrptr;
		}
		while ((remaining -= 0x4) != 0x0);
	    }
#endif
	}
	else
	{
	    remaining = mcount - 0x1;	/* count opcode immediately */
#ifdef I80386
	    if (aprefix != 0x0)
	    {
		putbin(aprefix);
		--remaining;
	    }
	    if (oprefix != 0x0)
	    {
		putbin(oprefix);
		--remaining;
	    }
	    if (sprefix != 0x0)
	    {
		putbin(sprefix);
		--remaining;
	    }
#endif
	    if (page != 0x0)
	    {
		putbin(page);
		--remaining;
	    }
	    putbin(opcode);
	    if (remaining != 0x0)
	    {
		if (postb != 0x0)
		{
		    putbin(postb);
		    --remaining;
		}
#ifdef I80386
		if (sib != NO_SIB)
		{
		    putbin(sib);
		    --remaining;
		}
#endif
		if (remaining != 0x0)
		    putbinoffset(lastexp.offset, remaining);
	    }
#ifdef I80386
	    if (immcount != 0x0)
		putbinoffset(immadr.offset, immcount);
#endif
	}
	/* else no code for this instruction, or already generated */
    }
}

/* initialise private variables */

PUBLIC void initbin()
{
    binmin = -1;		/* greater than anything */
}

/* write char to binary file or directly to memory */

PUBLIC void putbin(c)
opcode_pt c;
{
    if (binfil != 0x0)
    {
	if (!binaryc)		/* pass 1, just record limits */
	{
	    if (binmbuf < binmin)
		binmin = binmbuf;
	    if (++binmbuf > binmax)
		binmax = binmbuf;
	}
	else
	{
	    if (binfbuf > binmbuf)
		error(BWRAP);	/* file buffer ahead of memory buffer */
	    else
	    {
		innum = binfil;
		while (binfbuf < binmbuf)
		{
		    writec(0x0);/* pad with nulls if file buffer behind */
		    ++binfbuf;
		}
		writec(c);
		binmbuf = ++binfbuf;
	    }
	}
    }
    else if (binaryc && !(lcdata & UNDBIT))
	/* memory output, and enabled */
    {
	register char *bufptr;

	if ((bufptr = (char *) binmbuf) >= asmbeg && bufptr < heapptr)
	    error(OWRITE);
	else
	    *bufptr = c;
	++binmbuf;
    }
}

/* write sized offset to binary file or directly to memory */

PRIVATE void putbinoffset(offset, size)
offset_t offset;
count_t size;
{
    char buf[sizeof offset];

#if SIZEOF_OFFSET_T > 0x2
    u4cn(buf, offset, size);
#else
    u2cn(buf, offset, size);
#endif
    putbin(buf[0]);
    if (size > 0x1)
	putbin(buf[1]);
    if (size > 0x2)
    {
	putbin(buf[2]);
	putbin(buf[3]);
    }
}
