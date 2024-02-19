/*{{{  #includes*/
#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <local/bool.h>

#define MISC_C

#include "origami.h"
#include "macros.h"
/*}}}  */

char base_name[]=".origami";

/*{{{  completebase*/
#ifdef STD_C
char *completebase(char *s, char *t)
#else
char *completebase(s,t) char *s, *t;
#endif
{
  char *h=getenv("HOME");
 
  if (h==NULL) *s='\0';
  else
  {
    strcpy(s,h);
    strcat(s,PATH_SEP);
  }
  strcat(s,keybase);
  strcat(s,t);
  return(s);
}
/*}}}  */
/*{{{  fileprompt*/
#ifdef STD_C
char *fileprompt(char n[_POSIX_PATH_MAX+1])
#else
char *fileprompt(n) char n[_POSIX_PATH_MAX+1];
#endif
{
  readprompt(n,M_FILENAME,_POSIX_PATH_MAX);
  return(n);
}
/*}}}  */
 
/*{{{  environment-variables*/
#ifdef STD_C
void put_vars(int cursor_x_pos)
#else
void put_vars(cursor_x_pos)
int cursor_x_pos;
#endif
{
  static char envfile[_POSIX_PATH_MAX+10];
  static char envword[MAX_FIELD_SIZE+10];
  static char envline[MAX_FIELD_SIZE+10];
  char x[MAX_FIELD_SIZE+1];
  char fn[_POSIX_PATH_MAX+1];
  char *s,*w;
 
  /*{{{  filename*/
  if (real_head==real_tail)
    *fn='\0';
  else
    line_of(fn,real_tail);
  sprintf(envfile,"ORIFILE=%s",fn);
  putenv(envfile);
  /*}}}  */
  /*{{{  line*/
  copyin(x,current,FALSE);
  sprintf(envline,"ORILINE=%s",x);
  putenv(envline);
  /*}}}  */
  /*{{{  word*/
  s=x+cursor_x_pos-1;
  if (*s && (isalpha(*s) || *s=='_')) {
    /*{{{  expand*/
    while (s!=x && (isalpha(*(s-1)) || isdigit(*(s-1)) || *(s-1)=='_')) s--;
    w=s;
    while (*w && (isalpha(*w) || isdigit(*w) || *w=='_')) w++;
    *w='\0';
    /*}}}  */
  } else
    *s='\0';
  sprintf(envword,"ORIWORD=%s",s);
  putenv(envword);
  /*}}}  */
}
/*}}}  */
