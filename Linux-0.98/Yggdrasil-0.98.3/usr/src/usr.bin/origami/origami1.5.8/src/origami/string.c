/*{{{  #includes*/
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
 
#include <local/bool.h>
 
#define STRING_C

#include "origami.h"
/*}}}  */
 
/*{{{  strrstr only a hack,suboptimal performance.Shouldn't it be in the library?*/
#ifdef STD_C
char *strrstr(char *s1,char *s2)
#else
char *strrstr(s1,s2) char *s1,*s2;
#endif
{
  char *f,*g;

  g=s1;
  while ((f=strstr(g,s2))!=NULL) g=f+1;
  return (g==s1 ? NULL : g-1);
}
/*}}}  */
/*{{{  strinsert*/
#ifdef STD_C
void strinsert(char *src,char *dst,int pos)
#else
void strinsert(src, dst, pos)
char *src, *dst;
int pos;
#endif /* STD_C */
{
  int slen, dlen;

  if (--pos < 0) return;
  dlen = strlen(dst);
  dst += dlen;
  dlen -= pos;
  if (dlen <= 0) {strcpy(dst, src);return;}
  slen = strlen(src);
  do {dst[slen] = *dst;--dst;} while (--dlen >= 0);
  dst++;
  while (--slen >= 0) *dst++ = *src++;
}
/*}}}  */
/*{{{  strpos2*/
#ifdef STD_C
int strpos2(char *s,char *pat,int pos)
#else
int strpos2(s, pat, pos)
char *s, *pat;
int pos;
#endif /* STD_C */
{
  char *cp, ch;
  int slen;

  if (--pos < 0) return 0;
  slen = strlen(s) - pos;
  cp = s + pos;
  if (!(ch = *pat++)) return 0;
  pos = strlen(pat);
  slen -= pos;
  while (--slen>=0) if (*cp++ == ch && !strncmp(cp, pat, pos)) return cp - s;
  return 0;
}
/*}}}  */
/*{{{  strsub*/
#ifdef STD_C
char *strsub(char *ret,char *s,int pos,int len)
#else
char *strsub(ret, s, pos, len)
char *ret, *s;
int pos, len;
#endif /* STD_C */
{
  char *s2;

  if (--pos < 0 || len <= 0) {*ret = 0;return ret;}
  while (pos > 0) {
    if (!*s++) {*ret = 0;return ret;}
    pos--;
  }
  s2 = ret;
  while (--len >= 0)
    if (!(*s2++ = *s++)) return ret;
  *s2 = 0;
  return ret;
}
/*}}}  */
/*{{{  proc_replace*/
#ifdef STD_C
int proc_replace(char *search, char *replace, char *line, int position)
#else
int proc_replace(search, replace, line, position)
char *search, *replace, *line;
int position;
#endif
{
  int ls, ll, i;
  int count=0;
  bool rep;
  char STR[MAX_FIELD_SIZE + 1];
 
  ls = strlen(search);
  ll = strlen(line);
  if (ll < position + ls - 1) return(count);
  rep = TRUE;
  for (i = 1; i <= ls; i++)
    rep = (rep && line[position + i - 2] == search[i - 1]);
  if (rep) {
    strcpy(STR,line);
    strcpy(STR+position-1,replace);
    strcat(STR,line+position+ls-1);
    strcpy(line, STR);
    count++;
  }
  return(count);
}
/*}}}  */
