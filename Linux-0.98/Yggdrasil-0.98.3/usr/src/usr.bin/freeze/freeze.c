#include "freeze.h"
#include "lz.h"
#include "huf.h"
#include "patchlevel.h"

/*
 * Freeze - data freezing program
 * Version 1.0:
 * This program is made from GNU compress.c and Yoshizaki/Tagawa's
 * lzhuf.c. (Thanks to all of them.)
 * The algorithm is modified for using in pipe
 * (added ENDOF symbol in Huffman table).
 * Version 1.1:
 * Check for lack of bytes in frozen file when melting.
 * Put the GetBit routine into DecodeChar for reduce function-
 * call overhead when melting.
 * Version 1.2:
 * Added delayed coding a la COMIC.
 * Now freeze works on Intels (*NIX, Microsoft, Turbo),
 * Sun (SunOS).
 * Version 2.0:
 * Buffer size is now 8192 bytes, maximum match length - 256 bytes.
 * Improved hash function (with tuning of hash-table)
 * Version 2.1: Noticeable speedup: Insert_Node and Get_Next_Match
 * are now separated. (Boyer-Moore string matching)
 * Version 2.2: Tunable static Huffman table for position information,
 * this info may be given in the command string now.
 * Version 2.2.3: Bug fixes, 10% freezing speedup.
 * Version 2.3: Minor bug fixes (DOS filenames handling, backward
 * compatibility feature improved, "bits" compression ratio display,
 * preventive check for special files), speedups, more comments added.
 * Version 2.3.1: Typedefs cleaned, utime bug on the m88k corrected
 * (pa@verano.sba.ca.us, clewis@ferret.ocunix.on.ca (Chris Lewis)),
 * "chain threshold" euristic used for speedup (in "greedy" mode) -
 * a la ZIP (Jean-Loup Gailly). Max. hash bits reduced to 16.
 * Version 2.3.2: Adaptation to TOS 1.04 (fifi@hiss.han.de), UTIMES
 * handling (jik@athena.mit.edu).
 */

static char ident[] = "@(#) freeze.c 2.3.%d %s leo@s514.ipmce.su\n";

int exit_stat = 0;

void Usage() {
#ifdef DEBUG

# ifdef DOS
    fprintf(stderr,"Usage: freeze [-cdDfitvVg] [file | +type ...]\n");
# else
    fprintf(stderr,"Usage: freeze [-cdDfvVg] [file | +type ...]\n");
# endif /* DOS */

#else

# ifdef DOS
    fprintf(stderr,"Usage: freeze [-cdfitvVg] [file | +type ...]\n");
# else
    fprintf(stderr,"Usage: freeze [-cdfvVg] [file | +type ...]\n");
# endif /* DOS */

#endif /* DEBUG */
}

void (*meltfunc)();     /* To call something for melting */

short topipe = 0,       /* Write output on stdout, suppress messages */
      precious = 1,     /* Don't unlink output file on interrupt */
      quiet = 1,        /* Don't tell me about freezing */
      do_melt = 0,      /* freeze means "freeze" */
      greedy = 0,       /* GREEDY parsing */
      force = 0;        /* "Force" flag */

char ofname [MAXNAMLEN];
struct stat statbuf;    /* Used by 'main' and 'copystat' routines */

char inbuf[BUFSIZ * 4], outbuf[BUFSIZ * 4];

#ifdef DOS
   char *last_sep();	/* last slash, backslash, or colon */
   char tail[2];        /* 2nd and 3rd chars of file extension */
# ifdef TEXT_DEFAULT
	short image = O_TEXT;
# else
	short image = O_BINARY;
# endif
#else
#  define last_sep(s) strrchr((s), '/')  /* Unix always uses slashes */
char deffile[] = "/etc/default/freeze";
#endif

#ifdef DEBUG
short debug = 0;
short verbose = 0;
char * pr_char();
long symbols_out = 0, refers_out = 0;
#endif /* DEBUG */

/* Do not sleep when freeze works :-) */
long indc_count, indc_threshold;
long file_length = 0;   /* initial length of file */

