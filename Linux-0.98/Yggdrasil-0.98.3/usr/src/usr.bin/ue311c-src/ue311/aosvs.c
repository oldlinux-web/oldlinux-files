/*
 * aosvs.c  -  AOS/VS & MV/UX version of the termio.c module
 *
 * The functions in this module deal with the O/S in reading/writing
 * characters from/to the screen.  We also deal with various wiedness things
 * in dealing with the AOS/VS file system such as ACL's and elementsizes.
 * This has been written primarily for AOS/VS but should work for MV/UX as
 * well.  No promises though.
 *
 * AOS/VS and MV/UX are products of Data General Corporation, Westboro MA.
 *
 * Authors:
 *          Daniel Lawrence --- doing MicroEmacs... stuff copied into here...
 *          Douglas Rady ------ most of the rest of the stuff here
 * Credits:
 *          Michael Meissner -- beating Doug over the head with advice and info.
 *                              on AOS/VS "C" compiler.     THANK YOU!!!!!!!!!!
 *          Bill Benedetto
 *          & friends      ---- beta victims, bug finders/fixers
 *          GNU --------------- inspiration, software tools, clean readable BSD
 *                              and USG code examples and interesting comments.
 *
 * Definition: uka = Unixly Known As
 *
 * Revision/Hack History
 *  MicroEMACS v3.<9p,10> AOS/VS History
 *  
 *  3.09.16.00 - 3.09.16.09         ??-???-??           dcr
 *      Before written history.  Suffice to say that .00 - .04 was just the
 *      AOS/VS port from 3.8z, 3.9e, 3.9i and 3.9n with major cleaning up and
 *      rewriting of the AOS/VS stuff.  Sub-revisions 3.09.16.05 - 3.09.16.09
 *      being optimizations and various fixes.
 *      Yawn...
 *
 *  3.09.16.10                      25-Aug-88           dcr
 *      Changed  do_system_end()  in  aosvs.c  to check for execution of a 
 *	macro as basis for not executing an  mlreply("Continue")  call.
 *      Also fixed  stime()  in  aosvs.c  so it just plain worked.
 *      #$&^@*&(+$#!!!!
 *      Also added  REPLYNL  in  estruct.h  and made use of CR or NL in the
 *      mlreply()  routine in  input.c  compile time conditional.
 *
 *  3.09.16.11                      13-Sep-88           dcr
 *      Start of written hisory.  This is a momentous event in the course of
 *      AOS/VS MicroEMACS history: doug writes some change doc.!!! Yeah!!!
 *      Changed several functions to force the  $builtin  version as a trade of
 *      more code size for more speed (usually defaulted to $builtin anyway...).
 *
 *  3.09.16.12                      14-Sep-88           dcr
 *      Recompiled with revision 4.00 of AOS/VS C.  A miracle...
 *
 *  3.09.16.13                      12-Oct-88           dcr
 *      Changed to have the temporary file for pipecmd() be created in a system
 *      wide temporary file directory known as :TMP, uka /tmp.  You paranoids
 *      can create :TMP as a CPD with max. size of 2048 blocks and an ACL of
 *      +,WE and you should be able to relax a nibble.
 *      Creates temp. file with ACL of  username,OWAR
 *
 *  3.09.16.14                      13-Oct-88           dcr
 *      Put in MCA's hack to reduce memory consumption when VIEWing a file.
 *      Modified  lalloc()  in line.c  Test case of paru.h saved 27 2Kb pages.
 *
 *  3.10.00.00                      09-Nov-88           dcr
 *      Begin code porting for 3.10 BETA.
 *
 *  3.10.00.01                      20-Dec-88           dcr
 *      Implemented multi-language messages in  aosvs.c  per 3.10 BETA.
 *
 *  3.10.00.02                      26-Dec-88           dcr
 *      Tracked down some un-documented runtime optimizations and got
 *      a working version. Also resolved timeset() references.
 *
 *  3.10.00.03                      17-Jan-89           dcr
 *      Cleaned up for shipping to Dan.  Put in some missing code for input.c
 *
 *  3.10.00.04                      17-Jan-89           dcr
 *      Put in some optimizations regarding TTflush() and ostring() in the
 *      bind.c, input.c and aosvs.c files.  This freed 2Kb... but not for long.
 *
 *  3.10.00.05                      17-Jan-89           dcr
 *      Some memory optimization in  exec.c  and  input.c for small regain.
 *
 *  3.10.00.06 - 3.10.00.27     02-Mar-89 - 10-Mar-89   dcr
 *      Various changes. Cleaned up the AOS/VS changes in other modules so 
 *      we actually work right.  Added conditionals for using either BSD or
 *      USG console i/o (BSD doesn't seem to work).  Reclaimed about 4Kb of 
 *      memory in various places and made some of these optimizations compile
 *      time conditional. We are still using .890+ ms to start up intead of the
 *      .520+ ms that we took with 3.9p. Not much hope there... but...
 *      Various tweaks here and there.  Added the skip of nulls when reading in
 *      a file (most unpleasant results if we don't).  Lost some memory to the
 *      USG console i/o stuff.  Threw out the BSD console i/o stuff, sigh....
 *
 *  3.10.00.28                      10-Mar-89           dcr
 *      Stuck in the  aosvs$unix_to_aosvs_path()  routine to replace the code
 *      in resolve_pathname().  The new routine does a fairly complete job of
 *      converting Unix(tm) pathnames to AOS/VS format. It also handles the 
 *      Ms-Dog '\' path seperator.
 *  
 *  3.10.00.29                      16-Mar-89           dcr
 *      Changed pathname expansion to be invisible to user.  Required changes
 *      to  fileio.c  to #if AOSVS  replace the fopen() calls with our xxfopen()
 *      call which does the pathname expansion.  Allowed removal of all the
 *      related to the EXPPATH define.  Also changed several routines here.
 *      Allows user to reference buffer & file names w/o "unexpanding" the 
 *      orginal pathname.
 *      Diff'd & sent to Dan.
 *
 */

#nolist
#include        <stdio.h>           /* get the usual */
#list
#include        "estruct.h"         /* get the MicroEMACS stuff */

#ifdef  AOSVS | MV_UX               /* should this be done? */

#define dashertermdef   1           /* might not be used any more */

#nolist
#include        "edef.h"            /* get the MicroEMACS extern's */
#include        "elang.h"
#list

/* There is no kitchen sink in C so we can't include it. Maybe in C++... */
#nolist
#include    <paru.h>                /* AOS/VS system mnemonics */
#include    <sysid.h>               /* AOS/VS system call mnemonics */
#include    <packets/characteristics.h> /* sys call packet for terminal char.*/
#include    <packets/create.h>      /* sys call packet to create files */
#include    <packets/filestatus.h>  /* sys call packet to get file info */
#include    <packets/misc.h>        /* misc. sys call packets (?SYSPRV) */

/* And now... the Unix(tm) stuff... */
#include    <fcntl.h>               /* terminal file control stuff */
#include    <signal.h>              /* the signals... */
#include    <termio.h>              /* more terminal control stuff */
#list

/*
    Variables!!!   Functions!!!  Externals!!!!
*/
struct  termio  old_in_termio;  /* original stdin terminal characteristics */
struct  termio  new_in_termio;  /* stdin characteristics to use inside */

int kbdpoll;                    /* type ahead polling flag      */
int kbdflgs;                    /* saved keyboard fd flags      */
int kbdqp;                      /* there is a char in kbdq      */
char kbdq;                      /* char we've already read      */

