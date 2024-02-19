/*{{{  #includes*/
#include <termcap.h>
#include <limits.h>
#include <stdio.h>
 
#include <local/bool.h>
 
#define SCREEN_C

#include "origami.h"
/*}}}  */
 
PRIVATE char termcap_area[512];
PRIVATE int lines;
PRIVATE char *cm,*ce,*cd,*al,*dl,*so,*se,*ti,*te;
int sg;
 
int screen_lines;
bool dirty;
 
/*{{{  outchar*/
#ifdef STD_C
PRIVATE outchar(char ch)
#else
PRIVATE outchar(ch) char ch;
#endif
{
  putchar(ch);
}
/*}}}  */
/*{{{  full_window*/
#ifdef STD_C
full_window (void)
#else
full_window ()
#endif
{
  W_oy = 1;
  W_dy = screen_lines;
}
/*}}}  */
/*{{{  gotoxy*/
#ifdef STD_C
gotoxy(int x,int y)
#else
gotoxy(x,y) int x,y;
#endif
{
  tputs (tgoto(cm,x-1,y-1),1,outchar);
}
/*}}}  */
/*{{{  clreol*/
#ifdef STD_C
clreol(void)
#else
clreol()
#endif
{
  tputs(ce,1,outchar);
}
/*}}}  */
/*{{{  insLine*/
#ifdef STD_C
insLine(void)
#else
insLine()
#endif
{
  if (al) tputs(al,1,outchar); else dirty=TRUE;
}
/*}}}  */
/*{{{  DelLine*/
#ifdef STD_C
DelLine(void)
#else
DelLine()
#endif
{
  if (dl) tputs(dl,1,outchar); else dirty=TRUE;
}
/*}}}  */
/*{{{  ClrFullScr*/
#ifdef STD_C
ClrFullScr(void)
#else
ClrFullScr()
#endif
{
  tputs(cd,1,outchar);
}
/*}}}  */
/*{{{  standout*/
#ifdef STD_C
standout(void)
#else
standout()
#endif
{
  if (so) tputs(so,1,outchar);
}
/*}}}  */
/*{{{  standend*/
#ifdef STD_C
standend(void)
#else
standend()
#endif
{
  if (se) tputs(se,1,outchar);
}
/*}}}  */
/*{{{  init_terminal*/
/* only a poor initialization, but I am still working */
PUBLIC void init_terminal()
{
  if (ti) tputs(ti,1,outchar);
}
/*}}}  */
/*{{{  reset_terminal()*/
PUBLIC void reset_terminal()
{
  if (te) tputs(te,1,outchar);
}
/*}}}  */
/*{{{  complain*/
#ifdef STD_C
PRIVATE complain(char *msg,char *cap)
#else
PRIVATE complain(msg,cap) char *msg, *cap;
#endif
{
  printf(F_TERMA,msg,cap);
  exit(1);
}
/*}}}  */
/*{{{  get_terminal_capability*/
#ifdef STD_C
get_terminal_capability(void)
#else
get_terminal_capability()
#endif
{
  char bp[1024];
  char *terminal;
  char *getenv();
  char *pointer;
 
  if ((terminal=getenv("TERM"))==NULL) {
    printf(M_NO_TERM);
    return (1);
  }
  switch (tgetent(bp,terminal)) {
    /*{{{  1*/
    case -1: {
      printf(M_NO_TERMCAP);
      return (1);
      break;
    }
    /*}}}  */
    /*{{{  0*/
    case 0: {
      printf(F_TERMB,terminal);
      return (1);
      break;
    }
    /*}}}  */
  }
 
  screen_lines = tgetnum("li");
  W_dx = tgetnum("co");
 
  pointer = termcap_area;
  if ((cm=tgetstr("cm",&pointer))==NULL) complain("cursor motion","cm");
  if ((ce=tgetstr("ce",&pointer))==NULL) complain("clear to end of line","ce");
  if ((cd=tgetstr("cd",&pointer))==NULL) complain("clear to end of display","cd");
  al=tgetstr("al",&pointer);
  dl=tgetstr("dl",&pointer);
  so=tgetstr("so",&pointer);
  se=tgetstr("se",&pointer);
  sg=tgetnum("sg");
  if (sg == -1) sg=0;
  ti=tgetstr("ti",&pointer);
  te=tgetstr("te",&pointer);
  return (0);
}
/*}}}  */
