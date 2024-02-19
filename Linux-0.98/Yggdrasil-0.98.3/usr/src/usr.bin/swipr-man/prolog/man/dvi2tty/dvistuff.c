

#include "dvi2tty.h"
#include <sys/types.h>
#include <sys/stat.h>
#if defined(MSDOS)
#include <math.h>
#endif
#include "commands.h"

#define VERSIONID            2 /* dvi version number that pgm handles      */
#define VERTICALEPSILON 450000L /* crlf when increasing v more than this   */

#define rightmargin     152    /* nr of columns allowed to the right of h=0*/
#define leftmargin      -50    /* give some room for negative h-coordinate */
#define LINELEN         203    /* rightmargin - leftmargin + 1 */

#define MOVE            TRUE   /* if advancing h when outputing a rule     */
#define STAY            FALSE  /* if not advancing h when outputing a rule */

#define absolute        0      /* for seeking in files                     */
#define relative        1

#define FORM             12    /* formfeed                                 */
#define SPACE            32    /* space                                    */
#define DEL             127    /* delete                                   */

#define LASTCHAR        127    /* max dvi character, above are commands    */

#define IMIN(a, b)      (a<b ? a : b)
#define IMAX(a, b)      (a>b ? a : b)

#define get1()          num(1)
#define get2()          num(2)
#define get3()          num(3)
#define get4()          num(4)
#define sget1()         snum(1)
#define sget2()         snum(2)
#define sget3()         snum(3)
#define sget4()         snum(4)

/* char *dvistuff = "(#)@  dvistuff.c  3.1 23/01/89 M.J.E. Mol (c) 1989" */

/*---------------------------------------------------------------------------*/

typedef struct {
    long hh;
    long vv;
    long ww;
    long xx;
    long yy;
    long zz;
} stackitem;

typedef struct lineptr {        /* the lines of text to be output to outfile */
    long            vv;                 /* vertical position of the line     */
    int             charactercount;     /* pos of last char on line          */
    struct lineptr *prev;               /* preceding line                    */
    struct lineptr *next;               /* succeeding line                   */
    char            text[LINELEN+1];    /* leftmargin...rightmargin          */
} linetype;

/*---------------------------------------------------------------------------*/

bool        pageswitchon;       /* true if user-set pages to print           */
bool        sequenceon;         /* false if pagesw-nrs refers to TeX-nrs     */
bool        scascii;            /* if true make Scand. nat. chars right      */
bool        noffd;              /* if true output ^L instead of formfeed     */

int         opcode;             /* dvi-opcodes                               */

long        h, v;               /* coordinates, horizontal and vertical      */
long        w, x, y, z;         /* horizontal and vertical amounts           */

long        pagecounter;        /* sequence page number counter              */
long        backpointer;        /* pointer for offset to previous page       */
long        pagenr;             /* TeX page number                           */
int         stackmax;           /* stacksize required                        */

long        maxpagewidth;       /* width of widest page in file              */
long        charwidth;          /* aprox width of character                  */

linetype *  currentline;        /* pointer to current line on current page   */
linetype *  firstline;          /* pointer to first line on current page     */
linetype *  lastline;           /* pointer to last line on current page      */
int         firstcolumn;        /* 1st column with something to print        */

stackitem * stack;              /* stack for dvi-pushes                      */
int         sptr;               /* stack pointer                             */

/*---------------------------------------------------------------------------*/