/*
    some D.G. supplied AOS/VS & MV/UX specific functions

    _toaos_fid() - changes UNIX(tm)  pathname to AOS/VS pathname (Thank you!)
    traceback() - calls the ?SNAP LANG_RT routine for error traceback
*/    
/*extern int  _toaos_fid(char*, char* );*/
extern VOID traceback(int );

/*
    This is how we make an MV/Eclipse accumulator in C with almost all options.
*/
union accumulator {                 /* dearly beloved, we are gathered here...*/
    unsigned long * ptr;            /* pointer to unsigned long (generic) */
    char *          cptr;           /* pointer to char */
    unsigned int  * pint;           /* pointer to unsigned int */
    unsigned short *psht;           /* pointer to short */
    unsigned long   ulng;           /* unsigned long */
    signed long     lng;            /* signed long */
    unsigned int    uin;            /* unsigned int */
    signed int      in;             /* signed int */
    unsigned short usht;            /* unsigned short */
    unsigned char   chr;            /* a char, unsigned of course!!! */
} ac0, ac1, ac2;                    /* our bountiful accumulators, sigh... */
    

P_CREATE    create_pkt;             /* file create system call packet */
P_FSTAT     fstat_pkt;              /* file status system call packet */
char acl_buf[ $MXACL ];             /* ACL buffer, gotta protect things... */
char *crt_eol="\013";               /* Dasher D2xx commands */
char *crt_eop="\014";               /* Dasher D2xx commands */
int dimsts, revsts;
int su_mode;                        /* Superuser mode flag */
#define TLINE_LEN   512
char tline[TLINE_LEN];                /* command line for cli/shell/program calls */

extern VOID do_system();        /* calls the cli/shell/program */
extern VOID do_system_end();    /* cleans up after cli/shell/program calls */
extern VOID init_tline();       /* inits the command line for cli/shell... */
extern VOID ttputs();
extern VOID in_init();
extern FILE *ffp;               /* file stream pointer used in fileio.c */
extern int vttidy();            /* MicroEMACS routine to tidy up the screen */

extern int              aosvs$expand_pathname();
extern void             aosvs$unix_to_aosvs_path();
#endif

FILE *STDIN, *STDOUT;           /* Needed since the array of files went away.*/
                                /* Of course, if _iob changes, we'll have to */
				/* bend over since we use  ->_file  in _iob. */

static int mexist;	/* is the mouse driver installed? */
static int nbuttons;	/* number of buttons on the mouse */
static int oldbut;	/* Previous state of mouse buttons */

/*------------------------------------------------------------------------------
 *  resolve_full_pathname(char*, char*) - resolves a filename or pathname to
 *  full AOS/VS pathname via the ?GRNAME system call.  If the file does not
 *  exsist then the current working directory is assumed by  AOS/VS.
 *
 *  Returns  FIOSUC  if  from_path  is found or  FIOFNF  if it isn't found.
 */
int resolve_full_pathname(from_path, to_path)

char *from_path, *to_path;      /* resolve "from" pathname "to" pathname */
{
    if (aosvs$expand_pathname(from_path, to_path))
        return(FIOFNF);

    return(FIOSUC);
}

/*
 *  ffwopen() - AOS/VS specific version of the ffwopen() routine found in the
 *  fileio.c source.  This version will attempt to recreate the edit file
 *  (if it exists) with the existing edit file ACL, elementsize, filetype and
 *  recordsize parameters as determined via a ?FSTAT filestatus system call.
 *
 *  Returns  FIOSUC  if file is opened or  FIOERR  if not opened.
 *
 *	DOUG, we need to make the mode param work here, or ignore it....
 */
int ffwopen(bfilnam, mode, sfilnam)

char *bfilnam;    /* buffer file name */
char *mode;	  /* mode for file open (w = write, a = append) */
char *sfilnam;    /* save file name or NULL */
{
    char bfnam[NFILEN], sfnam[NFILEN];
    char *tptr;

    /* some initializations */
    zero((char *) &create_pkt, sizeof(create_pkt));
    zero((char *) &fstat_pkt, sizeof(fstat_pkt));
    zero(acl_buf, $MXACL);

    strcpy(bfnam, bfilnam);
    if (sfilnam) {
        strcpy(sfnam, sfilnam);
	resolve_full_pathname(sfnam, sfnam);
        tptr = sfnam;
    } else {
        tptr = bfnam;    
    }
    
    create_pkt.ctim = -1L;      /* take default file creation time */
    create_pkt.cacp = -1L;      /* take default file creation acl */
    create_pkt.cdel = -1L;      /* take default file creation elementsize */
    create_pkt.cmil = -1L;      /* take default file creation max. index levels */
    fstat_pkt.stim = -1L;
    fstat_pkt.sacp = -1L;

    /* attempt to get full AOS/VS pathname of the file */
    if  ((resolve_full_pathname(bfnam, bfnam)) == FIOFNF) { /* edit file found? */
        /* bfnam not found, create one with default specs. */
        create_pkt.cftyp_format = $ORDS;    /* data sensitive record type */
        create_pkt.cftyp_entry  = $FTXT;    /* text file type */
        ac2.ptr = &create_pkt;
        ac1.lng = 0L;

        ac0.cptr = tptr;

        /* have AOS/VS attempt to create the file */
        if  (sys($CREATE, &ac0, &ac1, &ac2))
            goto fubar;

    } else {    /* bfnam found, get filestatus info. for recreation of bfnam */ 
        ac2.ptr = &fstat_pkt;
        ac1.lng = 0L;
        ac0.cptr = bfnam;
        /* have AOS/VS attempt to get the file information for us */
        if  (sys($FSTAT, &ac0, &ac1, &ac2))
            goto fubar;

        /*
            Get ACL of bfnam.  If we can't get that then we get the
            default ACL and use that.
        */
        ac0.cptr = bfnam;
        ac1.cptr = acl_buf;
        ac2.lng = 0L;
        if  (sys($GACL, &ac0, &ac1, &ac2)) {    /* try to get the file ACL */
            /* can't get ACL of file, get default ACL */
            ac0.lng = 0L;
            ac2.lng = 0L;
            ac1.cptr = acl_buf;
            sys($DACL, &ac0, &ac1, &ac2); /* try to get user default ACL */
        }

        /*
            Delete  sfnam  file.  We don't care about any errors on this.
        */
        ac0.cptr = tptr;
        ac1.lng = 0L;
        ac2.lng = 0L;
        sys($DELETE, &ac0, &ac1, &ac2);   /* delete it */
    
        /*
            Set up the packet for the file create system call
        */
        create_pkt.cftyp_format = fstat_pkt.styp_format;    /* file format  */
        create_pkt.cftyp_entry  = fstat_pkt.styp_type;      /* file type    */
        create_pkt.ccps = fstat_pkt.scps;   /* recordsize, if any */
        create_pkt.cacp = acl_buf;          /* acl buffer ptr   */
        create_pkt.cdeh = fstat_pkt.sdeh;   /* element size     */
        create_pkt.cmil = fstat_pkt.smil;   /* max. index level */

        /*
            Make system call to create a file with supplied specs.
        */
        ac2.ptr  = &create_pkt;
        ac0.cptr = tptr;

        ac1.lng = 0L;
        if  (sys($CREATE, &ac0, &ac1, &ac2))    /* attempt file create */
            goto fubar;
    }

    if  ((ffp = fopen(tptr, "a")) == NULL)
            goto fubar;

#if     ISADIR
    if  (isadirectory(ffp)) {   /* check to see if file is a directory */
        mlwrite(TEXT216);       /* bitch... */
	ffclose(ffp);           /* yes, close it and get out!!! */
        return(FIOERR);         /* actual checking code in O/S modules */
    }
#endif

    return(FIOSUC);

    /*
        Common error exit for all  ffwopen()  errors
    */
fubar:
    mlwrite(TEXT155);
/*    mlwrite("Cannot open file for writing");*/
    return(FIOERR);
}

