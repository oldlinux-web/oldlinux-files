/*
 *   bin86/as/readsrc.c
 *
 *   Copyright (C) 1992 Bruce Evans
 */

#define MAXLINE 256		/* when this is made bigger, fix pfcc not
				 * to store the string length in a byte-
				 * sized variable */

/* readsrc.c - read source files for assembler */

#include <sys/types.h>
#include <fcntl.h>
#include "const.h"
#include "type.h"
#include "flag.h"
#include "file.h"
#include "globvar.h"
#include "macro.h"
#include "scan.h"
#undef EXTERN
#define EXTERN
#include "source.h"

struct fcbstruct		/* file control block structure */
{
    fd_t fd;			/* file descriptor */
    char *lineptr;		/* current spot in line */
    char *buf;			/* start of buffer (after partial line) */
    char *first;		/* start of partial line before buf */
    char *limit;		/* end of used part of input buffer */
    unsigned blocksize;		/* chars from last read and partial line flag */
    struct fbufstruct *includer;/* buffer of file which included current one */
};

struct fbufstruct		/* file buffer structure */
{
    struct fcbstruct fcb;	/* status after opening an include sub-file */
    char fpartline[MAXLINE + 1];/* buffer for partial line */
    char fbuf[INBUFSIZE + 1];	/* buffer to read into */
    char fname[1];		/* file name (variable length), 1 for null */
};

struct get_s			/* to record included files */
{
    fd_t fd;
    unsigned line;
    long position;
};

PRIVATE char hid_filnambuf[FILNAMLEN + 1];	/* buffer for file name */

PRIVATE struct get_s hid_getstak[MAXGET];	/* GET stack */
PRIVATE struct get_s *getstak;	/* ptr */

PRIVATE struct fcbstruct input;	/* current input file control block */
				/* input.lineptr is not kept up to date */
				/* input.fd depends on zero init */
PRIVATE struct fbufstruct xyz;
PRIVATE struct fbufstruct *inputbuf;	/* current input file buffer */
					/* its fcb only to date in includes */

PRIVATE char hid_linebuf[LINLEN];	/* line buffer */
PRIVATE char *maclinebuf;
PRIVATE char *maclineptr;

FORWARD void clearsource P((void));
FORWARD void line_too_long P((void));

PRIVATE void clearsource()
{
    input.includer = inputbuf;
    inputbuf = &xyz;
    input.first = input.limit = input.buf = inputbuf->fbuf;
    *(lineptr = linebuf = input.first - 1) = EOLCHAR;
    input.blocksize = 0;
}

PRIVATE void line_too_long()
{
    symname = linebuf + (LINLEN - 1);	/* spot for the error */
    error(LINLONG);		/* so error is shown in column LINLEN - 1 */
}

/* initialise private variables */

PUBLIC void initsource()
{
    filnamptr = hid_filnambuf;
    getstak = hid_getstak + MAXGET;
    clearsource();		/* sentinel to invoke blank skipping */
}

PUBLIC fd_t open_input(name)
char *name;
{
    fd_t fd;

    if ((unsigned) (fd = open(name, O_RDONLY)) > 255)
	as_abort("error opening input file");
    clearsource();
    return fd;
}

/*
  handle GET pseudo_op
  stack state of current file, open new file and reset global state vars
  file must be seekable for the buffer discard/restore method to work
*/

PUBLIC void pget()
{
#if OLD
    if (infiln >= MAXGET)
	error(GETOV);
    else
    {
	skipline();
	listline();
	if (infiln != 0)
	{
	    --getstak;
	    getstak->fd = infil;
	    getstak->line = linum;
	    getstak->position = lseek(infil, 0L, 1) - (inbufend - inbufptr);
	    ++infiln;
	    linum = 0;
	    infil = open_input(lineptr - 1);
	}
    }
#else
    abort();
#endif
}

/* process end of file */
/* close file, unstack old file if current one is included */
/* otherwise switch pass 0 to pass 1 or exit on pass 2 */
/* end of file may be from phyical end of file or an END statement */

