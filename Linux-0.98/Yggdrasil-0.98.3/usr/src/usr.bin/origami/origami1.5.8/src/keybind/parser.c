#define PARSER_C
/*{{{  #includes*/
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <local/bool.h>

#include "../common/keys.h"
#include "keybind.h"
/*}}}  */
 
/*{{{  variables*/
char nullc='\0';
char *op_def_name = &nullc;
TOKEN *m_def;
char abort_key= 0;
bool ab_set=FALSE;
/*}}}  */
 
/*{{{  parse the file*/
#ifdef STD_C
bool process_file(void)
#else
bool process_file()
#endif
{
  /*{{{  variables*/
  tokens token;
  char code[alias_lg];
  char prot[alias_lg];
  char name[name_lg];
  TOKEN tokenlist[macro_lg];
  TOKEN *tl_ptr;
  int macro_pos=MAX_MACRO;
  bool named=FALSE;
  int auto_macro=0;
  /*}}}  */
 
  int_no=0;
  token=get_token(FALSE);
  do {
    /*{{{  one keybind-command*/
    switch (token) {
      case ENDFILE: break;
      case BEGIN: {
        token=get_token(FALSE);
        switch (token) {
          /*{{{  alias command*/
          case KEYALIAS: {
            char *x;
          
            /*{{{  name given?*/
            if (get_token(FALSE)!=NAME) {
              error_po();
              fprintf(stderr,M_EXPALIAS);
              return(TRUE);
            }
            /*}}}  */
            strcpy(name,tk_string);
            if ((x=parse_keysequence(code,prot))==NULL) return(TRUE);
            creat_alias(name,code,x-code);
            /*{{{  incorrect commandend?*/
            if (get_token(FALSE)!=END) {
              error_po();
              fprintf(stderr,M_ALIEND);
              return(TRUE);
            }
            /*}}}  */
            break;
          }
          /*}}}  */
          /*{{{  keybind command*/
          case KEYDEF: {
            TOKEN t;
            char *n,*x;
          
            /*{{{  opcode given?*/
            token=get_token(FALSE);
            /*{{{  if opcode*/
            if (token==OPCODE) {
              t=tk_key->num;
              n=tk_key->name;
            /*}}}  */
            /*{{{  if operation*/
            } else if (token==OPERATION) {
              if (tk_operation->place==0) {
                error_po();
                fprintf(stderr,M_NODEBOUND);
                return(TRUE);
              }
              t=(TOKEN)((int)O_EXE_MACRO+tk_operation->place);
              n=tk_operation->op_name;
            /*}}}  */
            /*{{{  else error*/
            } else {
              error_po();
              fprintf(stderr,M_NOCOMMAND);
              return(TRUE);
            }
            /*}}}  */
            /*}}}  */
            if ((x=parse_keysequence(code,prot))==NULL) return(TRUE);
            /*{{{  incorrect end of command?*/
            if (get_token(FALSE)!=END) {
              error_po();
              fprintf(stderr,M_ALIEND);
              return(TRUE);
            }
            /*}}}  */
            if (t==O_BREAK)
              /*{{{  abort*/
              if ((x-code)!=1) {
                error_po();
                fprintf(stderr,M_NOABORT);
                return(TRUE);
              } else if (ab_set) {
                error_po();
                fprintf(stderr,M_DUPAB);
                return(TRUE);
              } else {
                write_break_rc(abort_key= *code);
                ab_set=TRUE;
              }
              /*}}}  */
            else if (add_keysequence(t,code,x-code)) return(TRUE);
            write_bind(n,prot);
            break;
          }
          /*}}}  */
          /*{{{  forward-declaration*/
          case FORWARD: {
            /*{{{  check if name given*/
            if (get_token(FALSE)!=NAME) {
              error_po();
              fprintf(stderr,M_MACNAME);
              return(TRUE);
            }
            /*}}}  */
            strcpy(name,tk_string);
            tokenlist[0]=(TOKEN)((int)O_EXE_MACRO+macro_pos);
            tokenlist[1]=M_END_MACRO;
            creat_op(name,FALSE,1,tokenlist,macro_pos--);
            /*{{{  incorrect end of command?*/
            if (get_token(FALSE)!=END) {
              error_po();
              fprintf(stderr,M_DEFOEND);
              return(TRUE);
            }
            /*}}}  */
            break;
          }
          /*}}}  */
          /*{{{  automacro*/
          case DEFAUTO: {
            /*{{{  already given?*/
            if (auto_macro) {
              error_po();
              fprintf(stderr,M_DUPAUTO);
              return(TRUE);
            }
            /*}}}  */
            /*{{{  check if macroname given*/
            if (get_token(FALSE)!=OPERATION) {
              error_po();
              fprintf(stderr,M_INVMNAME);
              return(TRUE);
            }
            /*}}}  */
            write_auto_rc(tk_operation->place);
            /*{{{  incorrect end of command?*/
            if (get_token(FALSE)!=END) {
              error_po();
              fprintf(stderr,M_DEFOEND);
              return(TRUE);
            }
            /*}}}  */
            break;
          }
          /*}}}  */
          /*{{{  define a new operation*/
          case DEFOP: {
            /*{{{  check if name given*/
            if (get_token(FALSE)!=NAME) {
              error_po();
              fprintf(stderr,M_DEFONAME);
              return(TRUE);
            }
            /*}}}  */
            strcpy(name,tk_string);
            /*{{{  prepare M_CALL-information*/
            op_def_name=name;
            m_def=tokenlist;
            /*}}}  */
            if ((tl_ptr=parse_macro(tokenlist))==NULL) return(TRUE);
            /*{{{  reset M_CALL-information*/
            op_def_name = &nullc;
            m_def=NULL;
            /*}}}  */
            creat_op(name,TRUE,tl_ptr-tokenlist,tokenlist,0);
            /*{{{  incorrect end of command?*/
            if (get_token(FALSE)!=END) {
              error_po();
              fprintf(stderr,M_DEFOEND);
              return(TRUE);
            }
            /*}}}  */
            break;
          }
          /*}}}  */
          /*{{{  macro commands*/
          case DEFMACRO:
          case INITMACRO: {
            bool def=(bool)(token==DEFMACRO);
            readtags tag=(token==DEFMACRO) ? RC_DEFMACRO : RC_INITMACRO;
            int po;
          
            if ((token=get_token(FALSE))==NAME) {
              /*{{{  declaration & definition*/
              /*{{{  no macros available*/
              if (!macro_pos) {
                error_po();
                fprintf(stderr,M_FIXFULL);
                return(TRUE);
              }
              /*}}}  */
              po = macro_pos--;
              strcpy(name,tk_string);
              op_def_name=name;
              m_def=tokenlist;
              /*}}}  */
            } else if (token==OPERATION) {
              /*{{{  definition*/
              if (tk_operation->defined) {
                error_po();
                fprintf(stderr,M_DUPDEF);
                return(TRUE);
              }
              strcpy(name,tk_operation->op_name);
              tk_operation->defined=TRUE;
              po = (int)*(tk_operation->ops)-(int)O_EXE_MACRO;
              op_def_name=name;
              /*}}}  */
            } else {
              error_po();
              fprintf(stderr,M_INVMNAME);
              return(TRUE);
            }
            if ((tl_ptr=parse_macro(tokenlist))==NULL) return(TRUE);
            if (write_macro_rc(tag,po,tl_ptr-tokenlist,tokenlist)) return(TRUE);
            if (token!=OPERATION) {
              /*{{{  macro as operation storing*/
              if (def) {
                tokenlist[0]=(TOKEN)((int)O_EXE_MACRO+po);
                creat_op(name,TRUE,1,tokenlist,po);
              } else
                creat_op(name,TRUE,tl_ptr-tokenlist,tokenlist,po);
              m_def=NULL;
              op_def_name = &nullc;
              /*}}}  */
            }
            /*{{{  incorrect end of command?*/
            if (get_token(FALSE)!=END) {
              error_po();
              fprintf(stderr,M_MDEFNAME);
              return(TRUE);
            }
            /*}}}  */
            break;
          }
          /*}}}  */
          /*{{{  define the name of this binding*/
          case BINDNAME: {
            if (named) {
              error_po();
              fprintf(stderr,M_DUPKEY);
              return(TRUE);
            }
            named=TRUE;
            if (get_token(FALSE)!=NAME) {
              fprintf(stderr,M_KEYNAME);
              return(TRUE);
            }
            if (write_name_rc(tk_string)) return(TRUE);
            if (get_token(FALSE)!=END) {
              fprintf(stderr,M_WANTEND);
              return(TRUE);
            }
            break;
          }
          /*}}}  */
          /*{{{  defmark*/
          case DEFMARK: {
            int i,j;
            char marks[4][tag_length-1];
            char mark_name[name_lg];
          
            /*{{{  errorcases*/
            if (get_token(FALSE)!=NAME) {
              error_po();
              fprintf(stderr,M_MARKNAME);
              return(TRUE);
            }
            /*}}}  */
            strcpy(mark_name,tk_string);
            /*{{{  get all mark_characters*/
            /*{{{  ( ?*/
            if (get_token(FALSE)!=BEGIN) {
              error_po();
              fprintf(stderr,M_WANTBEGIN);
              return(TRUE);
            }
            /*}}}  */
            /*{{{  get all tags*/
            for (i=0;i<4;i++) {
              /*{{{  ( ?*/
              if (get_token(FALSE)!=BEGIN) {
                error_po();
                fprintf(stderr,M_WANTBEGIN);
                return(TRUE);
              }
              /*}}}  */
              /*{{{  get tag_length-1 chars*/
              for (j=0;j<(tag_length-1);j++) {
                /*{{{  no char -> error*/
                if (get_token(TRUE)!=CHAR) {
                  error_po();
                  fprintf(stderr,M_WANTCHAR);
                  return(TRUE);
                }
                /*}}}  */
                marks[i][j]=tk_char;
              }
              /*}}}  */
              /*{{{  ) ?*/
              if (get_token(FALSE)!=END) {
                error_po();
                fprintf(stderr,M_WANTEND);
                return(TRUE);
              }
              /*}}}  */
            }
            /*}}}  */
            /*{{{  ) ?*/
            if (get_token(FALSE)!=END) {
              error_po();
              fprintf(stderr,M_WANTEND);
              return(TRUE);
            }
            /*}}}  */
            /*}}}  */
            /*{{{  ) ?*/
            if (get_token(FALSE)!=END) {
              error_po();
              fprintf(stderr,M_WANTEND);
              return(TRUE);
            }
            /*}}}  */
            write_mark_rc(mark_name,marks);
            break;
          }
          /*}}}  */
          default:
            error_po();
            fprintf(stderr,M_COMMANDLINE);
            return(TRUE);
        }
        break;
      }
      default:
        error_po();
        fprintf(stderr,M_NOKCOMMAND);
        return(TRUE);
    }
    /*}}}  */
  } while ((token=get_token(FALSE))!=ENDFILE);
  /*{{{  check forward-declarations*/
  { OP **act=new_op;
  
    while (*act!=NULL) {
      if (!(*act)->defined) {
        fprintf(stderr,F_DNDMAC,(*act)->op_name);
        return(TRUE);
      }
      act++;
    }
  }
  /*}}}  */
  return(FALSE);
}
/*}}}  */