/*
 *  unlink() - delete a file - called from  writeout()  in  file.c
 *
 *  This routine replaces the DG supplied  unlink()  since we don't use the
 *  link() unlink() combination to rename files.  Saves some memory.
 *
 */
int unlink(del_fnam)
 
char *del_fnam;        /* name of file to delete */
{
    char dtmp[NFILEN];

    strcpy(dtmp, del_fnam);
    resolve_full_pathname(dtmp, dtmp);
    ac0.cptr = dtmp;
    ac1.lng = 0L;
    ac2.lng = 0L;
    if (sys($DELETE, &ac0, &ac1, &ac2))    /* attempt to delete it */
        return(-1);                           /* normal error return */
    return(0);                                /* normal okay return */
}

/*
 *  rename() - rename a file - called from  writeout()  in  file.c
 */
int rename(from_nam, to_nam)

char *from_nam;     /* rename from name */
char *to_nam;       /* rename to name */
{
    char ftmp[NFILEN], ttmp[NFILEN];

    /*
        First we convert Unix(tm) or Ms-dog paths to Aos/Vs paths.
        Second we strip the actual filename from the "to" path by going to the
	end of the string and working our way backward until we find a pathname
	seperator which under AOS/VS is a colon (:).
    */
    strcpy(ftmp, from_nam);
    strcpy(ttmp, to_nam);
    resolve_full_pathname(ftmp, ftmp);
    resolve_full_pathname(ttmp, ttmp);

    ac1.cptr = ttmp + (sizeof(char) * strlen(ttmp));
    while ((ac1.cptr >= ttmp) && (*ac1.cptr != ':') && (*ac1.cptr != '='))
        --ac1.cptr;

    ++ac1.cptr;     /* move pointer from seperator to 1st char in filename */
    ac0.cptr = ftmp;
    ac2.lng = 0L;
    if  (sys($RENAME, &ac0, &ac1, &ac2))    /* attempt the rename */
        return(-1);   /* oops!!! */
    return(0);        /* okay... */
}

/*
 * This function gets called just before we go back home to the command
 * interpreter.
 */
VOID ttclose()
{
    fflush(stdout);
    fflush(stdin);
    ioctl(STDIN->_file, TCSETA, &old_in_termio);     /* restore terminal settings */
    fcntl(STDIN->_file, F_SETFL, kbdflgs);
}

/*
 * Write a character to the display.
 */
#if TTPUTC == 0
VOID ttputc(c)
{
    putc(c, stdout);
}
#endif

/*
 * Flush terminal buffer. Does real work where the terminal output is buffered
 * up. A no-operation on systems where byte at a time terminal I/O is done.
 */
#if TTFLUSH == 0
VOID ttflush()
{
    fflush(stdout);
}
#endif

unsigned int extcode(c)

unsigned int c;

{
    if ((c == 1 || c == 8 || c == 17 || c == 23) || (c >= 24 && c <=26)
        || (c >= 32 && c <= 110) || (c >= 112 && c <= 126))   /* 111 is mouse */
        return((c & 255) | SPEC);

    return(c & 255);
}

/*	input buffers and pointers	*/

#define	IBUFSIZE	64	/* this must be a power of 2 */

unsigned char in_buf[IBUFSIZE];	/* input character buffer */
int in_next = 0;		/* pos to retrieve next input character */
int in_last = 0;		/* pos to place most recent input character */

VOID in_init()  /* initialize the input buffer */

{
	in_next = in_last = 0;
}

int in_check()	/* is the input buffer non-empty? */

{
	if (in_next == in_last)
		return(FALSE);
	else
		return(TRUE);
}

VOID in_put(event)

int event;	/* event to enter into the input buffer */

{
	in_buf[in_last++] = event;
	in_last &= (IBUFSIZE - 1);
}

int in_get()	/* get an event from the input buffer */

{
	register int event;	/* event to return */

	event = in_buf[in_next++];
	in_next &= (IBUFSIZE - 1);
	return(event);
}
#if	MOUSE
int checkmouse()

{
	register int k;		/* current bit/button of mouse */
	register int event;	/* encoded mouse event */
	int newbut;		/* new state of the mouse buttons */
	int mousecol;		/* current mouse column */
	int mouserow;		/* current mouse row */
	int sstate;		/* current shift key status */

	/* check to see if any mouse buttons are different */
/*	rg.x.ax = 3;*/	/* Get button status and mouse position */
/*	int86(0x33, &rg, &rg);*/
/*	newbut   = rg.x.bx;*/
/*	mousecol = rg.x.cx >> 3;*/
	mouserow = rg.x.dx >> 3;

	/* get the shift key status as well */
	sstate = 0;
/*	rg.h.ah = 2;*/	/* return current shift status */
/*	int86(0x16, &rg, &rg);*/
/*	sstate = rg.h.al;*/

	for (k=1; k != (1 << nbuttons); k = k<<1) {
		/* For each button on the mouse */
		if ((oldbut&k) != (newbut&k)) {
			/* This button changed, generate an event */
			in_put(0);
			in_put(MOUS >> 8);
			in_put(mousecol);
			in_put(mouserow);
			event = ((newbut&k) ? 0 : 1);	/* up or down? */
			if (k == 2)			/* center button? */
				event += 4;
			if (k == 4)			/* right button? */
				event += 2;
			if (sstate & 3)			/* shifted */
				event += 'A';
			else if (sstate & 4)		/* controled? */
				event += 1;
			else
				event += 'a';		/* plain */
			in_put(event);
			oldbut = newbut;
			return(TRUE);
		}
	}
	return(FALSE);
}
#endif

int doschar()

{
/* USG - SysV console i/o - From  "C Users Journal", April 1989, Vol. 7, # 9 */
    if(kbdqp)               /* any typeahead known? */
        kbdqp = FALSE;      /* yes, clear flag & return typeahead char */
    else {                  /* no typeahead. */
        if (fcntl(STDIN->_file, F_SETFL, kbdflgs) < 0 && kbdpoll)
            return(FALSE);
        kbdpoll = FALSE;
        read(STDIN->_file, &kbdq, 1);   /* wait and get a char */
    }
    if (kbdq == '\036') {   /* D.G. Dasher CRT function key lead-in? */
        in_put(SPEC >> 8);  /* ??? stash in the keyboard buffer stuff ??? */
        return(0);
    }
    return (kbdq & 255);
}

/*
 * Read a character from the terminal, performing no editing and doing no echo
 * at all.
 */
int ttgetc()
{
        /* return any keystrokes waiting in the
        type ahead buffer */
ttc:    if (in_check())
		return(in_get());

	if (typahead())
	    return(doschar());

	/* with no mouse, this is a simple get char routine */
	if (mexist == FALSE || mouseflag == FALSE)
            return(doschar());

#if	MOUSE
	/* turn the mouse cursor on */
/*	rg.x.ax = 1;*/	/* Show Cursor */
/*	int86(0x33, &rg, &rg);*/
	/* loop waiting for something to happen */
	while (TRUE) {
		if (typahead())
			break;
		if (checkmouse())
			break;
	}

	/* turn the mouse cursor back off */
/*	rg.x.ax = 2;*/	/* Hide Cursor */
/*	int86(0x33, &rg, &rg);*/

        goto ttc:
#endif  /* MOUSE */
}

