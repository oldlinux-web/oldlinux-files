#define PARSMAC
/*{{{  #includes*/
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <local/bool.h>

#include "../common/keys.h"
#include "keybind.h"
/*}}}  */

/*{{{  parse_macro*/
#ifdef STD_C
TOKEN *parse_macro(TOKEN *buff)
#else
TOKEN *parse_macro(buff) TOKEN *buff;
#endif
{
  tokens token;

  /*{{{  leading (?*/
  if (get_token(FALSE)!=BEGIN) {
    error_po();
    fprintf(stderr,M_MACBEGIN);
    return(NULL);
  }
  /*}}}  */
  do {
    token=get_token(FALSE);
    switch (token) {
      /*{{{  repeat*/
      case REPEAT: {
        int count;
        TOKEN *new;
      
        if (get_token(FALSE)==NAME) {
          count=atoi(tk_string);
          if (count>0) {
            if (new=parse_macro(buff)) {
              count=(count-1)*(new-buff);
              while (count--) *new++ = *buff++;
              buff=new;
              break;
            }
          }
        }
        error_po();
        fprintf(stderr,M_NOREPEAT);
        return(NULL);
      }
      /*}}}  */
      /*{{{  while*/
      case WHILE: {
        TOKEN *whilestart,*jmpad;
      
        if ((jmpad=parse_cond(whilestart=buff))==NULL) return(NULL);
        if ((buff=generate_jmp(M_JMP_FALSE,jmpad,jmpad))==NULL) return(NULL);
        if ((buff=parse_macro(buff))==NULL) return(NULL);
        if ((buff=generate_jmp(M_JMP,buff,whilestart))==NULL) return(NULL);
        if (generate_jmp(M_JMP_FALSE,jmpad,buff)==NULL) return(NULL);
        break;
      }
      /*}}}  */
      /*{{{  do*/
      case DO: {
        TOKEN *dostart=buff;
      
        if ((buff=parse_macro(buff))==NULL) return(NULL);
        if (get_token(FALSE)!=WHILE) {
          error_po();
          fprintf(stderr,M_NOWHILE);
          return(NULL);
        }
        if ((buff=parse_cond(buff))==NULL) return(NULL);
        if ((buff=generate_jmp(M_JMP_TRUE,buff,dostart))==NULL) return(NULL);
        break;
      }
      /*}}}  */
      /*{{{  case*/
      case CASE: {
        TOKEN *condptr,*defaultjmp,*startcase=buff;
      
        /*{{{  create: jmp cases ; jmp end ; cases:*/
        if ((defaultjmp=generate_jmp(M_JMP,startcase,buff))==NULL) return(NULL);
        if ((buff=generate_jmp(M_JMP,defaultjmp,buff))==NULL) return(NULL);
        if (generate_jmp(M_JMP,startcase,buff)==NULL) return(NULL);
        /*}}}  */
        token=get_token(FALSE);
        do {
          switch (token) {
            /*{{{  BEGIN -> cond macro*/
            case BEGIN: {
              if ((condptr=parse_cond(buff))==NULL) return(NULL);
              if ((buff=generate_jmp(M_JMP_FALSE,condptr,condptr))==NULL)
                return(NULL);
              if ((buff=parse_macro(buff))==NULL) return(NULL);
              if ((buff=generate_jmp(M_JMP,buff,defaultjmp))==NULL)
                return(NULL);
              if (generate_jmp(M_JMP_FALSE,condptr,buff)==NULL)
                return(NULL);
              if (get_token(FALSE)!=END) {
                error_po();
                fprintf(stderr,M_CASEEND);
                return(NULL);
              }
              token=get_token(FALSE);
              break;
            }
            /*}}}  */
            /*{{{  DEFAULT*/
            case DEFAULT:
              if ((buff=parse_macro(buff))==NULL) return(NULL);
              token=get_token(FALSE);
              break;
            /*}}}  */
            case ESAC:
              break;
            default:
              error_po();
              fprintf(stderr,M_NOESAC);
              return(NULL);
          }
        } while (token!=ESAC);
        if (generate_jmp(M_JMP,defaultjmp,buff)==NULL) return(NULL);
        break;
      }
      /*}}}  */
      /*{{{  if*/
      case IF: {
        TOKEN *jmpadr,*endtrue;
      
        if ((jmpadr=parse_cond(buff))==NULL) return(NULL);
        if ((buff=generate_jmp(M_JMP_FALSE,jmpadr,jmpadr))==NULL) return(NULL);
        if ((endtrue=parse_macro(buff))==NULL) return(NULL);
        token=get_token(FALSE);
        switch (token) {
          /*{{{  if else fi*/
          case ELSE:
            if ((buff=generate_jmp(M_JMP,endtrue,endtrue))==NULL) return(NULL);
            if (generate_jmp(M_JMP_FALSE,jmpadr,buff)==NULL) return(NULL);
            if ((buff=parse_macro(buff))==NULL) return(NULL);
            if (generate_jmp(M_JMP,endtrue,buff)==NULL) return(NULL);
            if (get_token(FALSE)!=FI) {
              error_po();
              fprintf(stderr,M_NOFI);
              return(NULL);
            }
            break;
          /*}}}  */
          /*{{{  if fi*/
          case FI:
            if (generate_jmp(M_JMP_FALSE,jmpadr,endtrue)==NULL) return(NULL);
            buff=endtrue;
            break;
          /*}}}  */
          default:
            error_po();
            fprintf(stderr,M_NOELSEFI);
            return(NULL);
        }
        break;
      
      
      }
      /*}}}  */
      /*{{{  read-repeat*/
      case READ_REPEAT:
        *buff++ = M_READ_REPEAT;
        if ((buff=put_var(buff))==NULL) return(NULL);
        break;
      /*}}}  */
      /*{{{  counter*/
      case COUNTER:
        *buff++ = M_INT_STRING;
        if ((buff=put_var(buff))==NULL) return(NULL);
        *buff++ = (TOKEN)((int)O_EXE_MACRO+MAX_MACRO+1);
        break;
      /*}}}  */
      /*{{{  add_/set_counter*/
      case ADD_COUNTER:
      case SET_COUNTER:
        if (token==SET_COUNTER) *buff++ = M_SET_COUNTER;
        else *buff++ = M_ADD_COUNTER;
        if ((buff=put_var(buff))==NULL) return(NULL);
        if (get_token(FALSE)!=NAME) {
          error_po();
          fprintf(stderr,M_NOADDCOUNTER);
          return(NULL);
        }
        *buff++ = (TOKEN) atoi(tk_string);
        break;
      /*}}}  */
      /*{{{  sum_counter*/
      case SUM_COUNTER:
        *buff++ = M_SUM_COUNTER;
        if ((buff=put_var(buff))==NULL) return(NULL);
        if ((buff=put_var(buff))==NULL) return(NULL);
        break;
      /*}}}  */
      /*{{{  inv_counter*/
      case INV_COUNTER:
        *buff++ = M_INV_COUNTER;
        if ((buff=put_var(buff))==NULL) return(NULL);
        break;
      /*}}}  */
      /*{{{  message-exit/exit*/
      case MES_EXIT:
      case EXIT: {
         *buff++ = M_EXIT;
         if (token==MES_EXIT && (buff=get_message(buff))==NULL) {
           fprintf(stderr,M_NOMESSAGE);
           return(NULL);
         }
         *buff++ = M_END_MACRO;
         break;
      }
      /*}}}  */
      /*{{{  store_x*/
      case STORE_X:
        *buff++ = M_POS_TO_COUNTER;
        if ((buff=put_var(buff))==NULL) return(NULL);
        break;
      /*}}}  */
      /*{{{  store_y*/
      case STORE_Y:
        *buff++ = M_STORE_LINE_NO;
        if ((buff=put_var(buff))==NULL) return(NULL);
        break;
      /*}}}  */
      /*{{{  goto_y*/
      case GOTO_Y:
        *buff++ = M_GO_LINE;
        if ((buff=put_var(buff))==NULL) return(NULL);
        break;
      /*}}}  */
      /*{{{  goto_x*/
      case GOTO_X:
        if (get_token(FALSE)!=NAME) {
          error_po();
          fprintf(stderr,M_NOGOTO);
          return(NULL);
        }
        *buff++ = M_GO_X_POS;
        *buff++ = (TOKEN) atoi(tk_string);
        break;
      /*}}}  */
      /*{{{  goto_counter*/
      case GOTO_COUNTER:
        *buff++ = M_GO_COUNTER_X_POS;
        if ((buff=put_var(buff))==NULL) return(NULL);
        break;
      /*}}}  */
      /*{{{  opcode*/
      case OPCODE:
        *buff++=tk_key->num;
        break;
      /*}}}  */
      /*{{{  prompt prompt-counter*/
      case PROMPT:
      case PROMPT_COUNTER: {
        *buff++ = M_PROMPT;
        if (token==PROMPT)
          *buff++ = (TOKEN) -1;
        else if ((buff=put_var(buff))==NULL)
          return(NULL);
        /*{{{  prompt*/
        if ((buff=get_message(buff))==NULL) {
          fprintf(stderr,M_NOPROMPT);
          return(NULL);
        }
        /*}}}  */
        *buff++ = M_END_MACRO;
        break;
      }
      /*}}}  */
      /*{{{  macrostring*/
      case MACRO: {
        TOKEN *x=tk_macro;
      
        while (*x) *buff++ = *x++;
        break;
      }
      /*}}}  */
      /*{{{  previosly defined operation*/
      case OPERATION: {
        TOKEN *x=tk_operation->ops;
        int lg=tk_operation->length;
      
        while (lg--) *buff++ = *x++;
        break;
      }
      /*}}}  */
      case END: break;
      /*{{{  rekursiv on my one*/
      case NAME:
        if (!strcmp(op_def_name,tk_string)) {
          buff=generate_jmp(M_CALL,buff,m_def);
          break;
        }
      /*}}}  */
      /*{{{  default=error*/
      default:
        error_po();
        fprintf(stderr,M_NOMSTRING);
        return(NULL);
      /*}}}  */
    }
  }
  while (token!=END);
  return(buff);
}
/*}}}  */
