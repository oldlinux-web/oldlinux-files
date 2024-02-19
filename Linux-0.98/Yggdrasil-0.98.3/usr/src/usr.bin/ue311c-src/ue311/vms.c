/*	VMS.C	Operating system specific I/O and spawning functions
		For VAX/VMS operating system
		for MicroEMACS 3.10
		Copyright 1989 by Jeffrey A. Lomicka and Daniel M. Lawrence

	All-new code replaces the previous VMS/SMG implementation which
	prevented using non-SMG terminal drivers (ansi, termcap).  New
	approach to terminal I/O, new (and more) subprocess control
	functions, Kept emacs support, mail/notes interface.

	Some of the above may still be wishlist.

	12-Dec-89	Kevin A. Mitchell
			Start work on RMSIO code.
*/
#include	<stdio.h>
#include	"estruct.h"
#if	VMS
#include	"eproto.h"
#include	"edef.h"
#include	"elang.h"

#include ssdef
#include descrip
#include jpidef
#include iodef
#include ttdef
#include tt2def
#include msgdef
#include string
#include rms
#include ctype
/*
	These are the LIB$SPAWN mode flags.  There's no .h for
	them in VAX C V2.4.
*/
#define CLI$M_NOCONTROL 32
#define CLI$M_NOCLISYM 2
#define CLI$M_NOLOGNAM 4
#define CLI$M_NOKEYPAD 8
#define CLI$M_NOTIFY 16
#define CLI$M_NOWAIT 1
/*
	test macro is used to signal errors from system services
*/
#define test( s) {int st; st = (s); if ((st&1)==0) LIB$SIGNAL( st);}

/*
	This routine returns a pointer to a descriptor of the supplied
	string. The descriptors are static allocated, and up to
	"NUM_DESCRIPTORS" may be used at once.  After that, the old ones
	are re-used. Be careful!

	The primary use of this routine is to allow passing of C strings into
	VMS system facilities and RTL functions.

	There are three forms:

		descrp( s, l)	String descriptor for buffer s, length l
		descptr( s)	String descriptor for asciz buffer s
		DESCPTR( s)	String descriptor for buffer s, using sizeof()
*/
#define NUM_DESCRIPTORS 10
struct  dsc$descriptor_s *descrp(char *s, int l)
{
    static next_d = 0;
    static struct dsc$descriptor_s dsclist[ NUM_DESCRIPTORS];

    if (next_d >= NUM_DESCRIPTORS) next_d = 0;
    dsclist[ next_d].dsc$w_length = l;
    dsclist[ next_d].dsc$b_dtype =  DSC$K_DTYPE_T;
    dsclist[ next_d].dsc$b_class =  DSC$K_CLASS_S;
    dsclist[ next_d].dsc$a_pointer = s;
    return( &dsclist[ next_d++]);
}

/*
 * Make pointer to descriptor from Asciz string.
 */
struct dsc$descriptor_s *descptr(char *s)
{
    return( descrp( s, strlen( s)));
}

#define DESCPTR(s)	descrp( s, sizeof(s)-1)

/*
	These two structures, along with ttdef.h, are good for manipulating
	terminal characteristics.
*/
typedef struct
{	/* Terminal characteristics buffer */
    unsigned char class, type;
    unsigned short width;
    unsigned tt1 : 24;
    unsigned char page;
    unsigned long tt2;
} TTCHAR;

typedef struct
{	/* More terminal characteristics (hidden in the status block) */
    short status;
    char txspeed;
    char rxspeed;
    long trash;
} TTCHARIOSB;

typedef struct
{	/* Status block for ordinary terminal reads */
    unsigned short status, len, term, tlen;
} TTIOSB;

typedef struct
{	/* Status block for mailbox reads */
    unsigned short status, len; long sender_pid;
} MBIOSB;

typedef struct
{	/* Messages from the terminal or TW driver */
    short msgtype;	/* Expecting MSG$_TRMHANGUP */
    short unit;		/* Controller unit number */
    char ctrl_len;	/* Length of controller name (should be 3) */
    char ctrl[15];	/* Controller name (should be TWA) */
    short brdcnt;	/* Broadcast message byte count, if MSG$TRMBRDCST */
    char message[514];	/* First two bytes of broadcast message */
} TTMESSAGE;

static readonly int noterm[] = {0,0};	/* Terminator list of NONE */
static int newbrdcst = FALSE;	/* Flag - is message in Emacs buffer yet.*/

#define	MINREAD	128  /* Smallest read to queue */
#define TYPSIZE 1024 /* Typeahead buffer size, must be several times MINREAD */

static unsigned char tybuf[ TYPSIZE];	/* Typeahead buffer */
static unsigned tyin, tyout, tylen, tymax;/* Inptr, outptr, and length */

static TTIOSB ttiosb;		/* Terminal I/O status block */
static MBIOSB mbiosb;		/* Associated mailbox status block */
static TTMESSAGE mbmsg;		/* Associated mailbox message */
unsigned noshare short vms_iochan;/* VMS I/O channel open on terminal */
static short mbchan;		/* VMS I/O channel open on associated mbx */
static short waiting;		/* Flag FALSE if read already pending */
static short stalled;		/* Flag TRUE if I/O stalled by full buffer */


/*
	If we come from ME$EDIT, the "suspend-emacs" is not allowed, since
	it will tend to wake itself up and re-hiberneate itself, which is
	a problem.
*/
static short called = 0;		/* TRUE if called from ME$EDIT */
/*
	short_time[ 0] is the negative number of 100ns units of time to
	wait.  -10000 is 1ms, therefore 200ms (2 tenths of a second) is
	-2,000,000.  Hopefully this is long enough for the network delay
	that might be involved between seeing the ESC and seeing the
	characters that follow it.

	This will be initialized from the environment variable
	MICROEMACS$SHORTWAIT.
*/
static long short_time[2] = {-4000000, -1};

