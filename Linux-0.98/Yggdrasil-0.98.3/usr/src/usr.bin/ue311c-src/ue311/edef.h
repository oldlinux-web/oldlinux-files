/*	EDEF:		Global variable definitions for
			MicroEMACS 3.9

                        written by Daniel Lawrence
                        based on code by Dave G. Conroy,
                        	Steve Wilhite and George Jones
*/

#ifdef	maindef

/* for MAIN.C */

/* initialized global definitions */

NOSHARE int DNEAR fillcol = 72; 	/* Current fill column		*/
NOSHARE short kbdm[NKBDM];		/* Macro			*/
NOSHARE char *execstr = NULL;		/* pointer to string to execute */
NOSHARE char golabel[NPAT] = "";	/* current line to go to	*/
NOSHARE char paralead[NPAT] = " \t";	/* paragraph leadin chars	*/
NOSHARE char fmtlead[NPAT] = "";	/* format command leadin chars	*/
NOSHARE char mainbuf[] = "main";	/* name of main buffer		*/
NOSHARE char lterm[NSTRING];		/* line terminators on file write */
NOSHARE unsigned char wordlist[256];	/* characters considered "in words" */
NOSHARE int DNEAR wlflag = FALSE;	/* word list enabled flag	*/
NOSHARE int DNEAR clearflag = TRUE;	/* clear screen on screen change? */
NOSHARE int DNEAR execlevel = 0;	/* execution IF level		*/
NOSHARE int DNEAR eolexist = TRUE;	/* does clear to EOL exist?	*/
NOSHARE int DNEAR revexist = FALSE;	/* does reverse video exist?	*/
NOSHARE int DNEAR exec_error = FALSE;	/* macro execution error pending? */
NOSHARE int DNEAR flickcode = TRUE;	/* do flicker supression?	*/
CONST char *modename[] = { 		/* name of modes		*/
	"WRAP", "CMODE", "SPELL", "EXACT", "VIEW", "OVER",
	"MAGIC", "CRYPT", "ASAVE", "REP"};
CONST char modecode[] = "WCSEVOMYAR";	/* letters to represent modes	*/
NOSHARE int DNEAR numfunc = NFUNCS;	/* number of bindable functions */
NOSHARE int DNEAR gmode = 0;		/* global editor mode		*/
NOSHARE int DNEAR gflags = GFREAD;	/* global control flag		*/
NOSHARE int DNEAR gfcolor = 7;		/* global forgrnd color (white) */
NOSHARE int DNEAR gbcolor = 0;		/* global backgrnd color (black)*/
NOSHARE int DNEAR deskcolor = 0;	/* desktop background color	*/
NOSHARE int DNEAR gasave = 256; 	/* global ASAVE size		*/
NOSHARE int DNEAR gacount = 256;	/* count until next ASAVE	*/
NOSHARE int DNEAR sgarbf = TRUE; 	/* TRUE if screen is garbage	*/
NOSHARE int DNEAR mpresf = FALSE;	/* TRUE if message in last line */
NOSHARE int DNEAR clexec = FALSE;	/* command line execution flag	*/
NOSHARE int DNEAR mstore = FALSE;	/* storing text to macro flag	*/
NOSHARE int DNEAR discmd = TRUE; 	/* display command flag 	*/
NOSHARE int DNEAR disinp = TRUE; 	/* display input characters	*/
NOSHARE int DNEAR modeflag = TRUE;	/* display modelines flag	*/
NOSHARE int DNEAR timeflag = FALSE;	/* display time			*/
NOSHARE char DNEAR lasttime[6] = "";	/* last time string displayed	*/
NOSHARE int DNEAR popflag = TRUE;	/* pop-up windows enabled?	*/
NOSHARE int DNEAR posflag = TRUE;	/* display point position	*/
NOSHARE int cpending = FALSE;		/* input character pending?	*/
NOSHARE int charpending;		/* character pushed back	*/
NOSHARE int DNEAR sscroll = FALSE;	/* smooth scrolling enabled flag*/
NOSHARE int DNEAR hscroll = TRUE;	/* horizontal scrolling flag	*/
NOSHARE int DNEAR hjump = 1;		/* horizontal jump size 	*/
NOSHARE int DNEAR ssave = TRUE; 	/* safe save flag		*/
NOSHARE struct BUFFER *bstore = NULL;	/* buffer to store macro text to*/
NOSHARE int DNEAR vtrow = 0;		/* Row location of SW cursor	*/
NOSHARE int DNEAR vtcol = 0;		/* Column location of SW cursor */
NOSHARE int DNEAR ttrow = HUGE; 	/* Row location of HW cursor	*/
NOSHARE int DNEAR ttcol = HUGE; 	/* Column location of HW cursor */
NOSHARE int DNEAR lbound = 0;		/* leftmost column of current line
					   being displayed		*/
