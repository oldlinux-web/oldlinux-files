#include "../common/os.h"
#include "../common/rcformat.h"
#include "short.h"

#define PRIVATE static
 
/*{{{  STD_C*/
#ifdef STD_C
#  define P(s) s
#else
#  define P(s) ()
#endif
/*}}}  */
/*{{{  constants*/
#define alias_lg 128
#define macro_lg 2048
#define name_lg  128
/*}}}  */
/*{{{  token-definition*/
typedef enum {
  INCLUDE,
  ERROR,
  BINDNAME,
  ENDFILE,
  BEGIN,
  END,
  DOLLAR,
  META,
  CHAR,
  COMMA,
  KEYALIAS,
  KEYDEF,
  DEFOP,
  DEFMACRO,
  INITMACRO,
  DEFMARK,
  FORWARD,
  DEFAUTO,
  VARIABLE,
  OPCODE,
  NAME,
  MACRO,
  OPERATION,
  REPEAT,
  CASE,
  ESAC,
  DEFAULT,
  DO,
  WHILE,
  IF,
  ELSE,
  FI,
  NOT,
  AND,OR,
  COUNTER,
  TEST_FILED,
  TEST_FOLD_LINE,
  TEST_BEGIN_FOLD,
  TEST_END_FOLD,
  TEST_TEXT,
  TEST_TOP,
  TEST_BOTTOM,
  TEST_CHAR,
  TEST_CHANGED,
  TEST_LANG,
  COUNTER_NULL,
  TEST_BEGIN_LINE,
  TEST_END_LINE,
  SET_COUNTER,
  READ_REPEAT,
  ADD_COUNTER,
  SUM_COUNTER,
  INV_COUNTER,
  C_POSITIV,
  TEST_BEHIND_COUNTER,
  GOTO_X,
  GOTO_COUNTER,
  STORE_X,
  STORE_Y,
  GOTO_Y,
  EXIT,
  MES_EXIT,
  PROMPT,
  PROMPT_COUNTER
} tokens;
/*}}}  */
/*{{{  typedefs*/
typedef struct { char *name; tokens value; } keywords;
typedef struct { char *key_name; char *code; int length; } ALIAS;
typedef struct {
  char *op_name;
  bool defined;
  int length,place;
  TOKEN *ops;
} OP;
typedef struct { char *var_name; int no; } VARS;
typedef struct { char *name; TOKEN num; } KEYNAME;
/*}}}  */
/*{{{  KEYBIND*/
#ifndef KEYBIND_C
  extern void write_break_rc P((char));
  extern void write_mark_rc P((char*,char[4][tag_length-1]));
  extern bool write_name_rc P((char*));
  extern write_auto_rc P((int));
  extern bool write_macro_rc P((readtags,TOKEN,int,TOKEN*));
  extern void write_bind P((char *,char *));
  extern int k_counter;
  extern FILE *rc,*bind,*in,*popen();
  extern int int_no;
  extern bool verbose;
  extern char filebase[60];
#endif
/*}}}  */
/*{{{  PARCON*/
#ifndef PARCON
  extern TOKEN *parse_cond P((TOKEN*));
#endif
/*}}}  */
/*{{{  PARSMAC*/
#ifdef PARSMAC
  extern char nullc;
  extern char *op_def_name;
  extern TOKEN *m_def;
#else
  extern TOKEN *parse_macro P((TOKEN*));
#endif
/*}}}  */
/*{{{  CODEHELP*/
#ifndef CODEHELP
  extern TOKEN *get_message P((TOKEN*));
  extern TOKEN *generate_jmp P((TOKEN,TOKEN*,TOKEN*));
  extern char *parse_keysequence P((char*,char*));
  extern TOKEN *put_var P((TOKEN*));
#endif
/*}}}  */
/*{{{  PARSER*/
#ifndef PARSER_C
  extern char abort_key;
  extern bool ab_set;
  extern bool process_file P((void));
#endif
/*}}}  */
/*{{{  KEYTAB*/
#ifndef KEYTAB_C
  extern bool add_keysequence P((TOKEN,char*,int));
  extern bool write_keynodes P((void));
  extern KEY *init_keytab P((void));
