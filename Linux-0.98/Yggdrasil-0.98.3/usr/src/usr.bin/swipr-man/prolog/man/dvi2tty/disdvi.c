/*****************************************************************************/
/*                                                                           */
/*   disdvi  ---  disassembles TeX dvi files.                                */
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include "commands.h"
#if defined(MSDOS)
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#endif

#define LASTCHAR        127    /* max dvi character, above are commands    */

#define get1()           num(1)
#define get2()           num(2)
#define get3()           num(3)
#define get4()           num(4)
#define sget1()         snum(1)
#define sget2()         snum(2)
#define sget3()         snum(3)
#define sget4()         snum(4)


FILE * dvifp;
char * dvi_name;
long   pc = 0;

char *          malloc          ();

void            main            ();
void            bop             ();
void            preamble        ();
void            postamble       ();
void            postpostamble   ();
void            fontdef         ();
void            special         ();
void            printnonprint   ();
unsigned long   num             ();
long            snum            ();




/*---------------------------------------------------------------------------*/

void main(argc, argv)
int argc;
char **argv;
{
    register int opcode;                /* dvi opcode                        */
    register int i;

    if (argc > 2) {
        fprintf(stderr, "To many arguments\n");
        fprintf(stderr, "Usage: %s [dvi-file]\n", *argv);
        exit(1);
    }

    if (argc == 2) {
        if ((i = strlen(argv[1])) == 0) {
            fprintf(stderr, "Illegal empty filename\n");
            fprintf(stderr, "Usage: %s [dvi-file]\n", *argv);
            exit(2);
        }
        if ((i >= 5) && (argv[1][i-4] == '.') && (argv[1][i-3] == 'd') &&
              (argv[1][i-2] = 'v') && (argv[1][i-1] = 'i'))
            dvi_name = argv[1];
        else {
            dvi_name = malloc((i+4) * sizeof(char));
            strcpy(dvi_name, argv[1]);
            strcat(dvi_name, ".dvi");
        }
        if ((dvifp = fopen(dvi_name, "r")) == NULL) {
            perror(dvi_name);
            exit(3);
        }
    }
    else
        dvifp = stdin;

#if defined(MSDOS)
    setmode(fileno(dvifp), O_BINARY);
#endif

    while ((opcode = (int) get1()) != EOF) {    /* process until end of file */
        printf("%06ld: ", pc - 1);
        if ((opcode <= LASTCHAR) && isprint(opcode)) {
            printf("Char:     ");
            while ((opcode <= LASTCHAR) && isprint(opcode)) {
                putchar(opcode);
                opcode = (int) get1();
            }
            putchar('\n');
            printf("%06ld: ", pc - 1);
        }

        if (opcode <= LASTCHAR) 
            printnonprint(opcode);              /* it must be a non-printable */
        else if ((opcode >= FONT_00) && (opcode <= FONT_63))
            printf("FONT_%d\n", opcode - FONT_00);
        else
            switch (opcode) {
                case SET1     :
                case SET2     : 
                case SET3     :
                case SET4     : printf("SET%d:    %ld\n", opcode - SET1 + 1,
                                                       num(opcode - SET1 + 1));
                                break;
                case SET_RULE : printf("SET_RULE: %ld, %ld\n", sget4(),
                                                                sget4());
                                break;
                case PUT1     :
                case PUT2     :
                case PUT3     :
                case PUT4     : printf("PUT%d:     %ld\n", opcode - PUT1 + 1,
                                                       num(opcode - PUT1 + 1));
                                break;
                case PUT_RULE : printf("PUT_RULE: %ld, %ld\n", sget4(),
                                                                sget4());
                                break;
                case NOP      : printf("NOP\n");  break;
                case BOP      : bop();            break;
                case EOP      : printf("EOP\n");  break;
                case PUSH     : printf("PUSH\n"); break;
                case POP      : printf("POP\n");  break;
                case RIGHT1   :
                case RIGHT2   : 
                case RIGHT3   : 
                case RIGHT4   : printf("RIGHT%d:   %ld\n", opcode - RIGHT1 + 1,
                                                     snum(opcode - RIGHT1 + 1));
                                break;
                case W0       : printf("W0\n");   break;
                case W1       : 
                case W2       :
                case W3       :
                case W4       : printf("W%d:       %ld\n", opcode - W0,
                                                      snum(opcode - W0));
                                break;
                case X0       : printf("X0\n");   break;
                case X1       :
                case X2       :
                case X3       :
                case X4       : printf("X%d:       %ld\n", opcode - X0,
                                                      snum(opcode - X0));
                                break;
                case DOWN1    : 
                case DOWN2    : 
                case DOWN3    :
                case DOWN4    : printf("DOWN%d:    %ld\n", opcode - DOWN1 + 1,
                                                      snum(opcode - DOWN1 + 1));
                                break;
                case Y0       : printf("Y0\n");   break;
                case Y1       :
                case Y2       :
                case Y3       :
                case Y4       : printf("Y%d:       %ld\n", opcode - Y0,
                                                      snum(opcode - Y0));
                                break;
                case Z0       : printf("Z0\n");   break;
                case Z1       :
                case Z2       :
                case Z3       : 
                case Z4       : printf("Z%d:       %ld\n", opcode - Z0,
                                                      snum(opcode - Z0));
                                break;
                case FNT1     :
                case FNT2     :
                case FNT3     :
                case FNT4     : printf("FNT%d:     %ld\n", opcode - FNT1 + 1,
                                                       num(opcode - FNT1 + 1));
                                break;
                case XXX1     : 
                case XXX2     : 
                case XXX3     :
                case XXX4     : special(opcode - XXX1 + 1);     break;
                case FNT_DEF1 :
                case FNT_DEF2 :
                case FNT_DEF3 :
                case FNT_DEF4 : fontdef(opcode - FNT_DEF1 + 1); break;
                case PRE      : preamble();                     break;
                case POST     : postamble();                    break;
                case POST_POST: postpostamble();                break;
            }
    }

} /* main */


