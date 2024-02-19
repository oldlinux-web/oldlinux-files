/*
 * TIPC.C - TI Professional and compatible terminal driver for MicroEmacs.
 *      This version by Ronald Lepine, Moderator - TI Conference
 *      Byte Information Exchange.
 *
 *      BIX ID. ronlepine
 *      Programers Room - Ronald Lepine	(317) 742-5533  2400 baud
 *      AT&T: (302) 836-8398
 *      US Mail:
 *          Ron Lepine
 *          434 Shai Circle
 *          Bear, DE   19701-3604
 *
 *      Adapted from code by Danial Lawrence for the IBM-PC.
 * 
 * The routines in this file provide support for the TI-PC and other
 * compatible terminals. It truely writes directly to the screen RAM to do
 * screen output. It compiles into nothing if not a TI-PC driver.
 *
 * This driver for the TIPC is not the same one that Daniel Lawrence
 * distributes.  He uses TI's DSRs for most functions as you can see by
 * looking at the driver in ue311c.arc.  Dan's reverse video 
 * also does not work.  But do not blame him as I'm sure he does not
 * have a TIPC to test Emacs on and he *is* supportting the TIPC.
 * [Well, as you can see, it is the one I distribute now! - DAN]
 * This driver uses direct writes to screen memory except for the message
 * line like the IBM driver.  In fact I keep TI and IBM versions around by 
 * having my make file replace estruct.h with one that has a IBMPC defined
 * only when I compile IBMPC.C and then switching back to estruct.h with
 * TIPC defined. Its last step is of course linking in the correct object
 * module for each exe.
 *
 * You can define any buffer to be reverse video by setting the
 * foreground to "BLACK" and the background to any other color
 * such as "cyan" to get Black on Cyan text.  There is no way to get
 * colors such as 'White on blue' on the TIPC unless it has a
 * least a 1-plane graphics card in it.  If it does have a graphics
 * card installed the code to change overall background
 * color is tied to the $palette variable with the same values as
 * TI BASIC and the Tech Reference manual section 3.5.2 as well as
 * being the #define for the color here without the enable attribute.
 * The text mode forground/background code still works even when you
 * use a $palette backgound giving you a wide range of what can be
 * done even though some combinations don't really work too well.
 *
 * The only reason that there are any '#if color' directives is because of
 * the need to maintain structure alignments.  This is because TI's mono
 * is really 8 shades instead of 8 colors and all attributes  remain the
 * same.  You can have color and mono attached at the same time and
 * display on both without any program support or hardware changes.  So
 * you should define color. It *will* make a difference in other areas of
 * the MicroEmacs code. You also do not have to do anything special for
 * the palette code because writing to a nonexistant graphics plane on a
 * TI causes *no* ill effects (at least according to the tech manual).
 *
 * Note that if you recieve MicroEmacs for the TIPC compiled by me
 * it is compiled/linked so that wildcard file names on the command
 * line are automagicly expanded in case you want to work with several
 * similarly named files.
 *
 * Changes to the other files to support the the TIPC MEMAPed
 * version compiled under Zortech C/C++ 1.02 to 2.10 follow.
 * BTW - The version I used is 2.18 but the earlier versions should
 * still be similar since I have compiled previous versions under whatever
 * was current at the time and even the previous ZTC version sometimes.
 *
 * [I simply added Ron's #ifdef changes to the master sources - Dan]
 *
 *     I once thought there were three ways to correct the problems with
 * DSR writes to the *25th* line, I now know some don't work.
 *     One was to check the start register before all screen writes. After
 * some investigation and testing I found that the  6545A Start of screen
 * registers are write only.  So there is not way to correct cursor
 * postioning and line position whenever DOS has changed the register. 
 * Changing the start of screen pointer to offset 0 before screen writes
 * also didn't solve the problem completely.  As soon as you touched a key
 * after the error you ended up with the screen properly postioned, but
 * you had the error message on the screen until that line was updated so
 * you still ended up using a ^L to correct the screen.
 *     Two is to capture critical errors (mlwrite and errors are the sources
 * of DSR writes to the 25th line) and handle them myself (asm is
 * only clean way) and now looks like the only way it is possible but must
 * be used in conjuction with a modified method three too.
 *     Three was change writes to the message line to direct screen writes
 * and initialize the screen so that a critical error will write to the
 * text portion of the screen where any repaints would clear it.  The same
 * problems cropped up that did in number one where you needed to do a ^L
 * to totally clean up the screen if the error occured while on the message
 * line anyway so....  
 *     Therefore a combination of two and three is most likely the best
 * way if it can be done since TI does not recommend mixing DSR and direct
 * writes to screen (if  I could read the 6545 registers 0Bh and 0Ch I
 * could manage it though).  This is because the DSR writes *can* change
 * the screen start register. The problem with a 2+3 solution is mainly
 * that the extra code to handle single character writes, directly
 * controlling the cursor though the CRTC cursor registers, and the
 * intercept/write/read error code handling bloats the code a bit and
 * coding in anything but assembly language (which  then needs to be
 * interfaced to each C compiler that might be in use) is the only reasonable
 * way.  I just came came to the conclusion that it wasn't worth it since
 * a ^L will clear things up anyway after the error is corrected.
 * 
 * 12/29/89 -
 *   Changed lnptr, sline, and scptr to char/char*'s because TI writes
 *   only a character at a screen position and IBM a char + attribute.  TI's
 *   attributes are kept up by the hardware except when changed. So we only
 *   write a char at a time, not an int like the IBM code did.  This also
 *   corrected a minor bug with the message line eeol's which is why I
 *   looked at the driver code again.
 * 
 * 12/30/89 -
 *   added the capability of setting the background on machines with 
 *   graphics by setting the palette string with a number from 0 to 7.
 *   you do this with ^XA $palette  (note small letters) and a value.
 *   Values >= 8 will work since I do modulo division on the value
 *   but results are the same as the color values in TI BASIC.  The
 *   code, as always, works on all TI's but will not produce a change
 *   on non-graphics equiped TI's.  This code also does not need a color
 *   monitor to work since a TI doesn't know what kind of monitor is
 *   attached and may even have color and mono attached at once.
 * 
 *   With the code set up so you can still do reverse video on any TI
 *   by defining BLACK and yellow (or any other background color) you
 *   have a large variety of posible screen setups.  So experiment.
 * 
 * 12/31/89 to 1/7/90 - experimented with various methods of eliminating
 *   problems with mixed DSR/Direct_Memory writes.  Decided not to really
 *   attempt a solution at this time.  Write Ron Lepine if you have
 *   any feelings/suggestions on this problem, they are welcome.
 *
 * 1-9-90
 *   3.10.c(Beta) New IBMPC reverse video code added to TIPC driver.
 *
 *   Addional changes to 3.10.c(beta) to get it to compile with Zortech C/C++
 *     **** NOTE THESE CHANGES ARE NO LONGER NEEDED FOR ZTC 2.10 *****
 *      isearch line 172/5, 191 - change lines to 
 * 
 *            if (kfunc == &forwsearch || kfunc == &forwhunt ||
 *                kfunc == &backsearch || kfunc == &backhunt)
 *            {
 *                dir = (kfunc == &backsearch || kfunc == &backhunt)?
 *                REVERSE: FORWARD;
 *
 * 5-26-90
 *   Updated first part of this file to reflect 3.11beta line numbers
 *   while compiling 3.11 beta under Zortech C/C++ 2.1.
 *
 * 6-04-91
 *   Added last eight needed values to ctrans[] so standard emacs.rc file
 *   now works.
 */