static unsigned char tobuf[ TYPSIZE];	/* Output buffer */
static unsigned tolen;			/* Ammount used */
NOSHARE TTCHAR orgchar;			/* Original characteristics */
static TTCHARIOSB orgttiosb;		/* Original IOSB characteristics */

static readast()
{	/* Data arrived from the terminal */
    waiting = 1;
    if ((ttiosb.status == SS$_TIMEOUT) || (ttiosb.status & 1))
    {	/* Read completed okay, process the data */
	if (ttiosb.len)
	{	/* Got some data, adjust input queue parameters */
	    tylen += ttiosb.len;
	    tyin += ttiosb.len;
	    test( SYS$WAKE( 0, 0));
	    next_read( 1);
	}
	else
	{	/* The user seems to have stopped typing, issue a read
		that will wake us up when the next character is typed */
	    if (!mbchan) next_read( 0);
	}
    }
    else if (ttiosb.status != SS$_ABORT) LIB$SIGNAL( ttiosb.status);
}

/*
 * flag = TRUE to use timeout of 0.
 */
static next_read(int flag)
{
    if ( waiting || stalled)
    {	/* No current read outstanding, submit one */
	unsigned size;
/*
	Wrap the input pointer if out of room
*/
	waiting = 0;
	if (sizeof( tybuf) - tyin < MINREAD)
	{
	    tymax = tyin;
	    tyin = 0;
	}
	size = tymax - tylen;
	if (tyin + size > sizeof( tybuf)) size = sizeof( tybuf) - tyin;
	if (size >= MINREAD)
	{	/* Only read if there is enough room */
	    test( SYS$QIO(
		0, vms_iochan,
		flag ?
		    IO$_READVBLK | IO$M_NOECHO | IO$M_TRMNOECHO |
		    IO$M_NOFILTR | IO$M_TIMED
		:
		    IO$_READVBLK | IO$M_NOECHO | IO$M_TRMNOECHO |
		    IO$M_NOFILTR,
		&ttiosb, readast, 0, &tybuf[ tyin], flag ? size : 1,
		0, noterm, 0, 0
		));
	    stalled = 0;
	}
	else stalled = 1;
    }
}


/***********************************************************
* FUNCTION - RemoveEscapes - remove ANSI escapes from string
* (for broadcast messages that contain 'formatting')
***********************************************************/
static void RemoveEscapes(char *str)
{
    char *in=str,*out=str;

    while (*in)
    {
        switch (*in)
        {
            case 0x1b:
                in++; /* skip escape */
                if (*in != '[') /* not a CSI */
                {
                    switch (*in)
                    {
                        /* skip special characters */
                        case ';':
                        case '?':
                        case '0':
                            in++;
                    }
                    /* skip any intermediate characters 0x20 to 0x2f */
                    while (*in >= 0x20 && *in <= 0x2f) in++;
                    /* skip any final characters 0x30 to 0x7e */
                    if (*in >= 0x30 && *in <= 0x7e) in++;
		    break;
                }
                /* fall through to CSI */
            case 0x9b:	/* CSI */
                in++; /* skip CSI */
                /* skip any parameters 0x30 to 0x3f */
                while (*in >= 0x30 && *in <= 0x3f) in++;
                /* skip any intermediates 0x20 to 0x2f */
                while (*in >= 0x20 && *in <= 0x2f) in++;
                /* skip one final character 0x40 to 0x7e */
                if (*in >= 0x40 && *in <= 0x7e) in++;
		break;
            default:
            	*out++ = *in++;
        }
    }
    *out = 0;
}

/*
 * The argument msgbuf points to the buffer we want to
 * insert our broadcast message into. Handcraft the EOL
 * on the end.
 */
static brdaddline(BUFFER *msgbuf)
{
        register LINE   *lp;
        register int    i;
        register int    ntext;
        register int    cmark;
        register WINDOW *wp;

        ntext = strlen(brdcstbuf);
        if ((lp=lalloc(ntext)) == NULL)
                return(FALSE);
        for (i=0; i<ntext; ++i)
                lputc(lp, i, brdcstbuf[i]);
        msgbuf->b_linep->l_bp->l_fp = lp;       /* Hook onto the end    */
        lp->l_bp = msgbuf->b_linep->l_bp;
        msgbuf->b_linep->l_bp = lp;
        lp->l_fp = msgbuf->b_linep;
        msgbuf->b_dotp = lp;            /* move it to new line  */

        wp = wheadp;
        while (wp != NULL) {
                if (wp->w_bufp == msgbuf) {
                        wp->w_dotp  = lp;
                        wp->w_doto  = 0;
			for (cmark = 0; cmark < NMARKS; cmark++) {
                        	wp->w_markp[cmark] = NULL;
	                        wp->w_marko[cmark] = 0;
	                }
                        wp->w_flag |= WFMODE|WFHARD;
                }
                wp = wp->w_wndp;
        }
        update(FALSE);
        return(TRUE);
}

static chkbrdcst()
{
    BUFFER *msgbuf;            /* buffer containing messages */

    if (newbrdcst)
    {
        int oldrow=ttrow, oldcol=ttcol;

        SYS$SETAST(0);

        msgbuf = bfind("[-messages-]", TRUE, 0);

        if (msgbuf)
        {
            msgbuf->b_mode |= MDVIEW;
            msgbuf->b_flag |= BFINVS;
            brdaddline(msgbuf);
        }

	newbrdcst = FALSE;
        movecursor(oldrow, oldcol);
        TTmove(oldrow, oldcol);
        SYS$SETAST(1);
    }
}

