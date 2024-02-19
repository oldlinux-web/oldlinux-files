#define PARCON
/*{{{  #includes*/
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
 
#include <local/bool.h>
 
#include "../common/keys.h"
#include "keybind.h"
/*}}}  */
 
/*{{{  parse_cond*/
#ifdef STD_C
TOKEN *parse_cond(TOKEN *buff)
#else
TOKEN *parse_cond(buff) TOKEN *buff;
#endif
{
  switch(get_token(FALSE)) {
    /*{{{  single tests*/
    case TEST_FILED:
      *buff++ = M_FILED_FOLD;
      break;
    case TEST_FOLD_LINE:
      *buff++ = M_CLOSED_FOLD;
      break;
    case TEST_BEGIN_FOLD:
      *buff++ = M_BEGIN_FOLD_COMMENT;
      break;
    case TEST_END_FOLD:
      *buff++ = M_END_FOLD_COMMENT;
      break;
    case TEST_TEXT:
      *buff++ = M_TEXTLINE;
      break;
    case TEST_TOP:
      *buff++ = M_TOP_OF_FOLD;
      break;
    case TEST_BOTTOM:
      *buff++ = M_BOT_OF_FOLD;
      break;
    case TEST_BEGIN_LINE:
      *buff++ = M_BEGIN_OF_LINE;
      break;
    case TEST_END_LINE:
      *buff++ = M_END_OF_LINE;
      break;
    case TEST_CHANGED:
      *buff++ = M_CHANGED;
      break;
    /*}}}  */
    /*{{{  test-behind-counter*/
    case TEST_BEHIND_COUNTER:
      *buff++ = M_BEHIND_COUNTER_X_POS;
      if ((buff=put_var(buff))==NULL) return(NULL);
      break;
    /*}}}  */
    /*{{{  counter_null*/
    case COUNTER_NULL:
      *buff++ = M_NULL_COUNTER;
      if ((buff=put_var(buff))==NULL) return(NULL);
      break;
    /*}}}  */
    /*{{{  counter_positiv*/
    case C_POSITIV:
      *buff++ = M_POSITIV_COUNTER;
      if ((buff=put_var(buff))==NULL) return(NULL);
      break;
    /*}}}  */
    /*{{{  test_char x*/
    case TEST_CHAR:
      *buff++ = M_TEST_CHAR;
      if (get_token(TRUE)!=CHAR) {
        error_po();
        fprintf(stderr,M_TCNEEDSCHAR);
        return(NULL);
      }
      *buff++ = (TOKEN) tk_char;
      break;
    /*}}}  */
    /*{{{  test_language x*/
    case TEST_LANG:
      *buff++ = M_LANGUAGE;
      if (get_token(TRUE)!=CHAR) {
        error_po();
        fprintf(stderr,M_TCNEEDSCHAR);
        return(NULL);
      }
      *buff++ = (TOKEN) tk_char;
      break;
    /*}}}  */
    /*{{{  not condition*/
    case NOT:
      if (get_token(FALSE)!=BEGIN) {
        error_po();
        fprintf(stderr,M_NOTBEGIN);
        return(NULL);
      }
      if ((buff=parse_cond(buff))==NULL) return(NULL);
      *buff++ = M_NOT;
      if (get_token(FALSE)!=END) {
        error_po();
        fprintf(stderr,M_NOTEND);
        return(NULL);
      }
      break;
    /*}}}  */
    /*{{{  and(c1,c2)*/
    case AND: {
      TOKEN *first_jmp;
    
      if (get_token(FALSE)!=BEGIN) {
        error_po();
        fprintf(stderr,M_ANDBEGIN);
        return(NULL);
      }
      if ((first_jmp=parse_cond(buff))==NULL) return(NULL);
      if ((buff=generate_jmp(M_JMP_FALSE,first_jmp,first_jmp))==NULL) return(NULL);
      if (get_token(FALSE)!=COMMA) {
        error_po();
        fprintf(stderr,M_ANDCOMMA);
        return(NULL);
      }
      if ((buff=parse_cond(buff))==NULL) return(NULL);
      if (generate_jmp(M_JMP_FALSE,first_jmp,buff)==NULL) return(NULL);
      if (get_token(FALSE)!=END) {
        error_po();
        fprintf(stderr,M_ANDEND);
        return(NULL);
      }
      break;
    }
    /*}}}  */
    /*{{{  or(c1,c2)*/
    case OR: {
      TOKEN *first_jmp;
    
      if (get_token(FALSE)!=BEGIN) {
        error_po();
        fprintf(stderr,M_ORBEGIN);
        return(NULL);
      }
      if ((first_jmp=parse_cond(buff))==NULL) return(NULL);
      if ((buff=generate_jmp(M_JMP_TRUE,first_jmp,first_jmp))==NULL) return(NULL);
      if (get_token(FALSE)!=COMMA) {
        error_po();
        fprintf(stderr,M_ORCOMMA);
        return(NULL);
      }
      if ((buff=parse_cond(buff))==NULL) return(NULL);
      if (generate_jmp(M_JMP_TRUE,first_jmp,buff)==NULL) return(NULL);
      if (get_token(FALSE)!=END) {
        error_po();
        fprintf(stderr,M_OREND);
        return(NULL);
      }
      break;
    }
    /*}}}  */
    default:
      error_po();
      fprintf(stderr,M_NOTEST);
      return(NULL);
  }
  return(buff);
}
/*}}}  */