#define termdef 1                       /* don't define "term" external */

#include        <stdio.h>
#include        "estruct.h"
#include        "eproto.h"
#include        "edef.h"
#include        "elang.h"

#if     TIPC

#define NROW    25                      /* Screen height.               */
#define NCOL    80                      /* Screen width                 */
#define MARGIN  8                       /* size of minimim margin and   */
#define SCRSIZ  64                      /* scroll size for extended lines */
#define NPAUSE  200                     /* # times thru update to pause */
#define BEL     0x07                    /* BEL character.               */
#define ESC     0x1B                    /* ESC character.               */
#define SPACE   32                      /* space character              */
#define SCADD       0xDE000000L         /* address of screen RAM        */
#define ATTRADD     0xDE001800L         /* Address for attribute latch  */
#define BLUE_PLANE  0xC0000000L         /* Address Blue graphics plane  */ 
#define RED_PLANE   0xC8000000L         /* Address Red graphics plane   */ 
#define GREEN_PLANE 0xD0000000L         /* Address Green graphics plane */ 
#define CRTC_REG    0xDF810000L         /* 6545 CRT Controller access addr */
#define CHAR_ENABLE     0x08            /* TI attribute to show char    */
#define TI_REVERSE      0x10            /* TI attribute to reverse char */
#define BLACK   0+CHAR_ENABLE           /* TI attribute for Black       */
#define BLUE    1+CHAR_ENABLE           /* TI attribute for Blue        */
#define RED     2+CHAR_ENABLE           /* TI attribute for Red         */
#define MAGENTA 3+CHAR_ENABLE           /* TI attribute for Magenta     */
#define GREEN   4+CHAR_ENABLE           /* TI attribute for Green       */
#define CYAN    5+CHAR_ENABLE           /* TI attribute for Cyan        */
#define YELLOW  6+CHAR_ENABLE           /* TI attribute for Yellow      */
#define WHITE   7+CHAR_ENABLE           /* TI attribute for White       */


