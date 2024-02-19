/*{{{  #includes*/
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <local/bool.h>
#include <local/argparse.h>

#define MAIN_C

#include "origami.h"
/*}}}  */

/*{{{  variables*/
PRIVATE int gotoarg;
char keybase[_POSIX_PATH_MAX+1];
char *filearg;
bool c_hash_front=FALSE,
     split_line,
     auto_save=FALSE,
     do_auto_save=FALSE,
     verbose=FALSE,
     aborted,
#     ifdef SIGWINCH
       win_changed=FALSE,
#     endif
     interrupt_restore=FALSE;

int current_ind,f_begin=0, f_end=0, f_current=0;

element *null_item_ptr, *new_file_ptr;
/*}}}  */
/*{{{  language data*/
language_comment dialect_start = {
  "", "--", "(*", "/*", ";", ".\\\"", "C", "   ", "tds"
};

language_comment dialect_end = {
  "", "", "*)", "*/", ";", "", "", "   ", ""
};

char *language_string[NO_LANGUAGES + 1] = {
  "None", "Occam", "Pascal", "C", "Lisp", "Roff",
  "Fortran", "User", "Tds"
};
/*}}}  */
/*{{{  fold-marks*/
char fold_open_str[tag_length] = begin_standard;
char fold_close_str[tag_length] = end_standard;
char fold_file_str[tag_length] = file_standard;
char fold_line_str[tag_length] = line_standard;
/*}}}  */
/*{{{  argtab*/
#define argnum 14
 
PRIVATE ARG argtab[argnum] =
{
  's', INT_ARG, &m_s_depth,
  'l', INT_ARG, &gotoarg,
  'k', STRING_ARG, keybase,
  'n', BOOL_ARG, &fold_in,
  'v', BOOL_ARG, &browse_mode,
  'p', BOOL_ARG, &c_hash_front,
  'm', STRING_ARG, mark_mode,
  '?', BOOL_ARG, &aborted,
  'h', BOOL_ARG, &aborted,
  'e', BOOL_ARG, &echo_disabled,
  'o', BOOL_ARG, &overwrite,
  'a', INT_ARG, &alarm_time,
  'V', BOOL_ARG, &verbose,
  'A', BOOL_ARG, &auto_macro
};
/*}}}  */

/*{{{  main*/
#ifdef STD_C
void main(argc, argv) int argc; char *argv[];
#else
main(argc, argv) int argc; char *argv[];
#endif
{
#  ifdef TOS
    st_init();
#  endif
#  ifndef NO_MSG
    printf("%s\n,%s\n,%s\n",
    "originally by:- Martin Green, Ferranti Ltd, Cheadle Heath, Stockport, England",
    "Converted from Pascal and modified by Charlie Lawrence, Eastman Kodak Co.",
    "Emacs-mode and OCL added by Michael Haardt and Wolfgang Stumvoll, Aachen");
    sleep(1);
#  endif
  m_s_depth=M_DEPTH;
  strcpy(keybase,base_name);
  argc=argparse(argc,argv,argtab,argnum);
  /*{{{  auto-save started?*/
  if (alarm_time!=-1) {
    auto_save=TRUE;
    if (alarm_time<MIN_ALARMTIME || alarm_time>MAX_ALARMTIME)
      alarm_time=NORM_ALARMTIME;
  } else
    alarm_time=NORM_ALARMTIME;
  /*}}}  */
  /*{{{  errorcases in args*/
  if (m_s_depth<=0) {
    fprintf(stderr,M_ERR_ARG);
    exit(1);
  }
  if (argc>=2) {
    filearg = argv[1];
    f_begin=1;
    f_end=argc-1;
    f_current=f_begin;
  } else if (argc!=1 || aborted) {
    if (!aborted) fprintf(stderr,M_ERR_ARG);
    fprintf(stderr,"usage: origami [-ehopv?AV] [-a<auto-save-delay>] ");
    fprintf(stderr,"[-k<namebase>] [-l<number>] [-m<markname>] ");
    fprintf(stderr,"[-s<depth>] [filename] ...\n");
    exit(1);
  }
  /*}}}  */
  if (HostBegin()) exit(1);
  init_terminal();
  main_editor_loop(argv,gotoarg);
  reset_terminal();
  exit_origami(0);
}
/*}}}  */