#if     TYPEAH
/* typahead:    Check to see if any characters are already in the
                keyboard buffer.  Hurray for kludges!!!
*/
int typahead()
{
/* USG - SysV console i/o - From  "C Users Journal", April 1989, Vol. 7, # 9 */
    if (!kbdqp) {
        if (fcntl(STDIN->_file, F_SETFL, (kbdflgs | O_NDELAY)) < 0 && kbdpoll)
            return(FALSE);
        kbdpoll = TRUE;
        kbdqp = (1 == read(STDIN->_file, &kbdq, 1));
    }
    return(kbdqp);
}

#endif  /* TYPEAH */

/*      Spawn:  various DOS access commands
                for MicroEMACS ver 3.9e
*/


/*
 * Create a subjob with a copy of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as garbage so that you do a full
 * repaint. Bound to "^X C".
 */
int spawncli(f, n)
{
    register char *cp;

    /* don't allow this command if restricted */
    if (restflag)
        return(resterr());

#if     MV_UX
    TTflush();
    TTclose();                              /* stty to old settings */
    if ((cp = getenv("SHELL")) != NULL && *cp != '\0')
        system(cp);
    else
        system("exec /bin/sh");
#endif

#if     AOSVS
    init_tline();
    strcat(tline,"CHAIN/1=AB/2=AB,:CLI,EMACS_SET_CLI_PREFIX");
    do_system();
#endif

    sleep(2);
    return(TRUE);
}

/*
 * Run a one-liner in a subjob. When the command returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X !".
 */
int spawn(f, n)
{
    register int    s;
    char            line[NLINE];

    /* don't allow this command if restricted */
    if (restflag)
        return(resterr());

    if ((s=mlreply("!", line, NLINE)) != TRUE)
        return (s);
    init_tline();
    strcat(tline,line);
    do_system();
    do_system_end();
    return (TRUE);
}

/*
 * Run an external program with arguments. When it returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X $".
 */

int execprg(f, n)

{
    register int    s;
    char            line[NLINE];

    /* don't allow this command if restricted */
    if (restflag)
        return(resterr());

    if ((s=mlreply("!", line, NLINE)) != TRUE)
        return (s);
    strcpy(tline, line);
    do_system();
    do_system_end();
    return (TRUE);
}

/*
 * Pipe a one line command into a window
 * Bound to ^X @
 *
 * This command REQUIRES a directory named :TMP or a link by that name to 
 * another directory such as :SL_TEMPS.  This diectory should have an
 * ACL of  +,RWE  which will allow all users to access it.  This diectory is
 * where the output file for this command is placed.  This was done because
 * the sons of MircoEMACS do not always have the same privileges.  The file
 * is of the pathname :TMP:pid.MICRO_EMACS_COMMAND where "pid" is the PID
 * of the MicroEMACS process.  Also note that this sort of follows the UNIX(tm)
 * convention for a common temporary file directory.
 * The temp. file is created with an ACL of  username,OWAR
 *
 * See the THINGS_TO_DO.TXT for planned enhancements in this area.
 *
 */
int pipecmd(f, n)
{
    register int    s;      /* return status from CLI */
    register WINDOW *wp;    /* pointer to new window */
    register BUFFER *bp;    /* pointer to buffer to zot */
    char    line[NLINE];    /* command line send to shell */
    const char pipecmd_bname[] = "command";
    char pipecmd_filnam[NFILEN] = ":tmp:";  /* must be AOS/VS format!!!! */


    /* don't allow this command if restricted */
    if (restflag)
        return(resterr());

    /* get the users pid and build a pathname for our scratch file */
    ac0.ulng = 0L;
    ac2.ulng = 0L;
    ac1.ulng = -1L;
    sys($PNAME, &ac0, &ac1, &ac2);    /* get our PID */
    itoa(ac1.in, line);
    strcat(pipecmd_filnam, line);     /* build temp. filename */
    strcat(pipecmd_filnam, ".micro_emacs_command");

    zero((char *) &create_pkt, sizeof(create_pkt));
    zero(acl_buf, $MXACL);
    ac2.cptr = &acl_buf;
    ac0.ulng = -1L;
    ac1.ulng = -1L;
    sys($GUNM, &ac0, &ac1, &ac2);     /* get our user name */
    ac0.cptr = &acl_buf[strlen(acl_buf)];
    ac0.cptr++;
    *ac0.cptr = ($FACO | $FACW | $FACA | $FACR);  /* specify the ACL */
    ac0.cptr++;
    *ac0.cptr = '\000';
    create_pkt.cacp = &acl_buf;         /* load addr of temp file ACL */

    create_pkt.ctim = -1L;
    create_pkt.cdel = -1L;
    create_pkt.cmil = -1L;
    create_pkt.cftyp_format = $ORDS;    /* data sensitive */
    create_pkt.cftyp_entry  = $FTXT;    /* TEXT file type */
    ac2.ptr = &create_pkt;
    ac1.lng = 0L;
    ac0.cptr = &pipecmd_filnam;
    sys($CREATE, &ac0, &ac1, &ac2);   /* have AOS/VS create the temp file */

    /* get the command to pipe in */
    if ((s=mlreply("@", line, NLINE)) != TRUE)
        return(s);

    /* get rid of the command output buffer if it exists */
    if ((bp=bfind(pipecmd_bname, FALSE, 0)) != FALSE) {
        /* try to make sure we are off screen */
        wp = wheadp;
        while (wp) {
            if (wp->w_bufp == bp) {
                onlywind(FALSE, 1);
                break;
            }
            wp = wp->w_wndp;
        }
        if (zotbuf(bp) != TRUE)
            return(FALSE);
    }

    s = 0;                          /* init. index into line        */
    init_tline();
    strcat(tline,"LISTFILE,");      /* give CLI an @LIST to use     */
    strcat(tline,pipecmd_filnam);           /* tack on the filename for @LIST */
    strcat(tline,";");              /* separate the commands        */
    s = strpbrk(line,",; \t");      /* check for cmd line delimiters */
    if (s) {                        /* find any?    */
        strncat(tline,line,(s-(int) &line));/* get whats before the delimeter*/
        strcat(tline,"/L");         /* tack on "use @LIST file" switch */
        strcat(tline,s);            /* get the rest of the cmd line */
    } else {                        /* no delimiers...      */
         strcat(tline,line);        /* get the cmd line     */
        strcat(tline,"/L");         /* tack on "use @LIST file" switch*/
    }
    strcat(tline,";BYE/L=@NULL");   /* tell CLI to die quietly      */
    do_system();
    s = TRUE;

    /* split the current window to make room for the command output */
    if (splitwind(FALSE, 1) == FALSE)
        goto fubar;

    /* and read the stuff in */
    if (getfile(pipecmd_filnam, FALSE) == FALSE)
        goto fubar;

    /* make this window in VIEW mode, update all mode lines */
    curwp->w_bufp->b_mode |= MDVIEW;
    wp = wheadp;
    while (wp) {
        wp->w_flag |= WFMODE;
        wp = wp->w_wndp;
    }

    /* and get rid of the temporary file */
    unlink(pipecmd_filnam);
    return(TRUE);

fubar:
    unlink(pipecmd_filnam);
    return(FALSE);
}

