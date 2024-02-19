#include "config.h"

/*
 *	main.c - main for nroff word processor
 *
 *	similar to Unix(tm) nroff or RSX-11M RNO. adaptation of text processor
 *	given in "Software Tools", Kernighan and Plauger.
 *
 *	adapted for atariST/TOS by Bill Rosenkranz 11/89
 *	net:	rosenkra@hall.cray.com
 *	CIS:	71460,17
 *	GENIE:	W.ROSENKRANZ
 *
 *	original author:
 *
 *	Stephen L. Browning
 *	5723 North Parker Avenue
 *	Indianapolis, Indiana 46220
 *
 *	history:
 *
 *	- Originally written in BDS C;
 *	- Adapted for standard C by W. N. Paul
 *	- Heavily hacked up to conform to "real" nroff by Bill Rosenkranz
 */

#define NRO_MAIN			/* to define globals in nro.h */

#include <stdio.h>
#ifdef GEMDOS
#include <sys\types.h>
#include <sys\time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include "nroff.h"


main (argc, argv)
int     argc;
char   *argv[];
{
	register int	i;
	int		swflg;
	int		ifp = 0;
	char	       *ptmp;
#ifndef GEMDOS
	char	       *pterm;
	char		capability[100];
	char	       *pcap;
	char	       *ps;
#endif



	/*
	 *   set up initial flags and file descriptors
	 */
	swflg       = FALSE;
	ignoring    = FALSE;
	hold_screen = FALSE;
	debugging   = FALSE;
	stepping    = FALSE;
	mc_ing      = FALSE;
	out_stream  = stdout;
	err_stream  = stderr;
	dbg_stream  = stderr;


	/*
	 *   this is for tmp files, if ever needed. it SHOULD start
	 *   out without the trailing slash. if not in env, use default
	 */
	if (ptmp = getenv ("TMPDIR"))
		strcpy (tmpdir, ptmp);
	else
		strcpy (tmpdir, ".");


	/*
	 *   handle terminal for \fB, \fI
	 */
#ifdef GEMDOS
	strcpy (s_standout, "\33p");		/* atari/TOS is easy... */
	strcpy (e_standout, "\33q");
	strcpy (s_bold, "\33p");		/* atari/TOS is easy... */
	strcpy (e_bold, "\33q");
	strcpy (s_italic, "\33p");
	strcpy (e_italic, "\33q");
#else
	s_standout[0] = '\0';
	e_standout[0] = '\0';
	s_bold[0]     = '\0';
	e_bold[0]     = '\0';
	s_italic[0]   = '\0';
	e_italic[0]   = '\0';
	if (pterm = getenv ("TERM"))		/* it must exist first... */
	{
		/*
		 *   we currently use standout mode for all weirdness
		 *   lile BOLD, italic, etc.
		 */
		pcap = capability;
		if (ps = "\\E[7m") /*sorry had to hard code these for damn linux*/
		{
			/*
			 *   sun has padding in here. this is NOT portable.
			 *   better to use tputs() to strip it...
			 */
			while (*ps && *ps != 0x1b)	ps++;
			strcpy (s_standout, ps);
			strcpy (s_bold, ps);
			strcpy (s_italic, ps);
		}
		if (ps = "\\E[m")
		{
			while (*ps && *ps != 0x1b)	ps++;
			strcpy (e_standout, ps);
			strcpy (e_bold, ps);
			strcpy (e_italic, ps);
		}
	}
#endif



	/*
	 *   initialize structures (defaults)
	 */
	init ();


	/*
	 *   parse cmdline flags
	 */
	for (i = 1; i < argc; ++i)
	{
		if (*argv[i] == '-' || *argv[i] == '+')
		{
			if (pswitch (argv[i], &swflg) == ERR)
				err_exit (-1);
		}
	}


	/*
	 *   loop on files
	 */
	for (i = 1; i < argc; ++i)
	{
		if (*argv[i] != '-' && *argv[i] != '+')
		{
			/*
			 *   open this file...
			 */
			if ((sofile[0] = fopen (argv[i], "r")) == NULL_FPTR)
			{
				fprintf (err_stream,
					"***%s: unable to open file %s\n",
					myname, argv[i]);
				err_exit (-1);
			}
			else
			{
				/*
				 *   do it for this file...
				 */
				ifp = 1;
				profile ();
				fclose (sofile[0]);
			}
		}
		else if (*argv[i] == '-' && *(argv[i]+1) == 0)
		{
			/*
			 *   - means read stdin (anywhere in file list)
			 */
			sofile[0] = stdin;
			ifp = 1;
			profile ();
		}

	}


	/*
	 *   if no files, usage (should really use stdin...)
	 */
	if ((ifp == 0 && swflg == FALSE) || argc <= 1)
	{
		usage ();

		err_exit (-1);
	}


	/*
	 *   normal exit. this will fflush/fclose streams...
	 */
	err_exit (0);
}




