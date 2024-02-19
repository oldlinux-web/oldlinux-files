/*{{{  #includes*/
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#include <local/bool.h>
 
#define KEYTAB_C

#include "origami.h"
/*}}}  */
 
/*{{{  variables*/
PRIVATE KEY *toplevel;
PRIVATE KEY *find_current;
/*}}}  */
/*{{{  key_convert_lists key_convert_list*/
/* valid field key, browse key, screen key */
key_convert_lists key_convert_list = {
  /* O_NOP              */                {1,1,1},
  /*{{{  for macros*/
  /* M_ADD_COUNTER      */                {0,1,0},
  /* M_BEGIN_FOLD_COMMENT */              {0,1,0},
  /* M_BEGIN_OF_LINE    */                {1,1,0},
  /* M_BEHIND_COUNTER_X */                {1,1,0},
  /* M_BOT_OF_FOLD      */                {1,1,0},
  /* M_CALL             */                {0,1,0},
  /* M_CHANGED          */                {0,1,0},
  /* M_CLOSED_FOLD      */                {0,1,0},
  /* M_END_FOLD_COMMENT */                {0,1,0},
  /* M_END_MACRO        */                {0,1,1},
  /* M_END_OF_LINE      */                {1,1,0},
  /* M_EXIT             */                {0,1,0},
  /* M_FILED_FOLD       */                {0,1,0},
  /* M_GO_COUNTER_X_POS */                {1,1,0},
  /* M_GO_LINE          */                {0,1,0},
  /* M_GO_X_POS         */                {1,1,0},
  /* M_INT_STRING       */                {0,1,0},
  /* M_INV_COUNTER      */                {0,1,0},
  /* M_JMP              */                {0,1,0},
  /* M_JMP_FALSE        */                {0,1,0},
  /* M_JMP_TRUE         */                {0,1,0},
  /* M_LANGUAGE         */                {0,1,0},
  /* M_NOT              */                {0,1,0},
  /* M_NULL_COUNTER     */                {0,1,0},
  /* M_POSITIV_COUNTER  */                {0,1,0},
  /* M_POS_TO_COUNTER   */                {1,1,0},
  /* M_PROMPT           */                {0,0,0},
  /* M_READ_REPEAT      */                {0,0,0},
  /* M_SET_COUNTER      */                {0,1,0},
  /* M_STORE_LINE_NO    */                {0,1,0},
  /* M_SUM_COUNTER      */                {0,1,0},
  /* M_TEST_CHAR        */                {1,1,0},
  /* M_TEXTLINE         */                {0,1,0},
  /* M_TOP_OF_FOLD      */                {1,1,0},
  /*}}}  */
  /* O_ATTACH_FILE      */                {0,0,0},
  /* O_AUTO_FOLD        */                {0,0,0},
  /* O_AUTO_SAVE        */                {0,1,0},
  /* O_A_AUTO_SAVE      */                {0,1,1},
  /* O_A_ECHO           */                {0,1,0},
  /* O_A_OVER           */                {1,1,0},
  /* O_A_VIEW           */                {1,1,0},
  /* O_BOT_OF_FOLD      */                {0,1,1},
  /* O_BREAK            */                {0,1,1},
  /* O_CAPITALIZE_WORD  */                {1,0,0},
  /* O_CLOSE_FOLD       */                {0,1,0},
  /* O_COPY             */                {0,0,0},
  /* O_COPY_PICK        */                {0,1,1},
  /* O_CREATE_FOLD      */                {0,0,1},
  /* O_DEF_FIX          */                {0,1,0},
  /* O_DEF_MACRO        */                {0,1,0},
  /* O_DELETE           */                {1,0,0},
  /* O_DELTO_EOL        */                {1,0,0},
  /* O_DEL_CHAR_R       */                {1,0,0},
  /* O_DEL_LINE         */                {0,0,0},
  /* O_DESCRIBE_BINDINGS*/                {0,1,0},
  /* O_DOWN             */                {0,1,1},
  /* O_D_AUTO_SAVE      */                {0,1,1},
  /* O_D_ECHO           */                {0,1,0},
  /* O_D_OVER           */                {1,1,0},
  /* O_D_VIEW           */                {1,1,0},
  /* O_END_OF_LINE      */                {1,1,0},
  /* O_ENTER_FOLD       */                {0,1,0},
  /* O_EXIT_FOLD        */                {0,1,0},
  /* O_FILE_FOLD        */                {0,0,0},
  /* O_FILTER_BUFFER    */                {0,0,0},
  /* O_FIND             */                {0,1,0},
  /* O_FIND_REVERSE     */                {0,1,0},
  /* O_FINISH           */                {0,1,0},
  /* O_FIRST_LINE_TO_FOLD_HEADER */       {0,0,0},
  /* O_FOLD_INFO        */                {0,1,0},
  /* O_GOTO_LINE        */                {0,1,0},
  /* O_HELP             */                {0,1,0},
  /* O_INSERT_FILE      */                {0,0,0},
  /* O_ITS_REVERSE      */                {0,1,0},
  /* O_ITS_SEARCH       */                {0,1,0},
  /* O_LANGUAGE         */                {0,0,0},
  /* O_LEFT             */                {1,1,0},
  /* O_LOWERCASE_WORD   */                {1,0,0},
  /* O_MOVE             */                {0,0,0},
  /* O_NEXT_FILE        */                {0,1,1},
  /* O_OPEN_FOLD        */                {0,1,0},
  /* O_OPEN_NEW_FILE    */                {0,1,1},
  /* O_PAGE_DOWN        */                {0,1,1},
  /* O_PAGE_UP          */                {0,1,1},
  /* O_PICK             */                {0,0,0},
  /* O_PIPE_FROM_COMMAND*/                {0,0,0},
  /* O_PIPE_TO_COMMAND  */                {0,1,0},
  /* O_PREV_FILE        */                {0,1,1},
  /* O_PROMPT           */                {0,1,1},
  /* O_PUT_PICK         */                {0,0,0},
  /* O_QUERY_REPLACE    */                {0,0,0},
  /* O_QUIT             */                {0,1,0},
  /* O_REFRESH          */                {0,1,1},
  /* O_REMOVE_FOLD      */                {0,0,0},
  /* O_REPLACE          */                {0,0,0},
  /*{{{  argument-repeat*/
  /* O_REP_0            */                {0,1,0},
  /* O_REP_1            */                {0,1,0},
  /* O_REP_2            */                {0,1,0},
  /* O_REP_3            */                {0,1,0},
  /* O_REP_4            */                {0,1,0},
  /* O_REP_5            */                {0,1,0},
  /* O_REP_6            */                {0,1,0},
  /* O_REP_7            */                {0,1,0},
  /* O_REP_8            */                {0,1,0},
  /* O_REP_9            */                {0,1,0},
  /*}}}  */
  /* O_RESET_USER_MODE  */                {0,1,0},
  /* O_RETURN           */                {1,0,0},
  /* O_RIGHT            */                {1,1,0},
  /* O_SAVE_FILE        */                {0,1,1},
  /* O_SET_USER_MODE    */                {0,1,0},
  /* O_SHELL            */                {0,1,0},
  /* O_SHELL_COMMAND    */                {0,1,0},
  /* O_START_OF_LINE    */                {1,1,0},
  /* O_SUSPEND          */                {0,1,0},
  /* O_TOP_OF_FOLD      */                {0,1,1},
  /* O_TRANSPOSE_CHARACTERS*/             {1,0,0},
  /* O_UNDEL_CHAR       */                {1,0,0},
  /* O_UNDEL_LINE       */                {0,0,0},
  /* O_UP               */                {0,1,1},
  /* O_UPPER_CASE       */                {1,0,0},
  /* O_WORD_LEFT        */                {1,1,0},
  /* O_WORD_RIGHT       */                {1,1,0},
  /* O_WRITE_FILE       */                {0,1,0},
};
/*}}}  */
 