#endif
/*}}}  */
/*{{{  MESSAGES*/
#ifndef MESSAGE
  extern char F_BELONGS[];
  extern char F_BINDS[];
  extern char F_DEMAC[];
  extern char F_DNDMAC[];
  extern char F_DUPLICATE[];
  extern char F_ERRORPO[];
  extern char F_INCLUDE[];
  extern char F_INTS[];
  extern char F_ISMASKED[];
  extern char F_MARKT[];
  extern char F_MASKS[];
  extern char F_MDECLARE[];
  extern char F_NOBIND[];
  extern char F_NODES[];
  extern char F_NOKEYNUMBER[];
  extern char F_NOPIPE[];
  extern char F_NORC[];
  extern char F_OPERATION[];
  extern char F_SEQ[];
  extern char F_STMAC[];
  extern char M_ABORTUSED[];
  extern char M_ALIEND[];
  extern char M_ANDBEGIN[];
  extern char M_ANDCOMMA[];
  extern char M_ANDEND[];
  extern char M_CASEEND[];
  extern char M_COMMANDLINE[];
  extern char M_DEFOEND[];
  extern char M_DEFONAME[];
  extern char M_DEPTH[];
  extern char M_DUPDEF[];
  extern char M_DUPKEY[];
  extern char M_EXPALIAS[];
  extern char M_FALSEEND[];
  extern char M_FIXFULL[];
  extern char M_INCORRECTFILES[];
  extern char M_INVMNAME[];
  extern char M_KEYEND[];
  extern char M_KEYNAME[];
  extern char M_KSNOBEGIN[];
  extern char M_MACBEGIN[];
  extern char M_MACNAME[];
  extern char M_MARKNAME[];
  extern char M_MDEFNAME[];
  extern char M_DUPAUTO[];
  extern char M_DUPAB[];
  extern char M_NOABORT[];
  extern char M_NOADDCOUNTER[];
  extern char M_NOALIAS[];
  extern char M_NOBFILE[];
  extern char M_NOBINDNAME[];
  extern char M_NOCHAR[];
  extern char M_NOCNTRL[];
  extern char M_NOCODE[];
  extern char M_NOCOMMAND[];
  extern char M_NODEBOUND[];
  extern char M_NOELSEFI[];
  extern char M_NOESAC[];
  extern char M_NOFI[];
  extern char M_NOFNAME[];
  extern char M_NOGOTO[];
  extern char M_NOJUMP[];
  extern char M_NOKCOMMAND[];
  extern char M_NOKEYCODE[];
  extern char M_NOMACROTAG[];
  extern char M_NOMEMORY[];
  extern char M_NOMESSAGE[];
  extern char M_NOMETA[];
  extern char M_NOMSTRING[];
  extern char M_NOPROMPT[];
  extern char M_NOREPEAT[];
  extern char M_NOTBEGIN[];
  extern char M_NOTEND[];
  extern char M_NOTEST[];
  extern char M_NOVAR[];
  extern char M_NOWHILE[];
  extern char M_ORBEGIN[];
  extern char M_ORCOMMA[];
  extern char M_OREND[];
  extern char M_SORTFAILED[];
  extern char M_TCNEEDSCHAR[];
  extern char M_WANTBEGIN[];
  extern char M_WANTCHAR[];
  extern char M_WANTEND[];
#endif
/*}}}  */
/*{{{  SCANNER*/
#ifndef SCANNER
  extern tokens get_token P((bool));
  extern void error_po P((void));
  extern void creat_var P((char*));
  extern void init_alias P((void));
  extern void sc_list_keys P((void));
  extern int read_in P((void));
  extern void creat_alias P((char*,char*,int));
  extern creat_op P((char*,bool,int,TOKEN*,int));
  extern char   tk_char;
  extern ALIAS  *tk_alias;
  extern KEYNAME *tk_key;
  extern OP     *tk_operation;
  extern VARS   *tk_var;
  extern char   tk_string[];
  extern TOKEN  tk_macro[];
  extern ALIAS  *func_key[64];
  extern OP     *new_op[64];
  extern VARS   *vars[128];
  extern char   *source;
#else
  extern bool verbose;
#endif
/*}}}  */
/*{{{  NAME*/
#ifndef NAME_C
  extern KEYNAME bindings[];
  extern keywords keytab[];
#endif
/*}}}  */

#undef P