PASCAL NEAR ttopen();               /* Forward references.          */
PASCAL NEAR ttgetc();
PASCAL NEAR ttputc();
PASCAL NEAR ttflush();
PASCAL NEAR ttclose();
PASCAL NEAR timove();
PASCAL NEAR tieeol();
PASCAL NEAR tieeop();
PASCAL NEAR tibeep();
PASCAL NEAR tiopen();
PASCAL NEAR tikopen();
PASCAL NEAR tirev();
PASCAL NEAR ticres();
PASCAL NEAR ticlose();
PASCAL NEAR tikclose();
PASCAL NEAR tiputc();
PASCAL NEAR tifcol();
PASCAL NEAR tibcol();
PASCAL NEAR scinit();

int     revflag = FALSE;        /* are we currently in rev video?       */
int     cfcolor = -1;           /* current forground color              */
int     cbcolor = -1;           /* current background color             */
int     ctrans[] =              /* ANSI to TI color translation table   */
        {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE,
         BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE};

char *scptr[NROW];                      /* pointer to screen lines-8 bit on ti */
char sline[NCOL];                       /* screen line image               */

long scadd       = SCADD;               /* address of screen ram           */
long attradd     = ATTRADD;             /* address of attribute latch      */
long blue_plane  = BLUE_PLANE;          /* address of blue graphics plane  */
long red_plane   = RED_PLANE;           /* address of red graphics plane   */
long green_plane = GREEN_PLANE;         /* address of green graphics plane */
long crtc_reg    = CRTC_REG;           	/* 6545 CRT controller address     */

/*
 * Standard terminal interface dispatch table. Most of the fields point into
 * "termio" code.
 */
TERM    term    = {
        NROW-1,
        NROW-1,
        NCOL,
        NCOL,
        MARGIN,
        SCRSIZ,
        NPAUSE,
        tiopen,
        ticlose,
        tikopen,
        tikclose,
        ttgetc,
        tiputc,
        ttflush,
        timove,
        tieeol,
        tieeop,
        tieeop,
        tibeep,
        tirev,
        ticres
#if COLOR
        , tifcol,
        tibcol
#endif
};

