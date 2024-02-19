char *versio = "C-Kermit, 4E(070) 29 Jan 88";

/*  C K C M A I  --  C-Kermit Main program  */

/*
 4E, add long packet support, plus changes for Apollo and Data General
 support from SAS Institute, and for Macintosh from Planning Research Corp,
 plus several important bug fixes.
*/
/*
 Author: Frank da Cruz,
 Columbia University Center for Computing Activities (CUCCA), 1984-88.
 Copyright (C) 1984, 1988, Trustees of Columbia University in the City of New 
 York. Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided this
 copyright notice is retained.
*/
/*
 The Kermit file transfer protocol was developed at Columbia University.
 It is named after Kermit the Frog, star of the television series THE
 MUPPET SHOW; the name is used by permission of Henson Associates, Inc.
 "Kermit" is also Celtic for "free".
*/
/*
 Thanks to Herm Fischer of Encino CA for extensive contributions to version 4,
 and to the following people for their contributions over the years:

   Larry Afrin, Clemson U
   Stan Barber, Rice U
   Charles Brooks, EDN
   Bill Catchings, formerly of CUCCA
   Bob Cattani, Columbia U CS Dept
   Howard Chu, U of Michigan
   Bill Coalson, McDonnell Douglas
   Alan Crosswell, CUCCA
   Jeff Damens, formerly of CUCCA
   Joe R. Doupnik, Utah State U
   Glenn Everhart, RCA Labs
   Carl Fongheiser, CWRU
   Yekta Gursel, MIT
   Jim Guyton, Rand Corp
   Stan Hanks, Rice U.
   Ken Harrenstein, SRI
   Ron Heiby, Motorola Micromputer Division
   Steve Hemminger, Tektronix
   Randy Huntziger, NLM
   Phil Julian, SAS Institute
   Jim Knutson, U of Texas at Austin
   John Kunze, UC Berkeley
   David Lawyer, UC Irvine
   S.O. Lidie, Lehigh U
   Chris Maio, Columbia U CS Dept
   Leslie Mikesall, American Farm Bureau
   Martin Minow, DEC
   Tony Movshon, NYU
   Dan Murphy, ???
   Jim Noble, Planning Research Corporation
   Paul Placeway, Ohio State U
   Ken Poulton, HP Labs
   Frank Prindle, NADC
   Scott Ribe, ???
   Jack Rouse, SAS Institute
   Stew Rubenstein, Harvard
   Dan Schullman, DEC
   Gordon Scott, Micro Focus, Newbury UK
   David Sizeland, U of London Medical School
   Bradley Smith, UCLA
   Markku Toijala, Helsinki U of Technology
   Dave Tweten, AMES-NAS
   Walter Underwood, Ford Aerospace
   Pieter Van Der Linden, Centre Mondial (Paris)
   Wayne Van Pelt, GE/CRD
   Mark Vasoll & Gregg Wonderly, Oklahoma State University
   Stephen Walton, Ametek Computer
   Lauren Weinstein
   Joachim Wiesel, U of Karlsruhe
   Dave Woolley, CAP Communication Systems, London

 and many others.
*/

#include "ckcsym.h"			/* Macintosh needs this */
#include "ckcker.h"
#include "ckcdeb.h"

/* Text message definitions.. each should be 256 chars long, or less. */
#ifdef MAC
char *hlptxt = "\r\
MacKermit Server Commands:\r\
\r\
    BYE\r\
    FINISH\r\
    GET filespec\r\
    REMOTE CWD directory\r\
    REMOTE HELP\r\
    SEND filespec\r\
\r\0";
#else
#ifdef AMIGA
char *hlptxt = "C-Kermit Server Commands:\n\
\n\
GET filespec, SEND filespec, FINISH, BYE, REMOTE HELP\n\
\n\0";
#else
#ifdef MINIX
char *hlptxt = "C-Kermit Server REMOTE Commands:\n\
BYE CWD DELETE DIRECTORY FINISH GET HELP HOST SEND SPACE TYPE WHO\n\
\n\0";
#else /* MINIX */
char *hlptxt = "C-Kermit Server REMOTE Commands:\n\
\n\
GET files  REMOTE CWD [dir]    REMOTE DIRECTORY [files]\n\
SEND files REMOTE SPACE [dir]  REMOTE HOST command\n\
FINISH     REMOTE DELETE files REMOTE WHO [user]\n\
BYE        REMOTE HELP         REMOTE TYPE files\n\
\n\0";
#endif
#endif
#endif