NOSHARE int DNEAR taboff = 0;		/* tab offset for display	*/
NOSHARE int DNEAR tabsize = 8;		/* current hard tab size	*/
NOSHARE int DNEAR stabsize = 0;		/* current soft tab size (0: use hard tabs)  */
NOSHARE int DNEAR reptc = CTRL | 'U';	/* current universal repeat char*/
NOSHARE int DNEAR abortc = CTRL | 'G';	/* current abort command char	*/
NOSHARE int DNEAR sterm = CTRL | '[';	/* search terminating character */
NOSHARE int DNEAR searchtype = SRNORM;	/* current search style		*/
NOSHARE int DNEAR yankflag = FALSE;	/* current yank style		*/

NOSHARE int DNEAR prefix = 0;		/* currently pending prefix bits */
NOSHARE int DNEAR prenum = 0;		/*     "       "     numeric arg */
NOSHARE int DNEAR predef = TRUE;	/*     "       "     default flag */

NOSHARE int DNEAR quotec = CTRL | 'Q';	/* quote char during mlreply() */
NOSHARE CONST char *cname[] = {		/* names of colors		*/
	"BLACK", "RED", "GREEN", "YELLOW", "BLUE",
	"MAGENTA", "CYAN", "GREY",
	"GRAY", "LRED", "LGREEN", "LYELLOW", "LBLUE",
	"LMAGENTA", "LCYAN", "WHITE"};
  
NOSHARE int kill_index;			/* current index into kill ring */
NOSHARE KILL *kbufp[NRING];		/* current kill buffer chunk pointer*/
NOSHARE KILL *kbufh[NRING];		/* kill buffer header pointer	*/
NOSHARE	int kskip[NRING];		/* # of bytes to skip in 1st kill chunk */
NOSHARE int kused[NRING];		/* # of bytes used in last kill chunk*/
NOSHARE WINDOW *swindow = NULL; 	/* saved window pointer 	*/
NOSHARE int cryptflag = FALSE;		/* currently encrypting?	*/
NOSHARE int oldcrypt = FALSE;		/* using old(broken) encryption? */
NOSHARE short *kbdptr;			/* current position in keyboard buf */
NOSHARE short *kbdend = &kbdm[0];	/* ptr to end of the keyboard */
NOSHARE int DNEAR kbdmode = STOP;	/* current keyboard macro mode	*/
NOSHARE int DNEAR kbdrep = 0;		/* number of repetitions	*/
NOSHARE int DNEAR restflag = FALSE;	/* restricted use?		*/
NOSHARE int DNEAR lastkey = 0;		/* last keystoke		*/
NOSHARE int DNEAR seed = 0;		/* random number seed		*/
NOSHARE long envram = 0l;		/* # of bytes current in use by malloc */
NOSHARE int DNEAR macbug = FALSE;	/* macro debugging flag		*/
NOSHARE int DNEAR mouseflag = TRUE;	/* use the mouse?		*/
NOSHARE int DNEAR diagflag = FALSE;	/* diagonal mouse movements?	*/
CONST char errorm[] = "ERROR";		/* error literal		*/
CONST char truem[] = "TRUE";		/* true literal 		*/
CONST char falsem[] = "FALSE";		/* false litereal		*/
NOSHARE int DNEAR cmdstatus = TRUE;	/* last command status		*/
NOSHARE char palstr[49] = "";		/* palette string		*/
NOSHARE char lastmesg[NSTRING] = ""; 	/* last message posted		*/
NOSHARE char *lastptr = NULL;		/* ptr to lastmesg[]		*/
NOSHARE int DNEAR saveflag = 0; 	/* Flags, saved with the $target var */
NOSHARE char *fline = NULL;		/* dynamic return line		*/
NOSHARE int DNEAR flen = 0;		/* current length of fline	*/
NOSHARE int DNEAR rval = 0;		/* return value of a subprocess */
NOSHARE int DNEAR eexitflag = FALSE;	/* EMACS exit flag		*/
NOSHARE int DNEAR eexitval = 0; 	/* and the exit return value	*/
NOSHARE int xpos = 0;		/* current column mouse is positioned to*/
NOSHARE int ypos = 0;		/* current screen row	     "		*/
NOSHARE int nclicks = 0;	/* cleared on any non-mouse event	*/
NOSHARE int disphigh = FALSE;	/* display high bit chars escaped	*/

