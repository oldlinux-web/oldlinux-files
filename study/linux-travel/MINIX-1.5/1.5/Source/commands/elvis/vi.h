/* vi.h */

/* Author:
 *	Steve Kirkendall
 *	16820 SW Tallac Way
 *	Beaverton, OR 97006
 *	kirkenda@jove.cs.pdx.edu, or ...uunet!tektronix!psueea!jove!kirkenda
 */


/* This is the header file for my version of vi. */

#define VERSION "ELVIS version 1.2, by Steve Kirkendall"
#define COPYING	"This version of ELVIS is freely redistributable."

#include <errno.h>
extern int errno;

#include "curses.h"

#if M_SYSV || bsd
# include <fcntl.h>
#else
# define O_RDONLY	0
# define O_WRONLY	1
# define O_RDWR		2
#endif

#ifndef O_BINARY
# define O_BINARY	0
#endif

/* Oops!  These make all external symbols distinct to 7 characters */
#define v_shiftl	v_lshift
#define v_shiftr	v_rshift
#define redrawpre	preredraw
#define redrawpost	postredraw
#define movefsentence	m_fsentence
#define movebsentence	m_bsentence

/*------------------------------------------------------------------------*/
/* File names                                                             */

#ifndef TMPNAME
# define TMPNAME	"/usr/tmp/elvt%04x%04x"	/* temp file */
#endif
#ifndef CUTNAME
# define CUTNAME	"/usr/tmp/elvc%04x%04x"	/* cut buffer's temp file */
#endif
#define EXRC		".exrc"		/* init file in current directory */
#define HMEXRC		EXRC		/* init file in home directory */
#define KEYWORDPRG	"/usr/bin/ref"	/* keyword "help" program */
#define SCRATCHFILE 	"_scratch_"	/* name of a scratch file */
#define	TAGS		"tags"		/* tags file */
#define SLASH		'/'		/* used to separate parts of pathname */

/*------------------------------------------------------------------------*/
/* Miscellaneous constants.						  */

#define INFINITY	2000000001L	/* a very large integer */
#define MAXMAPS		20		/* number of :map keys */
#define LONGKEY		10		/* longest possible raw :map key */
#define MAXRCLEN	1000		/* longest possible .exrc file */

/*------------------------------------------------------------------------*/
/* These describe how temporary files are divided into blocks             */

#define BLKSIZE	1024		/* size of blocks */
#define MAXBLKS	(BLKSIZE / sizeof(unsigned short))
typedef union
{
	char		c[BLKSIZE];	/* for text blocks */
	unsigned short	n[MAXBLKS];	/* for the header block */
}
	BLK;

/*------------------------------------------------------------------------*/
/* These are used manipulate BLK buffers.                                 */

extern BLK	hdr;		/* buffer for the header block */
extern BLK	blkbuf[2];	/* buffers for text blocks */
extern BLK	*blkget();	/* given index into hdr.c[], reads block */
extern BLK	*blkadd();	/* inserts a new block into hdr.c[] */

/*------------------------------------------------------------------------*/
/* These are used to keep track of various flags                          */
extern struct _viflags
{
	short	file;		/* file flags */
}
	viflags;

/* file flags */
#define NEWFILE		0x0001	/* the file was just created */
#define READONLY	0x0002	/* the file is read-only */
#define HADNUL		0x0004	/* the file contained NUL characters */
#define MODIFIED	0x0008	/* the file has been modified */
#define NOFILE		0x0010	/* no name is known for the current text */
#define ADDEDNL		0x0020	/* newlines were added to the file */

/* macros used to set/clear/test flags */
#define setflag(x,y)	viflags.x |= y
#define clrflag(x,y)	viflags.x &= ~y
#define tstflag(x,y)	(viflags.x & y)
#define initflags()	viflags.file = 0;

/* The options */
extern char	o_autoindent[1];
extern char	o_autowrite[1];
#ifndef SET_NOCHARATTR
extern char	o_charattr[1];
#endif
extern char	o_columns[3];
extern char	o_exrefresh[1];
extern char	o_ignorecase[1];
extern char	o_keytime[3];
extern char	o_keywordprg[80];
extern char	o_lines[3];
extern char	o_magic[1];
extern char	o_paragraphs[30];
extern char	o_readonly[1];
extern char	o_report[3];
extern char	o_scroll[3];
extern char	o_sections[30];
extern char	o_shell[60];
extern char	o_shiftwidth[3];
extern char	o_sidescroll[3];
extern char	o_sync[1];
extern char	o_tabstop[3];
extern char	o_term[30];
extern char	o_vbell[1];
extern char	o_wrapmargin[3];
extern char	o_wrapscan[1];

