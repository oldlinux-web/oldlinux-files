/*
 *   bin86/ld/io.c
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* io.c - input/output and error modules for linker */

#include <sys/types.h>
#include <sys/stat.h>
#define MY_STAT_H
#include <fcntl.h>
#include <unistd.h>
#include "const.h"
#include "obj.h"		/* needed for LONG_OFFSETS and offset_t */
#include "type.h"
#include "globvar.h"

#define DRELBUFSIZE	2048
#define ERR		(-1)
#define ERRBUFSIZE	1024
#define INBUFSIZE	1024
#define OUTBUFSIZE	2048
#define TRELBUFSIZE	1024

#ifdef BSD_A_OUT
PRIVATE char *drelbuf;		/* extra output buffer for data relocations */
PRIVATE char *drelbufptr;	/* data relocation output buffer ptr */
PRIVATE char *drelbuftop;	/* data relocation output buffer top */
#endif
PRIVATE char *errbuf;		/* error buffer (actually uses STDOUT) */
PRIVATE char *errbufptr;	/* error buffer ptr */
PRIVATE char *errbuftop;	/* error buffer top */
PRIVATE char *inbuf;		/* input buffer */
PRIVATE char *inbufend;		/* input buffer top */
PRIVATE char *inbufptr;		/* end of input in input buffer */
PRIVATE int infd;		/* input file descriptor */
PRIVATE char *inputname;	/* name of current input file */
PRIVATE char *outbuf;		/* output buffer */
PRIVATE char *outbufptr;	/* output buffer ptr */
PRIVATE char *outbuftop;	/* output buffer top */
PRIVATE int outfd;		/* output file descriptor */
PRIVATE unsigned outputperms;	/* permissions of output file */
PRIVATE char *outputname;	/* name of output file */
PRIVATE char *refname;		/* name of program for error reference */
#ifdef BSD_A_OUT
PRIVATE char *trelbuf;		/* extra output buffer for text relocations */
PRIVATE char *trelbufptr;	/* text relocation output buffer ptr */
PRIVATE char *trelbuftop;	/* text relocation output buffer top */
PRIVATE int trelfd;		/* text relocation output file descriptor */
#endif
PRIVATE unsigned warncount;	/* count of warnings */

FORWARD void errexit P((char *message));
FORWARD void flushout P((void));
#ifdef BSD_A_OUT
FORWARD void flushtrel P((void));
#endif
FORWARD void outhexdigs P((offset_t num));
FORWARD void outputerror P((char *message));
FORWARD void put04x P((unsigned num));
FORWARD void putstrn P((char *message));
FORWARD void refer P((void));

PUBLIC void ioinit(progname)
char *progname;
{
    infd = ERR;
    if (*progname)
	refname = progname;	/* name must be static (is argv[0]) */
    else
	refname = "link";
#ifdef BSD_A_OUT
    drelbuf = malloc(DRELBUFSIZE);
    drelbuftop = drelbuf + DRELBUFSIZE;
#endif
    errbuf = malloc(ERRBUFSIZE);
    errbufptr = errbuf;
    errbuftop = errbuf + ERRBUFSIZE;
    inbuf = malloc(INBUFSIZE);
    outbuf = malloc(OUTBUFSIZE);/* outbuf invalid if this fails but then */
				/* will not be used - tableinit() aborts */
    outbuftop = outbuf + OUTBUFSIZE;
#ifdef BSD_A_OUT
    trelbuf = malloc(TRELBUFSIZE);
    trelbuftop = trelbuf + TRELBUFSIZE;
#endif
}

PUBLIC void closein()
{
    if (infd != ERR && close(infd) < 0)
	inputerror("cannot close");
    infd = ERR;
}

PUBLIC void closeout()
{
#ifdef BSD_A_OUT
    unsigned nbytes;
#endif

    flushout();
#ifdef BSD_A_OUT
    flushtrel();
    nbytes = drelbufptr - drelbuf;
    if (write(trelfd, drelbuf, nbytes) != nbytes)
	outputerror("cannot write");
#endif
    if (close(outfd) == ERR)
	outputerror("cannot close");
#ifdef BSD_A_OUT
    if (close(trelfd) == ERR)
	outputerror("cannot close");
#endif
}

PUBLIC void errtrace(name, level)
char *name;
{
    while (level-- > 0)
	putbyte(' ');
    putstrn(name);
}

PUBLIC void executable()
{
    int oldmask;

    if (errcount == 0)
    {
	oldmask = umask(0);
	umask(oldmask);
	chmod(outputname, outputperms | (EXEC_PERMS & ~oldmask));
    }
}

PUBLIC void flusherr()
{
    write(STDOUT_FILENO, errbuf, errbufptr - errbuf);
    errbufptr = errbuf;
}