/* uninitialized global definitions */

NOSHARE int DNEAR currow;	/* Cursor row			*/
NOSHARE int DNEAR curcol;	/* Cursor column		*/
NOSHARE int DNEAR thisflag;	/* Flags, this command		*/
NOSHARE int DNEAR lastflag;	/* Flags, last command		*/
NOSHARE int DNEAR curgoal;	/* Goal for C-P, C-N		*/
NOSHARE WINDOW *curwp; 		/* Current window		*/
NOSHARE BUFFER *curbp; 		/* Current buffer		*/
NOSHARE WINDOW *wheadp;		/* Head of list of windows	*/
NOSHARE BUFFER *bheadp;		/* Head of list of buffers 	*/
NOSHARE SCREEN *first_screen;	/* Head and current screen in list */
NOSHARE BUFFER *blistp;		/* Buffer for C-X C-B		*/
NOSHARE BUFFER *slistp;		/* Buffer for A-B		*/

NOSHARE char sres[NBUFN];	/* current screen resolution	*/

NOSHARE	char lowcase[HICHAR];	/* lower casing map		*/
NOSHARE	char upcase[HICHAR];	/* upper casing map		*/

NOSHARE char pat[NPAT];		/* Search pattern		*/
NOSHARE char tap[NPAT];		/* Reversed pattern array.	*/
NOSHARE char rpat[NPAT];	/* replacement pattern		*/

/*	Various "Hook" execution variables	*/

NOSHARE KEYTAB readhook;	/* executed on all file reads */
NOSHARE KEYTAB wraphook;	/* executed when wrapping text */
NOSHARE KEYTAB cmdhook;		/* executed before looking for a command */
NOSHARE KEYTAB writehook;	/* executed on all file writes */
NOSHARE KEYTAB exbhook;		/* executed when exiting a buffer */
NOSHARE KEYTAB bufhook;		/* executed when entering a buffer */

/* The variables matchline and matchoff hold the line
 * and offset position of the *start* of the match.
 * The variable patmatch holds the string that satisfies
 * the search command.
 */
NOSHARE int	matchlen;
NOSHARE int	matchoff;
NOSHARE LINE	*matchline;
NOSHARE char *patmatch = NULL;

#if	MAGIC
/*
 * The variables magical and rmagical determine if there
 * were actual metacharacters in the search and replace strings -
 * if not, then we don't have to use the slower MAGIC mode
 * search functions.
 */
NOSHARE short int DNEAR magical = FALSE;
NOSHARE short int DNEAR rmagical = FALSE;
NOSHARE MC mcpat[NPAT]; 	/* the magic pattern		*/
NOSHARE MC tapcm[NPAT]; 	/* the reversed magic pattern	*/
NOSHARE RMC rmcpat[NPAT];	/* the replacement magic array	*/
NOSHARE char *grpmatch[MAXGROUPS];	/* holds groups found in search */ 