/*------------------------------*/
/*	usage			*/
/*------------------------------*/
usage ()
{
	/*
	 *   note: -l may not work correctly
	 */
	fprintf (stderr, "Usage:   %s [options] file [...]\n", myname);
	fprintf (stderr, "Options: -a        no font changes\n");
	fprintf (stderr, "         -b        backspace\n");
	fprintf (stderr, "         -d        debug mode (file: nroff.dbg)\n");
#ifdef GEMDOS
	fprintf (stderr, "         -h        hold screen before desktop\n");
#endif
/*!!!	fprintf (stderr, "         -l        output to printer\n");*/
	fprintf (stderr, "         -m<name>  macro file (e.g. -man)\n");
	fprintf (stderr, "         -o<file>  error log file (stderr is default)\n");
	fprintf (stderr, "         -po<n>    page offset\n");
	fprintf (stderr, "         -pn<n>    initial page number\n");
	fprintf (stderr, "         -pl<n>    page length\n");
	fprintf (stderr, "         -s        step through pages\n");
	fprintf (stderr, "         -v        print version only\n");
	fprintf (stderr, "         +<n>      first page to do\n");
	fprintf (stderr, "         -<n>      last page to do\n");
	fprintf (stderr, "         -         use stdin (in file list)\n");
}




/*------------------------------*/
/*	init			*/
/*------------------------------*/
init ()
{

/*
 *	initialize parameters for nro word processor
 */

	extern long	time ();

#ifdef MINIX
	register int	i;
#else
	register long	i;
#endif
	time_t		tval;
	char	       *ctim;

	/*
	 *   misc global flags, etc...
	 */
	mc_space   = 2;
	mc_char    = '|';
	tval       = time (0L);
	ctim       = ctime (&tval);

	/*
	 *   basic document controls...
	 */
	dc.fill    = YES;
	dc.dofnt   = YES;
	dc.lsval   = 1;
	dc.inval   = 0;
	dc.rmval   = PAGEWIDTH - 1;
	dc.llval   = PAGEWIDTH - 1;
	dc.ltval   = PAGEWIDTH - 1;
	dc.tival   = 0;
	dc.ceval   = 0;
	dc.ulval   = 0;
	dc.cuval   = 0;
	dc.juval   = YES;
	dc.adjval  = ADJ_BOTH;
	dc.boval   = 0;
	dc.bsflg   = FALSE;
	dc.prflg   = TRUE;
	dc.sprdir  = 0;
	dc.flevel  = 0;
	dc.lastfnt = 1;
	dc.thisfnt = 1;
	dc.escon   = YES;
	dc.pgchr   = '%';
	dc.cmdchr  = '.';
	dc.escchr  = '\\';
	dc.nobrchr  = '\'';
	for (i = 0; i < 26; ++i)
		dc.nr[i] = 0;
	for (i = 0; i < 26; ++i)
		dc.nrauto[i] = 1;
	for (i = 0; i < 26; ++i)
		dc.nrfmt[i] = '1';


	/*
	 *   initialize internal regs. first zero out...
	 */
	for (i = 0; i < MAXREGS; i++)
	{
		rg[i].rname[0] = EOS;
		rg[i].rname[1] = EOS;
		rg[i].rname[2] = EOS;
		rg[i].rname[3] = EOS;
		rg[i].rauto = 1;
		rg[i].rval  = 0;
		rg[i].rflag = RF_READ;
		rg[i].rfmt  = '1';
	}


	/*
	 *   predefined regs. these are read/write:
	 */
	i = 0;

	strcpy (rg[i].rname, "%");		/* current page */
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ | RF_WRITE;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, "ct");		/* character type */
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ | RF_WRITE;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, "dl");		/* width of last complete di */
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ | RF_WRITE;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, "dn");		/* height of last complete di */
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ | RF_WRITE;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, "dw");		/* day of week (1-7) */
	rg[i].rval  = 0;
	if      (!strncmp (&ctim[0], "Sun", 3))
		rg[i].rval  = 1;
	else if (!strncmp (&ctim[0], "Mon", 3))
		rg[i].rval  = 2;
	else if (!strncmp (&ctim[0], "Tue", 3))
		rg[i].rval  = 3;
	else if (!strncmp (&ctim[0], "Wed", 3))
		rg[i].rval  = 4;
	else if (!strncmp (&ctim[0], "Thu", 3))
		rg[i].rval  = 5;
	else if (!strncmp (&ctim[0], "Fri", 3))
		rg[i].rval  = 6;
	else if (!strncmp (&ctim[0], "Sat", 3))
		rg[i].rval  = 7;
	rg[i].rauto = 1;
	rg[i].rflag = RF_READ | RF_WRITE;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, "dy");		/* day of month (1-31) */
	rg[i].rauto = 1;
	rg[i].rval  = atoi (&ctim[8]);
	rg[i].rflag = RF_READ | RF_WRITE;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, "hp");		/* current h pos on input */
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ | RF_WRITE;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, "ln");		/* output line num */
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ | RF_WRITE;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, "mo");		/* current month (1-12) */
	rg[i].rval  = 0;
	if      (!strncmp (&ctim[4], "Jan", 3))
		rg[i].rval  = 1;
	else if (!strncmp (&ctim[4], "Feb", 3))
		rg[i].rval  = 2;
	else if (!strncmp (&ctim[4], "Mar", 3))
		rg[i].rval  = 3;
	else if (!strncmp (&ctim[4], "Apr", 3))
		rg[i].rval  = 4;
	else if (!strncmp (&ctim[4], "May", 3))
		rg[i].rval  = 5;
	else if (!strncmp (&ctim[4], "Jun", 3))
		rg[i].rval  = 6;
	else if (!strncmp (&ctim[4], "Jul", 3))
		rg[i].rval  = 7;
	else if (!strncmp (&ctim[4], "Aug", 3))
		rg[i].rval  = 8;
	else if (!strncmp (&ctim[4], "Sep", 3))
		rg[i].rval  = 9;
	else if (!strncmp (&ctim[4], "Oct", 3))
		rg[i].rval  = 10;
	else if (!strncmp (&ctim[4], "Nov", 3))
		rg[i].rval  = 11;
	else if (!strncmp (&ctim[4], "Dec", 3))
		rg[i].rval  = 12;
	rg[i].rauto = 1;
	rg[i].rflag = RF_READ | RF_WRITE;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, "nl");		/* v pos of last base-line */
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ | RF_WRITE;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, "sb");		/* depth of str below base */
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ | RF_WRITE;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, "st");		/* height of str above base */
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ | RF_WRITE;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, "yr");		/* last 2 dig of current year*/
	rg[i].rauto = 1;
	rg[i].rval  = atoi (&ctim[22]);
	rg[i].rflag = RF_READ | RF_WRITE;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, "hh");		/* current hour (0-23) */
	rg[i].rauto = 1;
	rg[i].rval  = atoi (&ctim[11]);
	rg[i].rflag = RF_READ | RF_WRITE;
	rg[i].rfmt  = 2 | 0x80;
	i++;
	
	strcpy (rg[i].rname, "mm");		/* current minute (0-59) */
	rg[i].rauto = 1;
	rg[i].rval  = atoi (&ctim[14]);
	rg[i].rflag = RF_READ | RF_WRITE;
	rg[i].rfmt  = 2 | 0x80;
	i++;
	
	strcpy (rg[i].rname, "ss");		/* current second (0-59) */
	rg[i].rauto = 1;
	rg[i].rval  = atoi (&ctim[17]);
	rg[i].rflag = RF_READ | RF_WRITE;
	rg[i].rfmt  = 2 | 0x80;
	i++;
	

	/*
	 *   these are read only (by user):
	 */
	strcpy (rg[i].rname, ".$");		/* num args at current macro*/
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".A");		/* 1 for nroff */
	rg[i].rauto = 1;
	rg[i].rval  = 1;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".H");		/* hor resolution */
	rg[i].rauto = 1;
	rg[i].rval  = 1;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".T");		/* 1 for troff */
	rg[i].rauto = 0;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".V");		/* vert resolution */
	rg[i].rauto = 1;
	rg[i].rval  = 1;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".a");
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".c");
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".d");
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".f");		/* current font (1-4) */
	rg[i].rauto = 1;
	rg[i].rval  = 1;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".h");
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".i");		/* current indent */
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".l");		/* current line length */
	rg[i].rauto = 1;
	rg[i].rval  = PAGEWIDTH - 1;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".n");
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".o");		/* current offset */
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".p");		/* current page len */
	rg[i].rauto = 1;
	rg[i].rval  = PAGELEN;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".s");		/* current point size */
	rg[i].rauto = 1;
	rg[i].rval  = 1;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".t");
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".u");
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".v");		/* current v line spacing */
	rg[i].rauto = 1;
	rg[i].rval  = 1;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".w");		/* width of prev char */
	rg[i].rauto = 1;
	rg[i].rval  = 1;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".x");
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".y");
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
	i++;
	
	strcpy (rg[i].rname, ".z");
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';

	/*
	 *   page controls...
	 */
	pg.curpag   = 0;
	pg.newpag   = 1;
	pg.lineno   = 0;
	pg.plval    = PAGELEN;
	pg.m1val    = 2;
	pg.m2val    = 2;
	pg.m3val    = 2;
	pg.m4val    = 2;
	pg.bottom   = pg.plval - pg.m4val - pg.m3val;
	pg.offset   = 0;
	pg.frstpg   = 0;
	pg.lastpg   = 30000;
	pg.ehead[0] = pg.ohead[0] = '\n';
	pg.efoot[0] = pg.ofoot[0] = '\n';
	for (i = 1; i < MAXLINE; ++i)
	{
		pg.ehead[i] = pg.ohead[i] = EOS;
		pg.efoot[i] = pg.ofoot[i] = EOS;
	}
	pg.ehlim[LEFT]  = pg.ohlim[LEFT]  = dc.inval;
	pg.eflim[LEFT]  = pg.oflim[LEFT]  = dc.inval;
	pg.ehlim[RIGHT] = pg.ohlim[RIGHT] = dc.rmval;
	pg.eflim[RIGHT] = pg.oflim[RIGHT] = dc.rmval;

	/*
	 *   output buffer controls...
	 */
	co.outp   = 0;
	co.outw   = 0;
	co.outwds = 0;
	co.lpr    = FALSE;
	co.outesc = 0;
	for (i = 0; i < MAXLINE; ++i)
		co.outbuf[i] = EOS;

	/*
	 *   macros...
	 */
	for (i = 0; i < MXMDEF; ++i)
		mac.mnames[i] = NULL_CPTR;
	for (i = 0; i < MACBUF; ++i)
		mac.mb[i] = EOS;
	for (i = 0; i < MAXPBB; ++i)
		mac.pbb[i] = EOS;
	mac.lastp = 0;
	mac.emb   = &mac.mb[0];
	mac.ppb   = NULL_CPTR;

	/*
	 *   file descriptors (for sourced files)
	 */
	for (i = 0; i < Nfiles+1; ++i)
		sofile[i] = NULL_FPTR;
}