#if defined(MSDOS)
void            postamble       (void);
void            preamble        (void);
void            walkpages       (void);
void            initpage        (void);
void            dopage          (void);
void            skippage        (void);
void            printpage       (void);
bool            inlist          (long);
void            rule            (bool, long, long);
void            ruleaux         (long, long, char);
long            horizontalmove  (long);
int             skipnops        (void);
linetype    *   getline         (void);
linetype    *   findline        (void);
unsigned long   num             (int);
long            snum            (int);
void            outchar         (char);
void            putcharacter    (long);
void            setchar         (long);
#else
void            postamble       ();
void            preamble        ();
void            walkpages       ();
void            initpage        ();
void            dopage          ();
void            skippage        ();
void            printpage       ();
bool            inlist          ();
void            rule            ();
void            ruleaux         ();
long            horizontalmove  ();
int             skipnops        ();
linetype    *   getline         ();
linetype    *   findline        ();
unsigned long   num             ();
long            snum            ();
void            outchar         ();
void            putcharacter    ();
void            setchar         ();
#endif


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*
 * The main function for processing the dvi file.
 * Here we assume there are to file pointers: DVIfile and output.
 * Also we have a list of pages pointed to by 'currentpage',
 * which is only used (in 'inlist()') when a page list is given.
 */

void dvimain()
{

    postamble();                            /* seek and process the postamble */
    /* note that walkpages *must* immediately follow preamble */
    preamble();                             /* process preamble               */
    walkpages();                            /* time to do the actual work!    */

} /* dvimain */

/*---------------------------------------------------------------------------*/

void postamble()            /* find and process postamble, use random access */
{
    register long size;
    register int  count;
    struct stat st;

    fstat (fileno(DVIfile), &st);
    size = (long) st.st_size;                   /* get size of file          */
    count = -1;
    do {              /* back file up past signature bytes (223), to id-byte */
        if (size == 0)
            errorexit(nopst);
        size--;
        fseek(DVIfile, size, absolute);
        opcode = (int) get1();
        count++;
    } while (opcode == TRAILER);
    if (count < 4) {                            /* must have 4 trailer bytes */
         foo = count;
         errorexit(fwsgn);
    }
    if (opcode != VERSIONID)
        errorexit(badid);
    fseek(DVIfile, size-4, absolute);       /* back up to back-pointer       */
    fseek(DVIfile, sget4(), absolute);      /* and to start of postamble   */
    if (get1() != POST)
        errorexit(nopst);
    fseek(DVIfile, 20L, relative); /* lastpageoffset, numerator, denominator */
                                   /* magnification, maxpageheight           */
    maxpagewidth = sget4();
    charwidth = maxpagewidth / (ttywidth + espace); 
    stackmax = (int) get2();
    if ((stack = (stackitem *) malloc(stackmax * sizeof(stackitem))) == NULL)
       errorexit(stkrq);

    /* get2() -- totalpages */
    /* fontdefs */

} /* postamble */

/*---------------------------------------------------------------------------*/

void preamble()                 /* process preamble, use random access       */
{

    fseek(DVIfile, 0L, absolute);       /* read the dvifile from the start   */
    if ((opcode = skipnops()) != PRE)
        errorexit(nopre);
    opcode = (int) get1();        /* check id in preamble, ignore rest of it */
    if (opcode != VERSIONID)
        errorexit(badid);
    fseek(DVIfile, 12L, relative);  /* numerator, denominator, magnification */
    fseek(DVIfile, get1(), relative);         /* skip job identification     */

} /* preamble */

/*----------------------------------------------------------------------------*/

void walkpages()                  /* process the pages in the DVI-file */
{
    register bool wantpage;

    pagecounter = 0L;
    while ((opcode = skipnops()) != POST) {
        if (opcode != BOP)              /* should be at start of page now    */
            errorexit(nobop);
        else {
            pagecounter++;
            pagenr = sget4();           /* get TeX page number               */
            fseek(DVIfile, 36L, relative); /* skip page header */
            backpointer = sget4();      /* get previous page offset          */
            if (pageswitchon)
                if (sequenceon)
                    wantpage = inlist(pagecounter);
                else
                    wantpage = inlist(pagenr);
            else
                wantpage = TRUE;

            if (wantpage) {
                initpage();
                dopage();
                printpage();
            }
            else {
                skippage();
            }
        }
    }

} /* walkpages */

/*---------------------------------------------------------------------------*/

