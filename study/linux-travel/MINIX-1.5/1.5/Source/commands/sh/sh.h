/* -------- sh.h -------- */
/*
 * shell
 */

#define NULL    0
#define	LINELIM	1000
#define	NPUSH	8	/* limit to input nesting */

#define	NOFILE	20	/* Number of open files */
#define	NUFILE	10	/* Number of user-accessible files */
#define	FDBASE	10	/* First file usable by Shell */

/*
 * values returned by wait
 */
#define	WAITSIG(s) ((s)&0177)
#define	WAITVAL(s) (((s)>>8)&0377)
#define	WAITCORE(s) (((s)&0200)!=0)

/*
 * library and system defintions
 */
typedef char * xint;	/* base type of jmp_buf, for broken compilers */

/*
 * shell components
 */
/* #include "area.h" */
/* #include "word.h" */
/* #include "io.h" */
/* #include "var.h" */

#define	QUOTE	0200

#define	NOBLOCK	((struct op *)NULL)
#define	NOWORD	((char *)NULL)
#define	NOWORDS	((char **)NULL)
#define	NOPIPE	((int *)NULL)

/*
 * Description of a command or an operation on commands.
 * Might eventually use a union.
 */
struct op {
	int	type;	/* operation type, see below */
	char	**words;	/* arguments to a command */
	struct	ioword	**ioact;	/* IO actions (eg, < > >>) */
	struct op *left;
	struct op *right;
	char	*str;	/* identifier for case and for */
};

#define	TCOM	1	/* command */
#define	TPAREN	2	/* (c-list) */
#define	TPIPE	3	/* a | b */
#define	TLIST	4	/* a [&;] b */
#define	TOR	5	/* || */
#define	TAND	6	/* && */
#define	TFOR	7
#define	TDO	8
#define	TCASE	9
#define	TIF	10
#define	TWHILE	11
#define	TUNTIL	12
#define	TELIF	13
#define	TPAT	14	/* pattern in case */
#define	TBRACE	15	/* {c-list} */
#define	TASYNC	16	/* c & */

/*
 * actions determining the environment of a process
 */
#define	BIT(i)	(1<<(i))
#define	FEXEC	BIT(0)	/* execute without forking */

/*
 * flags to control evaluation of words
 */
#define	DOSUB	1	/* interpret $, `, and quotes */
#define	DOBLANK	2	/* perform blank interpretation */
#define	DOGLOB	4	/* interpret [?* */
#define	DOKEY	8	/* move words with `=' to 2nd arg. list */
#define	DOTRIM	16	/* trim resulting string */

#define	DOALL	(DOSUB|DOBLANK|DOGLOB|DOKEY|DOTRIM)

Extern	char	**dolv;
Extern	int	dolc;
Extern	int	exstat;
Extern  char	gflg;
Extern  int	talking;	/* interactive (talking-type wireless) */
Extern  int	execflg;
Extern  int	multiline;	/* \n changed to ; */
Extern  struct	op	*outtree;	/* result from parser */

Extern	xint	*failpt;
Extern	xint	*errpt;

struct	brkcon {
	jmp_buf	brkpt;
	struct	brkcon	*nextlev;
} ;
Extern	struct brkcon	*brklist;
Extern	int	isbreak;

/*
 * redirection
 */
struct ioword {
	short	io_unit;	/* unit affected */
	short	io_flag;	/* action (below) */
	char	*io_name;	/* file name */
};
#define	IOREAD	1	/* < */
#define	IOHERE	2	/* << (here file) */
#define	IOWRITE	4	/* > */
#define	IOCAT	8	/* >> */
#define	IOXHERE	16	/* ${}, ` in << */
#define	IODUP	32	/* >&digit */
#define	IOCLOSE	64	/* >&- */

#define	IODEFAULT (-1)	/* token for default IO unit */

Extern	struct	wdblock	*wdlist;
Extern	struct	wdblock	*iolist;

/*
 * parsing & execution environment
 */
extern struct	env {
	char	*linep;
	struct	io	*iobase;
	struct	io	*iop;
	xint	*errpt;
	int	iofd;
	struct	env	*oenv;
} e;

/*
 * flags:
 * -e: quit on error
 * -k: look for name=value everywhere on command line
 * -n: no execution
 * -t: exit after reading and executing one command
 * -v: echo as read
 * -x: trace
 * -u: unset variables net diagnostic
 */
extern	char	*flag;

extern	char	*null;	/* null value for variable */
extern	int	intr;	/* interrupt pending */

Extern	char	*trap[_NSIG+1];
Extern	char	ourtrap[_NSIG+1];
Extern	int	trapset;	/* trap pending */

extern	int	heedint;	/* heed interrupt signals */

Extern	int	yynerrs;	/* yacc */

Extern	char	line[LINELIM];
extern	char	*elinep;

/*
 * other functions
 */