/*
 * filter a buffer through an external DOS program
 * Bound to ^X #
 */
int filter(f, n)

{
    mlwrite(TEXT217);
/*    mlwrite("[Not available yet under AOS/VS]");*/
    sleep(1);
    return(FALSE);
}


/*
    return a system dependant string with the current time
    original version didn't work.  modified idea of bill benedetto by
    doug rady.  note the use of sys($ITIME, ...)  instead of sys_itime() 
*/
char *PASCAL NEAR timeset()

{
    register char *sp;      /* temp string pointer */
    short int tvec[2];
    extern char *dg_ctime();

    ac0.ulng = 0L;
    ac1.ulng = 0L;
    ac2.ulng = 0L;
    sys($ITIME, &ac0, &ac1, &ac2);    /* get system time */
    tvec[0] = ac2.in;
    ac1.lng /= 32768L;
    tvec[1] = (int)(ac1.lng/2L);
    sp = dg_ctime(tvec);
    sp[ strlen(sp)-1 ] = NULL;
    return(sp);
}

VOID init_tline()
{
    extern char *curdir();

    tline[0] = '\000';
    strcat(tline, "DIR,");
    curdir(&tline[4]);
    strcat(tline, ";");
    return;
}

VOID do_system()
{
    movecursor(term.t_nrow, 0);             /* Seek to last line.   */
    mlerase();
    TTclose();                              /* stty to old modes    */
    system(tline);
    TTkopen();
    sgarbf = TRUE;
}

VOID do_system_end()
{
    int s;

    if  (clexec == TRUE) {
        mlputs(TEXT188);
/*        mlputs("[End]");*/
        TTflush();
        while ((s = tgetc()) != '\r' && s != ' ')
            ;
    }
    return;
}

/*
    Data General AOS/VS terminal handling routines
  
    Known types are:
      DASHER D2xx/4xx series - support primarily for D2xx series
      written by Doug Rady (based on ANSI.C and VMSVT.C)
 */
extern  VOID    ttopen();
extern  VOID    ttkopen();
extern  VOID    ttkclose();
extern  VOID    tteeol();
extern  VOID    tteeop();
extern  VOID    ttbeep();
extern  VOID    dashermove();
extern  VOID    ansimove();
extern  VOID    dasherrev();
extern  VOID    ansirev();
extern  int     ttcres();
#if     COLOR
extern  VOID    ttfcol();
extern  VOID    ttbcol();
#endif
extern  VOID    dasherdim();
extern  VOID    ansidim();
extern  VOID    spal();

#define NROWS   24              /* normal # of screen rows */
#define MXROWS  24              /* max # of screen rows */
#define NCOLS   80              /* normal mode # of screen columns*/
#define MXCOLS  135             /* wide mode # of screen columns*/
#define MARGIN  4               /* size of minimim margin and   */
#define SCRSIZ  64              /* scroll size for extended lines */
#define NPAUSE  100             /* # times thru update to pause */
#define ESC     0x16
#define BEL     7

/*
 * Dispatch table. All the
 * hard fields just point into the
 * terminal I/O code.
 */
noshare TERM    term    = {
        MXROWS -1,
        NROWS -1,
        MXCOLS,
        NCOLS,
	0, 0,
        MARGIN,
        SCRSIZ,
        NPAUSE,
        &ttopen,
        &ttclose,
        &ttkopen,
        &ttkclose,
        &ttgetc,
#if TTPUTC == 0
        &ttputc,
#endif
#if TTFLUSH == 0
        &ttflush,
#endif
        &dashermove,
        &tteeol,
        &tteeop,
        &tteeop,
        &ttbeep,
        &dasherrev,
        &ttcres,
        &dasherdim,
#if     COLOR
        &ttfcol,
        &ttbcol
#endif
};

/*
    dashermove - Move the cursor for DG Dasher
 */
VOID dashermove(row, col)
{
    TTputc('\020');
    TTputc(col);
    TTputc(row);
}

/*
    ansimove - Move the cursor for ANSI crt
 */
VOID ansimove(row, col)

int row, col;
{
    char rc_tmp[ 8 ];

    ++row;
    ++col;
    ttputs("\033[");
    itoa(row, rc_tmp);
    ttputs(rc_tmp);
    TTputc(';');
    itoa(col, rc_tmp);
    ttputs(rc_tmp);
    TTputc('H');
}

/*
 *  dasherrev - set the reverse video status for DG Dasher
 */
VOID dasherrev(status)

int status;     /* TRUE = reverse video, FALSE = normal video */
{
    if (status)
        ttputs("\036\104");
    else
        ttputs("\036\105");
}

/*
 *  ansirev- set the reverse video status for ANSI crt
 */
VOID ansirev(status)

int status;
{
    if  (dimsts) {
        if (status) {
            ttputs("\033[0;2;7m");
        } else {
            ttputs("\033[0;2m");
        }
    } else {
        if (status) {
            ttputs("\033[0;7m");
        } else {
            ttputs("\033[0m");
        }
    }
    if  (status)
        revsts = TRUE;
    else
        revsts = FALSE;
}

/*
 *  dasherdim - set the dim/bright video status for DG Dasher
 */
VOID dasherdim(status)

int status;     /* TRUE = dim video, FALSE = bright video */
{
    if (status)
       TTputc('\034');
    else
       TTputc('\035');
}

/*
 *  ansidim - set the dim/bright video status for ANSI crt
 */
VOID ansidim(status)

int status;
{
    if  (revsts) {
        if (status) {
            ttputs("\033[0;2;7m");
        } else {
            ttputs("\033[0;7m");
        }
    } else {
        if (status) {
            ttputs("\033[0;2m");
        }
        else
        {
            ttputs("\033[0m");
        }
    }
    if  (status)
        dimsts = TRUE;
    else
        dimsts = FALSE;
}

/*
    ttcres - Change screen resolution (what resolution?)
*/
int ttcres()
{
        return(TRUE);
}

VOID spal()          /* change palette string */

{
        /*      Does nothing here       */
}

#if     COLOR
/*
    ttfcol - Set the forground color (not implimented)
 */
VOID ttfcol()
{
}

/*
    ttbcol - Set the background color (not implimented)
 */
 
VOID ttbcol()
{
}
#endif  /* COLOR */

/*
    tteeol - Erase to end of line
 */
VOID tteeol()
{
    ttputs(crt_eol);
}


/*
    tteeop - Erase to end of page (clear screen)
 */
VOID tteeop()
{
    ttputs(crt_eop);
}


/*
    ttbeep - Ring the bell
 */
VOID ttbeep()
{
    TTputc('\007');
}

VOID ttputs(str)
char *str;
{
    while(*str) {
        putc(*str, stdout);
	str++;
    }
}

/*
    ttopen() - open the terminal and change characteristics for our use
 */