static mbreadast()
{
    if (mbiosb.status & 1)
    {	/* Read completed okay, check for hangup message */
	if (mbmsg.msgtype == MSG$_TRMHANGUP)
	{
		/* Got a termination message, process it */
	}
	else if (mbmsg.msgtype == MSG$_TRMUNSOLIC)
	{	/* Got unsolicited input, get it */
	    next_read(1);
	}
	else if (mbmsg.msgtype == MSG$_TRMBRDCST)
	{	/* Got broadcast, get it */
		/* Hard-coding the mbmsg.brdcnt to 511 is a temp solution.*/
	    mbmsg.brdcnt = 511;
	    memcpy(brdcstbuf, mbmsg.message, 511);
	    brdcstbuf[511] = 0;

	    RemoveEscapes(brdcstbuf);
	    pending_msg = newbrdcst = TRUE;
	}
	else
	{
	}
	test( SYS$QIO(	    /* Post a new read to the associated mailbox */
	    0, mbchan, IO$_READVBLK, &mbiosb,
	    mbreadast, 0, &mbmsg, sizeof( mbmsg),
	    0, 0, 0, 0
	    ));
    }
    else if (mbiosb.status != SS$_ABORT) LIB$SIGNAL( mbiosb.status);
}

PASCAL NEAR ttopen()
{
    TTCHAR newchar;		/* Adjusted characteristics */
    int status;
    char *waitstr;

    tyin = 0;
    tyout = 0;
    tylen = 0;
    tymax = sizeof( tybuf);
    status = LIB$ASN_WTH_MBX(	/* Create a new PY/TW pair */
	descptr( "SYS$OUTPUT:"),
	&sizeof( mbmsg),
	&sizeof( mbmsg),
	&vms_iochan,
	&mbchan);
    if ((status & 1) == 0)
    {	/* The assign channel failed, was it because of the mailbox? */
	if (status == SS$_DEVACTIVE)
	{	/* We've been called from NOTES, so we can't use the mailbox */
	    test( SYS$ASSIGN( descptr( "SYS$OUTPUT:"), &vms_iochan, 0, 0));
	    mbchan = 0;
	}
	else LIB$SIGNAL( status);
    }
    waiting = 0;		/* Block unsolicited input from issuing read */
    stalled = 0;		/* Don't start stalled */
    if (mbchan) test( SYS$QIO(		/* Post a read to the associated mailbox */
	0, mbchan, IO$_READVBLK, &mbiosb,
	mbreadast, 0, &mbmsg, sizeof( mbmsg),
	0, 0, 0, 0
	));
/*
	Fetch the characteristics and adjust ourself for proper operation.
*/
    test( SYS$QIOW(
	0, vms_iochan, IO$_SENSEMODE, &orgttiosb,
	0, 0, &orgchar, sizeof( orgchar), 0, 0, 0, 0));
    newchar = orgchar;
    newchar.tt2 |= TT2$M_PASTHRU;	/* Gives us back ^U, ^X, ^C, and ^Y. */
    newchar.tt2 |= TT2$M_BRDCSTMBX;	/* Get broadcast messages */
    newchar.tt1 &= ~TT$M_MBXDSABL;	/* Make sure mailbox is on */
    newchar.tt1 |= TT$M_NOBRDCST;	/* Don't trash the screen with these */

/*
	Hostsync allows super-fast typing (workstation paste, PC
	send-file) without loss of data, as long as terminal supports
	XON/XOFF.  VWS and DECWindows terminal emulators require HOSTSYNC
	for PASTE operations to work, even though there is no wire involved.
*/
    newchar.tt1 |= TT$M_HOSTSYNC;
/*
	If you MUST, and if you know you don't need XON/XOFF
	synchronization, you can get ^S and ^Q back as data by defining
	XONDATA in ESTRUCT.H.  This is guarnteed to fail on VT125, VT100's
	over 3600 baud, any serial line terminal with smooth scroll
	enabled, VT200's over 4800 baud.  This is guarnteed to WORK if you
	are using a VT330/340 with SSU enabled, a VWS or DECWindows
	terminal emulator.  Note that if XONDATA is not set, I trust the
	settings the user has, so you just $ SET TERM /[NO]TTSYNC as you wish.
*/
#if XONDATA
    newchar.tt1 &= ~TT$M_TTSYNC;
#endif
/*
	I checked in DISPLAY.C, and verified that the mrow and mcol
	numbers aren't used until after ttopen() is called.  I override
	the terminal-supplied numbers with large numbers of my own, so
	that workstation terminal resizes will work to reasonable limits.

	I don't just use the current sizes as the maximum, becuase it's
	possible to resize the terminal emulator after Emacs is started,
	or even to disconnect and reconnect with a new terminal size, so
	the maximums must not change over multiple calls to ttopen().

	Also note that I do the changes to newchar, so that the actual
	terminal window will be reduced to the maximum values Microemacs
	will support.
*/
    term.t_mrow = 72;		/* 72 is European full page */
    term.t_mcol = 256;		/* 256 is Wider than any termnal I've tried */
    if (newchar.page > term.t_mrow) newchar.page = term.t_mrow;
    term.t_nrow = newchar.page-1;
    if (newchar.width > term.t_mcol) newchar.width = term.t_mcol;
    term.t_ncol = newchar.width;
/*
	Set these new characteristics
*/
    test( SYS$QIOW(
	0, vms_iochan, IO$_SETMODE, 0,
	0, 0, &newchar, sizeof( newchar), 0, 0, 0, 0));
/*
	For some unknown reason, if I don't post this read (which will
	likely return right away) then I don't get started properly.
	It has something to do with priming the unsolicited input system.
*/
    test( SYS$QIO(
	0, vms_iochan,
	IO$_READVBLK | IO$M_NOECHO | IO$M_TRMNOECHO |
	    IO$M_NOFILTR | IO$M_TIMED,
	&ttiosb, readast, 0, tybuf, sizeof( tybuf),
	0, noterm, 0, 0
	));
/*
	Initialize the short_time value for esc-reads.  Larger values may
	be needed on network links.  I'm still experimeinting to get the
	best numbers.
*/
    waitstr = getenv( "MICROEMACS$SHORTWAIT");
    if (waitstr) short_time[ 0] = -asc_int( waitstr);
}

