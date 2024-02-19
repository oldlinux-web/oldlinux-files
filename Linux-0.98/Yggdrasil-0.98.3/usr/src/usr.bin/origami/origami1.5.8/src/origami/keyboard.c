/*{{{  #includes*/
#include <sys/types.h>
#ifdef LINUX
#include <termios.h>
#else
#include <sgtty.h>
#endif
#include <limits.h>
#include <signal.h>
#include <stdio.h>

#include <local/bool.h>

#define KEYBOARD_C

#include "origami.h"
/*}}}  */
 
/*{{{  variables*/
PRIVATE struct sgttyb new,old;
PRIVATE struct tchars torig_chars, twork_chars;
#ifndef NOLTCHARS
PRIVATE struct ltchars ltorig_chars, ltwork_chars;
#endif
 
PRIVATE bool echo;
bool echo_disabled=FALSE;
PRIVATE char command[PROMPT_LEN+1];
/*}}}  */
 
/*{{{  enable_echo*/
#ifdef STD_C
void enable_echo(bool yes)
#else
void enable_echo(yes) bool yes;
#endif
{
  echo=(!echo_disabled && yes);
}
/*}}}  */
/*{{{  switch_echo*/
#ifdef STD_C
void switch_echo(bool on)
#else
void switch_echo(on)
bool on;
#endif
{
  echo_disabled=!(echo=on);
}
/*}}}  */
 
/*{{{  enable_abort*/
#ifdef STD_C
void enable_abort(void)
#else
void enable_abort()
#endif
{
  if (general_abort_key == -1) return;
  signal(SIGINT, ori_abort);
  twork_chars.t_intrc=general_abort_key;
  ioctl(fileno(stdin), TIOCSETC, &twork_chars);
}
/*}}}  */
/*{{{  disable_abort*/
#ifdef STD_C
void disable_abort(void)
#else
void disable_abort()
#endif
{
  signal(SIGINT, SIG_IGN);
  twork_chars.t_intrc= -1;
  ioctl(fileno(stdin), TIOCSETC, &twork_chars);
}
/*}}}  */
/*{{{  set_abort*/
#ifdef STD_C
void set_abort(char key)
#else
void set_abort(key) char key;
#endif
{
  general_abort_key=key;
}
/*}}}  */

/*{{{  init_keyboard*/
#ifdef STD_C
void init_keyboard(void)
#else
void init_keyboard()
#endif
{
  /*{{{  get old mode and set new mode*/
  /* TIOCGETP = terminal input/output control get parameters */
  ioctl(fileno(stdin), TIOCGETP, &old);
  new=old;
  new.sg_flags|=CBREAK;
  new.sg_flags&=~(ECHO|CRMOD);
  ioctl(fileno(stdin), TIOCSETP, &new);
  /*}}}  */
  /*{{{  tchars*/
  ioctl(fileno(stdin), TIOCGETC, &torig_chars);
  /* portable way to disable all keys */
  memset(&twork_chars, -1, sizeof(struct tchars));
  ioctl(fileno(stdin), TIOCSETC, &twork_chars);
  /*}}}  */
#  ifndef NOLTCHARS
  /*{{{  ltchars*/
  ioctl(fileno(stdin), TIOCGLTC, &ltorig_chars);
  memset(&ltwork_chars, -1, sizeof(struct ltchars));
  ioctl(fileno(stdin), TIOCSLTC, &ltwork_chars);
  /*}}}  */
#  endif
  disable_abort();
  enable_echo(!echo_disabled);
}
/*}}}  */
/*{{{  reset_keyboard*/
#ifdef STD_C
void reset_keyboard(void)
#else
void reset_keyboard()
#endif
{
  ioctl(fileno(stdin), TIOCSETP, &old);
  ioctl(fileno(stdin), TIOCSETC, &torig_chars);
#  ifndef NOLTCHARS
  ioctl(fileno(stdin), TIOCSLTC, &ltorig_chars);
#  endif
}
/*}}}  */
 
/*{{{  get_key*/
#ifdef STD_C
TOKEN get_key (void)
#else
TOKEN get_key ()
#endif
{
  /*{{{  variables*/
  int result=1;
  unsigned char ch=general_abort_key;
  int lg=0;
  int cp=0;
  /*}}}  */
 
  fflush(stdout);
  /*{{{  scan a keyboard-sequence 'til token is complete*/
  while (result==1 && !aborted) {
    read(fileno(stdin), &ch, 1);
    lg++;
    result=find_key(ch);
    /*{{{  echo-handling*/
    if (echo && result==1) {
      /*{{{  append code to string*/
      if (ch>=' ') {
        /*{{{  x*/
        command[cp++]=ch;
        command[cp++]=' ';
        /*}}}  */
      } else if (ch=='\033') {
        /*{{{  M-*/
        command[cp++]='M';
        command[cp++]='-';
        /*}}}  */
      } else if (ch>=0 && ch<' ') {
        /*{{{  C-x*/
        command[cp++]='C';
        command[cp++]='-';
        command[cp++]='@'+ch;
        command[cp++]=' ';
        /*}}}  */
      } else {
        /*{{{  ?*/
        command[cp++]='?';
        command[cp++]=' ';
        /*}}}  */
      }
      command[cp]='\0';
      /*}}}  */
      message(command,FALSE);
    }
    /*}}}  */
  }
  /*}}}  */
  if (result) return (result);
  else if (ch==general_abort_key) {
    aborted=TRUE;
    return(O_BREAK);
  } else if (lg==1 && ch>=' ' && (TOKEN) ch < O_NOP)
    return(ch);
  message(M_NOT_BOUND, TRUE);
  return(O_NOP);
}
/*}}}  */