#ifdef MINIX
char *srvtxt = "\r\n\
C-Kermit server starting.\n\
\r\n\0";
#else
char *srvtxt = "\r\n\
C-Kermit server starting.  Return to your local machine by typing\r\n\
its escape sequence for closing the connection, and issue further\r\n\
commands from there.  To shut down the C-Kermit server, issue the\r\n\
FINISH or BYE command and then reconnect.\n\
\r\n\0";
#endif

/* Declarations for Send-Init Parameters */

int spsiz = DSPSIZ,                     /* Biggest packet size we can send */
    spsizf = 0,                         /* Flag to override what you ask for */
    rpsiz = DRPSIZ,                     /* Biggest we want to receive */
    urpsiz = DRPSIZ,			/* User-requested rpsiz */
    maxrps = MAXRP,			/* Maximum incoming long packet size */
    maxsps = MAXSP,			/* Maximum outbound l.p. size */
    maxtry = MAXTRY,			/* Maximum retries per packet */
    wsize = 1,				/* Window size */
    timint = DMYTIM,                    /* Timeout interval I use */
    rtimo = URTIME,                     /* Timeout I want you to use */
    timef = 0,                          /* Flag to override what you ask */
    npad = MYPADN,                      /* How much padding to send */
    mypadn = MYPADN,                    /* How much padding to ask for */
    bctr = 1,                           /* Block check type requested */
    bctu = 1,                           /* Block check type used */
    ebq =  MYEBQ,                       /* 8th bit prefix */
    ebqflg = 0,                         /* 8th-bit quoting flag */
    rqf = -1,				/* Flag used in 8bq negotiation */
    rq = 0,				/* Received 8bq bid */
    sq = 'Y',				/* Sent 8bq bid */
    rpt = 0,                            /* Repeat count */
    rptq = MYRPTQ,                      /* Repeat prefix */
    rptflg = 0;                         /* Repeat processing flag */

int capas = 10,				/* Position of Capabilities */
    atcapb = 8,				/* Attribute capability */
    atcapr = 0,				/*  requested */
    atcapu = 0,				/*  used */
    swcapb = 4,				/* Sliding Window capability */
    swcapr = 0,				/*  requested */
    swcapu = 0,				/*  used */
    lpcapb = 2,				/* Long Packet capability */
    lpcapr = 1,				/*  requested */
    lpcapu = 0;				/*  used */

CHAR padch = MYPADC,                    /* Padding character to send */
    mypadc = MYPADC,                    /* Padding character to ask for */
    seol = MYEOL,                       /* End-Of-Line character to send */
    eol = MYEOL,                        /* End-Of-Line character to look for */
    ctlq = CTLQ,                        /* Control prefix in incoming data */
    myctlq = CTLQ;                      /* Outbound control character prefix */


/* Packet-related variables */

int pktnum = 0,                         /* Current packet number */
    prvpkt = -1,                        /* Previous packet number */
    sndtyp,                             /* Type of packet just sent */
    rsn,				/* Received packet sequence number */
    rln,				/* Received packet length */
    size,                               /* Current size of output pkt data */
    osize,                              /* Previous output packet data size */
    maxsize,                            /* Max size for building data field */
    spktl = 0;				/* Length packet being sent */

CHAR sndpkt[MAXSP+100],                 /* Entire packet being sent */
    recpkt[MAXRP+200],                  /* Packet most recently received */
    *rdatap,				/* Pointer to received packet data */
    data[MAXSP+4],			/* Packet data buffer */
    srvcmd[MAXRP+4],                    /* Where to decode server command */
    *srvptr,                            /* Pointer to above */
    mystch = SOH,                       /* Outbound packet-start character */
    stchr = SOH;                        /* Incoming packet-start character */

/* File-related variables */

#ifdef datageneral
CHAR filnam[256];                       /* Name of current file. */
#else
CHAR filnam[50];                        /* Name of current file. */
#endif

int nfils;                              /* Number of files in file group */
long fsize;                             /* Size of current file */

/* Communication line variables */

CHAR ttname[50];                        /* Name of communication line. */