PRIVATE void flushout()
{
    unsigned nbytes;

    nbytes = outbufptr - outbuf;
    if (write(outfd, outbuf, nbytes) != nbytes)
	outputerror("cannot write");
    outbufptr = outbuf;
}

#ifdef BSD_A_OUT
PRIVATE void flushtrel()
{
    unsigned nbytes;

    nbytes = trelbufptr - trelbuf;
    if (write(trelfd, trelbuf, nbytes) != nbytes)
	outputerror("cannot write");
    trelbufptr = trelbuf;
}
#endif

PUBLIC void openin(filename)
char *filename;
{
#if 0 /* XXX - this probably won't work with constructed lib names? */
    if (infd == ERR || strcmp(inputname, filename) != 0)
#endif
    {
	closein();
	inputname = filename;	/* this relies on filename being static */
	if ((infd = open(filename, O_RDONLY)) < 0)
	    inputerror("cannot open");
	inbufptr = inbufend = inbuf;
    }
}

PUBLIC void openout(filename)
char *filename;
{
    struct stat statbuf;

    outputname = filename;
    if ((outfd = creat(filename, CREAT_PERMS)) == ERR)
	outputerror("cannot open");
    if (fstat(outfd, &statbuf) != 0) 
	outputerror("cannot stat");
    outputperms = statbuf.st_mode;
    chmod(filename, outputperms & ~EXEC_PERMS);
#ifdef BSD_A_OUT
    drelbufptr = drelbuf;
#endif
    outbufptr = outbuf;
#ifdef BSD_A_OUT
    if ((trelfd = open(filename, O_WRONLY)) == ERR)
	outputerror("cannot reopen");
    trelbufptr = trelbuf;
#endif
}

PRIVATE void outhexdigs(num)
register offset_t num;
{
    if (num >= 0x10)
    {
	outhexdigs(num / 0x10);
	num %= 0x10;
    }
    putbyte(hexdigit[num]);
}

PRIVATE void put04x(num)
register unsigned num;
{
    putbyte(hexdigit[num / 0x1000]);
    putbyte(hexdigit[(num / 0x100) & 0x0F]);
    putbyte(hexdigit[(num / 0x10) & 0x0F]);
    putbyte(hexdigit[num & 0x0F]);
}

#ifdef LONG_OFFSETS

PUBLIC void put08lx(num)
register offset_t num;
{
    put04x(num / 0x10000);
    put04x(num % 0x10000);
}

#else /* not LONG_OFFSETS */

PUBLIC void put08x(num)
register offset_t num;
{
    putstr("0000");
    put04x(num);
}

#endif /* not LONG_OFFSETS */

PUBLIC void putbstr(width, str)
unsigned width;
char *str;
{
    unsigned length;
    
    for (length = strlen(str); length < width; ++length)
	putbyte(' ');
    putstr(str);
}

PUBLIC void putbyte(ch)
int ch;
{
    register char *ebuf;

    ebuf = errbufptr;
    if (ebuf >= errbuftop)
    {
	flusherr();
	ebuf = errbufptr;
    }
    *ebuf++ = ch;
    errbufptr = ebuf;
}

PUBLIC void putstr(message)
char *message;
{
    while (*message != 0)
	putbyte(*message++);
}

PRIVATE void putstrn(message)
char *message;
{
    putstr(message);
    putbyte('\n');
    flusherr();
}

PUBLIC int readchar()
{
    int ch;
	
    register char *ibuf;
    int nread;

    ibuf = inbufptr;
    if (ibuf >= inbufend)
    {
	ibuf = inbufptr = inbuf;
	nread = read(infd, ibuf, INBUFSIZE);
	if (nread <= 0)
	{
	    inbufend = ibuf;
	    return ERR;
	}
	inbufend = ibuf + nread;
    }
    ch = (unsigned char) *ibuf++;
    inbufptr = ibuf;
    return ch;
}

PUBLIC void readin(buf, count)
char *buf;
unsigned count;
{
    int ch;
    
    while (count--)
    {
	if ((ch = readchar()) < 0)
	    prematureeof();
	*buf++ = ch;
    }
}

PUBLIC bool_pt readineofok(buf, count)
char *buf;
unsigned count;
{
    int ch;
    
    while (count--)
    {
	if ((ch = readchar()) < 0)
	    return TRUE;
	*buf++ = ch;
    }
    return FALSE;
}

PUBLIC void seekin(offset)
long offset;
{
    inbufptr = inbufend = inbuf;
    if (lseek(infd, offset, SEEK_SET) < 0)
	prematureeof();
}