/*------------------------------------------------------------------------*/
/* These help support the single-line multi-change "undo" -- shift-U      */

extern char	U_text[BLKSIZE];
extern long	U_line;

/*------------------------------------------------------------------------*/
/* These are used to refer to places in the text 			  */

typedef long	MARK;
#define markline(x)	(long)((x) / BLKSIZE)
#define markidx(x)	(int)((x) & (BLKSIZE - 1))
#define MARK_UNSET	((MARK)0)
#define MARK_FIRST	((MARK)BLKSIZE)
#define MARK_LAST	((MARK)(nlines * BLKSIZE))
#define MARK_AT_LINE(x)	((MARK)((x) * BLKSIZE))

#define NMARKS	28
extern MARK	mark[NMARKS];	/* marks 'a through 'z, plus mark '' */
extern MARK	cursor;		/* mark where line is */

/*------------------------------------------------------------------------*/
/* These are used to keep track of the current & previous files.	  */

extern long	origtime;	/* modification date&time of the current file */
extern char	origname[256];	/* name of the current file */
extern char	prevorig[256];	/* name of the preceding file */
extern long	prevline;	/* line number from preceding file */

/*------------------------------------------------------------------------*/
/* misc housekeeping variables & functions				  */

extern int	tmpfd;		/* fd used to access the tmp file */
extern long	lnum[MAXBLKS];	/* last line# of each block */
extern long	nlines;		/* number of lines in the file */
extern int	forceit;	/* force a command (e.g. ":q!") */
extern char	args[BLKSIZE];	/* file names given on the command line */
extern int	argno;		/* the current element of args[] */
extern int	nargs;		/* number of filenames in args */
extern long	changes;	/* counts changes, to prohibit short-cuts */
extern int	mustredraw;	/* boolean: force total redraw of screen? */
extern long	redrawafter;	/* line# of first line to redraw */
extern long	redrawpre;	/* line# of last line changed, before change */
extern long	redrawpost;	/* line# of last line changed, after change */
extern BLK	tmpblk;		/* a block used to accumulate changes */
extern long	topline;	/* file line number of top line */
extern int	leftcol;	/* column number of left col */
#define		botline	 (topline + LINES - 2)
#define		rightcol (leftcol + COLS - 1)
extern int	physcol;	/* physical column number that cursor is on */
extern int	physrow;	/* physical row number that cursor is on */
extern int	exwrote;	/* used to detect verbose ex commands */
extern int	doingdot;	/* boolean: are we doing the "." command? */
extern long	rptlines;	/* number of lines affected by a command */
extern char	*rptlabel;	/* description of how lines were affected */
extern char	*fetchline();	/* read a given line from tmp file */
extern char	*parseptrn();	/* isolate a regexp in a line */
extern MARK	paste();	/* paste from cut buffer to a given point */
extern char	*wildcard();	/* expand wildcards in filenames */
extern MARK	input();	/* inserts characters from keyboard */
extern char	*linespec();	/* finds the end of a /regexp/ string */
#define		ctrl(ch) ((ch)&037)
#ifndef NO_RECYCLE
extern long	allocate();	/* allocate a free block of the tmp file */
#endif

/*------------------------------------------------------------------------*/
/* macros that are used as control structures                             */

#define BeforeAfter(before, after) for((before),bavar=1;bavar;(after),bavar=0)
#define ChangeText	BeforeAfter(beforedo(FALSE),afterdo())

extern int	bavar;		/* used only in BeforeAfter macros */

/*------------------------------------------------------------------------*/
/* These are the movement commands.  Each accepts a mark for the starting */
/* location & number and returns a mark for the destination.		  */

