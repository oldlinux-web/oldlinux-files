#define SCANNER
/*{{{  #includes*/
#include <ctype.h>
#include <termcap.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <local/bool.h>

#include "../common/keys.h"
#include "keybind.h"
/*}}}  */
 
/*{{{  variable declarations*/
#define f_s_depth 4
int f_s_ptr=0;
FILE *file_stack[f_s_depth];
int po_stack[f_s_depth];
int inp_stack[f_s_depth];
char *name_stack[f_s_depth];
int input=EOF;
int current_line_no=0;
char   tk_char;
ALIAS  *tk_alias;
KEYNAME *tk_key;
OP     *tk_operation;
VARS   *tk_var;
char   tk_string[name_lg];
TOKEN  tk_macro[macro_lg];
ALIAS  *func_key[64];
OP     *new_op[64];
VARS   *vars[128];
char   *source;
/*}}}  */
 
/*{{{  list all keywords/operations*/
#ifdef STD_C
void sc_list_keys(void)
#else
void sc_list_keys()
#endif
{
  /*{{{  all operations known in origami*/
  { KEYNAME *key=bindings;
    while (key->num != O_NOP) printf("%s\n",(key++)->name);
  }
  /*}}}  */
  /*{{{  all keywords known from keybind*/
  { keywords *key=keytab;
    while (key->value != ERROR) printf("%s\n",(key++)->name);
  }
  /*}}}  */
}
/*}}}  */
/*{{{  create an operation entry*/
#ifdef STD_C
void creat_op(char *name,bool mode,int lg,TOKEN *code,int place)
#else
creat_op(name,mode,lg,code,place)
char *name;int lg,place; TOKEN *code; bool mode;
#endif
 
/*{{{  comment*/
/* arguments are:
  name - name of the macro/function
  mode - the code is defined (forward)
  lg   - number of tokens
  code - the tokenlist
  place- the number of the used origami-macro-buffer or 0 for defmac
*/
/*}}}  */
{
  OP **act=new_op;
 
  while (*act!=NULL) act++;
  *act=(OP*)malloc(sizeof(OP));
  (*act)->op_name=(char*)malloc(strlen(name)+1);
  strcpy((*act)->op_name,name);
  (*act)->length=lg;
  (*act)->defined=mode;
  (*act)->place=place;
  (*act)->ops=malloc((1+lg)*sizeof(int));
  memcpy((*act)->ops,code,lg*sizeof(int));
  /*{{{  verbose-handling*/
  if (verbose) {
    if (place)
      fprintf(stderr,F_BELONGS,name);
    else
      fprintf(stderr,F_OPERATION,name,lg);
  }
  /*}}}  */
}
/*}}}  */
/*{{{  create an variable entry*/
#ifdef STD_C
void creat_var(char *name)
#else
creat_var(name) char *name;
#endif
{
  VARS **act=vars;

  while (*act!=NULL) act++;
  *act=(VARS*)malloc(sizeof(VARS));
  (*act)->var_name=(char*)malloc(strlen(name)+1);
  strcpy((*act)->var_name,name);
  (*act)->no=int_no++;
}
/*}}}  */
/*{{{  create an alias entry*/
#ifdef STD_C
void creat_alias(char *name, char *code, int lg)
#else
creat_alias(name,code,lg) char *name,*code; int lg;
#endif
{
  ALIAS **act= func_key;
 
  while (*act!=NULL) act++;
  *act=(ALIAS*)malloc(sizeof(ALIAS));
  (*act)->key_name=(char*)malloc(strlen(name)+1);
  strcpy((*act)->key_name,name);
  (*act)->code=(char*)malloc(lg+1);
  memcpy((*act)->code,code,lg);
  (*act)->length=lg;
}
/*}}}  */
/*{{{  init alias list*/
/*{{{  void init_default_alias()*/
#ifdef STD_C
void init_default_alias(void)
#else
void init_default_alias()
#endif
{
  creat_alias("esc","\033",1);
  creat_alias("tab","\t",1);
  creat_alias("return","\r",1);
#  ifdef TOS
  creat_alias("meta","\xFF",1);  /* create aliases for special ST */
  creat_alias("smeta","\xFE",1); /* keyboard meta codes */
  creat_alias("cmeta","\xFD",1);
  creat_alias("ameta","\xFC",1);
  creat_alias("mmeta","\xFB",1);
#  endif
}
/*}}}  */
/*{{{  void init_termcap_alias()*/
#ifndef TOS
  PRIVATE char termcap_area[1024];
 
  /*{{{  type definition for a termcap key alias*/
  typedef struct
  {
    char *capability, *alias;
  } term_alias;
  /*}}}  */
  /*{{{  declaration of termcap key alias table*/
  term_alias alias_table[] =
  {
    { "kb", "backspace" },
    { "kB", "backtab" },
    { "kd", "down" },
    { "kD", "rubout" },
    { "kE", "clreol" },
    { "kh", "home" },
    { "kl", "left" },
    { "kL", "delline" },
    { "kN", "next_page" },
    { "kP", "prev_page" },
    { "kr", "right" },
    { "ku", "up" },
    { "k0", "f0" },
    { "k1", "f1" },
    { "k2", "f2" },
    { "k3", "f3" },
    { "k4", "f4" },
    { "k5", "f5" },
    { "k6", "f6" },
    { "k7", "f7" },
    { "k8", "f8" },
    { "k9", "f9" },
    { "k;", "f10" }
  };
  /*}}}  */