void initpage()
{

    h = 0L;  v = 0L;                        /* initialize coordinates   */
    x = 0L;  w = 0L;  y = 0L;  z = 0L;      /* initialize amounts       */
    sptr = 0;                               /* initialize stack         */
    currentline = getline();                /* initialize list of lines */
    currentline->vv = 0L;
    firstline   = currentline;
    lastline    = currentline;
    firstcolumn = rightmargin;
    if (pageswitchon) {
        if ((sequenceon && (pagecounter != firstpage->pag)) ||
            (!sequenceon && (pagenr != firstpage->pag)))
            if (noffd)
                fprintf(output, "^L\n");
            else
                putc(FORM, output);
    }
    else
        if (backpointer != -1)              /* not FORM at first page   */
            if (noffd)
                fprintf(output, "^L\n");
            else
                putc(FORM, output);

} /* initpage */

/*----------------------------------------------------------------------------*/

void dopage()
{

    while ((opcode = (int) get1()) != EOP) {    /* process page until eop */
        if (opcode > POST_POST)
            errorexit(illop);
        else if (opcode <= LASTCHAR)
            outchar((char) opcode);
        else
            switch (opcode) {
                case SET1     : setchar(get1()); break;
                case SET2     : setchar(get2()); break;
                case SET3     : setchar(get3()); break;
                case SET4     : setchar(get4()); break;
                case SET_RULE : rule(MOVE, sget4(), sget4()); break;
                case PUT1     : putcharacter(get1()); break;
                case PUT2     : putcharacter(get2()); break;
                case PUT3     : putcharacter(get3()); break;
                case PUT4     : putcharacter(get4()); break;
                case PUT_RULE : rule(STAY, sget4(), sget4()); break;
                case NOP      : break;  /* no-op */
                case BOP      : errorexit(bdbop); break;
/*              case EOP      : break;  strange place to have EOP */
                case PUSH     : if (sptr >= stackmax)            /* push */
                                     errorexit(stkof);
                                stack[sptr].hh = h;
                                stack[sptr].vv = v;
                                stack[sptr].ww = w;
                                stack[sptr].xx = x;
                                stack[sptr].yy = y;
                                stack[sptr].zz = z;
                                sptr++;
                                break;
                case POP      : if (sptr == 0)                   /* pop */
                                    errorexit(stkuf);
                                sptr--;
                                h = stack[sptr].hh;
                                v = stack[sptr].vv;
                                w = stack[sptr].ww;
                                x = stack[sptr].xx;
                                y = stack[sptr].yy;
                                z = stack[sptr].zz;
                                break;
                case RIGHT1   : (void) horizontalmove(sget1()); break;
                case RIGHT2   : (void) horizontalmove(sget2()); break;
                case RIGHT3   : (void) horizontalmove(sget3()); break;
                case RIGHT4   : (void) horizontalmove(sget4()); break;
                case W0       : h += w; break;
                case W1       : w = horizontalmove(sget1()); break;
                case W2       : w = horizontalmove(sget2()); break;
                case W3       : w = horizontalmove(sget3()); break;
                case W4       : w = horizontalmove(sget4()); break;
                case X0       : h += x; break;
                case X1       : x = horizontalmove(sget1()); break;
                case X2       : x = horizontalmove(sget2()); break;
                case X3       : x = horizontalmove(sget3()); break;
                case X4       : x = horizontalmove(sget4()); break;
                case DOWN1    : v += sget1(); break;
                case DOWN2    : v += sget2(); break;
                case DOWN3    : v += sget3(); break;
                case DOWN4    : v += sget4(); break;
                case Y0       : v += y; break;
                case Y1       : y = sget1(); v += y; break;
                case Y2       : y = sget2(); v += y; break;
                case Y3       : y = sget3(); v += y; break;
                case Y4       : y = sget4(); v += y; break;
                case Z0       : v += z; break;
                case Z1       : z = sget1(); v += z; break;
                case Z2       : z = sget2(); v += z; break;
                case Z3       : z = sget3(); v += z; break;
                case Z4       : z = sget4(); v += z; break;
                case FNT1     :
                case FNT2     :
                case FNT3     :                       /* ignore font changes */
                case FNT4     : fseek(DVIfile, (long) opcode-FNT1+1, relative);
                                break;
                case XXX1     : fseek(DVIfile, get1(), relative); break;
                case XXX2     : fseek(DVIfile, get2(), relative); break;
                case XXX3     : fseek(DVIfile, get3(), relative); break;
                case XXX4     : fseek(DVIfile, get4(), relative); break;
                case FNT_DEF1 :
                case FNT_DEF2 :
                case FNT_DEF3 :                   /* ignore font definitions */
                case FNT_DEF4 : fseek(DVIfile, (long) opcode-FNT_DEF1+1+12,
                                               relative);
                                fseek(DVIfile, get1() + get1(), relative);
                                break;
                case PRE      : errorexit(bdpre); break;
                case POST     : errorexit(bdpst); break;
                case POST_POST: errorexit(bdpp); break;
            }
    }

} /* dopage */