VOID ttopen()
{
    int sys_err;
    P_CHAR_EX   crt_info;               /* for ?GECHR system call */
    /*
    set some traps
    */
    signal(SIGTRAP, &traceback);
    signal(SIGIOT,  &traceback);
    signal(SIGILL,  &traceback);
    signal(SIGSEGV, &traceback);
    signal(SIGTERM, &traceback);
/*    signal(SIGINT,  &traceback);*/
    signal(SIGQUIT, &traceback);
    signal(SIGEMT,  &traceback);
    signal(SIGFPE,  &traceback);
    signal(SIGKILL, &traceback);
    signal(SIGBUS,  &traceback);
    signal(SIGSYS,  &traceback);
    signal(SIGTERM, &traceback);

    ac0.in = fchannel(stdout);    /* make sure it is opened */
    ac0.in = fchannel(stdin);     /* make sure it is opened */
    ac1.ulng = (BIT0 | (sizeof(crt_info)/2)); /* get characteristics flag */
    ac2.ptr = (unsigned long*) &crt_info;

    sys_err = sys($GECHR, &ac0, &ac1, &ac2);    /* ?GECHR system call */

    /*
        copy the actual stdio pointer macro values into pointers
    */
    STDIN   = stdin;
    STDOUT  = stdout;

    /*
        set LPP & CPL in case they aren't == defaults - idea from bill benedetto
        resetting the max. row value is condtional because some of us can
        display more than LPP lines on a screen.
    */
    term.t_ncol = (short)crt_info.char_cpl;     /* get CPL */
    term.t_nrow = ((short)crt_info.char_lpp -1);/* get LPP */
    if  (term.t_nrow > term.t_mrow)   /* only reset max. row if LPP is > */
        term.t_mrow = term.t_nrow;      /* default max. row */

#if XXCRT
    if  (termcode == 0) { /* CRT type given on command line? */
        /* nope, we must figure it out */
    }
    
    switch  (termcode) {
        case 0: /* Generic ANSI compliant */
            crt_eol     = "\033[K";
            crt_eop     = "\033[J";
            term.t_move = &ansimove;
            term.t_rev  = &ansirev;
            term.t_dim  = &ansidim;
/*
            term.t_getkey = &ansigetkey;
*/
            break;
        case 1: /* DEC VT100 */
        case 2: /* DEC VT100K */
            crt_eol     = "\033[K";
            crt_eop     = "\033[J";
            term.t_move = &ansimove;
            term.t_rev  = &ansirev;
            term.t_dim  = &ansidim;
/*
            term.t_getkey = &vt100getkey;
*/
            break;
        case 4: /* DEC VT102 */
        case 5: /* DEC VT102K */
            crt_eol     = "\033[K";
            crt_eop     = "\033[J";
            term.t_move = &ansimove;
            term.t_rev  = &ansirev;
            term.t_dim  = &ansidim;
            crt_func = (INS_CHAR | INS_LINE | DEL_CHAR | DEL_LINE);
/*
            term.t_getkey = &vt100getkey;
*/
            break;
        case 7: /* DEC VT220 */
        case 8: /* DEC VT220K */
            crt_eol     = "\033[K";
            crt_eop     = "\033[J";
            term.t_move = &ansimove;
            term.t_rev  = &ansirev;
            term.t_dim  = &ansidim;
/*
            term.t_getkey = &vt220getkey;
*/
            break;
        case 6: /* D.G. Dasher D4xx */
            crt_func = (INS_CHAR | INS_LINE | DEL_CHAR | DEL_LINE);
            break;
        case 3: /* D.G. Dasher D2xx */
        default;
    }
#else
    if  ((crt_info.char_cdt != char_d2xx)    /* is not CRT3 or D2xx ?  and */
    &&  (crt_info.char_cdt != char_d4xx)) {  /* is not CRT6 or D4xx or D5xx ? */
        crt_eol     = "\033[K";
        crt_eop     = "\033[J";
        term.t_move = &ansimove;
        term.t_rev  = &ansirev;
        term.t_dim  = &ansidim;
    }
#endif  /* XXCRT */

    /*
        change terminal charactersitcs to Unix(tm) raw mode
    */
    ioctl(STDIN->_file, TCGETA, &old_in_termio);    /* save old settings */
    new_in_termio.c_iflag = 0;            /* setup new settings */
    new_in_termio.c_oflag = 0;
    new_in_termio.c_lflag = 0;
    new_in_termio.c_cc[VTIME] = 0;
    new_in_termio.c_cc[VMIN] = 1;
    new_in_termio.c_line = BELL_LD;       /* emulate unix(tm) line handling */
    new_in_termio.c_cflag = old_in_termio.c_cflag;
    ioctl(STDIN->_file, TCSETA, &new_in_termio);
    kbdflgs = fcntl(STDIN->_file, F_GETFL, 0);
    dimsts = 0;
    revsts = 0;

    /*
    check for mouse here
    */
    mexist = 0;
    nbuttons = 0;
    oldbut = 0;

    /*
        on all screens we are not sure of the initial position
        of the cursor
    */
    ttrow = 999;
    ttcol = 999;

    /* assume terminal has following */
    eolexist = TRUE;
    revexist = TRUE;
    strcpy(sres, "NORMAL");

    /*
        here we lower the priority of this task so that the console
	reader task will always get control when we get a char.
    */
    ac0.ulng = 0L;                      /* will get TID of this task */
    ac1.ulng = 0L;                      /* will get task pri. this task */
    ac2.ulng = 0L;                      /* who knows... */
    sys($MYTID, &ac0, &ac1, &ac2);
    ac1.ulng = 100L;                    /* new task priority */
    ac2.ulng = 0L;                      /* must be zero... */
    sys($IDPRI, &ac1, &ac0, &ac2);      /* do it!!! */

}

/*
    open the keyboard
*/  
VOID ttkopen()
{
    /*
        activate the MircoEmacs console characteristics
    */
    ioctl(STDIN->_file, TCSETA, &new_in_termio);
    fcntl(STDIN->_file, F_SETFL, kbdflgs);
    kbdqp = 0;
    kbdpoll = FALSE;
    kbdq = '\000';
    in_init();
}

VOID ttkclose()
{
}

#if     FLABEL
fnclabel(f, n)          /* label a function key */

int f,n;        /* default flag, numeric argument [unused] */

{
        /* on machines with no function keys...don't bother */
        return(TRUE);
}
#endif

/*
     Change the current working directory
*/
PASCAL NEAR int chdirectory()

{
#if CHDIR                               /* include this code?   */

    /* don't allow this command if restricted    */
    if (restflag)
        return(resterr());

    if ((ac0.in = mlreply("Directory: ", tline, NLINE)) != TRUE)
        return(ac0.in);
	
    ac0.in = chdir(tline);    /* change the current working directory */

    /*
    tell the story... success or failure
    */
    if (ac0.in) {
        mlwrite("Error- directory not changed.");
        return(ac0.in);
    }
    else {
        mlwrite("Directory changed.");
	return(TRUE);
    }
#endif
}


#if ORMDNI

/*
    superuser on/off toggle routines to override those annoying ACLs

    usage:  superuser_on();  or  superuser_off();

*/
int superuser_on()

{
    int err;

    if  (restflag)
        return(resterr());

    ac0.lng = -1L;
    ac1.ulng = 0L;
    ac2.ulng = 0L;
    err = 0;

    if  ((err = sys($SUSER, &ac0, &ac1, &ac2)))
        if  (err = ERPRV)
	    mlwrite("Error: you are not allowed use of superuser.");
        else
	    mlwrite("Error turning superuser ON.");

    return(err);
}


int superuser_off()

{
    int err;

    if  (restflag)
        return(resterr());

    ac0.ulng = 1L;
    ac1.ulng = 0L;
    ac2.ulng = 0L;
    err = 0;

    if  ((err = sys($SUSER, &ac0, &ac1, &ac2)))
        mlwrite("Error turning superuser OFF.");

    return(err);
}
#endif

