#define CODEHELP
/*{{{  #includes*/
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
 
#include <local/bool.h>
 
#include "../common/keys.h"
#include "keybind.h"
/*}}}  */
 
/*{{{  create a jump-command*/
#ifdef STD_C
TOKEN *generate_jmp(TOKEN j, TOKEN *ad, TOKEN *dest)
#else
TOKEN *generate_jmp(j,ad,dest) TOKEN j, *ad, *dest;
#endif
{
  if (j!=M_JMP && j!=M_JMP_TRUE && j!=M_JMP_FALSE && j!=M_CALL) {
    fprintf(stderr,M_NOJUMP);
    return(NULL);
  }
  *ad++ = j;
  *ad= dest-(ad+1);
  return(ad+1);
}
/*}}}  */
/*{{{  parse a keycode sequence*/
#ifdef STD_C
char *parse_keysequence(char *cod, char *pro)
#else
char *parse_keysequence(cod,pro) char *cod,*pro;
#endif
{
  char *c=cod;
  char *p=pro;
  tokens token;
 
  /*{{{  check (*/
  if (get_token(TRUE)!=BEGIN) {
    error_po();
    fprintf(stderr,M_KSNOBEGIN);
    return(NULL);
  }
  /*}}}  */
  /*{{{  read 'til )*/
  do {
    token=get_token(TRUE);
    switch (token) {
      /*{{{  CHAR-handling*/
      case CHAR:
        *c++=tk_char;
        /*{{{  generate protocol of this binding*/
        if (tk_char==127)      {*p++='C';*p++='-';*p++='?';}
        else if (tk_char>=' ') *p++=tk_char;
        else                   {*p++='C';*p++='-';*p++=tk_char+'A'-1;}
        *p++=' ';
        /*}}}  */
        break;
      /*}}}  */
      /*{{{  META-handling*/
      case META:
        *c++=27;
        *p++='M';*p++='-';
        break;
      /*}}}  */
      /*{{{  ALIAS-handling*/
      case DOLLAR: {
        char *s=tk_alias->code;
        char *n=tk_alias->key_name;
        int  lg=tk_alias->length;
      
        while (lg--) *c++ = *s++;
        while (*n) *p++ = *n++;
        *p++=' ';
        break;
      }
      /*}}}  */
      case END: break;
      /*{{{  default=error*/
      default: {
        error_po();
        fprintf(stderr,M_NOKEYCODE);
        return(NULL);
      }
      /*}}}  */
    }
  } while (token!=END);
  /*}}}  */
  *p='\0';
  return(c);
}
/*}}}  */
/*{{{  put variable-adress*/
#ifdef STD_C
TOKEN *put_var(TOKEN *buff)
#else
TOKEN *put_var(buff) TOKEN *buff;
#endif
{
  tokens token=get_token(FALSE);
 
  switch (token) {
    case NAME:
      *buff++ = (TOKEN) int_no;
      creat_var(tk_string);
      break;
    case VARIABLE:
      *buff++ = (TOKEN) tk_var->no;
      break;
    default:
      error_po();
      fprintf(stderr,M_NOVAR);
      return(NULL);
  }
  return(buff);
}
/*}}}  */
/*{{{  get_message*/
#ifdef STD_C
TOKEN *get_message(TOKEN *buff)
#else
TOKEN *get_message(buff) TOKEN *buff;
#endif
{
  tokens read=get_token(FALSE);
  TOKEN *x=tk_macro;
 
  if (read!=MACRO) {
    /*{{{  complex prompt ( ... )*/
    /*{{{  no begin*/
    if (read!=BEGIN) {
      error_po();
      fprintf(stderr,M_NOMESSAGE);
      return(NULL);
    }
    /*}}}  */
    read=get_token(FALSE);
    while ((read==MACRO || read==COUNTER) && buff!=NULL) {
      x=tk_macro;
      if (read==MACRO)
        while (*x) *buff++ = *x++;
      else {
        *buff++ = M_INT_STRING;
        buff = put_var(buff);
      }
      read=get_token(FALSE);
    }
    /*{{{  errors?*/
    if (buff==NULL) return(NULL);
    if (read!=END) {
      error_po();
      fprintf(stderr,M_WANTEND);
      return(NULL);
    }
    /*}}}  */
    /*}}}  */
  } else
    while (*x) *buff++ = *x++;
  return(buff);
}
/*}}}  */