/*----------------------------------------------------------------------------*/

void skippage()                /* skip past one page */
{
    register int opcode;

    while ((opcode = (int) get1()) != EOP) {
        if (opcode > POST_POST)
            errorexit(illop);
        else
            switch (opcode) {
                case SET1     :
                case PUT1     :
                case RIGHT1   :
                case W1       :
                case X1       :
                case DOWN1    :
                case Y1       :
                case Z1       :
                case FNT1     : fseek(DVIfile, 1L, relative); break;
                case SET2     :
                case PUT2     :
                case RIGHT2   :
                case W2       :
                case X2       :
                case DOWN2    :
                case Y2       :
                case Z2       :
                case FNT2     : fseek(DVIfile, 2L, relative); break;
                case SET3     :
                case PUT3     :
                case RIGHT3   :
                case W3       :
                case X3       :
                case DOWN3    :
                case Y3       :
                case Z3       :
                case FNT3     : fseek(DVIfile, 3L, relative); break;
                case SET4     :
                case PUT4     :
                case RIGHT4   :
                case W4       :
                case X4       :
                case DOWN4    :
                case Y4       :
                case Z4       :
                case FNT4     : fseek(DVIfile, 4L, relative); break;
                case SET_RULE :
                case PUT_RULE : fseek(DVIfile, 8L, relative); break;
                case BOP      : errorexit(bdbop); break;
                case XXX1     : fseek(DVIfile, get1(), relative); break;
                case XXX2     : fseek(DVIfile, get2(), relative); break;
                case XXX3     : fseek(DVIfile, get3(), relative); break;
                case XXX4     : fseek(DVIfile, get4(), relative); break;
                case FNT_DEF1 :
                case FNT_DEF2 :
                case FNT_DEF3 :
                case FNT_DEF4 : fseek(DVIfile, (long) opcode-FNT_DEF1+1+12,
                                      relative);
                                fseek(DVIfile, get1()+get1(), relative);
                                break;
                case PRE      : errorexit(bdpre); break;
                case POST     : errorexit(bdpst); break;
                case POST_POST: errorexit(bdpp); break;
        }
    }

} /* skippage */

/*---------------------------------------------------------------------------*/

void printpage()       /* 'end of page', writes lines of page to output file */
{
    register int  i, j;
    register char ch;

    if (sptr != 0)
        fprintf(stderr, "dvi2tty: warning - stack not empty at eop.\n");
    for (currentline = firstline; currentline != nil;
          currentline = currentline->next) {
        if (currentline != firstline) {
            foo = ((currentline->vv - currentline->prev->vv)/VERTICALEPSILON)-1;
            if (foo > 3)
                foo = 3;        /* linespacings not too large */
            for (i = 1; i <= (int) foo; i++)
                putc('\n', output);
        }
        if (currentline->charactercount >= leftmargin) {
            foo = ttywidth - 2;
            for (i = firstcolumn, j = 1; i <= currentline->charactercount;
                   i++, j++) {
                ch = currentline->text[i - leftmargin];
                if (ch >= SPACE)
                    putc(ch, output);
                if ((j > (int) foo) && (currentline->charactercount > i+1)) {
                        fprintf(output, "*\n");         /* if line to large */
                        fprintf(output, " *");          /* mark output      */
                        j = 2;
                }
            } 
        }
        putc('\n', output);
    } 

    currentline = firstline;
    while (currentline->next != nil) {
        currentline = currentline->next;
        free(currentline->prev);
    }
    free(currentline);              /* free last line */
    currentline = nil;

} /* printpage */