extern union REGS rg;


PASCAL NEAR tifcol(color)           /* set the current output color */

int color;                          /* color to set */
{
    cfcolor = ctrans[color];
}


PASCAL NEAR tibcol(color)           /* set the current background color */
                                    /* color to set */
int color;
{
    cbcolor = ctrans[color];        
}


PASCAL NEAR timove(row, col)

int row;
int col;
{
    rg.h.ah = 2;            /* set cursor position function code */
    rg.h.dh = col;
    rg.h.dl = row;
    int86(0x49, &rg, &rg);
}


PASCAL NEAR tieeol()        /* erase to the end of the line */

{
    int i;                  /* loop variable                    */
    char *lnptr;            /* pointer to the destination line  */
    int ccol;               /* current column cursor lives      */
    int crow;               /* current row  cursor lives        */

    /* find the current cursor position */
    rg.h.ah = 3;            /* read cursor position function code */
    int86(0x49, &rg, &rg);
    ccol = rg.h.dh;         /* record current column */
    crow = rg.h.dl;         /* and row */

    lnptr = &sline[0];      /* set things up */
    for (i=0; i < term.t_ncol; i++)
        *lnptr++ = SPACE;

    memset(attradd, cfcolor, 1);                /* write current attrs to latch */
    movmem(&sline[0], scptr[crow]+ccol, term.t_ncol-ccol);

}


PASCAL NEAR tiputc(ch)      /* put a character at the current position */
                            /* in the current colors */
int ch;
{
    memset(attradd, cfcolor, 1);    /* write current attrs to latch */
    rg.h.ah = 0x0E;                 /* write char to screen with DSR */
    rg.h.al = ch;                   
    int86(0x49, &rg, &rg); 
}


PASCAL NEAR tieeop()        /* Actually a clear screen */

{
    rg.h.ah = 0x13;         /* Clear Text Screen, Home Cursor and */
    int86(0x49, &rg, &rg);  /* Set screen start register to 0     */
}


PASCAL NEAR tirev(state)    /* change reverse video state */

int state;          /* TRUE = reverse, FALSE = normal */
{
    revflag = state;
}


PASCAL NEAR ticres()    /* Change screen resolution. Should we add the  */
                        /* the 720x350 mode besides the normal 720x300  */
                        /* mode for those who can handle it?  It really */
                        /* gains us nothing since we don't gain lines   */
                        /* and the code works when started in that res. */
                        /* Let me (Ron Lepine) know what you think      */
{
    return(TRUE);
}


PASCAL NEAR tibeep()

{
    bdos(6, BEL, 0);
}


PASCAL NEAR tiopen()

{
    strcpy(sres, "NORMAL");
    revexist = TRUE;
    revflag = FALSE;
    scinit();
    ttopen();
}


PASCAL NEAR tikopen()

{
    /* Does nothing */
}


PASCAL NEAR ticlose()

{
    memset(attradd, WHITE, 1);  /* write normal attrbute to latch      */
                    /* Makes sure we return with a normal color        */
                    /* and not reverse video or such.  Attribute       */
                    /* Latch will otherwise hold last color and        */
                    /* attributes it wrote to the screen, which in     */
                    /* some cases is reverse video.                    */
}


PASCAL NEAR tikclose()

{
    memset(attradd, WHITE, 1);  /* write normal attrbute to latch         */
                    /* Dan doesn't close the the terminal when shelling   */
                    /* This ensures the shell starts with a normal color  */
                    /* the same way ticlose does without changing         */
                    /* *any* other source file.  The only real reason we  */
                    /* place it here is to change as few files as posible */
}


