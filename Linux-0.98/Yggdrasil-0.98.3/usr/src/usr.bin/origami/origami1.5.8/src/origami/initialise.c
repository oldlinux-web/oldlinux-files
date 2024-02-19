/*{{{  #includes*/
#include <signal.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <local/bool.h>

#define INITIALIZE_C

#include "origami.h"
#include "../common/rcformat.h"
/*}}}  */
 
/*{{{  variables*/
char mark_mode[mark_name_lg];
PRIVATE FILE *rcfile;
/*}}}  */
 
/*{{{  parse rcfile*/
#ifdef STD_C
PRIVATE bool rc_file_parser(void)
#else
PRIVATE bool rc_file_parser()
#endif
{
  KEY *top_level=NULL;
  int  macro,lg;
  int  input;
 
  macro_int=NULL;
  if (auto_macro) auto_macro=0; else auto_macro = -1;
  while ((input=getc(rcfile))!=RC_ENDE) {
    switch (input) {
      /*{{{  keydef*/
      case RC_DEFKEY: {
        int nodes;
        KEY *tp;
        int of;
      
        /*{{{  already defined?*/
        if (top_level!=NULL) return(TRUE);
        /*}}}  */
        /*{{{  get space for table*/
        nodes=getw(rcfile);
        if ((top_level=tp=malloc(nodes*sizeof(KEY)))==NULL) {
          printf("%s\n",M_NO_MEMORY);
          return(TRUE);
        }
        /*}}}  */
        /*{{{  read all nodes*/
        while (nodes--) {
          tp->code=getw(rcfile);
          tp->key=getc(rcfile);
          /*{{{  read offsets and buildt pointers*/
          of=getw(rcfile);
          if (of) tp->next=tp+of; else tp->next=NULL;
          of=getw(rcfile);
          if (of) tp->n_level=tp+of; else tp->n_level=NULL;
          tp++;
          /*}}}  */
        }
        /*}}}  */
        set_key_top(top_level);
        break;
      }
      /*}}}  */
      /*{{{  macrodef/init*/
      case RC_INITMACRO:
      case RC_DEFMACRO: {
        TOKEN *x;
      
        macro=getw(rcfile);
        lg=getw(rcfile);
        /*{{{  Errors*/
        if (macro<1 || macro>=MACRO_COUNT || !fix_macros[macro].defineable
            || (fix_macros[macro].string!=NULL)) return(TRUE);
        /*}}}  */
        fix_macros[macro].defineable=(input==RC_INITMACRO);
        if ((x=fix_macros[macro].string=malloc((1+lg)*sizeof(TOKEN)))==NULL) {
          printf("%s\n",M_NO_MEMORY);
          return(TRUE);
        }
        while (lg--) *x++=getw(rcfile);
        *x=M_END_MACRO;
        break;
      }
      /*}}}  */
      /*{{{  keyname*/
      case RC_KEYNAME: {
        int i=keyn_lg;
        char *n=keybinding_name;
      
        while (i--) *n++=getc(rcfile);
        *n='\0';
        strcat(keybinding_name,",");
        break;
      }
      /*}}}  */
      /*{{{  ints*/
      case RC_INTS:
        input=getw(rcfile);
        if (macro_int!=NULL) return(TRUE);
        if (NULL==(macro_int=calloc(input,sizeof(int)))) {
          printf("%s\n",M_NO_MEMORY);
          return(TRUE);
        }
        break;
      /*}}}  */
      /*{{{  auto_macro*/
      case RC_AUTOM:
        input=getc(rcfile);
        if (auto_macro==0) auto_macro=input;
        break;
      /*}}}  */
      /*{{{  break*/
      case RC_BREAK: {
        set_abort(getc(rcfile));
        break;
      }
      /*}}}  */
      /*{{{  marks*/
      case RC_MARKS: {
        char name[mark_name_lg];
        int i;
      
        /*{{{  read stored name*/
        for (i=0;i<(mark_name_lg-1);i++)
          name[i]=getc(rcfile);
        name[mark_name_lg-1]='\0';
        /*}}}  */
        if (!strcmp(mark_mode,name)) {
          /*{{{  reset mark-strings*/
          *mark_mode='\0';
          for (i=0;i<(tag_length-1);i++) fold_open_str[i]=getc(rcfile);
          for (i=0;i<(tag_length-1);i++) fold_line_str[i]=getc(rcfile);
          for (i=0;i<(tag_length-1);i++) fold_file_str[i]=getc(rcfile);
          for (i=0;i<(tag_length-1);i++) fold_close_str[i]=getc(rcfile);
          /*}}}  */
        } else
          /*{{{  overread the data*/
          for (i=0;i<(4*(tag_length-1));i++,getc(rcfile));
          /*}}}  */
        break;
      }
      /*}}}  */
      /*{{{  error!*/
      default: return(TRUE);
      /*}}}  */
    }
  }
  /*{{{  if marks needed and not found -> error*/
  if (*mark_mode) {
    printf(M_NO_MARKS);
    return(TRUE);
  }
  /*}}}  */
  /*{{{  no needed auto-macro given?*/
  if (!auto_macro) {
    printf(M_NO_AUTO);
    return(TRUE);
  }
  /*}}}  */
  return(FALSE);
}
/*}}}  */
/*{{{  HostBegin*/
#ifdef STD_C
int HostBegin(void)
#else
int HostBegin()
#endif /* STD_C */
{
  char keyfile[_POSIX_PATH_MAX+1];
 
#  ifndef TOS
  if (getenv("HOME")==NULL) {
    printf (M_NO_HOME);
    return (1);
  }
#  endif
  /*{{{  parse rc*/
  /*{{{  open rc-file*/
  if ((rcfile = fopen(completebase(keyfile,"rc"),READ_BIN)) == 0) {
    printf (F_UNABLE,keyfile);
    return (1);
  }
  /*}}}  */
  if (rc_file_parser()) {
    printf(M_NO_KTAB);
    return(1);
  }
  fclose(rcfile);
  /*}}}  */
  if (get_terminal_capability()) return(1);
  init_keyboard();
  init_signals();
  full_window();
  ClrScr();
  return (0);
}
/*}}}  */
/*{{{  initialise links in editor*/
#ifdef STD_C
void    initialise(void)
#else
void    initialise()
#endif
{
  element        *WITH;
 
  WITH = null_item_ptr = dispose_tail = dispose_head = malloc(sizeof(element));
  WITH->strng = NULL;
  WITH->selected = FALSE;
  WITH->foldline = NOT_FOLD;
  WITH->UU.U1.indent = 0;
  WITH->UU.U1.fold_indent = 0;
  WITH->fold = NULL;
  WITH->UU.U1.fold_type = 0;
  WITH->UU.U1.fold_contents = 1;
  WITH->fold_close_line = 4;
  real_tail = real_head;
  proc_new(&new_file_ptr);
  proc_new(&pick_head);
  pick_tail = pick_head;
  ClrFullScr();
  if (overwrite) title_op(SET_OVER);
}
/*}}}  */