/*----------------------------------------------------------------------------*/


void bop()
{
    int i;

    printf("BOP       page number      : %ld\n", sget4());
    for (i=0; i < 3; i++) {
        printf("%06ld: ", pc - 1);
        printf("          %6ld  %6ld  %6ld\n", sget4(), sget4(), sget4()); 
    }
    printf("%06ld: ", pc - 1);
    printf("          prev page offset : %06ld\n", sget4()); 

} /* bop */


/*---------------------------------------------------------------------------*/

void postamble() 
{

    printf("POST      last page offset : %06ld\n", sget4());
    printf("%06ld: ", pc - 1);
    printf("          numerator        : %ld\n", get4());
    printf("%06ld: ", pc - 1);
    printf("          denominator      : %ld\n", get4());
    printf("%06ld: ", pc - 1);
    printf("          magnification    : %ld\n", get4());
    printf("%06ld: ", pc - 1);
    printf("          max page height  : %ld\n", get4());
    printf("%06ld: ", pc - 1);
    printf("          max page width   : %ld\n", get4());
    printf("%06ld: ", pc - 1);
    printf("          stack size needed: %d\n", (int) get2());
    printf("%06ld: ", pc - 1);
    printf("          number of pages  : %d\n", (int) get2());

} /* postamble */

void preamble()
{
    register int i;

    printf("PRE       version          : %d\n", (int) get1());
    printf("%06ld: ", pc - 1);
    printf("          numerator        : %ld\n", get4());
    printf("%06ld: ", pc - 1);
    printf("          denominator      : %ld\n", get4());
    printf("%06ld: ", pc - 1);
    printf("          magnification    : %ld\n", get4());
    printf("%06ld: ", pc - 1);
    i = (int) get1();
    printf("          job name (%03d)   :", i);
    while (i-- > 0)
        putchar((int) get1());
    putchar('\n');

} /* preamble */


void postpostamble()
{
    register int i;
 
    printf("POSTPOST  postamble offset : %06ld\n", get4());
    printf("%06ld: ", pc - 1);
    printf("          version          : %d\n", (int) get1());
    while ((i = (int) get1()) == TRAILER) {
        printf("%06d: ", pc - 1);
        printf("TRAILER\n");
    }
    while (i != EOF) {
        printf("%06ld: ", pc - 1);
        printf("BAD DVI FILE END: 0x%02X\n", i);
        i = (int) get1();
    }

} /* postpostamble */



void special(x)
register int x;
{
    register long len;
    register long i;

    len = num(x);
    printf("XXX%d:     %ld bytes\n", x, len);
    printf("%06ld: ", pc - 1);
    for (i = 0; i < len; i++)
        putchar((int) get1());
    putchar('\n');

} /* special */



void fontdef(x)
register int x;
{
    register int i;

    printf("FNT_DEF%d: %ld\n", x, num(x));
    printf("%06ld: ", pc - 1);
    printf("          checksum         : %ld\n", get4());
    printf("%06ld: ", pc - 1);
    printf("          scale            : %ld\n", get4());
    printf("%06ld: ", pc - 1);
    printf("          design           : %ld\n", get4());
    printf("%06ld: ", pc - 1);
    printf("          name             : ");
    for (i = (int) get1() + (int) get1(); i > 0; i--)
        putchar((int) get1());
    putchar('\n');

} /* fontdef */



