/*
 *   bin86/as/as.c
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* as.c - assembler */

/*
  usage: as source [-b [bin]] [-lm [list]] [-n name] [-o obj] [-s sym] [-guw]
  in any order (but no repeated file options)
*/

#include <sys/types.h>
#include <fcntl.h>
#include "const.h"
#include "type.h"
#include "byteord.h"
#include "macro.h"
#undef EXTERN
#define EXTERN
#include "file.h"
#include "flag.h"
#include "globvar.h"

PRIVATE struct block_s hid_blockstak[MAXBLOCK];	/* block stack */
PRIVATE struct lc_s hid_lctab[NLOC];	/* location counter table */
PRIVATE struct if_s hid_ifstak[MAXBLOCK];	/* if stack */
PRIVATE struct schain_s hid_mcpar[MACPSIZ];	/* MACRO params */
PRIVATE struct macro_s hid_macstak[MAXBLOCK];	/* macro stack */
PRIVATE struct sym_s *hid_spt[SPTSIZ];	/* hash table */

FORWARD void initp1 P((void));
FORWARD int my_creat P((char *name, char *message));
FORWARD void process_args P((int argc, char **argv));
FORWARD void summary P((fd_t fd));
FORWARD void summ_number P((unsigned num));
FORWARD void usage P((void));

#define USERMEM (sizeof(int) <= 2 ? (unsigned) 0xAC00 : (unsigned) 0x20000L)

PUBLIC int main(argc, argv)
int argc;
char **argv;
{
    heapptr = malloc(USERMEM);
    heapend = heapptr + USERMEM;
    if (heapptr == 0)
	as_abort("cannot allocate memory");
#ifdef SOS_EDOS
    heapend = stackreg() - STAKSIZ;
#endif
    initp1();
    initp1p2();
    inst_keywords();
    initbin();
    initobj();
    initsource();		/* only nec to init for unsupported mem file */
    typeconv_init(BIG_ENDIAN, LONG_BIG_ENDIAN);
    warn.global = TRUE;		/* constant */
    process_args(argc, argv);
    initscan();

    assemble();			/* doesn't return, maybe use setjmp */

    /* NOTREACHED */
    return 0;
}

PUBLIC void as_abort(message)
char *message;
{
    write(STDOUT, "as: ", 4);
    write(STDOUT, message, strlen(message));
    write(STDOUT, "\n", 1);
    exit(1);
}

PUBLIC void finishup()
{
    bintrailer();
    objtrailer();
    if (list.global ||symgen)
	gensym();		/* output to lstfil and/or symfil */
    if (list.global ||toterr != 0 || totwarn != 0)
	summary(lstfil);
    if (lstfil != STDOUT && (toterr != 0 || totwarn != 0))
	summary(STDOUT);
    statistics();
    exit(toterr != 0 ? 1 : 0);	/* should close output files and check */
}

/* initialise constant nonzero values */

PRIVATE void initp1()
{
#ifdef I80386
    idefsize = defsize = sizeof (char *) > 2 ? 4 : 2;
#endif
    lctabtop = lctab + NLOC;
    lstfil = STDOUT;
    mapnum = 15;		/* default map number for symbol table */
    spt_top = (spt = hid_spt) + SPTSIZ;
}

/* initialise nonzero values which start same each pass */

PUBLIC void initp1p2()
{
    register struct lc_s *lcp;

    ifflag = TRUE;
    pedata = UNDBIT;		/* program entry point not defined */
    blockstak = hid_blockstak + MAXBLOCK;
    ifstak = hid_ifstak + MAXIF;
    macstak = hid_macstak + MAXMAC;
    macptop = (macpar = hid_mcpar) + MACPSIZ;
    lctabtop = (lcptr = lctab = hid_lctab) + NLOC;
    for (lcp = lctab; lcp < lctabtop; ++lcp)
	/* init of lcdata/lc (many times) in loop to save space */
    {
	lcp->data = lcdata = RELBIT;	/* lc relocatable until 1st ORG */
	lcp->lc = lc = 0;
    }
}

