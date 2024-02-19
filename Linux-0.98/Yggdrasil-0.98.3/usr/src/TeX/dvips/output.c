/*
 *   This is dvips, a freely redistributable PostScript driver
 *   for dvi files.  It is (C) Copyright 1987 by Tomas Rokicki.
 *   You may modify and use this program to your heart's content,
 *   so long as you send modifications to Tomas Rokicki.  It can
 *   be included in any distribution, commercial or otherwise, so
 *   long as the banner string defined in structures.h is not
 *   modified (except for the version number) and this banner is
 *   printed on program invocation, or can be printed on program
 *   invocation with the -? option.
 */
/*
 *   output.c of dvisw software package.  This code (C) 1986 Radical Eye
 *   Software.
 *
 *   These routines do most of the communicating with the printer.
 *
 *   LINELENGTH tells the maximum line length to send to the printer.
 */
#define LINELENGTH (78)
#include "structures.h"
/*
 *   The external routines called here:
 */
extern void error() ;
extern FILE *search() ;
/*
 *   These are the external variables used by these routines.
 */
extern shalfword hh, vv ;
extern fontdesctype *curfnt ;
extern FILE *bitfile ;
extern char *oname ;
extern Boolean landscape ;
/*
 *   We need a few statics to take care of things.
 */
static shalfword rhh, rvv ;
static shalfword linepos = 0 ;
static Boolean instring ;
static Boolean lastspecial = 1 ;
static shalfword d ;
static int lastfont ;
static void chrcmd();			/* just a forward declaration */

/*
 *   Some very low level primitives to send output to the printer.
 */

/*
 *   This routine copies a file down the pipe.  Search path uses the
 *   header path.
 */
void
copyfile(s)
	char *s ;
{
   FILE *f = search(HEADERPATH, s) ;
   int c, prevc ;

   if (f==NULL) {
      fprintf(stderr, "(%s) %s\n", HEADERPATH, s) ;
      error("! couldn't find header file") ;
   } else {

#define COPYSIZE (BUFSIZ * 8)
     
     char buffer[ COPYSIZE ];
     int bytes;

     if (linepos != 0)
       (void)putc('\n', bitfile) ;

     while (! feof(f) ) {
       int bytes = fread(buffer, sizeof(char), COPYSIZE, f);
       if ( bytes == 0 ) {
	 perror("fread");
	 break;
       } else {
	 int written = fwrite(buffer, sizeof(char), bytes, bitfile);
	 if (written == 0 || bytes != written) {
	   perror("fwrite");
	   break;
	 }
       }
     }

     if (bytes > 0 || bytes <= COPYSIZE) {
       prevc = buffer[ bytes - 1] ;
     }
     else {
       prevc = 0;
     }

     if (prevc != '\n')
       (void)putc('\n', bitfile) ;
     linepos = 0 ;
   }
}

/*
 *   This next routine writes out a `special' character.  In this case,
 *   we simply put it out, since any special character terminates the
 *   preceding token.
 */
void
specialout(c)
	char c ;
{
   if (linepos + 1 > LINELENGTH) {
      (void)putc('\n', bitfile) ;
      linepos = 0 ;
   }
   (void)putc(c, bitfile) ;
   linepos++ ;
   lastspecial = 1 ;
}

static void
scout(c)
	char c ;
{
   if (! instring) {
/*
 *   If we are within 5, we send a carriage return.
 *   This eliminates extraneous backslash-newlines.
 */
      if (linepos + 5 > LINELENGTH) {
         (void)putc('\n', bitfile) ;
         linepos = 0 ;
      }
      (void)putc('(', bitfile) ;
      linepos++ ;
      lastspecial = 1 ;
      instring = 1 ;
   }
   if (c<' ' || c>127) {
      if (linepos + 5 > LINELENGTH) {
         (void)putc('\\', bitfile) ;
         (void)putc('\n', bitfile) ;
         linepos = 0 ;
      }
      (void)putc('\\', bitfile) ;
      linepos++ ;
      (void)putc('0' + ((c >> 6) & 7), bitfile) ;
      linepos++ ;
      (void)putc('0' + ((c >> 3) & 7), bitfile) ;
      linepos++ ;
      (void)putc('0' + (c & 7), bitfile) ;
      linepos++ ;
   } else if (c == '(' || c == ')' || c == '\\') {
      if (linepos + 3 > LINELENGTH) {
         (void)putc('\\', bitfile) ;
         (void)putc('\n', bitfile) ;
         linepos = 0 ;
      }
      (void)putc('\\', bitfile) ;
      linepos++ ;
      (void)putc(c, bitfile) ;
      linepos++ ;
   } else {
      if (linepos + 2 > LINELENGTH) {
         (void)putc('\\', bitfile) ;
         (void)putc('\n', bitfile) ;
         linepos = 0 ;
      }
      (void)putc(c, bitfile) ;
      linepos++ ;
   }
}