#endif
 
#ifdef STD_C
  void init_termcap_alias(void)
#else
  void init_termcap_alias()
#endif
{
#  ifndef TOS
    /*{{{  unix/minix init*/
    /*{{{  variable declaration*/
    char bp[1024];
    char *terminal, *pointer, *cap;
    term_alias *run=alias_table;
    /*}}}  */
    
    /*{{{  get terminal type*/
    if ((terminal=getenv("TERM"))==NULL) return;
    /*}}}  */
    /*{{{  get termcap entry*/
    if (tgetent(bp,terminal)<=0) return;
    /*}}}  */
    /*{{{  process termcap key alias table*/
    pointer = termcap_area;
    while (run->capability)
    {
      if ((cap=tgetstr(run->capability,&pointer))!=NULL)
      creat_alias(run->alias,cap,strlen(cap));
      run++;
    }
    /*}}}  */
    /*}}}  */
#  else
    /*{{{  tos init*/
    creat_alias("backspace","\x08",1);  /* backspace */
    creat_alias("rubout","\xFF\x07",2); /* delete */
    creat_alias("home","\xFF\x01",2);   /* ClrHome */
    creat_alias("left","\xFF\x03",2);   /* arrow left */
    creat_alias("right","\xFF\x04",2);  /* arrow right */
    creat_alias("up","\xFF\x02",2);     /* arrow up */
    creat_alias("down","\xFF\x05",2);   /* arrow down */
    creat_alias("next_page","\xFDo",2); /* Ctrl-arrow down */
    creat_alias("prev_page","\xFDl",2); /* Ctrl-arrow up */
    creat_alias("f1","\xFF\x0a",2);     /* F1 */
    creat_alias("f2","\xFF\x0b",2);     /* F2 */
    creat_alias("f3","\xFF\x0c",2);     /* F3 */
    creat_alias("f4","\xFF\x0e",2);     /* F4 */
    creat_alias("f5","\xFF\x0f",2);     /* F5 */
    creat_alias("f6","\xFF\x10",2);     /* F6 */
    creat_alias("f7","\xFF\x11",2);     /* F7 */
    creat_alias("f8","\xFF\x12",2);     /* F8 */
    creat_alias("f9","\xFF\x13",2);     /* F9 */
    creat_alias("f10","\xFF\x14",2);    /* F10 */
    /*}}}  */
#  endif
}
/*}}}  */
/*{{{  void init_alias()*/
void init_alias()
{
  init_default_alias();
  init_termcap_alias();
}
/*}}}  */
/*}}}  */
/*{{{  error_po()*/
#ifdef STD_C
void error_po(void)
#else
void error_po()
#endif
{
  int i=0;
 
  for (;i<=f_s_ptr;i++)
  fprintf(stderr,F_ERRORPO,
                (i?name_stack[i]:source),
                (i==f_s_ptr?current_line_no:po_stack[i]));
}
/*}}}  */
/*{{{  read a character from input, count lines*/
#ifdef STD_C
int read_in(void)
#else
int read_in()
#endif
{
  input=getc(in);
  if (input=='\n') current_line_no++;
  return(input);
}
/*}}}  */
/*{{{  get a token from input file*/
#ifdef STD_C
tokens get_single_token(bool code)
#else
tokens get_single_token(code) bool code;
#endif
{
  /*{{{  skip comments, spaces  ...*/
  while (input=='\n' || input==' ' || input==';' || input=='{' || input=='}')
  {
    if (input==';' || input=='{' || input=='}')
    {
      while (input!='\n' && input!=EOF) read_in();
    }
    read_in();
  }
  /*}}}  */
 
  switch (input)
  {
    /*{{{  single char tokens*/
    case EOF: {read_in();return(ENDFILE);}
    case '(': {read_in();return(BEGIN);}
    case ')': {read_in();return(END);}
    case ',': {read_in();return(COMMA);}
    /*}}}  */
    default :
    if (code) {
      /*{{{  code-parser*/
      switch (input)
      {
        /*{{{  Meta-handling*/
        case 'M':
        {
          read_in();
          if (input!='-')
          {
            error_po();
            fprintf(stderr,M_NOMETA);
            return(ERROR);
          }
          read_in();
          return(META);
        }
        /*}}}  */
        /*{{{  Control-handling*/
        case 'C':
        {
          /*{{{  read -" and error-check*/
          read_in();
          if (input!='-')
          {
            error_po();
            fprintf(stderr,M_NOCNTRL);
            return(ERROR);
          }
          /*}}}  */
          read_in();
          if (input=='?') tk_char=127;
          else tk_char=toupper(input)-'A'+1;
          read_in();
          return(CHAR);
        }
        /*}}}  */
        /*{{{  Char-handling*/
        case '"':
        {
          tk_char=read_in();
          if (input<' ' || input>127)
          {
            error_po();
            fprintf(stderr,M_NOCHAR);
            return(ERROR);
          }
          read_in();
          return(CHAR);
          break;
        }
        /*}}}  */
        /*{{{  Alias-handling*/
        case '$':
        {
          char *s=tk_string;
          ALIAS **a=func_key;
        
          read_in();
          /*{{{  read aliasname*/
          while (input!=' ' && input!='\n'
          && input!='=' && input!=EOF && input!=')')
          {
            *s++=input;
            read_in();
          }
          *s='\0';
          /*}}}  */
          while (*a!=NULL &&
          strcmp((*a)->key_name,tk_string)) a++;
          if (*a!=NULL) { tk_alias = *a; return(DOLLAR); }
          error_po();
          fprintf(stderr,M_NOALIAS);
          return(ERROR);
        }
        /*}}}  */
        /*{{{  default=error*/
        default :
        {
          error_po();
          fprintf(stderr,M_NOCODE);
          return(ERROR);
        }
        /*}}}  */
      }
      /*}}}  */
    } else {
      /*{{{  macro-parser*/
      switch (input)
      {
        /*{{{  string-handling*/
        case '"':
        {
          TOKEN *t=tk_macro;
        
          read_in();
          do {
            /*{{{  Test auf < ' ' oder EOF*/
            if (input<' ') {
              error_po();
              fprintf(stderr,M_FALSEEND);
              return(ERROR);
            }
            /*}}}  */
            *t++ = (TOKEN) input;
            read_in();
          } while (   input!=' ' && input!='\n'
                   && input!=')' && input!=EOF && input!='"');
          *t= (TOKEN) 0;
          return(MACRO);
          break;
        }
        /*}}}  */
        /*{{{  opcode or keyword handling*/
        default :
        {
          char *s=tk_string;
          keywords *key=keytab;
          OP **a=new_op;
          VARS **v=vars;
        
          while (input!=' ' && input!='\n' && input!=')' && input!=','
          && input!='=' && input!=EOF && input!='(')
          { *s++=input;read_in(); }
          *s='\0';
          /*{{{  search as keyword*/
          while (key->name != NULL && strcmp(key->name,tk_string)) key++;
          if (key->value!=ERROR) return(key->value);
          /*}}}  */
          /*{{{  search as opcode*/
          tk_key=bindings;
          while (tk_key->num != O_NOP && strcmp(tk_key->name,tk_string))
          tk_key++;
          if (tk_key->num!=O_NOP) return(OPCODE);
          /*}}}  */
          /*{{{  search as operation*/
          while (*a!=NULL &&
          strcmp((*a)->op_name,tk_string)) a++;
          if (*a!=NULL) { tk_operation = *a; return(OPERATION); }
          /*}}}  */
          /*{{{  search as variable*/
          while (*v!=NULL &&
          strcmp((*v)->var_name,tk_string)) v++;
          if (*v!=NULL) { tk_var = *v; return(VARIABLE); }
          /*}}}  */
          return(NAME);
        }
        /*}}}  */
      }
      /*}}}  */
    }
  }
  return(ERROR);
}
/*}}}  */
/*{{{  get a token*/
#ifdef STD_C
tokens get_token(bool code)
#else
tokens get_token(code) bool code;
#endif
{
  tokens t=get_single_token(code);
 
  switch (t)
  {
    /*{{{  ENDFILE -> goto upper level, if any*/
    case ENDFILE:
    if (f_s_ptr)
    {
      fclose(in);
      free(name_stack[f_s_ptr]);
      in=file_stack[--f_s_ptr];
      current_line_no=po_stack[f_s_ptr];
      input=inp_stack[f_s_ptr];
      return(get_token(code));
    }
    else return(ENDFILE);
    /*}}}  */
    /*{{{  INCLUDE -> go to lower level*/
    case INCLUDE:
    /*{{{  check if correct include*/
    if (get_single_token(FALSE)!=NAME)
    {
      error_po();
      fprintf(stderr,M_NOFNAME);
      return(ERROR);
    }
    if (f_s_ptr==f_s_depth)
    {
      fprintf(stderr,M_DEPTH);
      return(ERROR);
    }
    /*}}}  */
    po_stack[f_s_ptr]=current_line_no;
    inp_stack[f_s_ptr]=input;
    file_stack[f_s_ptr++]=in;
    name_stack[f_s_ptr]=malloc(strlen(tk_string)+1);
    strcpy(name_stack[f_s_ptr],tk_string);
    /*{{{  try to open*/
    if ((in=fopen(tk_string,"r"))==NULL) {
      fprintf(stderr,F_INCLUDE,tk_string);
      f_s_ptr--;
      return(ERROR);
    }
    /*}}}  */
    read_in();
    current_line_no=0;
    return(get_token(code));
    /*}}}  */
    default: return(t);
  }
}
/*}}}  */