extern MARK	moveup();			/* k */
extern MARK	movedown();		/* j */
extern MARK	moveright();		/* h */
extern MARK	moveleft();		/* l */
extern MARK	movetoline();		/* G */
extern MARK	movetocol();		/* | */
extern MARK	movefront();		/* ^ */
extern MARK	moverear();		/* $ */
extern MARK	movefword();		/* w */
extern MARK	movebword();		/* b */
extern MARK	moveeword();		/* e */
extern MARK	movefWord();		/* W */
extern MARK	movebWord();		/* B */
extern MARK	moveeWord();		/* E */
extern MARK	movefparagraph();	/* } */
extern MARK	movebparagraph();	/* { */
extern MARK	movefsection();		/* ]] */
extern MARK	movebsection();		/* [[ */
extern MARK	movematch();		/* % */
extern MARK	movefsentence();	/* ) */
extern MARK	movebsentence();	/* ( */
extern MARK	movetomark();		/* 'm */
extern MARK	movensrch();		/* n */
extern MARK	moveNsrch();		/* N */
extern MARK	movefsrch();		/* /regexp */
extern MARK	movebsrch();		/* ?regexp */
extern MARK	move_ch();		/* ; , */
extern MARK	movefch();		/* f */
extern MARK	movetch();		/* t */
extern MARK	moveFch();		/* F */
extern MARK	moveTch();		/* T */
extern MARK	moverow();		/* H L M */
extern MARK	movez();		/* z */
extern MARK	movescroll();		/* ^B ^F ^E ^Y ^U ^D */

/* Some stuff that is used by movement functions... */

extern MARK	adjmove();		/* a helper fn, used by move fns */

/* This macro is used to set the default value of cnt */
#define DEFAULT(val)	if (cnt < 1) cnt = (val)

/* These are used to minimize calls to fetchline() */
extern int	plen;	/* length of the line */
extern long	pline;	/* line number that len refers to */
extern long	pchgs;	/* "changes" level that len refers to */
extern char	*ptext;	/* text of previous line, if valid */
#ifdef CRUNCH
extern void	pfetch();
#else
# define pfetch(l)	if((l) != pline || changes != pchgs){\
				pline = (l);\
				ptext=fetchline(pline);\
				plen=strlen(ptext);\
				pchgs = changes;}
#endif

/* This is used to build a MARK that corresponds to a specific point in the
 * line that was most recently pfetch'ed.
 */
#define buildmark(text)	(MARK)(BLKSIZE * pline + (int)((text) - ptext))


/*------------------------------------------------------------------------*/
/* These are used to handle EX commands.				  */

typedef enum
{
	CMD_NULL,	/* NOT A VALID COMMAND */
     /*	CMD_ABBR,	/* "define an abbreviation" */
	CMD_ARGS,	/* "show me the args" */
	CMD_APPEND,	/* "insert lines after this line" */
	CMD_BANG,	/* "run a single shell command" */
	CMD_COPY,	/* "copy the selected text to a given place" */
	CMD_CD,		/* "change directories" */
	CMD_CHANGE,	/* "change some lines" */
	CMD_DELETE,	/* "delete the selected text" */
	CMD_EDIT,	/* "switch to a different file" */
	CMD_FILE,	/* "show the file's status" */
	CMD_GLOBAL,	/* "globally search & do a command" */
	CMD_INSERT,	/* "insert lines before the current line" */
	CMD_JOIN,	/* "join the selected line & the one after" */
	CMD_LIST,	/* "print lines, making control chars visible" */
	CMD_MAP,	/* "adjust the keyboard map" */
	CMD_MARK,	/* "mark this line" */
	CMD_MKEXRC,	/* "make a .exrc file" */
	CMD_MOVE,	/* "move the selected text to a given place" */
	CMD_NEXT,	/* "switch to next file in args" */
     /*	CMD_PRESERVE,	/* "act as though vi crashed" */
	CMD_PREVIOUS,	/* "switch to the previous file in args" */
	CMD_PRINT,	/* "print the selected text" */
	CMD_PUT,	/* "insert any cut lines before this line" */
	CMD_QUIT,	/* "quit without writing the file" */
	CMD_READ,	/* "append the given file after this line */
     /* CMD_RECOVER,	/* "recover file after vi crashes" - USE -r FLAG */
	CMD_REWIND,	/* "rewind to first file" */
	CMD_SET,	/* "set a variable's value" */
	CMD_SHELL,	/* "run some lines through a command" */
	CMD_SHIFTL,	/* "shift lines left" */
	CMD_SHIFTR,	/* "shift lines right" */
	CMD_SOURCE,	/* "interpret a file's contents as ex commands" */
     /* CMD_STOP,	/* same as CMD_SUSPEND */
     /* CMD_SUSPEND,	/* "suspend the vi session" */
	CMD_SUBSTITUTE,	/* "substitute text in this line" */
     /* CMD_TR,		/* "transliterate chars in the selected lines" */
	CMD_TAG,	/* "go to a particular tag" */
     /*	CMD_UNABBR,	/* "remove an abbreviation definition" */
	CMD_UNDO,	/* "undo the previous command" */
	CMD_UNMAP,	/* "remove a key sequence map */
	CMD_VERSION,	/* "describe which version this is" */
	CMD_VGLOBAL,	/* "apply a cmd to lines NOT containing an RE" */
	CMD_VISUAL,	/* "go into visual mode" */
	CMD_WRITE,	/* "write the selected(?) text to a given file" */
	CMD_XIT,	/* "write this file out (if modified) & quit" */
	CMD_YANK	/* "copy the selected text into the cut buffer" */
#ifdef DEBUG
	,
	CMD_DEBUG,	/* access to internal data structures */
	CMD_VALIDATE,	/* check for internal consistency */
#endif
}
	CMD;