/*----------------------------------------------------------------------------*/

bool inlist(pagenr)                         /* ret true if in list of pages */
register long pagenr;
{

    while ((currentpage->pag < 0) && (currentpage->pag != pagenr) &&
           !currentpage->all && (currentpage->nxt != nil))
        currentpage = currentpage->nxt;
    if ((currentpage->all && (pagenr < currentpage->pag)) ||
         (currentpage->pag == pagenr))
            return TRUE;
    else if (pagenr > 0) {
        while ((currentpage->pag < pagenr) && (currentpage->nxt != nil))
            currentpage = currentpage->nxt;
        if (currentpage->pag == pagenr)
            return TRUE;
    }
    return FALSE;

} /* inlist */

/*----------------------------------------------------------------------------*/

void rule(moving, rulewt, ruleht)
register bool moving;
register long rulewt, ruleht;
{   /* output a rule (vertical or horizontal), increment h if moving is true */

    register char ch;               /* character to set rule with            */
    register long saveh, savev;
                              /* rule   --   starts up the recursive routine */
    if (!moving)
        saveh = h;
    if ((ruleht <= 0) || (rulewt <= 0))
        h += rulewt;
    else {
        savev = v;
        if ((ruleht / rulewt) > 0)
            ch = '|';
        else if (ruleht > (VERTICALEPSILON / 2))
            ch = '=';
        else
            ch = '_';
        ruleaux(rulewt, ruleht, ch);
        v = savev;
    }
    if (!moving)
        h = saveh;

} /* rule */



void ruleaux(rulewt, ruleht, ch)     /* recursive  that does the job */
register long rulewt, ruleht;
register char ch;
{
    register long wt, lmh, rmh;

    wt = rulewt;
    lmh = h;                        /* save left margin                      */
    if (h < 0) {                    /* let rules that start at negative h    */
        wt -= h;                    /* start at coordinate 0, but let it     */
        h = 0;                      /*   have the right length               */
    }
    while (wt > 0) {                /* output the part of the rule that      */
        rmh = h;                    /*   goes on this line                   */
        outchar(ch);
        wt -= (h-rmh);              /* decrease the width left on line       */
    }
    ruleht -= VERTICALEPSILON;      /* decrease the height                   */
    if (ruleht > VERTICALEPSILON) { /* still more vertical?                  */
        rmh = h;                    /* save current h (right margin)         */
        h = lmh;                    /* restore left margin                   */
        v -= (VERTICALEPSILON + VERTICALEPSILON / 10);
        ruleaux(rulewt, ruleht, ch);
        h = rmh;                    /* restore right margin                  */
    }

} /* ruleaux */

/*----------------------------------------------------------------------------*/