PRIVATE int my_creat(name, message)
char *name;
char *message;
{
    int fd;

    if ((fd = creat(name, CREAT_PERMS)) < 0 || fd > 255)
	as_abort(message);
    return fd;
}

PRIVATE void process_args(argc, argv)
int argc;
char **argv;
{
    char *arg;
    bool_t isnextarg;
    char *nextarg;

    if (argc <= 1)
	usage();
    do
    {
	arg = *++argv;
	if (arg[0] == '-')
	{
	    if (arg[2] != 0)
		usage();	/* no multiple options */
	    isnextarg = FALSE;
	    if (argc > 2)
	    {
		nextarg = argv[1];
		if (nextarg[0] != 0 && nextarg[0] != '-')
		    isnextarg = TRUE;
	    }
	    switch (arg[1])
	    {
#ifdef I80386
	    case '0':
		idefsize = defsize = 0x2;
		break;
	    case '3':
		idefsize = defsize = 0x4;
		break;
	    case 'a':
		asld_compatible = TRUE;
		break;
#endif
	    case 'b':
		if (!isnextarg || binfil != 0)
		    usage();
		binfil = my_creat(nextarg, "error creating binary file");
		binaryg = TRUE;
		--argc;
		++argv;
		break;
	    case 'g':
		globals_only_in_obj = TRUE;
		break;
#ifdef I80386
	    case 'j':
		jumps_long = TRUE;
		break;
#endif
	    case 'l':
		list.global = TRUE;
		goto get_any_list_file;
	    case 'm':
		maclist.global = TRUE;
	get_any_list_file:
		if (isnextarg)
		{
		    if (lstfil != STDOUT)
			usage();
		    lstfil = my_creat(nextarg, "error creating list file");
		    --argc;
		    ++argv;
		}
		break;
	    case 'n':
		if (!isnextarg)
		    usage();
		truefilename = nextarg;
		--argc;
		++argv;
		break;
	    case 'o':
		if (!isnextarg || objfil != 0)
		    usage();
		objectg = TRUE;
		objfil = my_creat(nextarg, "error creating object file");
		--argc;
		++argv;
		break;
	    case 's':
		if (!isnextarg || symfil != 0)
		    usage();
		symgen = TRUE;
		symfil = my_creat(nextarg, "error creating symbol file");
		--argc;
		++argv;
		break;
	    case 'u':
		inidata = IMPBIT | SEGM;
		break;
	    case 'w':
		warn.semaphore = -1;
		break;
	    default:
		usage();	/* bad option */
	    }
	}
	else if (infil != 0)
	    usage();		/* no multiple sources */
	else
	{
	    if (strlen(arg) > FILNAMLEN)
		as_abort("source file name too long");
	    infil = open_input(strcpy(filnamptr, arg));
	    infiln = infil0 = 1;
	}
    }
    while (--argc != 1);
    inidata = (~binaryg & inidata) | (RELBIT | UNDBIT);
}				/* IMPBIT from inidata unless binaryg */

PRIVATE void summary(fd)
int fd;
{
    innum = fd;
    writenl();
    summ_number(toterr);
    writesn(" errors");
    summ_number(totwarn);
    writesn(" warnings");
}

PRIVATE void summ_number(num)
unsigned num;
{
    /* format number like line numbers, build it at free spot heapptr */
    *build_number(num, LINUM_LEN, heapptr) = 0;
    writes(heapptr);
}

PRIVATE void usage()
{
    as_abort(
#ifdef I80386
"usage: as [-03agjuw] [-b [bin]] [-lm [list]] [-n name] [-o obj] [-s sym] src");
#else
    "usage: as [-guw] [-b [bin]] [-lm [list]] [-n name] [-o obj] [-s sym] src");
#endif
}