#if     ISADIR
int isadirectory(fstream)

FILE *fstream;

{
    return(isadir(fstream->_file));
}
#endif


/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*  All aosvs$ library routines Copyright (c) 1989 by Douglas C. Rady       */
/*                                                                          */
/*                                                                          */
/****************************************************************************/

/*
    aosvs$bsd_dir.h -- a replacement inlcude file under AOS/VS for:
	<dir.h> -- definitions for 4.2BSD-compatible directory access

        Taken from GNU's emacs/etc/ndir.h for porting GNU stuff to AOS/VS.

        All of the usual fields are defined but dd_buf is defined as a char*
	so we can pass a poniter to a template under AOS/VS.
*/

#ifndef DIRSIZ
struct direct {				/* data from readdir() */
	long		d_ino;		/* inode number of entry */
	unsigned short	d_reclen;	/* length of this record */
	unsigned short	d_namlen;	/* length of string in d_name */
	char		d_name[$MXFN];	/* name of file */
    };
#endif

typedef struct {
	int	dd_fd;			/* file desc. - channel# under aos/vs */
	int	dd_loc;			/* offset in block */
	int	dd_size;		/* amount of valid data */
        char   *dd_buf;                 /* pointer to wildcard/template */
    }	DIR;			        /* stream data from opendir() */

/*
    set up the MV/Eclipse accumulators used by the  aosvs$  routines
*/
$align(1) $low32k union aosvs$accumulator { /* dearly beloved, we are gathered here...*/
    unsigned long * ptr;            /* pointer to unsigned long */
    char *          cptr;           /* pointer to char */
    unsigned int  * pint;           /* pointer to unsigned int */
    unsigned short *psht;           /* pointer to short */
    unsigned long   ulng;           /* unsigned long */
    signed long     lng;            /* signed long */
    unsigned int    uin;            /* unsigned int */
    signed int      in;             /* signed int */
    unsigned short usht;            /* unsigned short */
    unsigned char   chr;            /* a char, unsigned of course!!! */
} aosvs$ac0, aosvs$ac1, aosvs$ac2;  /* our bountiful accumulators, sigh... */


/*
    aosvs$bsd_dir.c -- fake 4.2BSD directory access routines for AOS/VS

    System call city...

*/

P_GNFN aosvs$bsd_gnfn_pkt;

/*
    aosvs$bsd_closedir
*/
void closedir(dir_stream)  /* $name("aosvs$bsd_closedir") */

DIR *dir_stream;

{
    aosvs$ac0.ulng = 0L;
    aosvs$ac2.ulng = 0L;
    aosvs$ac1.in = dir_stream->dd_fd;   /* load channel number */
    sys($GCLOSE, &aosvs$ac0, &aosvs$ac1, &aosvs$ac2);
}


/*
    aosvs$bsd_opendir
*/

DIR *opendir(dir_name) /* name$("aosvs$bsd_opendir") */

char *dir_name;

{

#include    "packets/block_io.h"
    
    P_GOPEN gopen_pkt;
    DIR *dir_stream;
    int err;
    char t_name[$MXPL];

    err = 0;
    dir_stream = (DIR *) malloc(sizeof(DIR));
    zero((char *) dir_stream, sizeof(DIR));
    zero((char *) &aosvs$bsd_gnfn_pkt, sizeof(aosvs$bsd_gnfn_pkt));
    zero((char *) &gopen_pkt, sizeof(gopen_pkt));

    err = aosvs$expand_pathname(dir_name, t_name);
    aosvs$ac0.cptr = t_name;
    aosvs$ac1.lng = -1L;
    aosvs$ac2.ptr = &gopen_pkt;

    /* Try to ?GOPEN the file. */
    if (err = sys($GOPEN, &aosvs$ac0, &aosvs$ac1, &aosvs$ac2)) {
        free(dir_stream);       /* Error Will Robinson! Error! Error! */
        return(NULL);
    }

    /* Make sure it is some type of directory! */
    if ((gopen_pkt.opty_type != $FDIR) && (gopen_pkt.opty_type != $FLDU)
    && (gopen_pkt.opty_type != $FCPD)) {    /* AOS/VS 7.62 directory types. */
        dir_stream->dd_fd = (int)gopen_pkt.opch;
        closedir(dir_stream);
        free(dir_stream);       /* Error Will Robinson! Error! Error! */
        return(NULL);
    }

    dir_stream->dd_fd = (int)gopen_pkt.opch;    /* stash the channel number */
    return(dir_stream);
}


/*
    aosvs$bsd_readdir
*/

struct direct *readdir(dir_stream)   /* name$("aosvs$bsd_readdir") */

DIR *dir_stream;

{
    struct direct *dptr;

    dptr = NULL;
    aosvs$ac0.in = 0;
    aosvs$ac1.in = dir_stream->dd_fd;   /* load channel number */

    dptr = (struct direct *) malloc(sizeof(struct direct));
    zero((char *)dptr, sizeof(struct direct));

    if (dir_stream->dd_loc)
        aosvs$bsd_gnfn_pkt.nfky     = (short)dir_stream->dd_loc;
    else
        aosvs$bsd_gnfn_pkt.nfky     = 0;

    if (dir_stream->dd_buf)         /* if passed a template */
        aosvs$bsd_gnfn_pkt.nftp = dir_stream->dd_buf;   /* load it into packet */
    else
        aosvs$bsd_gnfn_pkt.nftp   = (char *) -1L;   /* load default flag */

    aosvs$bsd_gnfn_pkt.nfnm   = dptr->d_name;       /* load buffer ptr */
    aosvs$ac2.ptr  = &aosvs$bsd_gnfn_pkt;
    if  (sys($GNFN, &aosvs$ac0, &aosvs$ac1, &aosvs$ac2) == 0)
    {
        /* save the AOS/VS internal pointer */
        dir_stream->dd_loc = (int) aosvs$bsd_gnfn_pkt.nfky;

        /* load the direct struct values */
        dptr->d_ino = (long) aosvs$bsd_gnfn_pkt.nfky;   /* fake an inode */
        dptr->d_reclen = sizeof(struct direct);         /* why? why not? */
	dptr->d_namlen = strlen(dptr->d_name);          /* handy to have */
	return(dptr);
    }

    free(dptr);
    return(NULL);    
}


/*
    aosvs$bsd_seekdir
*/

void seekdir(dir_stream, pos)  /* name$("aosvs$bsd_seekdir") */

DIR *dir_stream;
long pos;

{
    dir_stream->dd_loc = (short)pos;
}


/*
    aosvs$bsd_telldir
*/

long telldir(dir_stream)  /* name$("aosvs$bsd_telldir") */

DIR *dir_stream;

{
    return(dir_stream->dd_loc);
}


/*
aosvs$unix_to_aosvs_path.c -- convert a Unix(tm) pathname to a Aos/Vs pathname
                              We also accept the Ms-Dos '\' seperator and
			      convert it to the Unix(tm) '/' seperator.  We do
			      not deal with Ms-Dos device specifiers. The '\'
			      is handled since most current Ms-Dos C compilers
			      can deal with either '\' or '/'.

usage:
    aosvs$unix_to_aosvs(u_path, a_path);

where:
data item name          data type           description
----------------------- ------------------  -----------------------------------
u_path                  char *              char * of Unix(tm) pathname,
                                            end with null.
a_path                  char *              char * for Aos/Vs pathname, MUST be
                                            $MXPL in length.

-------------------------------------------------------------------------------
edit history

who  mm/dd/yy  rev #  what.....................................................
---  --------  -----  ---------------------------------------------------------
dcr  01/27/89  01.00  birth, new life, creation...
dcr  03/02/89  01.01  cleaned up, added internal temp. storage for path.
dcr  03/02/89  01.02  added code to deal with Ms-Dog '\' seperator.
dcr  03/02/89  01.03  added code to skip out if first char is legal Aos/Vs char.
                      This makes us "just like" _toaos_fid().
*/
void aosvs$unix_to_aosvs_path(u_path, a_path)

