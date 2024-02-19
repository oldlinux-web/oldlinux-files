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
 *   This routine squeezes a PostScript file down to it's
 *   minimum.  We parse and then output it.
 */
#include "stdio.h"
#define LINELENGTH (78)
static int linepos = 0 ;
static int lastspecial = 1 ;
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
      (void)putchar('\n') ;
      linepos = 0 ;
   }
   (void)putchar(c) ;
   linepos++ ;
   lastspecial = 1 ;
}

void
strout(s)
	char *s ;
{
   if (linepos + strlen(s) > LINELENGTH) {
      (void)putchar('\n') ;
      linepos = 0 ;
   }
   linepos += strlen(s) ;
   while (*s != 0)
      (void)putchar(*s++) ;
   lastspecial = 1 ;
}

void
cmdout(s)
	char *s ;
{
   int l ;

   l = strlen(s) ;
   if (linepos + l + 1 > LINELENGTH) {
      (void)putchar('\n') ;
      linepos = 0 ;
      lastspecial = 1 ;
   }
   if (! lastspecial) {
      (void)putchar(' ') ;
      linepos++ ;
   }
   while (*s != 0) {
      (void)putchar(*s++) ;
   }
   linepos += l ;
   lastspecial = 0 ;
}

char buf[1000] ;

main()
{
   int c ;
   char *b ;
   char seeking ;

   printf("%%!\n") ;
   while (1) {
      c = getchar() ;
      if (c==EOF)
         break ;
      if (c=='%') {
         while ((c=getchar())!='\n') ;
      }
      if (c <= ' ')
         continue ;
      switch (c) {
case '{' :
case '}' :
case '[' :
case ']' :
         specialout(c) ;
         break ;
case '<' :
case '(' :
         if (c=='(')
            seeking = ')' ;
         else
            seeking = '>' ;
         b = buf ;
         *b++ = c ;
         do {
            c = getchar() ;
            *b++ = c ;
            if (c=='\\')
               *b++ = getchar() ;
         } while (c != seeking) ;
         *b++ = 0 ;
         strout(buf) ;
         break ;
default:
         b = buf ;
         while ((c>='A'&&c<='Z')||(c>='a'&&c<='z')||
                (c>='0'&&c<='9')||(c=='/')||(c=='@')||
                (c=='-')||(c=='.')||(c=='#')
		||(c=='$')||(c=='|')||(c=='_')) {
            *b++ = c ;
            c = getchar() ;
         }
         if (b == buf) {
            fprintf(stderr, "Oops!  Missed a case: %c.\n", c) ;
            exit(1) ;
         }
         *b++ = 0 ;
         (void)ungetc(c, stdin) ;
         cmdout(buf) ;
      }
   }
   if (linepos != 0)
      (void)putchar('\n') ;
   exit(0) ;
}