int parity,                             /* Parity specified, 0,'e','o',etc */
    flow,                               /* Flow control, 1 = xon/xoff */
    speed = -1,                         /* Line speed */
    turn = 0,                           /* Line turnaround handshake flag */
    turnch = XON,                       /* Line turnaround character */
    duplex = 0,                         /* Duplex, full by default */
    escape = 034,                       /* Escape character for connect */
    delay = DDELAY,                     /* Initial delay before sending */
    mdmtyp = 0;                         /* Modem type (initially none)  */

    int tlevel = -1;			/* Take-file command level */

/* Statistics variables */

long filcnt,                    /* Number of files in transaction */
    flci,                       /* Characters from line, current file */
    flco,                       /* Chars to line, current file  */
    tlci,                       /* Chars from line in transaction */
    tlco,                       /* Chars to line in transaction */
    ffc,                        /* Chars to/from current file */
    tfc;                        /* Chars to/from files in transaction */

int tsecs;                      /* Seconds for transaction */

/* Flags */

int deblog = 0,                         /* Flag for debug logging */
    pktlog = 0,                         /* Flag for packet logging */
    seslog = 0,                         /* Session logging */
    tralog = 0,                         /* Transaction logging */
    displa = 0,                         /* File transfer display on/off */
    stdouf = 0,                         /* Flag for output to stdout */
    xflg   = 0,                         /* Flag for X instead of F packet */
    hcflg  = 0,                         /* Doing Host command */
    fncnv  = 1,                         /* Flag for file name conversion */
    binary = 0,                         /* Flag for binary file */
    savmod = 0,                         /* Saved file mode */
    cmask  = 0177,			/* Connect byte mask */
    fmask  = 0377,			/* File byte mask */
    warn   = 0,                         /* Flag for file warning */
    quiet  = 0,                         /* Be quiet during file transfer */
    local  = 0,                         /* Flag for external tty vs stdout */
    server = 0,                         /* Flag for being a server */
    cnflg  = 0,                         /* Connect after transaction */
    cxseen = 0,                         /* Flag for cancelling a file */
    czseen = 0,                         /* Flag for cancelling file group */
    keep = 0;                           /* Keep incomplete files */

/* Variables passed from command parser to protocol module */

char parser();                          /* The parser itself */
char sstate  = 0;                       /* Starting state for automaton */
char *cmarg  = "";                      /* Pointer to command data */
char *cmarg2 = "";                      /* Pointer to 2nd command data */
char **cmlist;                          /* Pointer to file list in argv */

/* Miscellaneous */

char **xargv;                           /* Global copies of argv */
int  xargc;                             /* and argc  */

extern char *dftty;                     /* Default tty name from ckx???.c */
extern int dfloc;                       /* Default location: remote/local */
extern int dfprty;                      /* Default parity */
extern int dfflow;                      /* Default flow control */

/*  M A I N  --  C-Kermit main program  */

#ifdef apollo
/* On the Apollo, intercept main to insert a cleanup handler */
ckcmai(argc,argv) int argc; char **argv; {
#else
main(argc,argv) int argc; char **argv; {
#endif

    char *strcpy();

/* Do some initialization */

    xargc = argc;                       /* Make global copies of argc */
    xargv = argv;                       /* ...and argv. */
    sstate = 0;                         /* No default start state. */
    strcpy(ttname,dftty);               /* Set up default tty name. */
    local = dfloc;                      /* And whether it's local or remote. */
    parity = dfprty;                    /* Set initial parity, */
    flow = dfflow;                      /* and flow control. */
    if (sysinit() < 0) doexit(BAD_EXIT); /* And system-dependent things. */

/*** attempt to take ini file before doing command line ***/

    cmdini();				/* Sets tlevel */
    while (tlevel > -1) {		/* Execute init file. */
	sstate = parser();		/* Loop getting commands. */
        if (sstate) proto();            /* Enter protocol if requested. */
    }

/* Look for a UNIX-style command line... */

    if (argc > 1) {                     /* Command line arguments? */
        sstate = cmdlin();              /* Yes, parse. */
        if (sstate) {
            proto();                    /* Take any requested action, then */
            if (!quiet) conoll("");     /* put cursor back at left margin, */
            if (cnflg) conect();        /* connect if requested, */
            doexit(GOOD_EXIT);          /* and then exit with status 0. */
        }
    }

/* If no action requested on command line, enter interactive parser */

    herald();				/* Display program herald. */
    while(1) {				/* Loop getting commands. */
	sstate = parser();
        if (sstate) proto();            /* Enter protocol if requested. */
    }
}
