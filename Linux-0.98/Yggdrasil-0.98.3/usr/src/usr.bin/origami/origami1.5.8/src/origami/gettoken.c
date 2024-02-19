/*{{{  #includes*/
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include <local/bool.h>

#define GETTK_C

#include "origami.h"
/*}}}  */

/*{{{  vars*/
PRIVATE TOKEN **macro_stack,
             *macro_pos;
PRIVATE bool defining_macro=FALSE;
PRIVATE bool do_macro=TRUE,
             do_arg=TRUE;
PRIVATE int executing_macro=0;
PRIVATE int argument=0;
bool macro_tag;
char user_mode[user_mode_lg+1];
int m_s_depth,
    *macro_int,
    auto_macro=(int) FALSE;
TOKEN macro_string[32];
TOKEN keyboard_macro[MAX_MACRO_LENGTH];
/*{{{  macro_entry fix_macros[MACRO_COUNT+1]=*/
macro_entry fix_macros[MACRO_COUNT+1]={
        {TRUE,keyboard_macro},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,NULL},
        {TRUE,macro_string}
};
/*}}}  */
/*}}}  */
 
/*{{{  ori_abort             set tags for signal interrupt*/
#ifdef STD_C
void ori_abort(void)
#else
void ori_abort()
#endif
{
  signal(SIGINT,SIG_IGN);
  do_arg=do_macro=FALSE;
  interrupt_restore=aborted=TRUE;
  if (argument) argument=0;
  signal(SIGINT,ori_abort);
}
/*}}}  */
/*{{{  stop_macros           stop macro-execution at correct point*/
#ifdef STD_C
void stop_macros(void)
#else
void stop_macros()
#endif
{
  macro_pos=macro_stack[executing_macro=0];
  if (!argument) disable_abort();
  title_op(RESET_EM);
}
/*}}}  */
/*{{{  get_arg               get a int-Argument from macro-buffer*/
#ifdef STD_C
TOKEN get_arg(void)
#else
TOKEN get_arg()
#endif
{
  return(*macro_pos++);
}
/*}}}  */
/*{{{  push_macro*/
#ifdef STD_C
PRIVATE bool push_macro(TOKEN *to)
#else
PRIVATE bool push_macro(to) TOKEN *to;
#endif
{
  /*{{{  stack-handling*/
  if (macro_stack==NULL) {
    macro_stack=malloc(m_s_depth*sizeof(TOKEN*));
    if (macro_stack==NULL) {
      message(M_NO_MEMORY,TRUE);
      return(TRUE);
    }
  } else if (executing_macro==m_s_depth) {
    stop_macros();
    message(M_STACK_OVER,TRUE);
    return(TRUE);
  }
  /*}}}  */
  macro_stack[executing_macro++] = macro_pos;
  macro_pos=to;
  return(FALSE);
}
/*}}}  */
/*{{{  call_auto_macro*/
void call_auto_macro()
{
  if (auto_macro<=0 || auto_macro>=MAX_MACRO) return;
  if (fix_macros[auto_macro].string != NULL) {
    push_macro(fix_macros[auto_macro].string);
    if (!argument && !executing_macro) enable_abort();
  }
  if (executing_macro==1) title_op(SET_EM);
}
/*}}}  */
/*{{{  prompt_string         generate the correct string from macro*/
#ifdef STD_C
void prompt_string(char *s)
#else
void prompt_string(s) char *s;
#endif
{
  char *x=s;
  TOKEN read;
 
  while ((read=get_arg())!=M_END_MACRO)
    if (read==M_INT_STRING) {
      sprintf(x,"%d",macro_int[get_arg()]);
      x= s+strlen(s);
    } else
      *x++ = (char) read;
  *x='\0';
}
/*}}}  */
/*{{{  single_key            get a token*/
#ifdef STD_C
PRIVATE TOKEN single_key(bool hide)
#else
PRIVATE TOKEN single_key(hide) bool hide;
#endif
{
  PRIVATE TOKEN rep_token=O_NOP;
  PRIVATE int macro_count;
  TOKEN ch;
 
  do {
    /*{{{  abort for bd systems*/
#    ifdef TOS
    abort_scanner();
#    endif
    /*}}}  */
    /*{{{  get next token*/
    if (executing_macro) {
      /*{{{  read a macro-token*/
      if (do_macro)
        ch = *macro_pos++;
      else {
        aborted=TRUE;
        stop_macros();
        ch=O_BREAK;
      }
      /*}}}  */
    } else {
      /*{{{  get token from keyboard*/
      if (argument)
        /*{{{  repeating*/
        if (do_arg) {
          ch=rep_token;
          if (!(--argument)) {
            title_op(RESET_ARG);
            disable_abort();
          }
        } else {
          ch=O_BREAK;
          title_op(RESET_ARG);
          argument=0;
          aborted=do_arg=TRUE;
        }
        /*}}}  */
      else {
        do_macro=TRUE;
        ch = get_key();
        /*{{{  handle O_REP_DEF*/
        switch (ch) {
          case O_REP_0:
          case O_REP_1:
          case O_REP_2:
          case O_REP_3:
          case O_REP_4:
          case O_REP_5:
          case O_REP_6:
          case O_REP_7:
          case O_REP_8:
          case O_REP_9: {
            enable_echo(FALSE);
            message(M_ARG,FALSE);
            ch=(argument=ch-O_REP_0)+'0';
            /*{{{  scan integer*/
            do  {
              putchar(ch);
              ch=get_key();
              if (ch<O_NOP && isdigit(ch)) argument=10*argument + ch - '0';
            } while (ch < O_NOP && isdigit(ch) && !aborted);
            /*}}}  */
            enable_echo(TRUE);
            /*{{{  error or abort*/
            if (ch>=O_REP_0 && ch<=O_REP_9) {
              argument=0;
              aborted=TRUE;
            }
            if (ch==O_BREAK || aborted) argument=0;
            /*}}}  */
            do_arg=TRUE;
            if (argument) {
              if (--argument) {
                title_op(SET_ARG);
                enable_abort();
                rep_token=ch;
              }
            } else
              rep_token=ch=O_BREAK;
          }
        }
        /*}}}  */
      }
      /*}}}  */
    }
    if (ch<' ')
      ch=O_NOP;
    /*}}}  */
    if (ch<O_EXE_MACRO || ch>O_CALL_STRING) {
      /*{{{  normal commands*/
      switch (ch) {
        /*{{{  M_STORE_LINE_NO*/
        case M_STORE_LINE_NO: {
          macro_int[get_arg()]=get_line_no();
          break;
        }
        /*}}}  */
        /*{{{  M_END_MACRO*/
        case M_END_MACRO: {
          macro_pos=macro_stack[--executing_macro];
          if (!executing_macro) {
            disable_abort();
            if (!hide) title_op(RESET_EM);
          }
          ch=(!executing_macro && !hide) ? O_PROMPT : O_NOP;
          break;
        }
        /*}}}  */
        /*{{{  M_CALL*/
        case M_CALL: {
          /*{{{  no macro => error*/
          if (!executing_macro) {
            message(M_NO_PROG,TRUE);
            ch=O_BREAK;
            break;
          }
          /*}}}  */
          ch = push_macro(macro_pos+get_arg()) ? O_BREAK : O_NOP;
          break;
        }
        /*}}}  */
        /*{{{  M_CHANGED*/
        case M_CHANGED: {
          macro_tag=file_changed;
          ch=O_NOP;
        }
        /*}}}  */
        /*{{{  M_JMP*/
        case M_JMP: {
          int diff;
        
          ch=O_NOP;
          if (!executing_macro) {
            message(M_NO_PROG,TRUE);
            ch=O_BREAK;
            break;
          }
          diff= get_arg();
          macro_pos=macro_pos+diff;
          break;
        }
        /*}}}  */
        /*{{{  M_JMP_FALSE*/
        case M_JMP_FALSE: {
          int diff;
        
          ch=O_NOP;
          if (!executing_macro) {
            message(M_NO_PROG,TRUE);
            ch=O_BREAK;
            break;
          }
          diff= get_arg();
          if (!macro_tag) macro_pos=macro_pos+diff;
          break;
        }
        /*}}}  */
        /*{{{  M_JMP_TRUE*/
        case M_JMP_TRUE: {
          int diff;
        
          ch=O_NOP;
          if (!executing_macro) {
            message(M_NO_PROG,TRUE);
            ch=O_BREAK;
            break;
          }
          diff= get_arg();
          if (macro_tag) macro_pos=macro_pos+diff;
          break;
        }
        /*}}}  */
        /*{{{  M_NOT*/
        case M_NOT: {
          ch=O_NOP;
          macro_tag=!macro_tag;
          break;
        }
        /*}}}  */
        /*{{{  M_LANGUAGE*/
        case M_LANGUAGE: {
          char x=get_arg();
        
          macro_tag=(toupper(*language_string[dialect])==toupper(x));
          ch=O_NOP;
          break;
        }
        /*}}}  */
        /*{{{  M_TEXTLINE*/
        case M_TEXTLINE: {
          ch=O_NOP;
          macro_tag=(current->foldline==NOT_FOLD);
          break;
        }
        /*}}}  */
        /*{{{  M_BEGIN_FOLD_COMMENT*/
        case M_BEGIN_FOLD_COMMENT: {
          ch=O_NOP;
          macro_tag = (current->foldline==START_OPEN_FOLD);
          break;
        }
        /*}}}  */
        /*{{{  M_CLOSED_FOLD*/
        case M_CLOSED_FOLD: {
          ch=O_NOP;
          macro_tag = (current->foldline==START_FOLD);
          break;
        }
        /*}}}  */
        /*{{{  M_END_FOLD_COMMENT*/
        case M_END_FOLD_COMMENT: {
          ch=O_NOP;
          macro_tag=(current->foldline==END_FOLD);
          break;
        }
        /*}}}  */
        /*{{{  M_FILED_FOLD*/
        case M_FILED_FOLD: {
          ch=O_NOP;
          macro_tag=(current->foldline==START_FILED);
          break;
        }
        /*}}}  */
        /*{{{  M_SET_COUNTER*/
        case M_SET_COUNTER: {
          int x=get_arg();
        
          ch=O_NOP;
          macro_int[x]=get_arg();
          break;
        }
        /*}}}  */
        /*{{{  M_INT_STRING*/
        case M_INT_STRING: {
          char st[32];
          char *s=st;
          TOKEN *t=macro_string;
        
          ch=O_NOP;
          sprintf(st,"%d",macro_int[get_arg()]);
          while (*s) *t++ = (TOKEN) *s++;
          *t = M_END_MACRO;
          break;
        }
        /*}}}  */
        /*{{{  M_READ_REPEAT*/
        case M_READ_REPEAT: {
          ch=O_NOP;
          macro_int[get_arg()]=argument;
          argument=0;
          do_arg=TRUE;
          break;
        }
        /*}}}  */
        /*{{{  M_NULL_COUNTER*/
        case M_NULL_COUNTER: {
          int x=get_arg();
        
          ch=O_NOP;
          macro_tag=(macro_int[x]==0);
          break;
        }
        /*}}}  */
        /*{{{  M_PROMPT*/
        case M_PROMPT: {
          int counter=get_arg();
          char string[PROMPT_LEN+1];
          char *s=string;
          TOKEN x;
        
          prompt_string(string);
          message(string,FALSE);
          s=string;
          /*{{{  read the string direct from keyboard*/
          disable_abort();
          enable_echo(FALSE);
          while (!aborted && (x=get_key())!=O_RETURN)
            if (x<O_NOP && (isdigit((char) x) || isalpha((char) x)))
              printf("%c",*s++ = (char) x);
            else if ((x==O_DELETE || x==O_DEL_CHAR_R) && s!=string) {
              /*{{{  delete*/
              putchar('\b'); putchar(' '); putchar('\b');
              *--s ='\0';
              /*}}}  */
            }
          *s=0;
          enable_echo(TRUE);
          enable_abort();
          /*}}}  */
          if (aborted) stop_macros();
          if (counter>=0)
            if (toupper(*string)==YES)
              macro_int[counter]=1;
            else if (toupper(*string)==NO)
              macro_int[counter]=0;
            else
              macro_int[counter]=atoi(string);
          ch=O_BREAK;
          break;
        }
        /*}}}  */
        /*{{{  M_POSITIV_COUNTER*/
        case M_POSITIV_COUNTER: {
          int x=get_arg();
        
          ch=O_NOP;
          macro_tag=(macro_int[x]>0);
          break;
        }
        /*}}}  */
        /*{{{  M_ADD_COUNTER*/
        case M_ADD_COUNTER: {
          int x=get_arg();
        
          ch=O_NOP;
          macro_int[x]=macro_int[x]+ get_arg();
          break;
        }
        /*}}}  */
        /*{{{  M_INV_COUNTER*/
        case M_INV_COUNTER: {
          int x=get_arg();
        
          ch=O_NOP;
          macro_int[x] = -macro_int[x];
          break;
        }
        /*}}}  */
        /*{{{  M_SUM_COUNTER*/
        case M_SUM_COUNTER: {
          int x=get_arg();
        
          ch=O_NOP;
          macro_int[x] += macro_int[get_arg()];
          break;
        }
        /*}}}  */
        /*{{{  O_SET_USER_MODE*/
        case O_SET_USER_MODE: {
          if (!hide) {
            *user_mode=',';
            readprompt(user_mode+1,M_WHICH_MODE,user_mode_lg);
          }
          ch=O_BREAK;
          break;
        }
        /*}}}  */
        /*{{{  O_RESET_USER_MODE*/
        case O_RESET_USER_MODE: {
          ch=O_BREAK;
          if (!hide) {
            *user_mode='\0';
            title_op(SHOW);
          }
          break;
        }
        /*}}}  */
        /*{{{  O_DEF_MACRO*/
        case O_DEF_MACRO: {
          if (hide)
            ch=O_BREAK;
          else {
            /*{{{  executing-macros => error*/
            if (executing_macro) {
              message(M_DEF_EXE,TRUE);
              stop_macros();
              ch=O_BREAK;
              break;
            }
            /*}}}  */
            defining_macro = !defining_macro;
            if (defining_macro)
              {macro_pos = fix_macros[0].string;macro_count=0;}
            else
              *macro_pos=M_END_MACRO;
            title_op(defining_macro ? SET_DM : RESET_DM);
            ch=O_PROMPT;
          }
          break;
        }
        /*}}}  */
        /*{{{  O_DEF_FIX*/
        case O_DEF_FIX: {
          ch=O_BREAK;
          if (!hide) {
            char temp[PROMPT_LEN+1];
            int x;
        
            /*{{{  testing error-cases*/
            if (defining_macro || executing_macro) {
              message(M_FIX_EXE,TRUE);
              defining_macro=FALSE;
              executing_macro=0;
              ch=O_BREAK;
              break;
            }
            /*}}}  */
            readprompt(temp,M_WHICH_FIX,20);
            if (aborted) break;
            x=atoi(temp);
            /*{{{  testing error-cases*/
            ch=0;
            if (x<1 || x>MAX_MACRO) {
              message(M_ERR_MACROPLACE,TRUE);
              break;
            }
            if (!fix_macros[x].defineable) {
              message(M_NO_DEFINE,TRUE);
              break;
            }
            /*}}}  */
            if (fix_macros[x].string!=NULL) free(fix_macros[x].string);
            fix_macros[x].string=malloc((macro_count+1)*sizeof(int));
            if (fix_macros[x].string==NULL) {
              message(M_NO_MEMORY,TRUE);
              break;
            }
            memcpy(fix_macros[x].string,fix_macros[0].string,
                   (macro_count+1)*sizeof(TOKEN));
          }break;
        }
        /*}}}  */
        /*{{{  default: if defining_macro, store token*/
        default: {
          if (defining_macro && !executing_macro) {
            *macro_pos++ = ch;
            /*{{{  macro-buffer full*/
            if (++macro_count == MAX_MACRO_LENGTH) {
              message(M_KEY_BUFF_FULL,TRUE);
              *--macro_pos=M_END_MACRO;
              defining_macro = FALSE;
            }
            /*}}}  */
          }
          break;
        }
        /*}}}  */
      }
      /*}}}  */
    } else {
      /*{{{  execute macros*/
      int number=ch-O_EXE_MACRO;
      
      switch (ch) {
        /*{{{  prehandle for exe_mac without break!*/
        case O_EXE_MACRO: {
          if (defining_macro) {
            message(M_EXE_EXE,TRUE);
            stop_macros();
            ch=O_BREAK;
            break;
          }
          if (macro_count==0) *fix_macros[0].string=M_END_MACRO;
        }
        /*}}}  */
        /*{{{  default*/
        default: {
          /*{{{  maybe store this call in keyboard-macro-buffer*/
          if (defining_macro && !executing_macro) {
            *macro_pos++ = ch;
            /*{{{  macro-buffer full*/
            if (++macro_count == MAX_MACRO_LENGTH) {
              message(M_KEY_BUFF_FULL,TRUE);
              *--macro_pos=M_END_MACRO;
              defining_macro = FALSE;
            }
            /*}}}  */
          }
          /*}}}  */
          if (fix_macros[number].string != NULL) {
            if (!argument && !executing_macro) enable_abort();
            ch = (push_macro(fix_macros[number].string) ? O_BREAK : O_NOP);
            if (executing_macro==1) title_op(SET_EM);
          } else
            ch = O_NOP;
        }
        /*}}}  */
      }
      /*}}}  */
    }
  } while (ch==O_NOP);
  return ch;
}
/*}}}  */
/*{{{  hide_key*/
#ifdef STD_C
TOKEN hide_key(bool keypress)
#else
TOKEN hide_key(keypress) bool keypress;
#endif
{
  TOKEN x;
 
  enable_echo(FALSE);
  x=single_key(keypress);
  enable_echo(TRUE);
  if (keypress) title_op(executing_macro ? SET_EM : RESET_EM);
  return(x);
}
/*}}}  */
/*{{{  edit_key              get a token and set *keypad*/
#ifdef STD_C
TOKEN edit_key(bool *keypad)
#else
TOKEN edit_key(keypad)
bool *keypad;
#endif
{
  TOKEN ch;
 
  ch=single_key(FALSE);
  *keypad=(ch>=O_NOP);
  return ch;
}
/*}}}  */