#endif

/* directive name table:
	This holds the names of all the directives....	*/

CONST char *dname[] = {
	"if", "else", "endif",
	"goto", "return", "endm",
	"while", "endwhile", "break",
	"force"
};

#if	DEBUGM
/*	vars needed for macro debugging output	*/
NOSHARE char outline[NSTRING];	/* global string to hold debug line text */
#endif

/*
 * System message notification (at the moment, VMS only).
 */
#if	VMS
NOSHARE char brdcstbuf[1024];		/* Broadcast messages */
NOSHARE int pending_msg = FALSE;	/* Flag - have we notified user yet.*/
#endif

#else

/* for all the other .C files */

/* initialized global external declarations */

NOSHARE extern int DNEAR fillcol;	/* Current fill column		*/
NOSHARE extern short kbdm[];		/* Holds kayboard macro data	*/
NOSHARE extern char *execstr;		/* pointer to string to execute */
NOSHARE extern char golabel[];		/* current line to go to	*/
NOSHARE extern char paralead[];		/* paragraph leadin chars	*/
NOSHARE extern char fmtlead[];		/* format command leadin chars	*/
NOSHARE extern char mainbuf[];		/* name of main buffer		*/
NOSHARE extern char lterm[];		/* line terminators on file write */
NOSHARE extern unsigned char wordlist[];/* characters considered "in words" */
NOSHARE extern int DNEAR wlflag;	/* word list enabled flag	*/
NOSHARE extern int DNEAR clearflag;	/* clear screen on screen change? */
NOSHARE extern int DNEAR execlevel;	/* execution IF level		*/
NOSHARE extern int DNEAR eolexist;	/* does clear to EOL exist?	*/
NOSHARE extern int DNEAR revexist;	/* does reverse video exist?	*/
NOSHARE extern int DNEAR exec_error;	/* macro execution error pending? */
NOSHARE extern int DNEAR flickcode;	/* do flicker supression?	*/
CONST extern char *modename[];		/* text names of modes		*/
CONST extern char modecode[];		/* letters to represent modes	*/
NOSHARE extern int DNEAR numfunc;	/* number of bindable functions */
NOSHARE extern KEYTAB keytab[];		/* key bind to functions table	*/
NOSHARE extern NBIND names[];		/* name to function table	*/
NOSHARE extern int DNEAR gmode;		/* global editor mode		*/
NOSHARE extern int DNEAR gflags;	/* global control flag		*/
NOSHARE extern int DNEAR gfcolor;	/* global forgrnd color (white) */
NOSHARE extern int DNEAR gbcolor;	/* global backgrnd color (black)*/
NOSHARE extern int DNEAR deskcolor;	/* desktop background color	*/
NOSHARE extern int DNEAR gasave;	/* global ASAVE size		*/
NOSHARE extern int DNEAR gacount;	/* count until next ASAVE	*/
NOSHARE extern int DNEAR sgarbf;	/* State of screen unknown	*/
NOSHARE extern int DNEAR mpresf;	/* Stuff in message line	*/
NOSHARE extern int DNEAR clexec;	/* command line execution flag	*/
NOSHARE extern int DNEAR mstore;	/* storing text to macro flag	*/
NOSHARE extern int DNEAR discmd;	/* display command flag 	*/
NOSHARE extern int DNEAR disinp;	/* display input characters	*/
NOSHARE extern int DNEAR modeflag;	/* display modelines flag	*/
NOSHARE extern int DNEAR timeflag;	/* display time			*/
NOSHARE extern char DNEAR lasttime[];	/* last time string displayed	*/
NOSHARE extern int DNEAR popflag;	/* pop-up windows enabled?	*/
NOSHARE extern int DNEAR posflag;	/* display point position	*/
NOSHARE extern int cpending;		/* input character pending?	*/
NOSHARE extern int charpending;		/* character pushed back	*/
NOSHARE extern int DNEAR sscroll;	/* smooth scrolling enabled flag*/
NOSHARE extern int DNEAR hscroll;	/* horizontal scrolling flag	*/
NOSHARE extern int DNEAR hjump;		/* horizontal jump size 	*/
NOSHARE extern int DNEAR ssave;		/* safe save flag		*/
NOSHARE extern struct BUFFER *bstore;	/* buffer to store macro text to*/
NOSHARE extern int DNEAR vtrow;		/* Row location of SW cursor	*/
NOSHARE extern int DNEAR vtcol;		/* Column location of SW cursor */
NOSHARE extern int DNEAR ttrow;		/* Row location of HW cursor	*/
NOSHARE extern int DNEAR ttcol;		/* Column location of HW cursor */
NOSHARE extern int DNEAR lbound;	/* leftmost column of current line
					   being displayed		*/