long horizontalmove(amount)
register long amount;
{

#if defined(MSDOS)
    if (labs(amount) > charwidth / 4L) {
#else
    if (abs(amount) > charwidth / 4L) {
#endif
        foo = 3*charwidth / 4;
        if (amount > 0)
            amount = ((amount+foo) / charwidth) * charwidth;
        else
            amount = ((amount-foo) / charwidth) * charwidth;
        h += amount;
        return amount;
    }
    else
        return 0;

}   /* horizontalmove */

/*----------------------------------------------------------------------------*/

int skipnops()                      /* skips by no-op commands  */
{
    register int opcode;

    while ((opcode = (int) num(1)) == NOP);
    return opcode;

} /* skipnops */

/*----------------------------------------------------------------------------*/

linetype *getline()             /* returns an initialized line-object */
{
    register int  i;
    register linetype *temp;

    if ((temp = (linetype *) malloc(sizeof(linetype))) == NULL) 
        errorexit(lnerq);
    temp->charactercount = leftmargin - 1;
    temp->prev = nil;
    temp->next = nil;
    for (i = 0; i < LINELEN; i++)
        temp->text[i] = ' ';
    temp->text[i] = '\0';
    return temp;

} /* getline */

/*----------------------------------------------------------------------------*/

linetype *findline()            /* find best fit line were text should go */
{                               /* and generate new line if needed        */
    register linetype *temp;
    register long topd, botd;

    if (v <= firstline->vv) {                      /* above first line */
        if (firstline->vv - v > VERTICALEPSILON) {
            temp = getline();
            temp->next = firstline;
            firstline->prev = temp;
            temp->vv = v;
            firstline = temp;
        }
        return firstline;
    }

    if (v >= lastline->vv) {                       /* below last line */
        if (v - lastline->vv > VERTICALEPSILON) {
            temp = getline();
            temp->prev = lastline;
            lastline->next = temp;
            temp->vv = v;
            lastline = temp;
        }
        return lastline;
    }

    temp = lastline;                               /* in between two lines */
    while ((temp->vv > v) && (temp != firstline))
        temp = temp->prev;

    /* temp->vv < v < temp->next->vv --- temp is above, temp->next is below */
    topd = v - temp->vv;
    botd = temp->next->vv - v;
    if ((topd < VERTICALEPSILON) || (botd < VERTICALEPSILON))
        if (topd < botd)                           /* take best fit */
            return temp;
        else
            return temp->next;

    /* no line fits suitable, generate a new one */
    currentline = getline();
    currentline->next = temp->next;
    currentline->prev = temp;
    temp->next->prev = currentline;
    temp->next = currentline;
    currentline->vv = v;
    return currentline;

} /* findline */

/*----------------------------------------------------------------------------*/

unsigned long num(size)
register int size;
{
    register int i;
    register long x = 0;

    for (i = 0; i < size; i++)
        x = (x << 8) + (unsigned) getc(DVIfile);
    return x;

} /* num */


long snum(size)
register int size;
{
    register int i;
    register long x = 0;

    x = getc(DVIfile);
    if (x & 0x80)
        x -= 0x100;
    for (i = 1; i < size; i++)
        x = (x << 8) + (unsigned) getc(DVIfile);
    return x;

} /* snum */

/*----------------------------------------------------------------------------*/

void outchar(ch)                     /* output ch to appropriate line */
register char ch;
{
    register int i, j;

/*     fprintf(stderr, "hor: %ld, ver: %ld\n", h, v); */
#if defined(MSDOS)
    if (labs(v - currentline->vv) > VERTICALEPSILON / 2L)
#else
    if (abs(v - currentline->vv) > VERTICALEPSILON / 2L)
#endif
        currentline = findline();
    switch (ch) {
        case 11  :  outchar('f'); ch = 'f'; break;  /* ligature        */
        case 12  :  outchar('f'); ch = 'i'; break;  /* ligature        */
        case 13  :  outchar('f'); ch = 'l'; break;  /* ligature        */
        case 14  :  outchar('f'); outchar('f');
                                  ch = 'i'; break;  /* ligature        */
        case 15  :  outchar('f'); outchar('f');
                                  ch = 'l'; break;  /* ligature        */
        case 16  :  ch = 'i'; break;
        case 17  :  ch = 'j'; break;
        case 25  :  outchar('s'); ch = 's'; break;  /* German double s */
        case 26  :  outchar('a'); ch = 'e'; break;  /* Dane/Norw ae    */
        case 27  :  outchar('o'); ch = 'e'; break;  /* Dane/Norw oe    */
        case 28  :  if (scascii)
                        ch = '|';                   /* Dane/Norw /o    */
                    else
                        ch = 'o';
                    break;
        case 29  :  outchar('A'); ch = 'E'; break;  /* Dane/Norw AE    */
        case 30  :  outchar('O'); ch = 'E'; break;  /* Dane/Norw OE    */
        case 31  :  if (scascii)
                        ch = '\\';                  /* Dane/Norw /O    */
                    else
                        ch = 'O';
                    break;
#if 0
        case 92  :  ch = '"'; break;                /* beginning qoute  */
        case 123 :  ch = '-'; break;
        case 124 :  ch = '_'; break;
        case 125 :  ch = '"'; break;
        case 126 :  ch = '"'; break;
#endif
    }
#if 0
    j = (int) (((double) h / (double) maxpagewidth) * (ttywidth-1)) + 1;
#else
    j = (int) (h / charwidth);
#endif
    if (j > rightmargin)
        j = rightmargin;
    else if (j < leftmargin)
        j = leftmargin;
    foo = leftmargin - 1;
    /*-------------------------------------------------------------*/
    /* The following is very specialized code, it handles national */
    /* Swedish characters. They are respectively: a and o with two */
    /* dots ("a & "o) and a with a circle (Oa). In Swedish "ASCII" */
    /* these characters replace }{|][ and \.  TeX outputs these by */
    /* first issuing the dots or circle and then backspace and set */
    /* the a or o.  When dvitty finds an a or o it searches in the */
    /* near vicinity for the character codes that represent circle */
    /* or dots and if one is found the corresponding national char */
    /* replaces the special character codes.                       */
    /*-------------------------------------------------------------*/
    if (scascii) {
        if ((ch == 'a') || (ch == 'A') || (ch == 'o') || (ch == 'O')) {
            for (i = IMAX(leftmargin, j-2);
                 i <= IMIN(rightmargin, j+2);
                 i++)
                if ((currentline->text[i - leftmargin] == 127) ||
                    (currentline->text[i - leftmargin] == 23))
                    foo = i;
            if (foo >= leftmargin) {
                j = (int) foo;
                switch (currentline->text[j - leftmargin]) {
                    case 127 : if (ch == 'a')
                                   ch = '{';
                               else if (ch == 'A')      /* dots ... */
                                   ch = '[';
                               else if (ch == 'o')
                                   ch = '|';
                               else if (ch == 'O')
                                   ch = '\\';
                               break;
                    case 23  : if (ch == 'a')
                                   ch = '}';
                               else if (ch == 'A')      /* circle */
                                   ch = ']';
                               break;
                }
            }
        }
    }
    /*----------------- end of 'Scandinavian code' ----------------*/
    if (foo == leftmargin-1)
        while ((currentline->text[j - leftmargin] != SPACE)
               && (j < rightmargin)) {
            j++;
            h += charwidth;
        }
    if ( ((ch >= SPACE) && (ch != DEL)) ||
         (scascii && (ch == 23)) ) {
        if (j < rightmargin)
            currentline->text[j - leftmargin] = ch;
        else
            currentline->text[rightmargin - leftmargin] = '@';
        if (j > currentline->charactercount)
            currentline->charactercount = j;
        if (j < firstcolumn)
            firstcolumn = j;
        h += charwidth;
    }

} /* outchar */

/*----------------------------------------------------------------------------*/

void putcharacter(charnr)            /* output character, don't change h */
register long charnr;
{
    register long saveh;

    saveh = h;
    if ((charnr >= 0) && (charnr <= LASTCHAR))
        outchar((char) charnr);
    else
        setchar(charnr);
    h = saveh;

} /* putcharacter */

/*----------------------------------------------------------------------------*/

void setchar(charnr)
long charnr;
{    /* should print characters with character code>127 from current font */
     /* note that the parameter is a dummy, since ascii-chars are<=127    */

    outchar('#');

} /* setchar */