PASCAL NEAR ttclose()
{
    if (tolen > 0)
    {	/* Buffer not empty, flush out last stuff */
	test( SYS$QIOW( 0, vms_iochan, IO$_WRITEVBLK  | IO$M_NOFORMAT,
	    0, 0, 0, tobuf, tolen, 0, 0, 0, 0));
	tolen = 0;
    }
    test( SYS$CANCEL( vms_iochan));	/* Cancel any pending read */
    test( SYS$QIOW(
	0, vms_iochan,IO$_SETMODE, 0,
	0, 0, &orgchar, sizeof( orgchar), 0, 0, 0, 0));
    if (mbchan) test( SYS$DASSGN( mbchan));
    test( SYS$DASSGN( vms_iochan));
}

PASCAL NEAR ttputc(int c)
{
    tobuf[ tolen++] = c;
    if (tolen >= sizeof( tobuf))
    {	/* Buffer is full, send it out */
	test( SYS$QIOW( 0, vms_iochan, IO$_WRITEVBLK | IO$M_NOFORMAT,
	    0, 0, 0, tobuf, tolen, 0, 0, 0, 0));
	tolen = 0;
    }
}

PASCAL NEAR ttflush()
{
/*
	I choose to ignore any flush requests if there is typeahead
	pending.  Speeds DECNet network operation by leaps and bounds
	(literally).
*/
    if (tylen == 0) if (tolen != 0)
    {	/* No typeahead, send it out */
	test( SYS$QIOW( 0, vms_iochan, IO$_WRITEVBLK  | IO$M_NOFORMAT,
	    0, 0, 0, tobuf, tolen, 0, 0, 0, 0));
	tolen = 0;
    }
}
/*
	ttgetc_shortwait is a routine that tries to read another
	character, and if one doesn't come in as fast as we expect
	function keys sequences to arrive, we return -1.  This is called
	after receving ESC to check for next character.  It's okay to wait
	too long, but the length of the delay controls how log the user
	waits after hitting ESC before results are seen.

	Note that we also wake from hibernation if a character arrives, so
	this never causes an undue delay if the user it actually typing.
*/
PASCAL NEAR ttgetc_shortwait()
{
    if (tylen == 0)
    {	/* Nothing immediately available, hibernate for a short time */
	test( SYS$SCHDWK( 0, 0, short_time, 0));
	test( SYS$HIBER());
    }

    return ((tylen == 0)? -1: ttgetc());
}

PASCAL NEAR ttgetc()
{
    register unsigned ret;

    chkbrdcst();
    while (tylen == 0)
    {	/* Nothing to send, wait for something interesting */
	ttflush();
	test(SYS$HIBER());
	chkbrdcst();
    }

    /*
     * Got something, return it.
     */
    SYS$SETAST( 0);
    ret = tybuf[ tyout++];

    if (tyout >= tymax) {
        tyout = 0;
        tymax = sizeof( tybuf);
    }

    tylen--;		/* Should be ADD_INTERLOCKED */

    if (stalled && (tylen < 2 * MINREAD)) {
	test( SYS$DCLAST( next_read, 1, 0));
    }

#if 0
/* This is obsolete - now pop-buffer the buffer [-messages-] to read
   your messages.
 */
    if (newbrdcst)
    {	/* New broadcast message, update broadcast variable */
	VDESC vd;
	findvar( "%brdcst", &vd, 0);
	svar( &vd, brdcstbuf);
	newbrdcst = FALSE;
    }
#endif
    SYS$SETAST( 1);
    return( ret);
}

/*
 * Typahead - any characters pending?
 */
PASCAL NEAR typahead()
{
    return( tylen != 0);
}

/*
 * Shell out to DCL.
 */