/*{{{  set_key_top*/
#ifdef STD_C
void set_key_top (KEY *top)
#else
void set_key_top (top)
KEY *top;
#endif
{
  toplevel = find_current = top;
}
/*}}}  */
/*{{{  find_key*/
#ifdef STD_C
int find_key (int key)
#else
int find_key (key)
int key;
#endif
{
  KEY *find=NULL;
 
  /*{{{  search in tree*/
  for (find = find_current->n_level;find;find = find->next)
    if (key == find->key) break;
  /*}}}  */
  /*{{{  sequence not found, end search*/
  if (find == NULL) {
    find_current = toplevel;
    return (0);
  }
  /*}}}  */
  /*{{{  an encoded sequence is found, end search*/
  if (find->code) {
    find_current = toplevel;
    return (find->code);
  }
  /*}}}  */
  /*{{{  the search continues*/
  if (find->n_level) {
    find_current = find;
    return (1);
  }
  /*}}}  */
  /*{{{  no level below this, the search ends*/
  find_current = toplevel;
  return (0);
  /*}}}  */
}
/*}}}  */
/*{{{  valid_key*/
#ifdef STD_C
bool valid_key(bool *keypad, TOKEN *ch)
#else
bool valid_key(keypad, ch) bool *keypad; TOKEN *ch;
#endif
{
  bool browse_key;

  browse_key = (*keypad && (*ch >= O_NOP) && key_convert_list[*ch - O_NOP][1]);
  if (browse_mode && !browse_key) {
    message(M_VIEW,TRUE);
    return FALSE;
  } else {
    if (!browse_key && !file_changed) {
      file_changed = TRUE;
      title_op(SET_CHANGE);
    }
    return TRUE;
  }
}
/*}}}  */
/*{{{  valid_field_key check if token is a correct field_key*/
#ifdef STD_C
bool valid_field_key(bool *keypad,TOKEN *ch)
#else
bool valid_field_key(keypad, ch) bool *keypad; TOKEN *ch;
#endif
{
  if (*keypad) return (key_convert_list[*ch - O_NOP][0]);
  else return TRUE;
}
/*}}}  */
/*{{{  valid_screen_key*/
#ifdef STD_C
bool valid_screen_key(bool *keypad,TOKEN *ch)
#else
bool valid_screen_key(keypad, ch) bool *keypad; TOKEN *ch;
#endif
{
  if (*keypad && select_on)
    return (key_convert_list[*ch - O_NOP][2]);
  else
    return TRUE;
}
/*}}}  */