char *u_path, *a_path;
{

    extern int _toaos_fid();        /* Data General library routine */

    /*
    local variables
    */
    register char *up, *ap;
    register int dec1;
    char t_path[$MXPL], octal[4];

    /*
    check for null ptrs... no tricks here please...
    */
    if ((u_path == NULL) || (a_path == NULL))
        return;

    /*
    copy to register vars.
    */
    up = u_path;            /* load ptr to Unix(tm) path */
    ap = t_path;            /* load ptr to temp. storage area */
    zero(t_path, $MXPL);    /* zero the temp. storage area */
            
    /*
    to be "just like" DG's _toaos_fid()  we skip out if the first char. is a
    legal Aos/Vs seperator.  This is from page 2-1 of the "Using Specialized
    C Functions" manual, DG part number 093-000585-00.
    */
    if ((*up == '^') || (*up == '@') || (*up == '=') || (*up == ':')) {
        strcpy(a_path, u_path);
	return;
    }

    /*
    step through the unix(tm) pathname and copy or translate bytes into
    the temp. storage area.
    */
    while (*up) {     /* better be NULL terminated!!! */
        if ((*up == '$') || (*up == '?') || (*up == '\\') || (*up == '_')
	|| ((*up >= '.') && (*up <= ':'))   /* thank you ASCII */
	|| ((*up >= 'A') && (*up <= 'Z'))
	|| ((*up >= 'a') && (*up <= 'z'))) {
            if (*up == '\\') {   /* convert ms-dos '\' to unix(tm) '/' */
	        *ap++ = '/';
		up++;
                continue;
            }
	    *ap++ = *up++;
            continue;
        }

        dec1 = 0;
        zero(octal, 4);
        *ap++ = '?';            /* marker for octal replacement */
        dec1 = (int)*up;
        otoa(dec1, octal);
        if (dec1 < 64)
	    *ap++ = '0';
        strcat(ap, octal);
        if (dec1 >= 64)
	    ap++;
        ap++;
	ap++;
        up++;
    }

    *ap = NULL;

    /*
    go home...
    */
    _toaos_fid(t_path, a_path);

    return;
}

/*
aosvs$expand_pathname.c

usage:
    err = aosvs$expand_pathname(c_path, x_path);

where:
data item name          data type           description
----------------------- ------------------  -----------------------------------
err                     int                 Error return, if any.
c_path                  char *              Current pathname.
x_path                  char *              Expanded pathname returned here.
                                            This must be at least $MXPL bytes.

-------------------------------------------------------------------------------
edit history

who  mm/dd/yy  rev #  what.....................................................
---  --------  -----  ---------------------------------------------------------
dcr  03/02/89  01.00  birth, new life, creation...

*/
 
/*
aosvs$expand_pathname.c

usage:
    err = aosvs$expand_pathname(c_path, x_path);

where:
data item name          data type           description
----------------------- ------------------  -----------------------------------
err                     int                 Error return, if any.
c_path                  char *              Current pathname.
x_path                  char *              Expanded pathname returned here.
                                            This must be at least $MXPL bytes.

-------------------------------------------------------------------------------
edit history

who  mm/dd/yy  rev #  what.....................................................
---  --------  -----  ---------------------------------------------------------
dcr  03/02/89  01.00  birth, new life, creation...

*/
 
int aosvs$expand_pathname(c_path, x_path)

char *c_path, *x_path;

{
    /*
    local variables
    */
    char t_path[$MXPL];

    /*
    things to do...
    */
    zero(t_path, $MXPL);
    aosvs$unix_to_aosvs_path(c_path, t_path);

    aosvs$ac2.in = $MXPL;
    aosvs$ac0.cptr = t_path;
    aosvs$ac1.cptr = x_path;
    if (sys($GRNAME, &aosvs$ac0, &aosvs$ac1, &aosvs$ac2))
        if ((aosvs$ac0.in == ERFDE) || (aosvs$ac0.in == ERFDE))
            strcpy(x_path, t_path);
        else
	    return(1);

    return(0);
}

extern DIR		*opendir();
extern struct direct	*readdir();
extern long		telldir();
extern void		seekdir();
extern void		closedir();

#define rewinddir( dirp )	seekdir( dirp, 0L )


/*	FILE Directory routines		*/

char gnfntmp[NFILEN];   /* wildcard template */
char gnfnpath[NFILEN];	/* path of file to find */
char gnfnrbuf[NFILEN];	/* return file buffer */
DIR *gnfndir;
struct direct *gnfndirect;

char PASCAL NEAR *getnfile();

/*  do a template directory search (for file name completion) */

char *PASCAL NEAR getffile(fspec)

char *fspec;	/* pattern to match */

{
	register int index;		/* index into various strings */

        /* clean up from our last time in here... */
        if (gnfndir) {
	    closedir(gnfndir);
	    free(gnfndir);
        }

        if (gnfndirect)
	        free(gnfndirect);

        /* init. some things... */
        zero(gnfnpath, NFILEN);
        zero(gnfnrbuf, NFILEN);
	zero(gnfntmp, NFILEN);
        gnfndir = NULL;
        gnfndirect = NULL;

        /* first parse the file path off the file spec */
	strcpy(gnfnpath, fspec);
	index = strlen(gnfnpath) - 1;
	while (index >= 0 && (gnfnpath[index] != '/' &&
				gnfnpath[index] != '\\' && gnfnpath[index] != ':'))
		--index;

	gnfnpath[index+1] = 0;

        if  ((gnfndir = opendir(gnfnpath)) == NULL)
	    return(NULL);

        /* build the wildcard or template to use in the lookup */
        strcpy(gnfntmp, &fspec[index+1]);
        strcat(gnfntmp, "+");
        gnfndir->dd_buf = gnfntmp;

        return(getnfile());
}

char *PASCAL NEAR getnfile()

{
	register int index;		/* index into various strings */

        zero(gnfnrbuf, NFILEN);         /* init return buffer */

	/* and call for the next file */
        if ((gnfndirect = readdir(gnfndir)) == NULL) {
	    closedir(gnfndir);
            free(gnfndir);
            gnfndir = NULL;
            return(NULL);
        }

	/* return the next file name! */
	strcpy(gnfnrbuf, gnfnpath);
        strcat(gnfnrbuf, gnfndirect->d_name);
	mklower(gnfnrbuf);
        free(gnfndirect);
        gnfndirect = NULL;
	return(gnfnrbuf);
}

FILE *xxfopen(fn, mode)     /* expand a pathname and open it */

char *fn, *mode;

{
    char tmppath[NFILEN];               /* temp. to hold expanded pathname */

    strcpy(tmppath, fn);                        /* load passed pathname */
    resolve_full_pathname(tmppath, tmppath);    /* expand it... */
    return(fopen(tmppath, mode));           /* try to open expanded pathname */
}

#endif  /* AOSVS */