PUBLIC void seekout(offset)
long offset;
{
    flushout();
    if (lseek(outfd, offset, SEEK_SET) != offset)
	outputerror("cannot seek in");
}

#ifdef BSD_A_OUT
PUBLIC void seektrel(offset)
long offset;
{
    flushtrel();
    if (lseek(trelfd, offset, SEEK_SET) != offset)
	outputerror("cannot seek in");
}
#endif

PUBLIC void writechar(ch)
int ch;
{
    register char *obuf;

    obuf = outbufptr;
    if (obuf >= outbuftop)
    {
	flushout();
	obuf = outbufptr;
    }
    *obuf++ = ch;
    outbufptr = obuf;
}

#ifdef BSD_A_OUT
PUBLIC void writedrel(buf, count)
register char *buf;
unsigned count;
{
    register char *rbuf;

    rbuf = drelbufptr;
    while (count--)
    {
	if (rbuf >= drelbuftop)
	    inputerror("data relocation buffer full while processing");
	*rbuf++ = *buf++;
    }
    drelbufptr = rbuf;
}
#endif

PUBLIC void writeout(buf, count)
register char *buf;
unsigned count;
{
    register char *obuf;

    obuf = outbufptr;
    while (count--)
    {
	if (obuf >= outbuftop)
	{
	    outbufptr = obuf;
	    flushout();
	    obuf = outbufptr;
	}
	*obuf++ = *buf++;
    }
    outbufptr = obuf;
}

#ifdef BSD_A_OUT
PUBLIC void writetrel(buf, count)
register char *buf;
unsigned count;
{
    register char *rbuf;

    rbuf = trelbufptr;
    while (count--)
    {
	if (rbuf >= trelbuftop)
	{
	    trelbufptr = rbuf;
	    flushtrel();
	    rbuf = trelbufptr;
	}
	*rbuf++ = *buf++;
    }
    trelbufptr = rbuf;
}
#endif

/* error module */

PRIVATE void errexit(message)
char *message;
{
    putstrn(message);
    exit(2);
}

PUBLIC void fatalerror(message)
char *message;
{
    refer();
    errexit(message);
}

PUBLIC void inputerror(message)
char *message;
{
    refer();
    putstr(message);
    putstr(" input file ");
    errexit(inputname);
}

PUBLIC void input1error(message)
char *message;
{
    refer();
    putstr(inputname);
    errexit(message);
}

PRIVATE void outputerror(message)
char *message;
{
    refer();
    putstr(message);
    putstr(" output file ");
    errexit(outputname);
}

PUBLIC void outofmemory()
{
    inputerror("out of memory while processing");
}

PUBLIC void prematureeof()
{
    inputerror("premature end of");
}

PUBLIC void redefined(name, message, archentry, deffilename, defarchentry)
char *name;
char *message;
char *archentry;
char *deffilename;
char *defarchentry;
{
    ++warncount;
    refer();
    putstr("warning: ");
    putstr(name);
    putstr(" redefined");
    putstr(message);
    putstr(" in file ");
    putstr(inputname);
    if (archentry != NULL)
    {
	putbyte('(');
	putstr(archentry);
	putbyte(')');
    }
    putstr("; using definition in ");
    putstr(deffilename);
    if (defarchentry != NULL)
    {
	putbyte('(');
	putstr(defarchentry);
	putbyte(')');
    }
    putbyte('\n');
}

PRIVATE void refer()
{
    putstr(refname);
    putstr(": ");
}

PUBLIC void reserved(name)
char *name;
{
    ++errcount;
    putstr("incorrect use of reserved symbol: ");
    putstrn(name);
}

PUBLIC void size_error(seg, count, size)
char seg;
offset_t count;
offset_t size;
{
    refer();
    putstr("seg ");
    outhexdigs((offset_t) seg);
    putstr(" has wrong size ");
    outhexdigs(count);
    putstr(", supposed to be ");
    outhexdigs(size);
    errexit("\n");
}

PUBLIC void undefined(name)
char *name;
{
    ++errcount;
    putstr("undefined symbol: ");
    putstrn(name);
}

PUBLIC void usage()
{
    putstr("usage: ");
    putstr(refname);
#ifdef BSD_A_OUT
    errexit("\
 [-03Mimrstz[-]] [-llib_extension] [-o outfile] [-Ccrtfile]\n\
       [-L libdir] [-Olibfile] [-T textaddr] infile...");
#else
    errexit("\
 [-03Mimstz[-]] [-llib_extension] [-o outfile] [-Ccrtfile]\n\
       [-L libdir] [-Olibfile] [-T textaddr] infile...");
#endif
}

PUBLIC void use_error(message)
char *message;
{
    refer();
    putstrn(message);
    usage();
}