SIGTYPE (*bgnd_flag)();

void writeerr(), copystat(), version(), tune_table();

/*****************************************************************
 *
 * Usage: freeze [-cdfivV] [-t type] [file ...]
 * Inputs:
 *
 *      -c:         Write output on stdout, don't remove original.
 *
 *      -d:         If given, melting is done instead.
 *
 *      -g:         Use "greedy" parsing (1.5% worse, 40% faster).
 *                  (Means nothing when melting). May be repeated.
 *
 *      -f:         Forces output file to be generated, even if one already
 *                  exists, and even if no space is saved by freezeing.
 *                  If -f is not used, the user will be prompted if stdin is
 *                  a tty, otherwise, the output file will not be overwritten.
 *
 *      -i:         Image mode (defined only under MS-DOS).  Prevents
 *                  conversion between UNIX text representation (LF line
 *                  termination) in frozen form and MS-DOS text
 *                  representation (CR-LF line termination) in melted
 *                  form.  Useful with non-text files.  Default if
 *                  BIN_DEFAULT specified.
 *
 *      -b:         Binary mode.  Synonym for -i.  MS-DOS only.
 *
 *      -t:         Text mode (defined only under MS-DOS).  Treat file
 *                  as text (CR-LF and ^Z special) in melted form.  Default
 *                  unless BIN_DEFAULT specified.
 *
 *      -v:         Write freezing statistics. -vv means "draw progress
 *                  indicator".
 *
 *      -V:         Write version and compilation options.
 *
 *      file ...:   Files to be frozen.  If none specified, stdin
 *		    is used.
 * Outputs:
 *      file.F:     Frozen form of file with same mode, owner, and utimes
 *	or stdout   (if stdin used as input)
 *
 * Assumptions:
 *      When filenames are given, replaces with the frozen version
 *      (.F suffix) only if the file decreases in size.
 * Algorithm:
 *      Modified Lempel-Ziv-SS method (LZSS), adaptive Huffman coding
 *      for literal symbols and length info.
 *      Static Huffman coding for position info. (Is it optimal ?)
 *      Lower bits of position info are put in output
 *      file without any coding because of their random distribution.
 */

/* From compress.c. Replace .Z --> .F etc */