PUBLIC void pproceof()
{
    if (infiln != 0)
	close(infil);
    if (lineptr == linebuf)
	list.current = FALSE;	/* don't list line after last unless error */
    if (infiln == infil0)
	/* all conditionals must be closed before end of main file (not GETs) */
    {
	if (blocklevel != 0)
	    error(EOFBLOCK);
	if (iflevel != 0)
	    error(EOFIF);
	if (lcdata & UNDBIT)
	    error(EOFLC);
	lcptr->data = lcdata;
	lcptr->lc = lc;
    }
    /* macros must be closed before end of all files */
    if (macload)
	error(EOFMAC);
    listline();			/* last line or line after last if error */
    if (infiln != infil0)
    {
	infil = getstak->fd;
	linum = getstak->line;
	if (--infiln != 0)
	    lseek(infil, getstak->position, 0);
	++getstak;
    }
    else if (!pass)
    {
	pass = TRUE;
	objheader();		/* while pass 1 data all valid */
	binmbuf = 0;		/* reset zero variables */
	maclevel = iflevel = blocklevel =
	    totwarn = toterr = linum = macnum = 0;
	initp1p2();		/* reset other varaiables */
	binaryc = binaryg;
#ifdef I80386
	defsize = idefsize;
#endif
	list.current = list.global;
	maclist.current = maclist.global;

	warn.current = TRUE;
	if (warn.semaphore < 0)
	    warn.current = FALSE;
	if (infiln != 0)
	    infil = open_input(filnamptr);
	binheader();
    }
    else
	finishup();
}

/*
  read 1 line of source.
  Source line ends with '\n', line returned is null terminated without '\n'.
  Control characters other than blank, tab and newline are discarded.
  Long lines (length > LINLEN) are truncated, and an error is generated.
  On EOF, calls pproceof(), and gets next line unless loading a macro.
  This is where macro lines are recursively expanded.
*/

PUBLIC void readline()
{
    listpre = FALSE;		/* not listed yet */
    if (maclevel != 0)
    {
      register char *bufptr;	/* hold *bufptr in a reg char variable */
      register char *reglineptr;	/* if possible (not done here) */
      char *oldbufptr;
      struct schain_s *parameters;
      char paramnum;
      unsigned char remaining;	/* space remaining in line + 2 */
				/* value 0 not used except for temp predec */
				/* value 1 means error already gen */
				/* values 1 and 2 mean no space */

      for (; maclevel != 0;
	     macpar = macstak->parameters, ++macstak, --maclevel)
	if (*(bufptr = macstak->text) != ETB)
 /* nonempty macro, process it and return without continuing the for loop */
	{
	    if (!macflag)
	    {
		maclinebuf = linebuf;
		maclineptr = lineptr;
		macflag = TRUE;
	    }
	    remaining = LINLEN + 2;
	    lineptr = linebuf = reglineptr = hid_linebuf;
	    while (*bufptr++ != EOLCHAR)
	    {
		if (bufptr[-1] == MACROCHAR && *bufptr >= '0' && *bufptr <= '9')
		{
		    parameters = macstak->parameters;
		    for (paramnum = *bufptr++; paramnum-- != '0';)
			if ((parameters = parameters->next) == NULL)
			    break;
		    if (parameters != NULL)
		    {
			for (oldbufptr = bufptr, bufptr = parameters->string;
			     *bufptr++ != 0;)
			{
			    if (--remaining <= 1)
			    {
				if (remaining != 0)
				    line_too_long();
				remaining = 1;
				break;	/* forget rest, param on 1 line */
			    }
			    *reglineptr++ = bufptr[-1];
			}
			bufptr = oldbufptr;
		    }
		}
		else
		{
		    if (--remaining <= 1)
		    {
			if (remaining != 0)
			    line_too_long();
			remaining = 1;
		    }
		    else
			*reglineptr++ = bufptr[-1];
		}
	    }
	    macstak->text = bufptr;
	    *reglineptr = EOLCHAR;
	    return;
	}
    }
    if (macflag)
    {
	linebuf = maclinebuf;
	lineptr = maclineptr;
	macflag = FALSE;
    }
again:
    ++linum;
    ++lineptr;			/* if eof, this is input.limit + 1 */
    if (input.blocksize != 0)	/* and this routine just resets eof */
    {
	if (lineptr < input.limit)	/* move back partial line */
	{
	    register char *col;

	    col = input.buf;
	    while ((*--col = *--input.limit) != EOLCHAR)
		;
	    input.first = col + 1;
	    ++input.limit;
	    input.blocksize = 0;
	}
	else			/* may be non-terminated line, don't stop */
	    lineptr = input.limit;
    }
    if (lineptr == input.limit)
    {
	lineptr = input.first;
	input.blocksize = read(infil, input.buf, INBUFSIZE);
	if (input.blocksize < 0)
		abort();
	if (input.blocksize == 0)
	{
	    clearsource();
	    pproceof();
	    if (macload)
	    {
		symname = lineptr;
		return;		/* macro not allowed across eof */
	    }
	    goto again;
	}
	input.first = input.buf;
	*(input.limit = input.buf + input.blocksize) = EOLCHAR;
    }
    linebuf = lineptr;
    if (lineptr >= input.limit)
	*(lineptr = input.limit = input.buf) = EOLCHAR;
}

PUBLIC void skipline()
{
    register char *reglineptr;

    reglineptr = lineptr - 1;
    while (*reglineptr != EOLCHAR)
	++reglineptr;
    lineptr = reglineptr;
}