extern		ex();
extern		vi();
extern		doexcmd();

extern void	cmd_append();
extern void	cmd_args();
extern void	cmd_cd();
extern void	cmd_delete();
extern void	cmd_edit();
extern void	cmd_file();
extern void	cmd_global();
extern void	cmd_join();
extern void	cmd_mark();
extern void	cmd_list();
extern void	cmd_map();
extern void	cmd_mkexrc();
extern void	cmd_next();
extern void	cmd_print();
extern void	cmd_put();
extern void	cmd_quit();
extern void	cmd_read();
extern void	cmd_rewind();
extern void	cmd_set();
extern void	cmd_shell();
extern void	cmd_shift();
extern void	cmd_source();
extern void	cmd_substitute();
extern void	cmd_tag();
extern void	cmd_undo();
extern void	cmd_version();
extern void	cmd_visual();
extern void	cmd_write();
extern void	cmd_xit();
extern void	cmd_move();
#ifdef DEBUG
extern void	cmd_debug();
extern void	cmd_validate();
#endif

/*----------------------------------------------------------------------*/
/* These are used to handle VI commands 				*/

extern MARK	v_1ex();	/* : */
extern MARK	v_mark();	/* m */
extern MARK	v_quit();	/* Q */
extern MARK	v_redraw();	/* ^L ^R */
extern MARK	v_ulcase();	/* ~ */
extern MARK	v_undo();	/* u */
extern MARK	v_xchar();	/* x */
extern MARK	v_Xchar();	/* X */
extern MARK	v_replace();	/* r */
extern MARK	v_overtype();	/* R */
extern MARK	v_selcut();	/* " */
extern MARK	v_paste();	/* p P */
extern MARK	v_yank();	/* y Y */
extern MARK	v_delete();	/* d D */
extern MARK	v_join();	/* J */
extern MARK	v_insert();	/* a A i I o O */
extern MARK	v_change();	/* c C */
extern MARK	v_subst();	/* s */
extern MARK	v_shiftl();	/* < */
extern MARK	v_shiftr();	/* > */
extern MARK	v_filter();	/* ! */
extern MARK	v_status();	/* ^G */
extern MARK	v_keyword();	/* ^K */
extern MARK	v_tag();	/* ^] */
extern MARK	v_xit();	/* ZZ */
extern MARK	v_undoline();	/* U */

/*----------------------------------------------------------------------*
/* These describe what mode we're in */

extern enum _mode
{
	MODE_EX,	/* executing ex commands */
	MODE_VI,	/* executing vi commands */
	MODE_COLON,	/* executing an ex command from vi mode */
	MODE_QUIT
}
	mode;

#define WHEN_VICMD	1	/* getkey: we're reading a VI command */
#define WHEN_VIINP	2	/* getkey: we're in VI's INPUT mode */
#define WHEN_VIREP	4	/* getkey: we're in VI's REPLACE mode */
#define WHEN_EX		8	/* getkey: we're in EX mode */