/*------------------------------*/
/*	pswitch			*/
/*------------------------------*/
pswitch (p, q)
register char  *p;
register int   *q;
{

/*
 *	process switch values from command line
 */

	int     swgood;
	char    mfile[256];
	char   *ptmac;
	int	indx;
	int	val;

	swgood = TRUE;
	if (*p == '-')
	{
		/*
		 *   since is STILL use the goofy atari/dri xmain code, i
		 *   look for both upper and lower case. if you use dLibs
		 *   (and if its startup code does not ucase the cmd line),
		 *   you can probably look for just lower case. gulam and
		 *   other shells typically don't change case of cmd line.
		 */
		switch (*++p)
		{
		case 0:					/* stdin */
			break;

		case 'a': 				/* font changes */
		case 'A': 
			dc.dofnt = NO;
			break;

		case 'b': 				/* backspace */
		case 'B': 
			dc.bsflg = TRUE;
			break;

		case 'd': 				/* debug mode */
		case 'D': 
			dbg_stream = fopen (dbgfile, "w");
			debugging  = TRUE;
			if (dbg_stream == NULL_FPTR)
			{
				fprintf (err_stream,
					"***%s: unable to open debug file %s\n",
					myname, dbgfile);

				dbg_stream  = stderr;
			}
			break;

		case 'h': 				/* hold screen */
		case 'H': 
			hold_screen = TRUE;
			break;

		case 'l': 				/* to lpr (was P) */
		case 'L': 
#ifdef GEMDOS
			out_stream = (FILE *) 0;
#else
			out_stream = fopen (printer, "w");
#endif
			co.lpr = TRUE;
			break;

		case 'm': 				/* macro file */
		case 'M': 
			/*
			 *   build macro file name. start with lib
			 *
			 *   put c:\lib\tmac in environment so we can
			 *   read it here. else use default. if you want
			 *   file from cwd, "setenv TMACDIR ." from shell.
			 *
			 *   we want file names like "tmac.an" (for -man)
			 */
			if (ptmac = getenv ("TMACDIR"))
			{
				/*
				 *   this is the lib path (e.g. "c:\lib\tmac")
				 */
				strcpy (mfile, ptmac);

				/*
				 *   this is the prefix (i.e. "\tmac.")
				 */
				strcat (mfile, TMACPRE);
			}
			else
				/*
				 *   use default lib/prefix (i.e.
				 *   "c:\lib\tmac\tmac.")
				 */
				strcpy (mfile, TMACFULL);

			/*
			 *   finally, add extension (e.g. "an")
			 */
			strcat (mfile, ++p);

			/*
			 *   open file and read it
			 */
			if ((sofile[0] = fopen (mfile, "r")) == NULL_FPTR)
			{
				fprintf (stderr,
					"***%s: unable to open macro file %s\n",
					myname, mfile);
				err_exit (-1);
			}
			profile ();
			fclose (sofile[0]);
			break;

		case 'o': 				/* output error log */
		case 'O': 
			if (!*(p+1))
			{
				fprintf (stderr,
					"***%s: no error file specified\n",
					myname);
				err_exit (-1);
			}
			if ((err_stream = fopen (p+1, "w")) == NULL_FPTR)
			{
				fprintf (stderr,
					"***%s: unable to open error file %s\n",
					myname, p+1);
				err_exit (-1);
			}
			break;

		case 'p': 				/* .po, .pn */
		case 'P':
			if (*(p+1) == 'o' || *(p+1) == 'O')	/* -po___ */
			{
				p += 2;
				set (&pg.offset, ctod (p), '1', 0, 0, HUGE);
				set_ireg (".o", pg.offset, 0);
			}
			else if (*(p+1) == 'n' || *(p+1) == 'N')/* -pn___ */
			{
				p += 2;
				set (&pg.curpag, ctod (p) - 1, '1', 0, -HUGE, HUGE);
				pg.newpag = pg.curpag + 1;
				set_ireg ("%", pg.newpag, 0);
			}
			else if (*(p+1) == 'l' || *(p+1) == 'L')/* -pl___ */
			{
				p += 2;
				set (&pg.plval, ctod (p) - 1, '1', 0,
					pg.m1val + pg.m2val + pg.m3val + pg.m4val + 1,
					HUGE);
				set_ireg (".p", pg.plval, 0);
				pg.bottom = pg.plval - pg.m3val - pg.m4val;
			}
			else					/* -p___ */
			{
				p++;
				set (&pg.offset, ctod (p), '1', 0, 0, HUGE);
				set_ireg (".o", pg.offset, 0);
			}
			break;

		case 'r':				/* set number reg */
		case 'R':
			if (!isalpha (*(p+1)))
			{
				fprintf (stderr,
					"***%s: invalid number register name (%c)\n",
					myname, (int) *(p+1));
			}
			else
			{
				/*
				 *   indx is the user num register and val
				 *   is the final value.
				 */
				indx = tolower (*(p+1)) - 'a';
				val  = atoi (p+2);
				set (&dc.nr[indx], val, '1', 0, -INFINITE, INFINITE);
			}
			break;

		case 's': 				/* page step mode */
		case 'S': 
			stepping = TRUE;
			break;

		case 'v': 				/* version */
		case 'V': 
			printf ("%s\n", version);
			*q = TRUE;
			break;

		case '0': 				/* last page */
		case '1': 
		case '2': 
		case '3': 
		case '4': 
		case '5': 
		case '6': 
		case '7': 
		case '8': 
		case '9': 
			pg.lastpg = ctod (p);
			break;

		default: 				/* illegal */
			swgood = FALSE;
			break;
		}
	}
	else if (*p == '+')				/* first page */
	{
		pg.frstpg = ctod (++p);
	}
	else						/* illegal */
	{
		swgood = FALSE;
	}


	if (swgood == FALSE)
	{
		fprintf (stderr, "***%s: illegal switch %s\n", myname, p);
		return (ERR);
	}

	return (OK);
}




/*------------------------------*/
/*	profile			*/
/*------------------------------*/
profile ()
{

/*
 *	process input files from command line
 */

	char    ibuf[MAXLINE];

	/*
	 *   handle nesting of includes (.so). note that .so causes dc.flevel
	 *   to be increased...
	 */
	for (dc.flevel = 0; dc.flevel >= 0; dc.flevel -= 1)
	{
		while (getlin (ibuf, sofile[dc.flevel]) != EOF)
		{
			/*
			 *   if line is a command or text
			 */
			if (ibuf[0] == dc.cmdchr)
			{
				comand (ibuf);
			}
			else
			{
				/*
				 *   this is a text line. first see if
				 *   first char is space. if it is, break
				 *   line.
				 */
				if (ibuf[0] == ' ')
					robrk ();
				text (ibuf);
			}
		}

		/*
		 *   close included file
		 */
		if (dc.flevel > 0)
			fclose (sofile[dc.flevel]);
	}
	if (pg.lineno > 0)
		space (HUGE);
}





