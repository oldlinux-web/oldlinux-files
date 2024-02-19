/*{{{  #includes*/
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include <local/bool.h>

#define PROMPT_C

#include "origami.h"
 
/*}}}  */

PRIVATE bool screen_message = TRUE;
 
/*{{{  message               put a message under statusline*/
#ifdef STD_C
void message(char *s, bool  brackets)
#else
void message(s, brackets)
char *s; bool brackets;
#endif
{
  gotoxy(1, MESSAGE_LINE);
  printf(brackets ? "[%s]" : "%s",s);
  clreol();
  fflush(stdout);
  screen_message = TRUE;
}
/*}}}  */
/*{{{  vmessage               maybe put a message under statusline*/
#ifdef STD_C
void vmessage(char *s)
#else
void vmessage(s)
char *s;
#endif
{
  if (verbose) {
    gotoxy(1, MESSAGE_LINE);
    printf("[%s]",s);
    clreol();
    fflush(stdout);
    screen_message = TRUE;
  }
}
/*}}}  */
/*{{{  err_message           put a message under statusline*/
#ifdef STD_C
void err_message(char *s,char *p, bool  brackets)
#else
void err_message(s,p, brackets)
char *s,*p; bool brackets;
#endif
{
  gotoxy(1, MESSAGE_LINE);
  printf(brackets ? "[%s %s: %s]" : "%s %s: %s",s,p,strerror(errno));
  clreol();
  fflush(stdout);
  screen_message = TRUE;
}
/*}}}  */
/*{{{  no_message            remove message and maybe put define_macro*/
#ifdef STD_C
void no_message(void)
#else
void no_message()
#endif
{
  if (screen_message) {
    gotoxy(1, MESSAGE_LINE);
    clreol();
    screen_message = FALSE;
  }
}
/*}}}  */
/*{{{  readprompt            read a string with prompt*/
#ifdef STD_C
char *readprompt(char *Result, char *prompt, int max)
#else
char *readprompt(Result, prompt, max) char *Result, *prompt; int max;
#endif
{
  char *s=Result;
  char STR1[PROMPT_LEN+3];
  TOKEN ch;
  bool running = TRUE;
  int lg=0;
 
  sprintf(STR1, "%s ? ", prompt);
  lg=strlen(STR1);
  message(STR1,FALSE);
  *s = '\0';
 
  do {
    ch = hide_key(FALSE);
    if (ch < O_NOP) {
      /*{{{  ascii*/
      gotoxy(++lg,MESSAGE_LINE);
      putchar(ch);
      *(s++)=(char) ch;
      *s='\0';
      /*}}}  */
    } else if (ch == O_DELETE) {
      /*{{{  delete*/
      if (s!=Result) {
        *(--s) = '\0';
        gotoxy(--lg + 1,MESSAGE_LINE);
        putchar(' ');
        putchar('\b');
      }
      /*}}}  */
    } else {
      if (ch==O_BREAK) *Result='\0';
      running = FALSE;
    }
  } while (strlen(Result) < max && running);
  if (aborted) *Result='\0';
  return Result;
}
/*}}}  */
/*{{{  yes                   ask for boolean*/
#ifdef STD_C
bool yes(char *q)
#else
bool yes(q)
char *q;
#endif
{
  char temp[8];
  bool y, yes_or_no;
 
  y = FALSE;
  yes_or_no = FALSE;
  do {
    readprompt(temp, q, 6);
    if (*temp != '\0') {
      temp[0] = toupper(temp[0]);
      y = (temp[0] == YES);
      yes_or_no = (y || temp[0] == NO);
    }
  } while (!yes_or_no && !aborted);
  return y;
}
/*}}}  */
/*{{{  help*/
#ifdef STD_C
bool help(bool h)
#else
bool help(h) bool h;
#endif
{
  char helpfile[_POSIX_PATH_MAX+1], line[80];
  TOKEN ch=' ';
  FILE *fp;
  int li;
 
  /*{{{  open file*/
  if ((fp=fopen(h ? completebase(helpfile,"help") : completebase(helpfile,"bind"),READ))==NULL)
  {
    err_message(h ? M_NO_HELP : M_NO_BIND, helpfile, TRUE);
    return FALSE;
  }
  /*}}}  */
  li=1;
  while (fgets(line,80,fp))
  {
    gotoxy(1,li); clreol(); fputs(line,stdout);
    if (li++==SCREEN_LEN)
    {
      li=1;
      message(M_ANY_Q,TRUE);
      ch=hide_key(FALSE);
    }
    if (aborted || ((ch=tolower(ch))=='q')) break;
  }
  if (ch!='q' && li>1)
  {
    while (li<=SCREEN_LEN)
    {
      gotoxy(1,li++);
      clreol();
    }
    message(M_ANY,TRUE); hide_key(FALSE);
  }
  if (fclose(fp)<0) err_message(M_CLOSE_FAILED,helpfile,TRUE);
  return TRUE;
}
/*}}}  */