static void
stringend()
{
   instring = 0 ;
   specialout(')') ;
}

void
cmdout(s)
	char *s ;
{
   int l ;

   /* hack added by dorab -- dont know if this is right */
   if (instring) {
	stringend();
	chrcmd('p');
   }
   l = strlen(s) ;
   if (linepos + l + 1 > LINELENGTH) {
      (void)putc('\n', bitfile) ;
      linepos = 0 ;
      lastspecial = 1 ;
   }
   if (! lastspecial) {
      (void)putc(' ', bitfile) ;
      linepos++ ;
   }
   while (*s != 0) {
      (void)putc(*s++, bitfile) ;
   }
   linepos += l ;
   lastspecial = 0 ;
}

static void
chrcmd(c)
	char c ;
{
   if (linepos + 2 > LINELENGTH) {
      (void)putc('\n', bitfile) ;
      linepos = 0 ;
      lastspecial = 1 ;
   }
   if (! lastspecial) {
      (void)putc(' ', bitfile) ;
      linepos++ ;
   }
   (void)putc(c, bitfile) ;
   linepos++ ;
   lastspecial = 0 ;
}

void
znumout(n)
	integer n ;
{
   char buf[10] ;

   (void)sprintf(buf, "%ld", n) ;
   cmdout(buf) ;
}

void
hexout(n)
	int n ;
{
   static char hexchar[] = "0123456789ABCDEF" ;

   if (linepos + 2 > LINELENGTH) {
      (void)putc('\n', bitfile) ;
      linepos = 0 ;
   }
   (void)putc(hexchar[n >> 4], bitfile) ;
   (void)putc(hexchar[n & 15], bitfile) ;
   linepos += 2 ;
}

void
fontout(n)
	int n ;
{
   char buf[6] ;

   if (instring) {
      stringend() ;
      chrcmd('p') ;
   }
   if (n < 27)
      (void)sprintf(buf, "f%c", 'a'+n-1) ;
   else
      (void)sprintf(buf, "f%d", n-27) ;
   cmdout(buf) ;
}

void
hvpos()
{
   int is = instring ;

   if (rvv != vv) {
      if (instring)
         stringend() ;
      numout(hh) ;
      numout(vv) ;
      chrcmd(is ? 'y' : 'a') ;
      rvv = vv ;
   } else if (rhh != hh) {
      if (instring) {
         stringend() ;
         if (hh - rhh < 5 && rhh - hh < 5) {
            chrcmd('p' + hh - rhh) ;
         } else if (hh - rhh < d + 5 && rhh - hh < 5 - d) {
            chrcmd('g' + hh - rhh - d) ;
            d = hh - rhh ;
         } else {
            numout(hh - rhh) ;
            chrcmd('b') ;
            d = hh - rhh ;
         }
      } else {
         numout(hh - rhh) ;
         chrcmd('w') ;
      }
   }
   rhh = hh ;
}

/*
 *   initprinter opens the bitfile and writes the initialization sequence
 *   to it.
 */
void
initprinter() {
   if (*oname != 0) {
      if ((bitfile=fopen(oname,"w"))==NULL)
         error("! couldn't open bit file") ;
   } else {
      bitfile = stdout ;
   }
}

/*
 *   cleanprinter is the antithesis of the above routine.
 */
void
cleanprinter()
{
   cmdout("@end") ;
   fprintf(bitfile, "\n") ;
   (void)fclose(bitfile) ;
   bitfile = NULL ;
}

/*
 *   pageinit initializes the output variables.
 */
void
pageinit()
{
   rhh = -3200 ;
   rvv = -3200 ;
   cmdout("bop") ;
   lastfont = -1 ;
   d = -3200 ;
}

/*
 *   This routine ends a page.
 */
void
pageend()
{
   if (instring) {
      stringend() ;
      chrcmd('g') ;
   }
   cmdout("eop") ;
}

/*
 *   drawrule draws a rule at the specified position.
 */
void
drawrule(rw, rh)
	shalfword rw, rh ;
{
   hvpos() ;
   if (instring) {
      stringend() ;
      chrcmd('p') ;
   }
   numout(rw) ;
   numout(rh) ;
   chrcmd('v') ;
}

/*
 *   drawchar draws a character at the specified position.
 */
void
drawchar(c, cc)
	chardesctype *c ;
	int cc ;
{
   hvpos() ;
   if (lastfont != curfnt->psname) {
      fontout((int)curfnt->psname) ;
      lastfont = curfnt->psname ;
   }
   scout(cc) ;
   rhh = hh + c->pixelwidth ;
   rvv = vv ;
}
