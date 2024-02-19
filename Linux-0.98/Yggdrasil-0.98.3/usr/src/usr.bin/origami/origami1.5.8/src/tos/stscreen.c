/* screen.c */

/*{{{  #includes*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <tos.h>
#include <string.h>
#include <vdi.h>
#include <aes.h>
#include "ORI_RSC.H"
/*}}}  */

#define SCREEN_C
#include "local\bool.h"
#include "..\origami\origami.h"

/*{{{  globals for screen.c*/
int sg=0;
int screen_lines;
bool dirty = FALSE;

int _x=0,_y=0;
/*}}}  */

/*{{{  global GEM variables - ap_id, window handles etc.*/
int work_in[12],work_out[57],
    phys_handle,handle,
    gl_hchar,gl_wchar,gl_hbox=0,gl_wbox=0,
    ap_id,
    wind_x,wind_y,wind_w,wind_h,
    wind_handle=-1;
/*}}}  */

/*{{{  void full_window(void)*/
void full_window(void)
{
  extern W_oy,W_dy;
  W_oy = 1;
  W_dy = screen_lines;
}
/*}}}  */
/*{{{  void gotoxy(int x,int y)*/
void gotoxy(int x,int y)
{
  _x = x-1;
  _y = y-1;
}
/*}}}  */
/*{{{  void clreol(void)*/
void clreol(void)
{
  int xyarray[4];
 
  xyarray[0] = _x*gl_wchar+wind_x;
  xyarray[1] = _y*gl_hchar+wind_y;
  xyarray[2] = wind_x+wind_w;
  xyarray[3] = xyarray[1]+gl_hchar;
  vr_recfl(handle,xyarray);
}
/*}}}  */
/*{{{  void insLine(void)*/
void insLine(void)
{
  int xy[8];
  MFDB mfdb = {0L};
 
  if (_y >= screen_lines-3)
    return;
  xy[0] = xy[4] = wind_x;
  xy[1] = wind_y+_y*gl_hchar;
  xy[2] = xy[6] = wind_x+wind_w-1;
  xy[3] = wind_y+(screen_lines-3)*gl_hchar;
  xy[5] = xy[1]+gl_hchar;
  xy[7] = xy[3]+gl_hchar;
  vro_cpyfm(handle,3,xy,&mfdb,&mfdb);
}
/*}}}  */
/*{{{  void DelLine(void)*/
void DelLine(void)
{
  int xy[8];
  MFDB mfdb = {0L};
 
  if (_y >= screen_lines-3)
    return;
  xy[0] = xy[4] = wind_x;
  xy[1] = wind_y+(_y+1)*gl_hchar;
  xy[2] = xy[6] = wind_x+wind_w-1;
  xy[3] = wind_y+(screen_lines-2)*gl_hchar;
  xy[5] = xy[1]-gl_hchar;
  xy[7] = xy[3]-gl_hchar;
  vro_cpyfm(handle,3,xy,&mfdb,&mfdb);
}
/*}}}  */
/*{{{  void ClrFullScr(void)*/
void ClrFullScr(void)
{
  int xyarray[4];
 
  xyarray[0] = wind_x;
  xyarray[1] = wind_y+_y*gl_hchar;
  xyarray[2] = wind_x+wind_w;
  xyarray[3] = wind_y+wind_h;
  vr_recfl(handle,xyarray);
}
/*}}}  */
/*{{{  void standout(void)*/
void standout(void)
{
  vst_effects(handle,1);
 
}
/*}}}  */
/*{{{  void standend(void)*/
void standend(void)
{
  vst_effects(handle,0);
}
/*}}}  */
/*{{{  int get_terminal_capability(void)*/
int get_terminal_capability(void)
{
  extern int W_dx;

  screen_lines = wind_h/gl_hchar;
  W_dx = wind_w/gl_wchar;
  return 0;
}
/*}}}  */
/*{{{  void vputs(const char *str, bool newline)*/
void vputs(char *str, bool newline)
{
  static char vbuf[256];
  char *d, *s = str;
  int len;
 
  do
  {
    d = vbuf;
    while(*s && *s != '\n')
      *d++ = *s++;
    *d = 0;
    len = (int)(d-vbuf);
    if (len > 0)
      v_gtext(handle,wind_x+_x*gl_wchar,wind_y+_y*gl_hchar,vbuf);
    if (*s == '\n' || newline)
    {
      _x=0;
      _y++;
      if (*s) s++;
    }
    else
      _x += len;
  } while (*s != 0);
}
/*}}}  */
/*{{{  void vputchar(char c)*/
void vputchar(char c)
{
  char buf[2];

  if (c=='\n')
  {
    _x=0;
    _y++;
    return;
  }
  if (c==8) /* Backspace */
  {
    _x--;
    *buf = ' ';
  }
  else
    buf[0] = c;
  buf[1] = 0;
  v_gtext(handle,wind_x+_x*gl_wchar,wind_y+_y*gl_hchar,buf);
  if (c!=8)
    _x++;
}
/*}}}  */

/*{{{  int printf(const char *format, ...) - printf routine that uses VDI output*/
int printf(const char *format, ...)
{
  char buffer[512];
  va_list arglist;
  register int result;
 
  va_start(arglist,format);
  result = vsprintf(buffer,format,arglist);
  va_end(arglist);
  vputs(buffer,FALSE);
  return result;
}
/*}}}  */
/*{{{  int puts(const char *str) - print string via VDI*/
int puts(const char *str)
{
  vputs((char *)str,TRUE);
  return '\n';
}
/*}}}  */
/*{{{  int fputs(const char *s, FILE *fp) - replacement for fputs(s,stdout)*/
int fputs(const char *s, FILE *fp)
{
  if (fp != stdout)
    return puts("fputs - internal error #4711");
  vputs((char *)s,FALSE);
  return '\n';
}
/*}}}  */
/*{{{  init_terminal*/
/* only a poor initialization, but I am still working */
PUBLIC void init_terminal(void)
{
}
/*}}}  */
/*{{{  reset_terminal()*/
PUBLIC void reset_terminal(void)
{
}
/*}}}  */