PASCAL NEAR scinit()    /* initialize the screen head pointers to */
                        /* logical screen */
{
    union {
        long laddr;     /* long form of address */
        char *paddr;    /* pointer form of address */
    } addr;

    char i;

    tieeop();		/* set logical = physical start of screen   */ 
        /* initialize The screen pointer array */
    for (i = 0; i < NROW; i++) {
        addr.laddr = scadd + (long)(NCOL * i);
        scptr[i] = addr.paddr;
    }
}


PASCAL NEAR scwrite(row, outstr, forg, bacg)    /* write a line out */

int row;        /* row of screen to place outstr on */
char *outstr;   /* string to write out (must be term.t_ncol long) */
int forg;       /* forground color of string to write */
int bacg;       /* background color */

{
    char    *lnptr;         /* Pointer to the destination line */
    char    i;

    /* Write the attribute byte to latch and setup the screen pointer      */
    /* If forg == 0 then you can change the back ground to color.  TI      */
    /* won't let you use two colors for forground and background unless    */
    /* you have graphics and set the graphics color to your background     */
    /* This is enabled in this driver though the $palette variable         */

    if ((forg != 0) & (!revflag)) { 
        forg = ctrans[forg];
        memset(attradd, forg, 1);
    }else if ((forg != 0) & (revflag)) {
        forg = ctrans[forg] + TI_REVERSE;
        memset(attradd, forg, 1);
    }else{
        bacg = ctrans[bacg] + TI_REVERSE;
        memset(attradd, bacg, 1); 
    }
    lnptr = &sline[0];
    for (i=0; i<term.t_ncol; i++)
        *lnptr++ = outstr[i];
    /* and send the string out */
    movmem(&sline[0], scptr[row], term.t_ncol);
}


PASCAL NEAR spal(palette)      /* change palette string */

char *palette;
{
    /* Turns on a graphics plane if it is installed         */
    /* Set value of $palette works the same as in TI BASIC  */
    /* and the Technical Reference manual section 3.5.2     */
    /* ie. Set $palette to 7 for a white background or to   */
    /* 1 for a blue background.                             */
    /* Causes no problems when graphics are not installed   */
    /* so there is no reason to check for the number of     */
    /* graphics planes installed.                           */

    switch ((atoi(palette) % 8)) {
        case 0:
            memset(blue_plane,   0,  0x7fff);
            memset(red_plane,    0,  0x7fff);
            memset(green_plane,  0,  0x7fff);
            break;
        case 1:
            memset(blue_plane,   0xff,  0x7fff);
            memset(red_plane,    0,     0x7fff);
            memset(green_plane,  0,     0x7fff);
            break;
        case 2:
            memset(blue_plane,   0,    0x7fff);
            memset(red_plane,    0xff, 0x7fff);
            memset(green_plane,  0,    0x7fff);
            break;
        case 3:
            memset(blue_plane,   0xff, 0x7fff);
            memset(red_plane,    0xff, 0x7fff);
            memset(green_plane,  0,    0x7fff);
            break;
        case 4:
            memset(blue_plane,   0,    0x7fff);
            memset(red_plane,    0,    0x7fff);
            memset(green_plane,  0xff, 0x7fff);
            break;
        case 5:
            memset(blue_plane,   0xff, 0x7fff);
            memset(red_plane,    0,    0x7fff);
            memset(green_plane,  0xff, 0x7fff);
            break;
        case 6:
            memset(blue_plane,   0,    0x7fff);
            memset(red_plane,    0xff, 0x7fff);
            memset(green_plane,  0xff, 0x7fff);
            break;
        case 7:
            memset(blue_plane,   0xff, 0x7fff);
            memset(red_plane,    0xff, 0x7fff);
            memset(green_plane,  0xff, 0x7fff);
            break;
    }
}


#if	FLABEL
PASCAL NEAR fnclabel(f, n)	/* label a function key */

int f,n;	/* default flag, numeric argument [unused] */

{
	/* on machines with no function keys...don't bother */
	/* TI/IBM function keys are handled in other code   */ 
	return(TRUE);
}
#endif


#else
tihello()
{
}
#endif