int	(*inbuilt())();	/* find builtin command */
char	*rexecve();
char	*space();
char	*getwd();
char	*strsave();
char	*evalstr();
char	*putn();
char	*itoa();
char	*unquote();
struct	var	*lookup();
struct	wdblock	*add2args();
struct	wdblock	*glob();
char	**makenv();
struct	ioword	*addio();
char	**eval();
int	setstatus();
int	waitfor();

void	onintr();	/* SIGINT handler */

/*
 * error handling
 */
void	leave();	/* abort shell (or fail in subshell) */
void	fail();		/* fail but return to process next command */
void	sig();		/* default signal handler */

/*
 * library functions and system calls
 */
long	lseek();
char	*strncpy();
int	strlen();
extern int errno;

/* -------- var.h -------- */

struct	var {
	char	*value;
	char	*name;
	struct	var	*next;
	char	status;
};
#define	COPYV	1	/* flag to setval, suggesting copy */
#define	RONLY	01	/* variable is read-only */
#define	EXPORT	02	/* variable is to be exported */
#define	GETCELL	04	/* name & value space was got with getcell */

Extern	struct	var	*vlist;		/* dictionary */

Extern	struct	var	*homedir;	/* home directory */
Extern	struct	var	*prompt;	/* main prompt */
Extern	struct	var	*cprompt;	/* continuation prompt */
Extern	struct	var	*path;		/* search path for commands */
Extern	struct	var	*shell;		/* shell to interpret command files */
Extern	struct	var	*ifs;		/* field separators */

struct	var	*lookup(/* char *s */);
void	setval(/* struct var *, char * */);
void	nameval(/* struct var *, char *val, *name */);
void	export(/* struct var * */);
void	ronly(/* struct var * */);
int	isassign(/* char *s */);
int	checkname(/* char *name */);
int	assign(/* char *s, int copyflag */);
void	putvlist(/* int key, int fd */);
int	eqname(/* char *n1, char *n2 */);

/* -------- io.h -------- */
/* io buffer */
struct iobuf {
  unsigned id;				/* buffer id */
  char buf[512];			/* buffer */
  char *bufp;				/* pointer into buffer */
  char *ebufp;				/* pointer to end of buffer */
};

/* possible arguments to an IO function */
struct ioarg {
	char	*aword;
	char	**awordlist;
	int	afile;		/* file descriptor */
	unsigned afid;		/* buffer id */
	long	afpos;		/* file position */
	struct iobuf *afbuf;	/* buffer for this file */
};
Extern struct ioarg ioargstack[NPUSH];
#define AFID_NOBUF	(~0)
#define AFID_ID		0

/* an input generator's state */
struct	io {
	int	(*iofn)();
	struct	ioarg	*argp;
	int	peekc;
	char	prev;		/* previous character read by readc() */
	char	nlcount;	/* for `'s */
	char	xchar;		/* for `'s */
	char	task;		/* reason for pushed IO */
};
Extern	struct	io	iostack[NPUSH];
#define	XOTHER	0	/* none of the below */
#define	XDOLL	1	/* expanding ${} */
#define	XGRAVE	2	/* expanding `'s */
#define	XIO	3	/* file IO */

/* in substitution */
#define	INSUB()	(e.iop->task == XGRAVE || e.iop->task == XDOLL)

/*
 * input generators for IO structure
 */
int	nlchar();
int	strchar();
int	qstrchar();
int	filechar();
int	herechar();
int	linechar();
int	gravechar();
int	qgravechar();
int	dolchar();
int	wdchar();

/*
 * IO functions
 */
int	eofc();
int	getc();
int	readc();
void	unget();
void	ioecho();
void	prs();
void	putc();
void	prn();
void	closef();
void	closeall();

/*
 * IO control
 */
void	pushio(/* struct ioarg arg, int (*gen)() */);
int	remap();
int	openpipe();
void	closepipe();
struct io *setbase(/* struct io * */);

extern	struct	ioarg	temparg;	/* temporary for PUSHIO */
#define	PUSHIO(what,arg,gen) ((temparg.what = (arg)),pushio(&temparg,(gen)))
#define	RUN(what,arg,gen) ((temparg.what = (arg)), run(&temparg,(gen)))

/* -------- word.h -------- */
#ifndef WORD_H
#define	WORD_H	1
struct	wdblock {
	short	w_bsize;
	short	w_nword;
	/* bounds are arbitrary */
	char	*w_words[1];
};

struct	wdblock	*addword();
struct	wdblock	*newword();
char	**getwords();
#endif

/* -------- area.h -------- */

/*
 * storage allocation
 */
char	*getcell(/* unsigned size */);
void	garbage();
void	setarea(/* char *obj, int to */);
int	getarea(/* char *obj */);
void	freearea(/* int area */);
void	freecell(/* char *obj */);

Extern	int	areanum;	/* current allocation area */

#define	NEW(type) (type *)getcell(sizeof(type))
#define	DELETE(obj)	freecell((char *)obj)