void main( argc, argv )
register int argc; char **argv;
{
    short overwrite = 0;  /* Do not overwrite unless given -f flag */
    char tempname[100];
    char **filelist, **fileptr;
    char *cp;

#ifdef TOS

    char *argv0 = "freeze.ttp"; /* argv[0] is not defined :-( */

#endif

#ifndef DOS
    char *malloc();
#endif

    extern SIGTYPE onintr();

#ifdef DOS
    char *sufp;
#else
    extern SIGTYPE oops();
#endif

#ifndef DOS
    if ( (bgnd_flag = signal ( SIGINT, SIG_IGN )) != SIG_IGN )
#endif
    {
	(void) signal ( SIGINT, onintr );
#ifdef __TURBOC__

#ifndef TOS
	setcbrk(1);

#endif
#endif
#ifndef DOS
	(void) signal ( SIGSEGV, oops );
#endif
    }

    filelist = fileptr = (char **)(malloc(argc * sizeof(*argv)));
    *filelist = NULL;

    if((cp = last_sep(argv[0])) != 0) {
	cp++;
    } else {

#ifdef TOS

	cp = argv0;

#else
	cp = argv[0];

#endif
    }

#ifdef DOS
/* use case-insensitive match: parent may not be command.com */
#ifdef MSDOS
    if (!stricmp(cp, "unfreeze.exe") || !stricmp(cp, "melt.exe")) {
#else /* TOS */
    if (!stricmp(cp, "unfreeze.ttp") || !stricmp(cp, "melt.ttp")) {
#endif
#else
    if (!strcmp(cp, "unfreeze") || !strcmp(cp, "melt")) {
#endif

	do_melt = 1;

#ifdef DOS	
#ifdef MSDOS
    } else if(stricmp(cp, "fcat.exe") == 0) {
#else /* TOS */
    } else if(stricmp(cp, "fcat.ttp") == 0) {
#endif
#else
    } else if(strcmp(cp, "fcat") == 0) {
#endif

	do_melt = 1;
	topipe = 1;

    } else {
	/* Freezing */

#ifndef DOS
	(void) defopen(deffile);
#else
	cp = strrchr(cp, '.');
	*++cp = 'C';
	*++cp = 'N';
	*++cp = 'F';
	*++cp = '\0';

#ifdef TOS
	(void) defopen(argv0);
#else
	(void) defopen(argv[0]);
#endif  /* TOS */
#endif  /* DOS */

    }
#ifdef BSD
    /* 4.2BSD dependent - take it out if not */
    setlinebuf( stderr );
#endif /* BSD */

    /* Argument Processing
     * All flags are optional.
     * -D => debug
     * -V => print Version; debug verbose
     * -d => do_melt
     * -v => unquiet
     * -g => greedy
     * -f => force overwrite of output file
     * -c => cat all output to stdout
     * if a string is left, must be an input filename.
     */
    for (argc--, argv++; argc > 0; argc--, argv++) {
	if (**argv == '-') {	/* A flag argument */
	    while (*++(*argv)) {	/* Process all flags in this arg */
		switch (**argv) {
#ifdef DEBUG
		    case 'D':
			debug = 1;
			break;
		    case 'V':
			verbose = 1;
#else
		    case 'V':
			version();
#endif /* DEBUG */
			break;
#ifdef DOS
		    case 'i':
		    case 'b':
			image = O_BINARY;	/* binary (aka image) mode */
			break;

		    case 't':			/* text mode */
			image = O_TEXT;
			break;
#endif
		    case 'v':
			quiet--;
			break;
		    case 'g':
			greedy++;
			break;
		    case 'd':
			do_melt = 1;
			break;
		    case 'f':
		    case 'F':
			overwrite = 1;
			force = 1;
			break;
		    case 'c':
			topipe = 1;
			break;
		    case 'q':
			quiet = 1;
			break;
		    default:
			fprintf(stderr, "Unknown flag: '%c'; ", **argv);
			Usage();
			exit(1);
		}
	    }
	}
	else {		/* Input file name */
	    *fileptr++ = *argv; /* Build input file list */
	    *fileptr = NULL;
	}
    }

# ifdef DEBUG
    if (verbose && !debug)
	version();
#endif

    if (*filelist != NULL) {
	for (fileptr = filelist; *fileptr; fileptr++) {
	    if (**fileptr == '+' && do_melt == 0) {
		tune_table(*fileptr + 1);
	/* If a file type is given, but no file names */
		if (filelist[1] == NULL)
			goto Pipe;
		continue;
	    }
	    exit_stat = 0;
	    if (do_melt != 0) {		       /* MELTING */

#ifdef DOS
		/* Check for .F or XF suffix; add one if necessary */
		cp = *fileptr + strlen(*fileptr) - 2;
		if ((*cp != '.' && *cp != 'X' && *cp != 'x') ||
		    (*(++cp) != 'F' && *cp != 'f')) {
		    (void) strcpy(tempname, *fileptr);
		    *tail = '\0';
		    if ((cp=strrchr(tempname,'.')) == NULL)
			(void) strcat(tempname, ".F");
		    else if(*(++cp) == '\0')
		    /* pseudo-extension: FOOBAR. */
			(void) strcat(tempname, "F");
		    else {
		    /* cp now points to file extension */
			tail[0] = cp[1];        /* save two chars */
			tail[1] = cp[2];
			*(++cp) = '\0';
			(void) strcat(tempname, "XF");
		    }
		    *fileptr = tempname;
		}
#else
		/* Check for .F suffix */
		if (strcmp(*fileptr + strlen(*fileptr) - 2, ".F") != 0) {
		    /* No .F: tack one on */
		    (void) strcpy(tempname, *fileptr);
		    (void) strcat(tempname, ".F");
		    *fileptr = tempname;
		}
#endif /*DOS */

		/* Open input file for melting */

		if (checkstat(*fileptr))
			continue;

#ifdef DOS
		if ((freopen(*fileptr, "rb", stdin)) == NULL)
#else
		if ((freopen(*fileptr, "r", stdin)) == NULL)
#endif
		{
			perror(*fileptr); continue;
		}

		/* Check the magic number */
		if (getchar() != MAGIC1)
			goto reject;
		switch (getchar()) {
#ifdef COMPAT
		case MAGIC2_1:
			meltfunc = melt1;
			break;
#endif
		case MAGIC2_2:
			meltfunc = melt2;
			break;
		default: reject:
			fprintf(stderr, "%s: not in frozen format\n",
				*fileptr);
			continue;
		}

		/* Generate output filename */
		precious = 1;
		(void) strcpy(ofname, *fileptr);
		ofname[strlen(*fileptr) - 2] = '\0';  /* Strip off .F */
#ifdef DOS
		(void) strcat(ofname, tail);
#endif
	    } else {

			/* FREEZING */
#ifdef DOS
		cp = *fileptr + strlen(*fileptr) - 2;
		if ((*cp == '.' || *cp == 'X' || *cp == 'x') &&
		    (*(++cp) == 'F' || *cp == 'f')) {
		    fprintf(stderr,"%s: already has %s suffix -- no change\n",
			*fileptr,--cp); /* } */
#else
		if (strcmp(*fileptr + strlen(*fileptr) - 2, ".F") == 0) {
		    fprintf(stderr, "%s: already has .F suffix -- no change\n",
			*fileptr);
#endif /* DOS */

		    continue;
		}
		/* Open input file for freezing */

		if (checkstat(*fileptr))
			continue;

#ifdef DOS
		if ((freopen(*fileptr, image == O_TEXT ? "rt" : "rb", stdin))
		    == NULL)
#else
		if ((freopen(*fileptr, "r", stdin)) == NULL)
#endif
		{
		    perror(*fileptr); continue;
		}

		/* Generate output filename */
		(void) strcpy(ofname, *fileptr);
#ifndef BSD     /* Short filenames */
		if ((cp = last_sep(ofname)) != NULL) cp++;
		else cp = ofname;
# ifdef DOS
		if (topipe == 0 && (sufp = strrchr(cp, '.')) != NULL &&
		    strlen(sufp) > 2) fprintf(stderr,
		    "%s: part of filename extension will be replaced by XF\n",
		    cp);
# else
		if (topipe == 0 && strlen(cp) > 12) {
		    fprintf(stderr,"%s: filename too long to tack on .F\n",cp);
		    continue;
		}
# endif /* DOS */
#endif  /* BSD               Long filenames allowed */
							 
#ifdef DOS
		/* There is no difference between FOOBAR and FOOBAR. names */
		if ((cp = strrchr(ofname, '.')) == NULL)
			(void) strcat(ofname, ".F");
		else if (cp[1] == '\0')
			/* FOOBAR. case */
			(void) strcat(ofname, "F");
		else {
			cp[2] = '\0';
			(void) strcat(ofname, "XF");
		}
#else
		(void) strcat(ofname, ".F");
#endif /* DOS */

	    }
	    /* Check for overwrite of existing file */
	    if (overwrite == 0 && topipe == 0) {
		if (stat(ofname, &statbuf) == 0) {
		    char response[2];
		    response[0] = 'n';
		    fprintf(stderr, "%s already exists;", ofname);
#ifndef DOS
		    if (foreground()) {
#endif
			fprintf(stderr,
			    " do you wish to overwrite %s (y or n)? ", ofname);
			(void) fflush(stderr);
			(void) read(2, response, 2);
			while (response[1] != '\n') {
			    if (read(2, response+1, 1) < 0) {	/* Ack! */
				perror("stderr"); break;
			    }
			}
#ifndef DOS
		    }
#endif
		    if (response[0] != 'y') {
			fprintf(stderr, "\tnot overwritten\n");
			continue;
		    }
		}
	    }
	    if(topipe == 0) {  /* Open output file */

#ifdef DEBUG
		if (do_melt == 0 || debug == 0) {
#endif
#ifdef DOS
		if (freopen(ofname, do_melt && image == O_TEXT ? "wt" : "wb",
		    stdout) == NULL)
#else		 
		if (freopen(ofname, "w", stdout) == NULL)
#endif
		{
		    perror(ofname); continue;
		}
#ifdef DEBUG
		}
#endif
		precious = 0;
		if(quiet != 1)  {
			fprintf(stderr, "%s:", *fileptr);
			indc_count = 1024;
		}
	    }
	    else {	/* output is to stdout */
#ifdef MSDOS
			/* freeze output always binary; melt output
			   is binary if image == O_BINARY
			*/
		if (do_melt == 0 || image == O_BINARY)
			setmode(fileno(stdout), O_BINARY);
#endif
	    }
	    /* Actually do the freezing/melting */
	    if (do_melt == 0)
		freeze();
#ifndef DEBUG
	    else
		(*meltfunc)();
#else
	    else if (debug && verbose)
		printcodes(meltfunc == (void(*)()) melt2);
	    else
		(*meltfunc)();
#endif /* DEBUG */

	/* check output status, and close to make sure data is written */
	    if ( ferror(stdout) || (!topipe && fclose(stdout) < 0))
		writeerr();

	    if(topipe == 0)
		copystat(*fileptr);     /* Copy stats */
	    precious = 1;
	 }
    } else {		/* Standard input */
Pipe:
	if (fstat(fileno(stdin), &statbuf)) {
		perror("stdin");
		exit(1);
	}
	file_length = statbuf.st_mode & S_IFREG ? statbuf.st_size : 0;

	indc_threshold = file_length / 100;
	if (indc_threshold < 4096)
		indc_threshold = 4096;
	if (do_melt)
		indc_threshold *= 3;

	topipe = 1;
	if (do_melt == 0) {
#ifdef MSDOS
			/* freeze output always binary */
			/* freeze input controlled by -i -t -b switches */
		setmode(fileno(stdout), O_BINARY);
		setmode(fileno(stdin), image);
#endif
		freeze();
		if(quiet != 1)
			putc('\n', stderr);
	} else {
#ifdef MSDOS
		    /* melt input always binary */
		    /* melt output to stdout binary if so requested */
	    setmode(fileno(stdin), O_BINARY);
	    setmode(fileno(stdout), image);
#endif
	    /* Check the magic number */
		if (getchar() != MAGIC1)
			goto badstdin;
		switch (getchar()) {
#ifdef COMPAT
		case MAGIC2_1:
			meltfunc = melt1;
			break;
#endif
		case MAGIC2_2:
			meltfunc = melt2;
			break;
		default: badstdin:
			fprintf(stderr, "stdin: not in frozen format\n");
			exit(1);
		}

#ifndef DEBUG
	    meltfunc();
#else
	    if (debug && verbose)
		printcodes(meltfunc == (void(*)()) melt2);
	    else
		meltfunc();
#endif /* DEBUG */
	}
    }
    exit(exit_stat);
    /*NOTREACHED*/
}

long in_count = 1;      /* length of input */
long bytes_out;         /* length of frozen output */

/* Calculates and prints the compression ratio w/o floating point OPs */

void prratio(stream, was, is)
FILE *stream;
long was, is;
{
	register long q;        /* This works everywhere */

	if (!is) is++;

	if(was > 214748L) {     /* 2147483647/10000 */
		q = was / (is / 10000L);
	} else {
		q = 10000L * was / is; /* Long calculations, though */
	}
	if (q < 0) {
		putc('-', stream);
		q = -q;
	}
	fprintf(stream, "%d.%02d%%", (int)(q / 100), (int)(q % 100));
#ifdef GATHER_STAT
	fprintf(stream, "(%ld / %ld)", was, is);
#endif
}

/* Calculates and prints bits/byte compression ratio as above */

void prbits(stream, was, is)
FILE *stream;
long was, is;
{
	register long q;

	if (!was) was++;

	if(is > 2684354L) {     /*  2147483647/800 */
		q = is / (was / 800L);
	} else {
		q = 800L * is / was;             
	}
	fprintf(stream, " (%d.%02d bits)", (int)(q / 100), (int)(q % 100));
}

/* There was an error when reading or writing files */

void writeerr()
{
    if (!topipe) {
	perror ( ofname );
	(void) unlink ( ofname );
    }
    exit ( 1 );
}

void copystat(ifname)
char *ifname;
{
#ifdef __TURBOC__
struct ftime utimbuf;
#else
#ifdef UTIMES
struct timeval timep[2];
#else
struct utimbuf timep;
#endif
#endif

    int mode;

#ifdef MSDOS
    if (_osmajor < 3) freopen("CON","at",stdout); else	  /* MS-DOS 2.xx bug */
#endif

    (void) fclose(stdout);

    if (exit_stat == 2 && (!force)) { /* No freezing: remove file.F */

	if(quiet != 1)
		fprintf(stderr, "-- file unchanged\n");

    } else {		    /* ***** Successful Freezing ***** */

	if (stat (ifname, &statbuf)) {  /* file disappeared ?! */
		perror(ifname);
		exit_stat = 1;
		return;
	}

	exit_stat = 0;

#ifdef TOS

	Fattrib(ofname,1,Fattrib(ifname,0,0));

#else
	mode = statbuf.st_mode & 07777;
	if (chmod(ofname, mode))		/* Copy modes */
	    perror(ofname);
#endif
#ifndef DOS
	/* Copy ownership */
	(void) chown(ofname, (int) statbuf.st_uid, (int) statbuf.st_gid);
#endif

#ifdef __TURBOC__
        getftime(fileno(stdin),&utimbuf);
        freopen(ofname,"rb",stdout);
        setftime(fileno(stdout),&utimbuf);
	(void) fclose(stdout);
#else
#ifdef UTIMES
	timep[0].tv_sec = statbuf.st_atime;
	timep[1].tv_sec = statbuf.st_mtime;
	timep[0].tv_usec = timep[1].tv_usec = 0;
	(void) utimes(ofname, timep);
#else
	timep.actime = statbuf.st_atime;
	timep.modtime = statbuf.st_mtime;

#if defined(__m88k__)
      timep.acusec=  statbuf.st_ausec;        /* pa@verano */
      timep.modusec= statbuf.st_musec;
#endif /* !m88k */

	/* Update last accessed and modified times */
	(void) utime(ofname, &timep);
#endif /* UTIMES */
#endif /* __TURBOC__ */
	if (unlink(ifname))	/* Remove input file */
	    perror(ifname);
	if(quiet != 1)
		fprintf(stderr, " -- replaced with %s\n", ofname);
	return;		/* Successful return */
    }

    /* Unsuccessful return -- one of the tests failed */
    if (unlink(ofname))
	perror(ofname);
}

/* Checks status of a file, returns 0 if the file may be frozen,
	or 1 otherwise; assigns this value to exit_stat
*/
int checkstat(ifname)
char *ifname;
{
	if (stat (ifname, &statbuf)) {
		perror(ifname);
		return exit_stat = 1;
	}

	/* Do NOT try to freeze /dev/null or /dev/tty...   */
	/* but you may freeze or melt everything to stdout */

#ifndef DOS
	if (!topipe) {
		if ((statbuf.st_mode & S_IFMT) != S_IFREG) {
			fprintf(stderr, "%s: ", ifname);
			fprintf(stderr, " not a regular file -- unchanged\n");
			return exit_stat = 1;

		} else if (statbuf.st_nlink > 1) {
			fprintf(stderr, "%s: ", ifname);
			fprintf(stderr, " has %d other links -- unchanged\n",
			statbuf.st_nlink - 1);
			return exit_stat = 1;
		}
	}
#endif /* MSDOS */

	file_length = statbuf.st_size;

	indc_threshold = file_length / 100;
	if (indc_threshold < 4096)
		indc_threshold = 4096;
	if (do_melt)
		indc_threshold *= 3;

	return exit_stat = 0;
}

#ifndef DOS
/*
 * This routine returns 1 if we are running in the foreground and stderr
 * is a tty. (as in compress(1))
 */
int foreground()
{
	if(bgnd_flag != SIG_DFL)  /* background? */
		return(0);
	else {                          /* foreground */
		if(isatty(2)) {		/* and stderr is a tty */
			return(1);
		} else {
			return(0);
		}
	}
}
#endif

/* Exception handler (SIGINT) */

SIGTYPE onintr ( ) {
    if (!precious) {            /* topipe == 1 implies precious == 1 */
	(void) fclose(stdout);
	(void) unlink(ofname);
    }
    exit(1);
}

/* Exception handler (SIGSEGV) */

SIGTYPE oops ( )        /* file is corrupt or internal error */
{
    (void) fflush(stdout);
    fprintf(stderr, "Segmentation violation occured...\n");
    exit ( 1 );
}

/* Prints version and compilation options */

void version()
{
	fprintf(stderr, ident, PATCHLEVEL, PATCHDATE);
	fprintf(stderr, "LZSS 8192/256 + Huffman coding\nOptions: ");
#ifdef COMPAT
	fprintf(stderr, "compatible with vers. 1.0, ");
#endif
#ifdef DEBUG
	fprintf(stderr, "DEBUG, ");
#endif
#ifdef BSD
	fprintf(stderr, "BSD, ");
#endif
#ifdef  __XENIX__
	fprintf(stderr, "XENIX, ");
#endif
#ifdef  __TURBOC__
	fprintf(stderr, "TURBO, ");
#endif
#ifdef GATHER_STAT
	fprintf(stderr, "GATHER_STAT, ");
#endif
	fprintf(stderr, "HASH: %d bits\n", BITS);
	fprintf(stderr, "Static Huffman table: %d %d %d %d %d %d %d %d\n",
		Table2[1], Table2[2], Table2[3], Table2[4],
		Table2[5], Table2[6], Table2[7], Table2[8]);
#ifdef DOS
	fprintf(stderr, "Default melted mode: %s\n",
			image == O_BINARY ? "binary" : "text");
#endif
	exit(0);
}

/* Deals with static Huffman table parameters.
	Input: command line option w/o leading `+'.
	Output: fills the array `Table2' if OK, exit(1) otherwise.
*/

void tune_table(type) char *type;
{
	extern char * defread();
	register char *s = defread(type), *t;
	static int v[8];
	int i, is_list = 0;
	if(!s) {
	/* try to consider 'type' as a list of values: n1,n2, ... */
		if(strrchr(type, ','))
			is_list = 1;
		else {
			fprintf(stderr, "\"%s\" - no such file type\n", type);
			exit(1);
		}
		if(sscanf(type, "%d,%d,%d,%d,%d,%d,%d,%d",
			v, v+1, v+2, v+3, v+4, v+5, v+6, v+7) != 8) {
			fprintf(stderr,
				"%s - a list of 8 numbers expected\n", type);
			exit(1);
		}
	}
	if(!is_list && (!(t = strrchr(s, '=')) ||
		sscanf(++t, "%d %d %d %d %d %d %d %d",
		v, v+1, v+2, v+3, v+4, v+5, v+6, v+7) != 8)) {
		fprintf(stderr,
			"\"%s\" - invalid entry\n", type);
		exit(1);
	}
	for(i = 0; i < 8; i++)
		Table2[i+1] = v[i];
	if(quiet < 0) {
		if(!is_list) {
			t = s;
		/* make full word */
			while(*s != '=' && *s != ' ' && *s != '\t') s++;
			*s = '\0';
		} else
			t = "";
		fprintf(stderr, "Using \"%s%s\" type\n", type, t);
	}
}

#ifdef DOS

/* MSDOS typically has ':' and '\\' separators, but some command
  processors (and the int 21h function handler) support '/' too.
  Find the last of these.
*/

char * last_sep(s)
register char *s;
{
	char *p;
	for (p = NULL; *s; s++)
	    if (*s == '/' || *s == '\\' || *s == ':')
		p = s;
	return(p);
}

#endif	/* DOS */
