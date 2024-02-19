/*
 *   bin86/as/genlist.c
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* genlist.c - generate listing and error reports for assembler */

#include "const.h"
#include "type.h"
#include "address.h"
#include "flag.h"
#include "file.h"
#include "globvar.h"
#include "macro.h"
#include "scan.h"
#include "source.h"

#define CODE_LIST_LENGTH (sizeof (struct code_listing_s) - 1)
				/* length of formatted code listing */
#define MAXERR 6		/* maximum errors listed per line */

struct error_s			/* to record error info */
{
    unsigned char errnum;
    unsigned char position;
};

/* code listing format */

struct code_listing_s
{
    union linum_macro_u
    {
	char linum[LINUM_LEN];
	struct
	{
	    char pad[1];
	    char mark[1];
	    char level[1];
	}
	 macro;
    }
     linum_or_macro;
    char padlinum[1];
    char lc[4];
    char padlc[1];
#ifdef I80386
    char lprefix[2];
    char aprefix[2];
    char oprefix[2];
    char sprefix[2];
#endif
    char page[2];
    char opcode[2];
    char postb[2];
#ifdef I80386
    char sib[2];
#endif
    char padopcode[1];
#if SIZEOF_OFFSET_T > 2
    char displ4[2];
    char displ3[2];
#endif
    char displ2[2];
    char displ1[2];
    char reldispl[1];
    char paddispl[1];
#ifdef I80386
    char imm4[2];
    char imm3[2];
    char imm2[2];
    char imm1[2];
    char relimm[1];
    char padimm[1];
#endif
    char nullterm;
};

PRIVATE struct error_s errbuf[MAXERR];	/* error buffer */
PRIVATE unsigned char errcount;	/* # errors in line */
PRIVATE bool_t erroverflow;	/* set if too many errors on line */

FORWARD char *build_1hex_number P((unsigned num, char *where));
FORWARD void list1 P((fd_t fd));
FORWARD void listcode P((void));
FORWARD void listerrors P((void));
FORWARD void paderrorline P((unsigned nspaces));

/* format 1 byte number as 2 hex digits, return ptr to end */

PRIVATE char *build_1hex_number(num, where)
unsigned num;
register char *where;
{
    static char hexdigits[] = "0123456789ABCDEF";

    where[0] = hexdigits[(num % 256) / 16];
    where[1] = hexdigits[num % 16];
    return where + 2;
}

/* format 2 byte number as 4 hex digits, return ptr to end */

PUBLIC char *build_2hex_number(num, where)
unsigned num;
char *where;
{
    return build_1hex_number(num, build_1hex_number(num / 256, where));
}

/* format 2 byte number as decimal with given width (pad with leading '0's) */
/* return ptr to end */

PUBLIC char *build_number(num, width, where)
unsigned num;
unsigned width;
register char *where;
{
    static unsigned powers_of_10[] = {1, 10, 100, 1000, 10000,};
    unsigned char digit;
    unsigned char power;
    register unsigned power_of_10;

    power = 5;			/* actually 1 more than power */
    do
    {
	for (digit = '0', power_of_10 = (powers_of_10 - 1)[power];
	     num >= power_of_10; num -= power_of_10)
	    ++digit;
	if (power <= width)
	    *where++ = digit;
    }
    while (--power != 0);
    return where;
}

/* record number and position of error (or error buffer overflow) */

PUBLIC void error(errnum)
error_pt errnum;
{
    register struct error_s *errptr;
    register struct error_s *errptrlow;
    unsigned char position;

    if (errnum < MINWARN || warn.current)
    {
	if (errcount >= MAXERR)
	    erroverflow = TRUE;
	else
	{
	    position = symname - linebuf;
	    for (errptr = errbuf + errcount;
		 errptr > errbuf && errptr->position > position;
		 errptr = errptrlow)
	    {
		errptrlow = errptr - 1;
		errptr->errnum = errptrlow->errnum;
		errptr->position = errptrlow->position;
	    }
	    errptr->errnum = errnum;
	    errptr->position = position;
	    ++errcount;
	    if (errnum >= MINWARN)
		++totwarn;
	    else
		++toterr;
	}
    }
}

/* list 1 line to list file if any errors or flags permit */
/* list line to console as well if any errors and list file is not console */

PUBLIC void listline()
{
    if (!listpre)
    {
	if (errcount || list.current && (!macflag || mcount != 0) ||
	    macflag && maclist.current)
	    list1(lstfil);
	if (errcount)
	{
	    if (lstfil != STDOUT)
		list1(STDOUT);
	    errcount = 0;
	    erroverflow = FALSE;
	}
    }
}

/* list 1 line unconditionally */

PRIVATE void list1(fd)
fd_t fd;
{
    innum = fd;
    listcode();
    write(innum, linebuf, lineptr - linebuf);
    writenl();
    if (errcount != 0)
	listerrors();
    listpre = TRUE;
}

/* list object code for 1 line */