PASCAL NEAR spawncli(int f, int n)
{
    register char *cp;

    /*
     * Don't allow this command if restricted
     */
    if (restflag) return(resterr());
    movecursor(term.t_nrow, 0);		    /* Seek to last line.   */
    TTclose();				    /* stty to old settings */
    test( LIB$SPAWN( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
    sgarbf = TRUE;
    TTopen();
    return(TRUE);
}

/*
 * Spawn a command.
 */
PASCAL NEAR spawn(int f, int n)
{
    register int    s;
    char	    line[NLINE];
    /*
     * Don't allow this command if restricted.
     */
    if (restflag) return(resterr());

    if ((s=mlreply("!", line, NLINE)) != TRUE)
	    return(s);
    TTputc('\n');		/* Already have '\r' */
    TTflush();
    TTclose();			/* stty to old modes */
    system(line);
    TTopen();
    TTflush();

    /* if we are interactive, pause here */
    if (clexec == FALSE) {
	    mlputs(TEXT6);
/*		       "\r\n\n[End]" */
	    tgetc();
    }
    sgarbf = TRUE;
    return(TRUE);
}

/*
 * Run an external program with arguments. When it returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X $".
 */
PASCAL NEAR execprg(int f, int n)
{
        register int    s;
        char            line[NLINE];

	/* Don't allow this command if restricted. */
	if (restflag)
		return(resterr());

        if ((s=mlreply("!", line, NLINE)) != TRUE)
                return(s);
        TTputc('\n');			/* Already have '\r' */
        TTflush();
        TTclose();			/* stty to old modes */
        system(line);
        TTopen();
        mlputs(TEXT188);		/* Pause. */
/*             "[End]" */
        TTflush();
        while ((s = tgetc()) != '\r' && s != ' ')
                ;
        sgarbf = TRUE;
        return(TRUE);
}

PASCAL NEAR pipecmd()
{
    register int    s;	    /* return status from CLI */
    register WINDOW *wp;    /* pointer to new window */
    register BUFFER *bp;    /* pointer to buffer to zot */
    char    line[NLINE];    /* command line send to shell */
    static char bname[] = "command.log";

    static char filnam[NFILEN] = "command.log";

    /* don't allow this command if restricted */
    if (restflag) return(resterr());

    /* get the command to pipe in */
    if ((s=mlreply("@", line, NLINE)) != TRUE) return(s);

    /* get rid of the command output buffer if it exists */
    if ((bp=bfind(bname, FALSE, 0)) != FALSE) {
	    /* try to make sure we are off screen */
	    wp = wheadp;
	    while (wp != NULL) {
		    if (wp->w_bufp == bp) {
			    onlywind(FALSE, 1);
			    break;
		    }
		    wp = wp->w_wndp;
	    }
	    if (zotbuf(bp) != TRUE)

		    return(FALSE);
    }

    TTputc('\n');		/* Already have '\r'	 */
    TTflush();
    TTclose();			/* stty to old modes    */

    test( LIB$SPAWN( descptr( line), DESCPTR( "NL:"), descptr( filnam),
	0, 0, 0, 0, 0, 0, 0, 0));
    TTopen();
    TTflush();
    sgarbf = TRUE;
    s = TRUE;

    if (s != TRUE)
	    return(s);

    /* split the current window to make room for the command output */
    if (splitwind(FALSE, 1) == FALSE)
		    return(FALSE);

    /* and read the stuff in */
    if (getfile(filnam, FALSE) == FALSE)
	    return(FALSE);

    /* make this window in VIEW mode, update all mode lines */
    curwp->w_bufp->b_mode |= MDVIEW;
    wp = wheadp;
    while (wp != NULL) {
	    wp->w_flag |= WFMODE;
	    wp = wp->w_wndp;
    }

    /* and get rid of the temporary file */
    delete(filnam);
    return(TRUE);
}

PASCAL NEAR filter(int f, int n)
{
        register int    s;	/* return status from CLI */
	register BUFFER *bp;	/* pointer to buffer to zot */
        char line[NLINE];	/* command line send to shell */
	char tmpnam[NFILEN];	/* place to store real file name */
	static char bname1[] = "fltinp.com";

	static char filnam1[] = "fltinp.com";
	static char filnam2[] = "fltout.log";

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	/* get the filter name and its args */
        if ((s=mlreply("#", line, NLINE)) != TRUE)
                return(s);

	/* setup the proper file names */
	bp = curbp;
	strcpy(tmpnam, bp->b_fname);	/* save the original name */
	strcpy(bp->b_fname, bname1);	/* set it to our new one */

	/* write it out, checking for errors */
	if (writeout(filnam1, "w") != TRUE) {
		mlwrite(TEXT2);
/*                      "[Cannot write filter file]" */
		strcpy(bp->b_fname, tmpnam);
		return(FALSE);
	}

        TTputc('\n');			/* Already have '\r'    */
        TTflush();
        TTclose();			/* stty to old modes    */
	s = 1;

	test( LIB$SPAWN( descptr( line), descptr( filnam1), descptr( filnam2),
	    0, 0, 0, &s, 0, 0, 0, 0));
        TTopen();
        TTflush();
        sgarbf = TRUE;
        s &= 1;

	/* on failure, escape gracefully */
	if (!s || (readin(filnam2,FALSE) == FALSE)) {
		mlwrite(TEXT3);
/*                      "[Execution failed]" */
		strcpy(bp->b_fname, tmpnam);
		delete(filnam1);
		delete(filnam2);
		return(s);
	}

	/* reset file name */
	strcpy(bp->b_fname, tmpnam);	/* restore name */
	bp->b_flag |= BFCHG;		/* flag it as changed */

	/* and get rid of the temporary file */
	delete(filnam1);
	delete(filnam2);
	return(TRUE);
}

/*
	The rename() function is built into the VMS C RTL, and need not be
	duplicated here.
*/

char *PASCAL NEAR timeset()
{
    register char *sp;		/* temp string pointer */
    char buf[16];		/* time data buffer */

    time(buf);
    sp = ctime(buf);
    sp[strlen(sp)-1] = 0;
    return(sp);
}

/*	FILE Directory routines		*/

static char fname[NFILEN];		/* path of file to find */
static char path[NFILEN];		/* path of file to find */
static char rbuf[NFILEN];		/* return file buffer */
static char *ctxtp = NULL;		/* context pointer */
static struct dsc$descriptor pat_desc;	/* descriptor for pattern */
static struct dsc$descriptor rbuf_desc;	/* descriptor for returned file name */

/*
 * Do a wild card directory search (for file name completion)
 * fspec is the pattern to match.
 */
char *PASCAL NEAR getffile(char *fspec)

{
	register int index;		/* index into various strings */
	register int point;		/* index into other strings */
	register int extflag;		/* does the file have an extention? */
	register int verflag;		/* does the file have a version? */
	register char *cp, c;

	/* first parse the file path off the file spec */
	strcpy(path, fspec);
	index = strlen(path) - 1;
	while (index >= 0 && (path[index] != ']' && path[index] != ':'))
		--index;

	path[index+1] = 0;

	/* check for a version number */
	point = strlen(fspec) - 1;
	verflag = FALSE;
	while (point >= 0) {
		if ((c=fspec[point]) == ';') {
			verflag = TRUE;
			break;
		} else if (c == '.' || c == ']' || c == ':')
			break;
		point--;
	}

	/* check for an extension */
	point = strlen(fspec) - 1;
	extflag = FALSE;
	while (point >= 0) {
		if ((c=fspec[point]) == '.') {
			extflag = TRUE;
			break;
		} else if (c == ']' || c == ':')
			break;
		point--;
	}

	/* construct the composite wild card spec */
	strcpy(fname, path);
	strcat(fname, &fspec[index+1]);
	strcat(fname, "*");
	if (!extflag)
		strcat(fname, ".*");
	if (!verflag)
		strcat(fname, ";*");

	pat_desc.dsc$a_pointer = fname;
	pat_desc.dsc$w_length = strlen(fname);
	pat_desc.dsc$b_dtype = DSC$K_DTYPE_T;
	pat_desc.dsc$b_class = DSC$K_CLASS_S;

	for (cp=rbuf; cp!=rbuf+NFILEN; *cp++=' ') ;
	rbuf_desc.dsc$a_pointer = rbuf;
	rbuf_desc.dsc$w_length = NFILEN;
	rbuf_desc.dsc$b_dtype = DSC$K_DTYPE_T;
	rbuf_desc.dsc$b_class = DSC$K_CLASS_S;

	LIB$FIND_FILE_END(&ctxtp);
	ctxtp = NULL;
	if (LIB$FIND_FILE(&pat_desc, &rbuf_desc, &ctxtp) != RMS$_SUC)
		return(NULL);

	/* return the first file name!
	 * isolate the filename and extension
	 * and append that on to the original path
	 */
	for (cp=rbuf; *cp!=' ' && cp!=rbuf+NFILEN-1; cp++) ;
	*cp = 0;
	for (cp--; *cp!=';' && cp!=rbuf-1; cp--) ;
	*cp = 0;
	for (cp--; *cp!=']' && cp!=rbuf-1; cp--) ;
	strcat(path,++cp);
	mklower(path);
	return(path);
}

char *PASCAL NEAR getnfile()
{
	register int index;		/* index into various strings */
	register int point;		/* index into other strings */
	register int extflag;		/* does the file have an extention? */
	char fname[NFILEN];		/* file/path for DOS call */
	register char *cp;

	/* and call for the next file */
	for (cp=rbuf; cp!=rbuf+NFILEN; *cp++=' ') ;
	if (LIB$FIND_FILE(&pat_desc, &rbuf_desc, &ctxtp) != RMS$_SUC)
		return(NULL);

	/* return the next file name!
	 * isolate the original path,
	 * isolate the filename and extension,
	 * and append filename/extension on to the original path
	 */
	for (cp=path+strlen(path)-1; *cp!=']' && cp!=path-1; cp--)
		;

	*++cp = 0;
	for (cp=rbuf; *cp!=' ' && cp!=rbuf+NFILEN-1; cp++)
		;
	*cp = 0;
	for (cp--; *cp!=';' && cp!=rbuf-1; cp--)
		;
	*cp = 0;
	for (cp--; *cp!=']' && cp!=rbuf-1; cp--)
		;
	strcat(path,++cp);
	mklower(path);
	return(path);
}

/*
	The following ME$EDIT entry point is used when MicroEmacs is
	called up from MAIL or NOTES.  Note that it may be called more than
	once, and that "main()" is never called.

	Mail/Notes entry point.  Should be declared UNIVERSAL in ME.OPT.
*/
ME$EDIT(struct dsc$descriptor *infile, struct dsc$descriptor *outfile)
{
    static int first_time = 1;
    char *instr, *outstr;
    register int status;
    register BUFFER *bp;	/* buffer list pointer */
    char bname[NBUFN];		/* buffer name of file to read */

    eexitflag = FALSE;
    called = 1;
    if (first_time)
    {
	first_time = 0;
	vtinit();
	if (eexitflag) goto abortrun;
	edinit(mainbuf); 	/* Buffers, windows */
	varinit();		/* user variables */
	initchars();		/* character set definitions */
	dcline( 0, NULL, TRUE);
    }
    else TTopen();

    outstr = strncpy( calloc( 1, 1+outfile->dsc$w_length),
	outfile->dsc$a_pointer, outfile->dsc$w_length);

    if (infile->dsc$w_length <= 0)
	instr = outstr;
    else instr = strncpy( calloc( 1, 1+infile->dsc$w_length),
	infile->dsc$a_pointer, infile->dsc$w_length);

    makename( bname, outstr);
    unqname(bname);
    bp = bfind(bname, TRUE, 0);
    strcpy(bp->b_fname, instr);
    bp->b_active = FALSE;
    swbuffer( bp);
    strcpy(bp->b_fname, outstr);
    bp->b_flag |= BFCHG;            /* flag it as changed */
    free( instr);
    free( outstr);
    sgarbf = TRUE;
    status = editloop();

abortrun:
    TTclose();
    return( status);
}

PASCAL NEAR bktoshell(int f, int n)
{
/*
	Pause this process and wait for it to be woken up
*/
    unsigned pid;
    unsigned char *env, *dir;
    int argc;
    char *argv[ 16];

    if (called)
    {
	mlwrite( "Called MicroEMACS can't be suspended.");
	return( FALSE);
    }

    env = getenv("MICROEMACS$PARENT");

    if (env == NULL)
    {
	mlwrite( "No parent process.");
	return( FALSE);
    }
    movecursor(term.t_nrow, 0);
    TTclose();

    test( LIB$DELETE_LOGICAL(
	DESCPTR( "MICROEMACS$PARENT"),
	DESCPTR( "LNM$JOB")));
    test( LIB$GETJPI( &JPI$_PID, 0, 0, &pid, 0, 0));
    test( LIB$SET_LOGICAL(
	DESCPTR( "MICROEMACS$PROCESS"),
	descptr( int_asc( pid)),
	DESCPTR( "LNM$JOB")));
    pid = asc_int( env);
    test( SYS$WAKE( &pid, 0));

    for(;;)
    {	/* Hibernate until MICROEMACS$COMMAND is defined */
	test( SYS$HIBER());
	env = getenv( "MICROEMACS$COMMAND");	/* Command line arguments */
	if (env != NULL) break;	/* Winter is over */
    }

    test( LIB$DELETE_LOGICAL(
	DESCPTR( "MICROEMACS$COMMAND"),
	DESCPTR( "LNM$JOB")));

    TTopen();

    argv[ 0] = env;
    argc = 1;
    for( ; ;)
    {	/* Define each argument */
	if (*env == 0x80)
	{ /* Seperator */
	    argv[argc++] = env+1;
	    if (argc > 15) break;
	    *env++ = 0;
	}
	else if (*env++ == 0) break;
    }
/*
	First parameter is default device
*/
    test( LIB$SET_LOGICAL(
	DESCPTR( "SYS$DISK"),
	descptr( argv[ 0]),
	0));
/*
	Second argument is default directory
*/
    test( SYS$SETDDIR( descptr( argv[ 1]), 0, 0));
/*
	Remaining came from command line
*/
    sgarbf = TRUE;
    dcline( argc-2, &argv[ 2], FALSE);
    return( TRUE);
}

#if RMSIO
/*
 * Here are the much faster I/O routines.  Skip the C stuff, use
 * the VMS I/O calls.  Puts the files in standard VMS format, too.
 */
#define successful(s)	((s) & 1)
#define unsuccessful(s) (!((s) & 1))

static struct FAB fab;		/* a file access block */
static struct RAB rab;		/* a record access block */

/*
 * Open a file for reading.
 */
PASCAL NEAR ffropen(char *fn)
{
        unsigned long status;

	/* initialize structures */
	fab=cc$rms_fab;
	rab=cc$rms_rab;

	fab.fab$l_fna = fn;
	fab.fab$b_fns = strlen(fn);
	fab.fab$b_fac = FAB$M_GET;
	fab.fab$b_shr = FAB$M_SHRGET;
	fab.fab$l_fop = FAB$M_SQO;

	rab.rab$l_fab = &fab;
	rab.rab$l_rop = RAB$M_RAH;	/* read-ahead for multibuffering */

	status=SYS$OPEN(&fab);
	if (status==RMS$_FLK)
	{
		/*
		 * File locking problem:
		 * Add the SHRPUT option, allowing shareability
		 * with other writers. This lets us read batch
		 * logs and stuff like that. I don't turn it on
		 * automatically since adding this sharing
		 * eliminates the read-ahead
		 */
		fab.fab$b_shr |= FAB$M_SHRPUT;
		status=SYS$OPEN(&fab);
	}

	if (successful(status))
	{
		if (unsuccessful(SYS$CONNECT(&rab)))
		{
			SYS$CLOSE(&fab);
			return(FIOFNF);
		}
	}
	else return(FIOFNF);

        return(FIOSUC);
}

/*
 * PASCAL NEAR ffwopen(char *fn, char *mode)
 *
 * fn = file name, mode = mode to open file.
 */
PASCAL NEAR ffwopen(char *fn, char *mode)
{
	unsigned long status;

	/* initialize structures */
	fab=cc$rms_fab;
	rab=cc$rms_rab;

	fab.fab$l_fna = fn;
	fab.fab$b_fns = strlen(fn);
	fab.fab$b_fac = FAB$M_PUT;	/* writing this file */
	fab.fab$b_shr = FAB$M_NIL;	/* no other writers */
	fab.fab$l_fop = FAB$M_SQO;	/* sequential ops only */
	fab.fab$b_rat = FAB$M_CR;	/* carriage returns on ends */
	fab.fab$b_rfm = FAB$C_VAR;	/* variable length file */

	rab.rab$l_fab = &fab;
	rab.rab$l_rop = RAB$M_WBH;	/* write behind - multibuffer */

	if (*mode == 'a')
	{
		/* append mode */
		rab.rab$l_rop = RAB$M_EOF;
		status=SYS$OPEN(&fab);
		if (status == RMS$_FNF)
		    status=SYS$CREATE(&fab);
	}
	else	/* *mode == 'w' */
	{
		/* write mode */
   		fab.fab$l_fop |= FAB$M_MXV; /* always make a new version */
		status=SYS$CREATE(&fab);
	}

	if (successful(status))
	{
	     status=SYS$CONNECT(&rab);
	     if (unsuccessful(status)) SYS$CLOSE(&fab);
	}

        if (unsuccessful(status)) {
                mlwrite(TEXT155);
/*                      "Cannot open file for writing" */
                return(FIOERR);
        }
        return(FIOSUC);
}

/*
 * Close a file. Should look at the status in all systems.
 */
PASCAL NEAR ffclose()
{
	unsigned long status;

	/* free this since we do not need it anymore */
	if (fline) {
		free(fline);
		fline = NULL;
	}

	status = SYS$DISCONNECT(&rab);
	if (successful(status)) status = SYS$CLOSE(&fab);
	else SYS$CLOSE(&fab);

        if (unsuccessful(status)) {
                mlwrite(TEXT156);
/*                      "Error closing file" */
                return(FIOERR);
        }
        return(FIOSUC);
}

/*
 * Write a line to the already opened file. The "buf" points to the buffer,
 * and the "nbuf" is its length, less the free newline. Return the status.
 * Check only at the newline.
 */
PASCAL NEAR ffputline(char buf[], int nbuf)
{
        register char *obuf=buf;

#if	CRYPT
	if (cryptflag)
	{
		/* get a reasonable buffer */
		if (fline && flen < nbuf)
		{
			free(fline);
			fline = NULL;
		}

		if (fline == NULL)
		{
			if ((fline=malloc(flen = nbuf+NSTRING))==NULL)
			{
				return(FIOMEM);
			}
		}

		/* copy data */
		memcpy(fline,buf,nbuf);

		/* encrypt it */
		crypt(fline,nbuf);

		/* repoint output buffer */
		obuf=fline;
	}
#endif

        /* set output buffer */
	rab.rab$l_rbf = obuf;
	rab.rab$w_rsz = nbuf;

	if (unsuccessful(SYS$PUT(&rab))) {
                mlwrite(TEXT157);
/*                      "Write I/O error" */
                return(FIOERR);
        }

        return(FIOSUC);
}

/*
 * Read a line from a file, and store the bytes in the supplied buffer. The
 * "nbuf" is the length of the buffer. Complain about long lines and lines
 * at the end of the file that don't have a newline present. Check for I/O
 * errors too. Return status.
 */
PASCAL NEAR ffgetline()
{
	unsigned long status;

	/* if we don't have an fline, allocate one */
	if (fline == NULL)
		if ((fline = malloc(flen = fab.fab$w_mrs?fab.fab$w_mrs+1:32768)) == NULL)
			return(FIOMEM);

	/* read the line in */
	rab.rab$l_ubf=fline;
	rab.rab$w_usz=flen;

	status=SYS$GET(&rab);
	if (status == RMS$_EOF) return(FIOEOF);
        if (unsuccessful(status)) {
                mlwrite(TEXT158);
/*                      "File read error" */
                return(FIOERR);
        }

	/* terminate and decrypt the string */
        fline[rab.rab$w_rsz] = 0;
#if	CRYPT
	if (cryptflag)
		crypt(fline, strlen(fline));
#endif
        return(FIOSUC);
}

#endif

/***********************************************************
* FUNCTION - addspec - utility function for expandargs
***********************************************************/
#define ADDSPEC_INCREMENT 10
static void PASCAL NEAR addspec(struct dsc$descriptor dsc, int *pargc,
				char ***pargv, int *pargcapacity)
{
    char *s;

    /* reallocate the argument array if necessary */
    if (*pargc == *pargcapacity)
    {
        if (*pargv)
            *pargv = realloc(*pargv,sizeof(**pargv) * (*pargcapacity += ADDSPEC_INCREMENT));
        else
            *pargv = malloc(sizeof(**pargv) * (*pargcapacity += ADDSPEC_INCREMENT));
    }


    /* allocate new argument */
    s=strncpy(malloc(dsc.dsc$w_length+1),dsc.dsc$a_pointer,dsc.dsc$w_length);
    s[dsc.dsc$w_length]=0;

    /* put into array */
    (*pargv)[(*pargc)++] = s;
}

/***********************************************************
* FUNCTION - expandargs - massage argc and argv to expand
* wildcards by calling VMS.
***********************************************************/
void PASCAL NEAR expandargs(int *pargc, char ***pargv)
{
    int argc = *pargc;
    char **argv = *pargv;

    int nargc=0;
    char **nargv=NULL;
    int nargcapacity=0;

    struct dsc$descriptor result_filespec={0,DSC$K_DTYPE_T,DSC$K_CLASS_D,NULL};

    /* loop over all arguments */
    while (argc--)
    {
	struct dsc$descriptor filespec={strlen(*argv),DSC$K_DTYPE_T,DSC$K_CLASS_S,*argv};
	unsigned long context=0;

	/* should check for wildcards: %, *, and "..." */
	if (**argv != '-' && (strchr(*argv,'%') || strchr(*argv,'*') ||
			      strstr(*argv,"...")))
	{
	    /* search for all matching filenames */
	    while ((LIB$FIND_FILE(&filespec,&result_filespec,&context)) & 1)
	    {
		    int i;

		    /* LIB$FIND_FILE returns uppercase. Lowercase it */
		    for (i=0;i<result_filespec.dsc$w_length;i++)
		        if (isupper(result_filespec.dsc$a_pointer[i]))
		            result_filespec.dsc$a_pointer[i] = tolower(result_filespec.dsc$a_pointer[i]);

                    addspec(result_filespec,&nargc,&nargv,&nargcapacity);
            }
        }
        else
	    addspec(filespec,&nargc,&nargv,&nargcapacity);

        LIB$FIND_FILE_END(&context);

        argv++;
    }

    STR$FREE1_DX(&result_filespec);

    *pargc=nargc;
    *pargv=nargv;
}

#else
PASCAL NEAR vms_hello()
{
}
#endif