NOSHARE extern int DNEAR taboff;	/* tab offset for display	*/
NOSHARE extern int DNEAR tabsize;	/* current hard tab size	*/
NOSHARE extern int DNEAR stabsize;	/* current soft tab size (0: use hard tabs)  */
NOSHARE extern int DNEAR reptc;		/* current universal repeat char*/
NOSHARE extern int DNEAR abortc;	/* current abort command char	*/
NOSHARE extern int DNEAR sterm;		/* search terminating character */
NOSHARE extern int DNEAR searchtype;	/* current search style		*/
NOSHARE extern int DNEAR yankflag;	/* current yank style		*/

NOSHARE extern int DNEAR prefix;	/* currently pending prefix bits */
NOSHARE extern int DNEAR prenum;	/*     "       "     numeric arg */
NOSHARE extern int DNEAR predef;	/*     "       "     default flag */

NOSHARE extern int DNEAR quotec;	/* quote char during mlreply() */
NOSHARE extern CONST char *cname[];	/* names of colors		*/
  
NOSHARE extern int kill_index;		/* current index into kill ring */
NOSHARE extern KILL *kbufp[];		/* current kill buffer chunk pointer */
NOSHARE extern KILL *kbufh[];		/* kill buffer header pointer	*/
NOSHARE	extern int kskip[];		/* # of bytes to skip in 1st kill chunk */
NOSHARE extern int kused[];		/* # of bytes used in kill buffer*/
NOSHARE extern WINDOW *swindow; 	/* saved window pointer 	*/
NOSHARE extern int cryptflag;		/* currently encrypting?	*/
NOSHARE extern int oldcrypt;		/* using old(broken) encryption? */
NOSHARE extern short *kbdptr;		/* current position in keyboard buf */
NOSHARE extern short *kbdend;		/* ptr to end of the keyboard */
NOSHARE extern int kbdmode;		/* current keyboard macro mode	*/
NOSHARE extern int kbdrep;		/* number of repetitions	*/
NOSHARE extern int restflag;		/* restricted use?		*/
NOSHARE extern int lastkey;		/* last keystoke		*/
NOSHARE extern int seed;		/* random number seed		*/
NOSHARE extern long envram;		/* # of bytes current in use by malloc */
NOSHARE extern int DNEAR macbug;	/* macro debugging flag		*/
NOSHARE extern int DNEAR mouseflag;	/* use the mouse?		*/
NOSHARE extern int DNEAR diagflag;	/* diagonal mouse movements?	*/
CONST extern char errorm[];		/* error literal		*/
CONST extern char truem[];		/* true literal 		*/
CONST extern char falsem[];		/* false litereal		*/
NOSHARE extern int DNEAR cmdstatus;	/* last command status		*/
NOSHARE extern char palstr[];		/* palette string		*/
NOSHARE extern char lastmesg[];		/* last message posted		*/
NOSHARE extern char *lastptr;		/* ptr to lastmesg[]		*/
NOSHARE extern int DNEAR saveflag;	/* Flags, saved with the $target var */
NOSHARE extern char *fline; 		/* dynamic return line */
NOSHARE extern int DNEAR flen; 		/* current length of fline */
NOSHARE extern int DNEAR rval; 		/* return value of a subprocess */
NOSHARE extern int DNEAR eexitflag;	/* EMACS exit flag */
NOSHARE extern int DNEAR eexitval;	/* and the exit return value */
NOSHARE extern int xpos;		/* current column mouse is positioned to */
NOSHARE extern int ypos;		/* current screen row	     "	 */
NOSHARE extern int nclicks;		/* cleared on any non-mouse event*/
NOSHARE extern int disphigh;		/* display high bit chars escaped*/

