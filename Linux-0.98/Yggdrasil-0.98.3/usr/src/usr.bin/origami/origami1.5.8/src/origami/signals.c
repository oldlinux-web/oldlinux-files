/*{{{  #includes*/
#include <sys/types.h>
#ifdef LINUX
#include <termios.h>
#else
#include <sgtty.h>
#endif
#include <pwd.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
 
#include <local/bool.h>
 
#define SIGNALS_C
 
#include "origami.h"
/*}}}  */
 
char general_abort_key = -1;
PRIVATE bool alarm_running=FALSE;
int alarm_time = -1;
 
/*{{{  got_alarm*/
#ifdef STD_C
void got_alarm(void)
#else
void got_alarm()
#endif
{
  signal(SIGALRM,SIG_IGN);
  do_auto_save= (auto_save && alarm_running);
  interrupt_restore=TRUE;
  signal(SIGALRM,got_alarm);
}
/*}}}  */
/*{{{  set_alarm*/
#ifdef STD_C
void set_alarm(int delay)
#else
void set_alarm(delay)
int delay;
#endif
{
  alarm_running=TRUE;
  if (delay!=0) alarm_time=delay;
  if (alarm_time<MIN_ALARMTIME || alarm_time>MAX_ALARMTIME)
    alarm_time=NORM_ALARMTIME;
  alarm(alarm_time*60);
}
/*}}}  */
/*{{{  reset_alarm*/
#ifdef STD_C
void reset_alarm()
#else
void reset_alarm()
#endif
{
  alarm_running=FALSE;
  alarm(0);
}
/*}}}  */
 
/*{{{  enable_sig_abort*/
#ifdef STD_C
void enable_sig_abort()
#else
void enable_sig_abort()
#endif
{
  if (general_abort_key== -1) return;
  signal(SIGINT, ori_abort);
}
/*}}}  */
/*{{{  disable_sig_abort*/
#ifdef STD_C
void disable_sig_abort(void)
#else
void disable_sig_abort()
#endif
{
  signal(SIGINT, SIG_IGN);
}
/*}}}  */
 
#ifdef SIGWINCH
/*{{{  window*/
PRIVATE void window()
{
  signal(SIGWINCH,SIG_IGN);
  win_changed=TRUE;
  signal(SIGWINCH,window);
}
/*}}}  */
#endif
 
/*{{{  bktoshell           suspend Origami and wait to wake up*/
#ifdef STD_C
void bktoshell(void)
#else
void bktoshell()
#endif
{
#  ifdef _POSIX_JOB_CONTROL
    int pid;
 
    message("suspended",TRUE);
    gotoxy(1, screen_lines);
    putchar('\n');
    fflush(stdout);
    reset_keyboard();
    pid = getpid();
    kill(pid,SIGTSTP);
#  else
    message("no job control",TRUE);
#  endif
}
/*}}}  */
/*{{{  rtfrmshell          wake origami up*/
#ifdef _POSIX_JOB_CONTROL
PRIVATE void rtfrmshell()
{
  init_keyboard();
  gotoxy(1, 1);
  ClrFullScr();
  restore(1);
  title_op(SHOW);
}
#endif
/*}}}  */

/*{{{  send me a mail concerning dumped file*/
#ifdef STD_C
PRIVATE void mail_me(char *name)
#else
PRIVATE void mail_me(name) char *name;
#endif
{
#  ifndef TOS
    /*{{{  variable declarations*/
    struct passwd *pw_entry;
    FILE *mail;
    char cmd[80], wdbuf[_POSIX_PATH_MAX+1], *wd=getcwd(wdbuf, _POSIX_PATH_MAX);
    /*}}}  */
 
    if ((wd=getcwd(wdbuf,_POSIX_PATH_MAX))!=NULL && (pw_entry=getpwuid(getuid()))!=NULL)
    {
      /*{{{  build command: mail username -s subject*/
      sprintf(cmd,"mail -s \"%s\" %s",M_MAIL_S,pw_entry->pw_name);
      /*}}}  */
      mail=popen(cmd,"w");
      if (mail!=NULL)
      /*{{{  send mail*/
      {
        fprintf(mail,"%s\n\n  %s/%s\n\n%s\n", M_MAIL_1, wd,name, M_MAIL_2);
        pclose(mail);
      }
      /*}}}  */
    }
#  endif
}
/*}}}  */
 
/*{{{  exit_origami*/
#ifdef STD_C
void exit_origami(int c)
#else
void exit_origami(c) int c;
#endif
{
  reset_keyboard();
  putchar('\n');
  exit(c);
}
/*}}}  */

/*{{{  die_a_quick_death*/
#ifdef STD_C
PRIVATE int die_a_quick_death(int sig,int code)
#else
int die_a_quick_death(sig,code) int sig,code;
#endif
{
  reset_keyboard();
  exit (1);
}
/*}}}  */
/*{{{  die_a_slow_death*/
#ifdef STD_C
PRIVATE int die_a_slow_death(int sig,int code)
#else
int die_a_slow_death(sig,code)
int sig,code;
#endif
{
  char name[_POSIX_PATH_MAX];
 
  if (file_changed) mail_me(dump_file(name));
  reset_keyboard();
  exit (1);
}
/*}}}  */
 
/*{{{  init_signals()*/
#ifdef STD_C
void init_signals(void)
#else
void init_signals()
#endif
{
  signal(SIGTERM,die_a_slow_death);
  signal(SIGHUP,die_a_slow_death);
  signal(SIGQUIT,die_a_slow_death);
  signal(SIGALRM,got_alarm);
#  ifdef SIGWINCH
  signal(SIGWINCH,window);
#  endif
  /*{{{  jobcontrol*/
#      ifdef _POSIX_JOB_CONTROL
  signal(SIGTSTP,SIG_DFL);      /* set signals so that we can */
  signal(SIGCONT,rtfrmshell);   /* suspend & restart Origami */
#      endif
  /*}}}  */
  disable_abort();
}
/*}}}  */
