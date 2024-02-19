#include <stdio.h>

/*{{{  void putw(int w, FILE *s)*/
#ifdef STD_C
void putw(int w, FILE *s)
#else
putw(w,s) int w; FILE *s;
#endif
{
  putc(w&0xff,s);
  putc(w>>8,s);
}
/*}}}  */
/*{{{  int getw(FILE *s)*/
#ifdef STD_C
int getw(FILE *s)
#else
getw(s) FILE *s;
#endif
{
  int w;

  w=(getc(s)&0xff);
  w|=(getc(s)<<8);
  return w;
}
/*}}}  */