/* uninitialized global external declarations */

NOSHARE extern int DNEAR currow;	/* Cursor row			*/
NOSHARE extern int DNEAR curcol;	/* Cursor column		*/
NOSHARE extern int DNEAR thisflag;	/* Flags, this command		*/
NOSHARE extern int DNEAR lastflag;	/* Flags, last command		*/
NOSHARE extern int DNEAR curgoal;	/* Goal for C-P, C-N		*/
NOSHARE extern WINDOW *curwp; 		/* Current window		*/
NOSHARE extern BUFFER *curbp; 		/* Current buffer		*/
NOSHARE extern WINDOW *wheadp;		/* Head of list of windows	*/
NOSHARE extern BUFFER *bheadp;		/* Head of list of buffers	*/
NOSHARE extern SCREEN *first_screen;	/* Head and current screen in list */
NOSHARE extern BUFFER *blistp;		/* Buffer for C-X C-B		*/
NOSHARE extern BUFFER *slistp;		/* Buffer for A-B		*/

NOSHARE extern char sres[NBUFN];	/* current screen resolution	*/

NOSHARE	extern char lowcase[HICHAR];	/* lower casing map		*/
NOSHARE	extern char upcase[HICHAR];	/* upper casing map		*/

NOSHARE extern char pat[];		/* Search pattern		*/
NOSHARE extern char tap[];		/* Reversed pattern array.	*/
NOSHARE extern char rpat[]; 		/* replacement pattern		*/

/*	Various "Hook" execution variables	*/

NOSHARE extern KEYTAB readhook;		/* executed on all file reads */
NOSHARE extern KEYTAB wraphook;		/* executed when wrapping text */
NOSHARE extern KEYTAB cmdhook;		/* executed before looking for a cmd */
NOSHARE extern KEYTAB writehook;	/* executed on all file writes */
NOSHARE extern KEYTAB exbhook;		/* executed when exiting a buffer */
NOSHARE extern KEYTAB bufhook;		/* executed when entering a buffer */

NOSHARE extern char *patmatch;
NOSHARE extern int matchlen;
NOSHARE extern int matchoff;
NOSHARE extern LINE *matchline;

#if	MAGIC
NOSHARE extern short int magical;
NOSHARE extern short int rmagical;
NOSHARE extern MC mcpat[NPAT];		/* the magic pattern		*/
NOSHARE extern MC tapcm[NPAT];		/* the reversed magic pattern	*/
NOSHARE extern RMC rmcpat[NPAT];	/* the replacement magic array	*/
NOSHARE extern char *grpmatch[MAXGROUPS];	/* holds groups found in search */ 
#endif

CONST extern char *dname[];		/* directive name table 	*/

#if	DEBUGM
/*	vars needed for macro debugging output	*/
NOSHARE extern char outline[];	/* global string to hold debug line text */
#endif

/*
 * System message notification (at the moment, VMS only).
 */
#if	VMS
NOSHARE extern char brdcstbuf[1024];		/* Broadcast messages */
NOSHARE extern int pending_msg;		/* Flag - have we notified user yet.*/
#endif

#endif

/* terminal table defined only in TERM.C */

#ifndef termdef
NOSHARE extern TERM	term;		/* Terminal information.	*/
#endif