void printnonprint(ch)
register int ch;
{

    printf("Char:     ");
    switch (ch) {
        case 11  :  printf("ff         /* ligature (non-printing) */"); break;
        case 12  :  printf("fi         /* ligature (non-printing) */"); break;
        case 13  :  printf("fl         /* ligature (non-printing) */"); break;
        case 14  :  printf("ffi        /* ligature (non-printing) */"); break;
        case 15  :  printf("ffl        /* ligature (non-printing) */"); break;
        case 16  :  printf("i          /* (non-printing) */");          break;
        case 17  :  printf("j          /* (non-printing) */");          break;
        case 25  :  printf("ss         /* german (non-printing) */");   break;
        case 26  :  printf("ae         /* scadinavian (non-printing) */");
                    break;
        case 27  :  printf("oe         /* scadinavian (non-printing) */");
                    break;
        case 28  :  printf("o          /* scadinavian (non-printing) */");
                    break;
        case 29  :  printf("AE         /* scadinavian (non-printing) */");
                    break;
        case 30  :  printf("OE         /* scadinavian (non-printing) */");
                    break;
        case 31  :  printf("O          /* scadinavian (non-printing) */");
                    break;
        default  :  printf("0x%2X", ch); break;
    }
    putchar('\n');

}



unsigned long num(size)
register int size;
{
    register int i;
    register long x = 0;

    pc += size;
    for (i = 0; i < size; i++)
        x = (x << 8) + (unsigned) getc(dvifp);
    return x;

} /* num */



long snum(size)
register int size;
{
    register int i;
    register long x = 0;

    pc += size;
    x = getc(dvifp);
    if (x & 0x80)
        x -= 0x100;
    for (i = 1; i < size; i++)
        x = (x << 8) + (unsigned) getc(dvifp);
    return x;

} /* snum */



/*


================================================================================
==                          DVI file format                                   ==
================================================================================
no_ops          >= 0 bytes     (NOP, nops before the preamble)
preamble_marker    1 ubyte     (PRE)
version_id         1 ubyte     (should be version 2)
numerator          4 ubytes    (numerater must equal the one in postamble)
denominator        4 ubytes    (denominator must equal the one in postamble)
magnification      4 ubytes    (magnification must equal the one in postamble)
id_len             1 ubyte     (lenght of identification string)
id_string     id_len ubytes    (identification string)

no_ops          >= 0 bytes     (NOP, nops before a page)
begin_of_page      1 ubyte     (BOP)
page_nr            4 sbytes    (page number)
do_be_do          36 bytes     (filler ????)
prev_page_offset   4 sbytes    (offset in file where previous page starts, -1 for none)
... PAGE DATA ...
end_of_page        1 ubyte     (EOP)

no_ops ???      >= 0 bytes     (NOPS, I think they are allowed here...)
postamble_marker   1 ubyte     (POST)
last_page_offset   4 sbytes    (offset in file where last page starts)
numerator          4 ubytes    (numerater must equal the one in preamble)
denominator        4 ubytes    (denominator must equal the one in preamble)
magnification      4 ubytes    (magnification must equal the one in preamble)
max_page_height    4 ubytes    (maximum page height)
max_page_width     4 ubytes    (maximum page width)
max_stack          2 ubytes    (maximum stack depth needed)
total_pages        2 ubytes    (number of pages in file)
... FONT DEFINITIONS ...

postamble_offset   4 sbytes    (offset in file where postamble starts)
version_id         1 ubyte     (should be version 2)
trailer         >= 4 ubytes    (TRAILER)
<EOF>


FONT DEFINITIONS:
   do {
      switch (c = getc(dvi_fp) {
          case FNTDEF1  :
          case FNTDEF2  :
          case FNTDEF3  :
          case FNTDEF4  :  define_font(c);
          case POSTPOST : break;
          default       : error;
      }
   } while (c != POSTPOST);

===== A font def looks like:

1,2,3 or 4 ubytes TeXfontnumber for FNTDEF1 .. FNTDEF4
4 ubytes checksum
4 ubytes scale
4 ubytes design size
1 byte deflen1
1 byte deflen2
deflen1 + deflen2 bytes fontname.

===== A special looks like:

1,2,3 or 4 ubytes telling length of special command for XXX1 .. XXX4
all bytes in the special command are used as defined in the dvi driver.


*/
