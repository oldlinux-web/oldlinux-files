/*{{{  #includes*/
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
 
#include <local/bool.h>
 
#define FIELDEDT_C
 
#include "origami.h"
#include "macros.h"
/*}}}  */
 
/*{{{  variables*/
bool overwrite=FALSE,
     file_changed = FALSE,
     browse_mode = FALSE;
PRIVATE char deleted_ch = ' ';
PRIVATE int x_offset=0;
PRIVATE bool x_off_changed=FALSE;
element *part_line;
/*}}}  */
 
/*{{{  change_edit_pos*/
#define change_edit_pos(by)     (first_edit_pos += (by), pre_edit_pos += (by))
/*}}}  */
/*{{{  x_shift*/
#ifdef STD_C
int x_shift(int po)
#else
int x_shift(po) int po;
#endif
{
  int s=0;
 
  while (po>(LEN-1)) {s += SHIFT; po -= SHIFT;}
  x_off_changed=(x_offset!=s);
  return(x_offset=s);
}
/*}}}  */
/*{{{  refresh_field*/
#ifdef STD_C
PRIVATE void refresh_field(int froms, int tos, LOC_field *LINK)
#else
PRIVATE void refresh_field(froms, tos, LINK)
int froms, tos;
LOC_field *LINK;
#endif
{
  int lg=strlen(LINK->field_str);

  if (froms<1) froms = 1;
  x_shift(*LINK->field_pos-1);
  if ((tos-x_offset)>=LEN) tos=x_offset+LEN-1;
  if (tos<froms) tos=froms;
  if (froms>lg) LINK->field_str[froms]='\0';
  if (!x_off_changed) {
    gotoxy(START-1+froms-x_offset,cursor_level);
    if (froms<=lg)
      printf("%.*s",tos-froms+1,LINK->field_str+froms-1);
    else
      printf(" ");
  } else {
    gotoxy(START,cursor_level);
    if (froms<=lg)
      printf("%.*s",LEN-1,LINK->field_str+x_offset);
    else
      printf(" ");
  }
  if ((lg-x_offset)>(LEN-1)) {
    gotoxy(LEN, cursor_level);
    putchar(END_OF_LINE);
  } else if ((lg-x_offset)<(LEN-1))
    clreol();
  fflush(stdout);
}
/*}}}  */
/*{{{  restore cursor*/
#ifdef STD_C
PRIVATE void restore_cursor(LOC_field *LINK)
#else
PRIVATE void restore_cursor(LINK)
LOC_field *LINK;
#endif
{
  x_shift(*LINK->field_pos-1);
  if (x_off_changed) refresh_field(x_offset+1,x_offset+LEN,LINK);
  gotoxy((int)(START + (*LINK->field_pos-1-x_offset) % LEN), cursor_level);
}
/*}}}  */
/*{{{  move_cursor*/
#ifdef STD_C
PRIVATE void move_cursor(int dist, LOC_field *LINK)
#else
PRIVATE void move_cursor(dist, LINK)
int dist;
LOC_field *LINK;
#endif
{
  if (*LINK->field_pos+dist<=0 || *LINK->field_pos+dist>MAX_FIELD_SIZE+1)
    return;
  *LINK->field_pos += dist;
  if (*LINK->field_pos > strlen(LINK->field_str) + 1)
    strcat(LINK->field_str," ");
  else if (*LINK->field_pos == strlen(LINK->field_str))
    if (LINK->field_str[strlen(LINK->field_str) - 1] == ' ')
      LINK->field_str[strlen(LINK->field_str) - 1] = '\0';
}
/*}}}  */
/*{{{  insert_*/
#ifdef STD_C
PRIVATE void insert_(char ch, LOC_field *LINK)
#else
PRIVATE void insert_(ch, LINK)
char ch;
LOC_field *LINK;
#endif
{
  int lg=strlen(LINK->field_str);

  /*{{{  errorcheck && prepare*/
    if (  (   *LINK->field_pos>pre_edit_pos
           || ch!=' '
           || *LINK->field_pos<= LINK->ind
           || overwrite
          )
        &&
          *LINK->field_pos < first_edit_pos) {
    vmessage(M_ERR_PO);
    return;
  }
  /*}}}  */
  if (overwrite) {
    /*{{{  replace char*/
    if ((*LINK->field_pos)>lg) {
      while ((*LINK->field_pos)>lg) LINK->field_str[lg++]=' ';
      LINK->field_str[lg]='\0';
    }
    LINK->field_str[(*LINK->field_pos)-1]=ch;
    /*}}}  */
  } else {
    /*{{{  insert char*/
    if (lg>=MAX_FIELD_SIZE) {
      message(M_LONG_LINE,TRUE);
      return;
    } else {
      if (*LINK->field_pos==1) {
        *line_buffer=ch;
        strcpy(line_buffer+1,LINK->field_str);
      } else
        sprintf(line_buffer,"%-*.*s%c%s",
                *LINK->field_pos - 1,
                *LINK->field_pos - 1,
                LINK->field_str,
                ch,
                ((lg >= *LINK->field_pos)
                 ? (LINK->field_str+ *LINK->field_pos -1)
                 : "")
               );
      strcpy(LINK->field_str, line_buffer);
    }
    /*}}}  */
  }
  if (*LINK->field_pos <= pre_edit_pos) change_edit_pos(1);
  (*LINK->field_pos)++;
  refresh_field(*LINK->field_pos - 1,*LINK->field_pos - 1 +LEN, LINK);
}
/*}}}  */
/*{{{  case_word*/
#ifdef STD_C
PRIVATE void case_word(LOC_field *LINK,int mode)
#else
PRIVATE void case_word(LINK, mode) LOC_field *LINK; int mode;
#endif
{
  int pos;
  char *run;

  if (*LINK->field_pos > strlen(LINK->field_str)) return;
  pos = *LINK->field_pos;
  run = &LINK->field_str[*LINK->field_pos-1];
  while (isalpha(*run)) {
    switch (mode) {
      case 0: *run=toupper(*run); mode=1; break;
      case 1: *run=tolower(*run); break;
      case 2: *run=toupper(*run); break;
    }
    (*LINK->field_pos)++;
    run++;
  }
  refresh_field(pos, strlen(LINK->field_str), LINK);
}
/*}}}  */
/*{{{  delete__*/
#ifdef STD_C
PRIVATE void delete__(LOC_field *LINK)
#else
PRIVATE void delete__(LINK)
LOC_field *LINK;
#endif
{
  int lg=strlen(LINK->field_str);
 
  /*{{{  before linestart*/
  if (*LINK->field_pos <= LINK->ind + 1) {
    LINK->field_key = FALSE;
    return;
  }
  /*}}}  */
  /*{{{  inv edit po*/
  if (*LINK->field_pos <= first_edit_pos && *LINK->field_pos > pre_edit_pos) {
    vmessage(M_ERR_PO);
    return;
  }
  /*}}}  */
  deleted_ch = LINK->field_str[*LINK->field_pos - 2];
  strcpy(line_buffer,LINK->field_str);
  if (*LINK->field_pos-2==0)
    strcpy(LINK->field_str,line_buffer+1);
  else
    sprintf(LINK->field_str,
            "%.*s%s",
            *LINK->field_pos-2,
            line_buffer,
            (((*LINK->field_pos - 1)<=lg)
             ? line_buffer + *LINK->field_pos - 1
             : "")
           );
  if (*LINK->field_pos <= pre_edit_pos) change_edit_pos(-1);
  (*LINK->field_pos)--;
  refresh_field(*LINK->field_pos,lg, LINK);
}
/*}}}  */
/*{{{  delete_under*/
#ifdef STD_C
PRIVATE void delete_under(LOC_field *LINK)
#else
PRIVATE void delete_under(LINK)
LOC_field *LINK;
#endif
{
  int lg=strlen(LINK->field_str);
 
  /*{{{  do not del_under*/
  if (   *LINK->field_pos > MAX_FIELD_SIZE
      || *LINK->field_pos <= LINK->ind
      || (*LINK->field_pos<first_edit_pos && *LINK->field_pos>=pre_edit_pos))
  {
    vmessage(M_ERR_PO);
    return;
  } else if (*LINK->field_pos>lg) {
    LINK->field_key = FALSE;
    return;
  } else if ((deleted_ch = LINK->field_str[*LINK->field_pos - 1])=='\0')
    return;
  /*}}}  */
  strcpy(line_buffer,LINK->field_str);
  if (*LINK->field_pos-1==0)
    strcpy(LINK->field_str,line_buffer+1);
  else
    sprintf(LINK->field_str,
            "%.*s%s",
            *LINK->field_pos - 1,
            line_buffer,
            (lg < *LINK->field_pos) ? "" : line_buffer+*LINK->field_pos
           );
  if (lg < *LINK->field_pos) strcat(LINK->field_str, " ");
  if (*LINK->field_pos<pre_edit_pos) change_edit_pos(-1);
  refresh_field(*LINK->field_pos,lg+1,LINK);
}
/*}}}  */
/*{{{  char_type*/
#ifdef STD_C
PRIVATE uchar char_type(char ch)
#else
PRIVATE uchar char_type(ch)
char ch;
#endif
{
  if (ch != ' ')
    if (isdigit(ch) || isalpha(ch))
      return 1;
    else
      return 2;
  else
    return 0;
}
/*}}}  */
/*{{{  symbol_start*/
#ifdef STD_C
PRIVATE bool symbol_start(LOC_field *LINK)
#else
PRIVATE bool symbol_start(LINK)
LOC_field *LINK;
#endif
{
  bool b;

  b = (*LINK->field_pos > 1 && *LINK->field_pos <= strlen(LINK->field_str));
  if (b) {
    if (LINK->field_str[*LINK->field_pos - 1] == ' ')
      return FALSE;
    else
      return (char_type(LINK->field_str[*LINK->field_pos - 2]) !=
              char_type(LINK->field_str[*LINK->field_pos - 1]));
  } else
    return TRUE;
}
/*}}}  */
/*{{{  field_edit*/
#ifdef STD_C
TOKEN field_edit(bool *keypad, int *field_pos_, char *field_str_)
#else
TOKEN field_edit(keypad, field_pos_, field_str_)
bool *keypad;
int *field_pos_;
char *field_str_;
#endif
{
  /*{{{  declarations*/
  LOC_field V;
  TOKEN Result, ch;
  int temp, tempf, temppo;
  element *WITH;
  /*}}}  */
 
  /*{{{  prepare editing field*/
  x_offset=0;
  V.field_pos = field_pos_;
  x_shift(*V.field_pos);
  V.field_str = field_str_;
  V.ind = current->UU.U1.indent;
  trailing_spaces(V.field_str);
  if (strlen(V.field_str) < V.ind) V.field_str[V.ind] = '\0';
  if (strlen(V.field_str) > MAX_FIELD_SIZE) V.field_str[MAX_FIELD_SIZE] = '\0';
  if (*V.field_pos <= V.ind) *V.field_pos = V.ind + 1;
  if (x_off_changed) refresh_field(x_offset+1, strlen(V.field_str), &V);
  /*}}}  */
  do {
    V.field_key = FALSE;
    do {
      restore_cursor(&V);
      ch = edit_key(keypad);
    } while (!valid_key(keypad, &ch));
    V.field_key = valid_field_key(keypad, &ch);
    if (V.field_key) {
      no_message();
      if (*keypad) {
        switch (ch) {   /*if and case*/
          /*{{{  M_BEGIN_OF_LINE*/
          case M_BEGIN_OF_LINE: {
            int po=1;
            char *t=V.field_str;
          
            while (*t && *t==' ') {t++;po++;}
            macro_tag=(po==*V.field_pos);
            break;
          }
          /*}}}  */
          /*{{{  M_TOP_OF_FOLD*/
          case M_TOP_OF_FOLD: {
            macro_tag=(current==head->next);
            break;
          }
          /*}}}  */
          /*{{{  M_BOT_OF_FOLD*/
          case M_BOT_OF_FOLD: {
            macro_tag=(current==tail);
            break;
          }
          /*}}}  */
          /*{{{  M_TEST_CHAR*/
          case M_TEST_CHAR: {
            char c=get_arg();
          
            if (V.field_str[*(V.field_pos)-1]=='\0') macro_tag=(c==' ');
            else macro_tag=(V.field_str[*(V.field_pos)-1]==c);
            break;
          }
          /*}}}  */
          /*{{{  M_END_OF_LINE*/
          case M_END_OF_LINE: {
            char *c = &V.field_str[*(V.field_pos)-1];
          
            macro_tag=TRUE;
            if (*(V.field_pos)>strlen(V.field_str)) break;
            while (*c)
              if (*c++!=' ') { macro_tag=FALSE; break;}
            break;
          }
          /*}}}  */
          /*{{{  M_GOTO..*/
          case M_GO_X_POS:
          case M_GO_COUNTER_X_POS: {
            int diff;
          
            if (ch==M_GO_X_POS)
              diff=get_arg() -*V.field_pos;
            else
              diff=macro_int[get_arg()] - *V.field_pos;
            if (diff>0)
              while (diff-- > 0) move_cursor(1, &V);
            else
              while (diff++ < 0) move_cursor(-1, &V);
            break;
          }
          /*}}}  */
          /*{{{  M_BEHIND_COUNTER_X_POS*/
          case M_BEHIND_COUNTER_X_POS: {
            macro_tag = *V.field_pos>macro_int[get_arg()];
            break;
          }
          /*}}}  */
          /*{{{  M_POS_TO_COUNTER*/
          case M_POS_TO_COUNTER: {
            macro_int[get_arg()]= *V.field_pos;
            break;
          }
          /*}}}  */
          /*{{{  O_UNDEL_CHAR*/
          case O_UNDEL_CHAR: {
            insert_(deleted_ch, &V);
            break;
          }
          /*}}}  */
          /*{{{  O_START_OF_LINE*/
          case O_START_OF_LINE: {
            *V.field_pos = V.ind + 1;
            trailing_spaces(V.field_str);
            if (*V.field_pos < strlen(V.field_str))
              while (V.field_str[*V.field_pos - 1] == ' ')
                (*V.field_pos)++;
            break;
          }
          /*}}}  */
          /*{{{  O_END_OF_LINE*/
          case O_END_OF_LINE: {
            trailing_spaces(V.field_str);
            *V.field_pos = strlen(V.field_str) + 1;
            if (*V.field_pos <= V.ind) *V.field_pos = V.ind + 1;
            break;
          }
          /*}}}  */
          /*{{{  O_WORD_RIGHT*/
          case O_WORD_RIGHT: {
            if (*V.field_pos < last_char(V.field_str))
              do move_cursor(1, &V);
              while (!symbol_start(&V));
            else
              move_cursor(1, &V);
            break;
          }
          /*}}}  */
          /*{{{  O_WORD_LEFT*/
          case O_WORD_LEFT: {
            if (*V.field_pos > first_char(V.field_str))
              do move_cursor(-1, &V);
              while (!(symbol_start(&V) && *V.field_pos<=strlen(V.field_str)));
            else {
              *V.field_pos = V.ind + 1;
            }
            break;
          }
          /*}}}  */
          /*{{{  O_DELTO_EOL*/
          case O_DELTO_EOL: {
            if (*V.field_pos >= first_edit_pos) {
              temp = strlen(V.field_str);
              if (*V.field_pos == 1)
                *V.field_str='\0';
              else {
                strcpy(line_buffer,V.field_str);
                sprintf(V.field_str, "%.*s",*V.field_pos - 1, line_buffer);
              }
              refresh_field(*V.field_pos, temp, &V);
            }
            break;
          }
          /*}}}  */
          /*{{{  O_LEFT*/
          case O_LEFT: {
            move_cursor(-1, &V);
            break;
          }
          /*}}}  */
          /*{{{  O_RIGHT*/
          case O_RIGHT: {
            move_cursor(1, &V);
            break;
          }
          /*}}}  */
          /*{{{  O_DEL_CHAR_R*/
          case O_DEL_CHAR_R: {
            delete_under(&V);
            break;
          }
          /*}}}  */
          /*{{{  O_DELETE*/
          case O_DELETE: {
            delete__(&V);
            break;
          }
          /*}}}  */
          /*{{{  O_RETURN*/
          case O_RETURN:   /* cr */ {
            if (*V.field_pos<pre_edit_pos)
              vmessage(M_ERR_PO);
            else if (!select_on) {
              /*{{{  not folding*/
              temp = strlen(V.field_str);
              tempf = first_char(V.field_str);
              temppo = tempf>temp ? *V.field_pos : tempf;
              WITH = current;
              if (current != tail || *V.field_pos == 1) {
                /*{{{  not at end*/
                if (!entered(current) || current != head->next ||
                    *V.field_pos > temp) {
                  if (WITH->foldline == NOT_FOLD || *V.field_pos <= tempf ||
                      *V.field_pos > temp) {
                    /*{{{  do the return*/
                    split_line = (*V.field_pos > tempf);
                    proc_new(&part_line);
                    part_line->UU.U1.indent = V.ind;
                    if (split_line) {
                      copy_line_to_parts(copy_to_end(line_buffer,
                                                     V.field_str,*V.field_pos),
                                         &part_line);
                      sprintf(V.field_str,"%.*s",*V.field_pos - 1,
                              strcpy(line_buffer, V.field_str));
                      refresh_field(*V.field_pos, temp, &V);
                    }
                    *V.field_pos = temppo;
                    spaces(line_buffer,*V.field_pos - V.ind - 1);
                    strcat(line_buffer,part_line->strng);
                    copy_line_to_parts(line_buffer, &part_line);
                    V.field_key = FALSE;
                    /*}}}  */
                  }
                }
                /*}}}  */
              }
              /*}}}  */
            }
            break;
          }
          /*}}}  */
          /*{{{  O_TRANSPOSE_CHARACTERS*/
          case O_TRANSPOSE_CHARACTERS: {
            if (*V.field_pos==1) break;
            { char *s = &V.field_str[(*V.field_pos)-1];
              char c = *(s-1);
          
              *(s-1) = *s;
              *s=c;
              refresh_field(*V.field_pos -2,strlen(V.field_str), &V);
              move_cursor(1,&V);
              break;
            }
          }
          /*}}}  */
          /*{{{  O_CAPITALIZE_WORD*/
          case O_CAPITALIZE_WORD: {
            case_word(&V, 0);
            break;
          }
          /*}}}  */
          /*{{{  O_LOWERCASE_WORD*/
          case O_LOWERCASE_WORD: {
            case_word(&V, 1);
            break;
          }
          /*}}}  */
          /*{{{  O_UPPERCAE_WORD*/
          case O_UPPERCASE_WORD: {
            case_word(&V, 2);
            break;
          }
          /*}}}  */
          /*{{{  O_A_VIEW*/
          case O_A_VIEW: {
            browse_mode=TRUE;
            title_op(SET_VIEW);
            break;
          }
          /*}}}  */
          /*{{{  O_D_VIEW*/
          case O_D_VIEW: {
            browse_mode=FALSE;
            title_op(RESET_VIEW);
            break;
          }
          /*}}}  */
          /*{{{  O_A_OVER*/
          case O_A_OVER: {
            overwrite=TRUE;
            title_op(SET_OVER);
            break;
          }
          /*}}}  */
          /*{{{  O_D_OVER*/
          case O_D_OVER: {
            overwrite=FALSE;
            title_op(RESET_OVER);
            break;
          }
          /*}}}  */
        }
      }
      if (!*keypad) insert_(ch, &V);
    }
  } while (V.field_key != FALSE);
  Result = ch;
  trailing_spaces(V.field_str);
  if (x_offset) {
    int x= *V.field_pos;
 
    *V.field_pos=0;
    refresh_field(1,LEN-1,&V);
    *V.field_pos=x;
  }
  return Result;
}
/*}}}  */
