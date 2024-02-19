/*
 *   bin86/ld/ld.c
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* ld.c - linker for Introl format (6809 C) object files 6809/8086/80386 */

#ifdef STDC_HEADERS_MISSING
extern int errno;
#else
#include <errno.h>
#endif

#include "const.h"
#include "byteord.h"
#include "type.h"
#include "globvar.h"

#define MAX_LIBS	(NR_STDLIBS + 5)
#define NR_STDLIBS	1

PUBLIC long text_base_address;	/* XXX */

PRIVATE bool_t flag[128];
PRIVATE char *libs[MAX_LIBS] = {
#ifdef MC6809
    "/usr/local/lib/m09/",
#else
    /* One of the following values will be inserted at run time. */
#   define std386lib	"/usr/local/lib/i386/"
#   define std86lib	"/usr/local/lib/i86/"
#endif
    0
};
PRIVATE int lastlib = NR_STDLIBS;

FORWARD char *buildname P((char *pre, char *mid, char *suf));
FORWARD char *expandlib P((char *fn));

PRIVATE char *buildname(pre, mid, suf)
char *pre;
char *mid;
char *suf;
{
    char *name;

    name = ourmalloc(strlen(pre) + strlen(mid) + strlen(suf) + 1);
    strcpy(name, pre);
    strcat(name, mid);
    strcat(name, suf);
    return name;
}

PRIVATE char *expandlib(fn)
char *fn;
{
    char *path;
    int i;

#ifndef MC6809
    libs[0] = flag['3'] ? std386lib : std86lib;
#endif

    for (i = lastlib - 1; i >= 0; --i)
    {
	path = ourmalloc(strlen(libs[i]) + strlen(fn) + 1);
	strcpy(path, libs[i]);
	strcat(path, fn);
	if (access(path, R_OK) == 0)
	    return path;
	ourfree(path);
    }
    return NULL;
}

PUBLIC int main(argc, argv)
int argc;
char **argv;
{
    register char *arg;
    int argn;
    static char crtprefix[] = "crt";
    static char crtsuffix[] = ".o";
    char *infilename;
    static char libprefix[] = "lib";
    static char libsuffix[] = ".a";
    char *outfilename;
    char *tfn;

    ioinit(argv[0]);
    objinit();
    syminit();
    typeconv_init(BIG_ENDIAN, LONG_BIG_ENDIAN);
    flag['z'] = flag['3'] = sizeof(char *) >= 4;
    outfilename = NULL;
    for (argn = 1; argn < argc; ++argn)
    {
	arg = argv[argn];
	if (*arg != '-')
	    readsyms(arg, flag['t']);
	else
	    switch (arg[1])
	    {
	    case '0':		/* use 16-bit libraries */
	    case '3':		/* use 32-bit libraries */
	    case 'M':		/* print symbols linked */
	    case 'i':		/* separate I & D output */
	    case 'm':		/* print modules linked */
#ifdef BSD_A_OUT
	    case 'r':		/* relocatable output */
#endif
	    case 's':		/* strip symbols */
	    case 't':		/* trace modules linked */
	    case 'z':		/* unmapped zero page */
		if (arg[2] == 0)
		    flag[arg[1]] = TRUE;
		else if (arg[2] == '-' && arg[3] == 0)
		    flag[arg[1]] = FALSE;
		else
		    usage();
		if (arg[1] == '0')	/* flag 0 is negative logic flag 3 */
		    flag['3'] = !flag['0'];
		break;
	    case 'C':		/* startfile name */
		tfn = buildname(crtprefix, arg + 2, crtsuffix);
		if ((infilename = expandlib(tfn)) == NULL)
		    fatalerror(tfn);
		readsyms(infilename, flag['t']);
		break;
	    case 'L':		/* library path */
		if (lastlib < MAX_LIBS)
		    libs[lastlib++] = arg + 2;
		else
		    fatalerror("too many library paths");
		break;
	    case 'O':		/* library file name */
		if ((infilename = expandlib(arg + 2)) == NULL)
		    fatalerror(arg);
		readsyms(infilename, flag['t']);
		break;
	    case 'T':		/* text base address */
		if (arg[2] != 0 || ++argn >= argc)
		    usage();
		errno = 0;    
		text_base_address = strtoul(argv[argn], (char **) NULL, 16);
		if (errno != 0)
		    use_error("invalid text address");
		break;
	    case 'l':		/* library name */
		tfn = buildname(libprefix, arg + 2, libsuffix);
		if ((infilename = expandlib(tfn)) == NULL)
		    fatalerror(tfn);
		readsyms(infilename, flag['t']);
		break;
	    case 'o':		/* output file name */
		if (arg[2] != 0 || ++argn >= argc || outfilename != NULL)
		    usage();
		outfilename = argv[argn];
		break;
	    default:
		usage();
	    }
    }
    linksyms(flag['r']);
    if (outfilename == NULL)
	outfilename = "a.out";
    writebin(outfilename, flag['i'], flag['3'], flag['s'], flag['z']);
    if (flag['m'])
	dumpmods();
    if (flag['M'])
	dumpsyms();
    flusherr();
    return errcount ? 1 : 0;
}