PRIVATE void listcode()
{
    unsigned char count;
    struct code_listing_s *listptr;
    int numlength;
    char *numptr;

    listptr = (struct code_listing_s *) heapptr;
    memset((char *) listptr, ' ', sizeof *listptr);
    listptr->nullterm = 0;
    if (macflag)
    {
	listptr->linum_or_macro.macro.mark[0] = '+';
	listptr->linum_or_macro.macro.level[0] = maclevel + ('a' - 1);
    }
    else
    {
	numlength = LINUM_LEN;
	numptr = listptr->linum_or_macro.linum;
	if (infiln != infil0)
	{
	    *numptr++ = infiln - infil0 + ('a' - 1);
	    numlength = LINUM_LEN - 1;
	}
	build_number(linum, numlength, numptr);
    }
    if ((count = mcount) != 0 || popflags & POPLC)
	build_2hex_number((u16_t) lc, listptr->lc);
    if (popflags & POPLO)
    {
#if SIZEOF_OFFSET_T > 2
	if (popflags & POPLONG)
	    build_2hex_number((u16_t) (lastexp.offset / (offset_t) 0x10000L),
			      listptr->displ4);
#endif
	if (popflags & POPHI)
	    build_2hex_number((u16_t) lastexp.offset, listptr->displ2);
	else
	    build_1hex_number((u16_t) lastexp.offset, listptr->displ1);
	if (lastexp.data & RELBIT)
	    listptr->reldispl[0] = '>';
    }
    else if (count != 0)
    {
#ifdef I80386
	if (aprefix != 0)
	{
	    --count;
	    build_1hex_number(aprefix, listptr->aprefix);
	}
	if (oprefix != 0)
	{
	    --count;
	    build_1hex_number(oprefix, listptr->oprefix);
	}
	if (sprefix != 0)
	{
	    --count;
	    build_1hex_number(sprefix, listptr->sprefix);
	}
#endif
	if (page != 0)
	{
	    build_1hex_number(page, listptr->page);
	    --count;
	}
	build_1hex_number(opcode, listptr->opcode);
	--count;
	if (postb != 0)
	{
	    --count;
	    build_1hex_number(postb,
#ifdef MC6809
			      count == 0 ? listptr->displ1 :
#endif
			      listptr->postb);
	}
#ifdef I80386
	if (sib != NO_SIB)
	{
	    --count;
	    build_1hex_number(sib, listptr->sib);
	}
#endif
	if (count > 0)
	{
	    build_1hex_number((opcode_pt) lastexp.offset, listptr->displ1);
	    if (lastexp.data & RELBIT)
		listptr->reldispl[0] = '>';
	}
	if (count > 1)
	    build_1hex_number((opcode_pt) lastexp.offset >> 0x8,
			      listptr->displ2);
#if SIZEOF_OFFSET_T > 2
	if (count > 2)
	{
	    build_1hex_number((opcode_pt) (lastexp.offset >> 0x10),
			      listptr->displ3);
	    build_1hex_number((opcode_pt) (lastexp.offset >> 0x18),
			      listptr->displ4);
	}
#endif
#ifdef I80386
	if (immcount > 0)
	{
	    build_1hex_number((opcode_pt) immadr.offset, listptr->imm1);
	    if (immadr.data & RELBIT)
		listptr->relimm[0] = '>';
	}
	if (immcount > 1)
	    build_1hex_number((opcode_pt) immadr.offset >> 0x8,
			      listptr->imm2);
	if (immcount > 2)
	{
	    build_1hex_number((opcode_pt) (immadr.offset >> 0x10),
			      listptr->imm3);
	    build_1hex_number((opcode_pt) (immadr.offset >> 0x18),
			      listptr->imm4);
	}
#endif
    }
    writes((char *) listptr);
}

/* list errors, assuming some */

PRIVATE void listerrors()
{
    unsigned char column;
    unsigned char errcol;	/* column # in error line */
    unsigned char errcolw;	/* working column in error line */
    char *errmsg;
    struct error_s *errptr;
    char *linep;
    unsigned char remaining;

    paderrorline(CODE_LIST_LENGTH - LINUM_LEN);
    remaining = errcount;
    column = 0;			/* column to match with error column */
    errcolw = errcol = CODE_LIST_LENGTH; /* working & col number on err line */
    errptr = errbuf;
    linep = linebuf;
    do
    {
	while (column < errptr->position)
	{
	    ++column;
	    if (*linep++ == '\t')	/* next tab (standard tabs only) */
		errcolw = (errcolw + 8) & 0xf8;
	    else
		++errcolw;
	    while (errcolw > errcol)
	    {
		writec(' ');
		++errcol;
	    }
	}
	if (errcolw < errcol)	/* position under error on new line */
	{
	    writenl();
	    paderrorline(errcolw - LINUM_LEN);
	}
	writec('^');
	writes(errmsg = build_error_message(errptr->errnum, heapptr));
	errcol += strlen(errmsg);
	++errptr;
    }
    while (--remaining != 0);
    writenl();
    if (erroverflow)
    {
	paderrorline(CODE_LIST_LENGTH - LINUM_LEN);
	writesn(build_error_message(FURTHER, heapptr));
    }
}

/* pad out error line to begin under 1st char of source listing */

PRIVATE void paderrorline(nspaces)
unsigned nspaces;
{
    int nstars = LINUM_LEN;

    while (nstars-- != 0)
	writec('*');		/* stars under line number */
    while (nspaces-- != 0)
	writec(' ');		/* spaces out to error position */
}

/* write 1 character */

PUBLIC void writec(c)
char c;
{
    write(innum, &c, 1);
}

/* write newline */

PUBLIC void writenl()
{
    writes(SOS_EOLSTR);
}

/* write 1 offset_t, order to suit target */

PUBLIC void writeoff(offset)
offset_t offset;
{
    char buf[sizeof offset];

#if SIZEOF_OFFSET_T > 2
    u4c4(buf, offset);
#else
    u2c2(buf, offset);
#endif
    write(innum, buf, sizeof buf);
}

/* write string */

PUBLIC void writes(s)
char *s;
{
    write(innum, s, strlen(s));
}

/* write string followed by newline */

PUBLIC void writesn(s)
char *s;
{
    writes(s);
    writenl();
}

/* write 1 word, order to suit target */

PUBLIC void writew(word)
unsigned word;
{
    char buf[2];

    u2c2(buf, (u16_t) word);
    write(innum, buf, sizeof buf);
}
