/*{{{  #includes*/
#include <sys/types.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
 
#include <local/bool.h>

#define FINDS_C
 
#include "origami.h"
#include "macros.h"
/*}}}  */

/*{{{  variables*/
char item_to_look_for[SEARCHLEN];
PRIVATE char replace_item[SEARCHLEN];
/*}}}  */
 
/*{{{  get_line_no  (from entered fold to current line)*/
#ifdef STD_C
int get_line_no(void)
#else
int get_line_no()
#endif
{
  int current_line_no=0;
  element *p;
 
  p = head->next;
  while (p!=current && p != tail) {
    if (p->foldline==START_FILED) current_line_no += 3; else current_line_no++;
    move_on(&p);
  }
  return current_line_no;
}
/*}}}  */
/*{{{  line_no      (from real head to given line)*/
#ifdef STD_C
int line_no(element *q)
#else
int line_no(q)
element *q;
#endif
{
  int i;
  element *p;
 
  i = 0;
  p = real_head;
  while (p != q) {
    if (p->foldline==START_FILED) i += 3; else i++;
    move_on(&p);
  }
  return i;
}
/*}}}  */
/*{{{  close_and_open_folds*/
#ifdef STD_C
void close_and_open_folds(element *qq)
#else
void close_and_open_folds(qq)
element *qq;
#endif
{
  element *q, *old_q;
 
  q = qq;
  while (q != head) {
    old_q = q;
    q = q->prec;
    if (q->foldline == START_FOLD && q->fold == old_q) pre_open_fold(q);
    if (q->foldline == END_FOLD) {
      if (q->other_end->foldline == START_OPEN_FOLD) {
        q = q->other_end;
        close_fold_at(q);
      }
    }
  }
  q = qq;
  while (q != tail) {
    q = q->next;
    if (q->foldline == START_OPEN_FOLD) close_fold_at(q);
  }
}
/*}}}  */
/*{{{  find_item*/
#ifdef STD_C
bool find_item(int *cursor_x_pos,bool backup)
#else
bool find_item(cursor_x_pos,backup) int *cursor_x_pos; bool backup;
#endif
{
  int position;
  element *p, *q;
  bool found;
  char *f;
 
  p = current;
  found = FALSE;
  if (backup) {
    /*{{{  search reverse*/
    while (p != head && !found) {
      copyin(line_buffer, p, FALSE);
      if (p == current) line_buffer[*cursor_x_pos-1]='\0';
      if ((f=strrstr(line_buffer, item_to_look_for))!=NULL) {
        found = TRUE;
        position=f-line_buffer+1;
      } else if (p->prec->foldline==START_FOLD && p->prec->next==p)
        p=p->prec->other_end;
      else
        p=p->prec;
    }
    /*}}}  */
  } else {
    /*{{{  search forward*/
    while (p != tail && !found) {
      copyin(line_buffer, p, FALSE);
      if (p == current) {
        position=strlen(line_buffer);
        if (position > *cursor_x_pos)
          f=strstr(line_buffer + *cursor_x_pos -1,item_to_look_for);
        else
          f=NULL;
      } else
         f=strstr(line_buffer,item_to_look_for);
      if (f==NULL)
        move_on(&p);
      else {
        found = TRUE;
        position=f-line_buffer+1;
      }
    }
    /*}}}  */
  }
  if (!found) return FALSE;
  q = current;
  close_and_open_folds(p);
  current = p;
  restore_element(SCREEN_LEN/2);
  if (p != q) {
    copyin(line_buffer, p, FALSE);
    if (backup)
      *cursor_x_pos = strrstr(line_buffer, item_to_look_for)-line_buffer+1;
    else
      *cursor_x_pos = strstr(line_buffer, item_to_look_for)-line_buffer+1;
  } else
    *cursor_x_pos = position;
  if (*cursor_x_pos < 1) *cursor_x_pos = 1;
  return TRUE;
}
/*}}}  */
/*{{{  get_search*/
#ifdef STD_C
void get_search(void)
#else
void get_search()
#endif
{
  char s[SEARCHLEN];
 
  readprompt(s,M_SEARCH,SEARCHLEN);
  if (*s!='\0')
    strcpy(item_to_look_for, s);
  else {
    message(M_SEARCH,FALSE);
    fputs(item_to_look_for, stdout);
  }
  if (aborted) *item_to_look_for='\0';
}
/*}}}  */
/*{{{  pre_find_element*/
#ifdef STD_C
element *pre_find_element(int new_line_no)
#else
element *pre_find_element(new_line_no)
int new_line_no;
#endif
{
  int current_line_no;
  element *p;

  current_line_no = 0;
  p = head->next;
  while (current_line_no < new_line_no && p != tail) {
    if (p->foldline==START_FILED) current_line_no += 3; else current_line_no++;
    move_on(&p);
  }
  return p;
}
/*}}}  */
/*{{{  find_element*/
#ifdef STD_C
void find_element(int new_line_no, uchar close_line)
#else
void find_element(new_line_no, close_line)
int new_line_no;
uchar close_line;
#endif
{
  element *p;

  p = pre_find_element(new_line_no);
  close_and_open_folds(p);
  current = p;
  restore_element(close_line);
}
/*}}}  */
/*{{{  goto_line*/
#ifdef STD_C
void goto_line(int *cursor_x_pos)
#else
void goto_line(cursor_x_pos) int *cursor_x_pos;
#endif
{
  int no=0;
  char command_str[PROMPT_LEN+1];
 
  readprompt(command_str,M_GOTO, 15);
  if (command_str[0] != '\0')
    if ((no=atoi(command_str))>0) {
      *cursor_x_pos = 1;
      find_element(no, SCREEN_LEN/2);
    }
}
/*}}}  */
/*{{{  find_field*/
#ifdef STD_C
PRIVATE void find_field(int po)
#else
PRIVATE void find_field(po)
int po;
#endif
{
  int offset;
 
  if (po<1) return;
  copyin(line_buffer, current, FALSE);
  if (po>strlen(line_buffer)) return;
  offset=x_shift(po);
  if (offset) {
    gotoxy(START,cursor_level);
    clreol();
  }
  gotoxy(START,cursor_level);
  printf(
    strlen(line_buffer+offset)<LEN ? "%.*s":"%.*s$",
    LEN-1,&line_buffer[offset]
  );
  gotoxy(START-1+po-offset,cursor_level);
  fflush(stdout);
}
/*}}}  */
/*{{{  query_replace*/
#ifdef STC_C
void query_replace(int *cursor_x_pos)
#else
void query_replace(cursor_x_pos) int *cursor_x_pos;
#endif
{
  char ch='\0';
 
  get_search();
  if ((*item_to_look_for!='\0') & !aborted) {
    get_replace();
    if (aborted) return;
    while (!aborted && find_item(cursor_x_pos,FALSE)) {
      copyin(current_dsp_line, current, FALSE);
      /*{{{  maybe menu*/
      if (ch!=DO_REST) {
        message(M_Q_R_PROMPT,FALSE);
        find_field(*cursor_x_pos);
        ch=hide_key(FALSE);
        ch=toupper(ch);
        if (ch==' ') ch=YES;
        if (ch==DO_REST) enable_abort();
      }
      /*}}}  */
      /*{{{  maybe replace*/
      if ((ch==YES || ch==DO_AB || ch==DO_REST) && !aborted) {
        proc_replace(item_to_look_for,replace_item,
                     current_dsp_line,*cursor_x_pos);
        *cursor_x_pos+=strlen(replace_item);
        copyout(current_dsp_line, current);
        write_dsp_line(current, cursor_level);
      } else
        *cursor_x_pos+=1;
      /*}}}  */
      if (ch==AB || ch==DO_AB) aborted=TRUE;
    }
    disable_abort();
    no_message();
  }
}
/*}}}  */
/*{{{  replace*/
#ifdef STD_C
void replace(int *cursor_x_pos)
#else
void replace(cursor_x_pos) int *cursor_x_pos;
#endif
{
  int counts=0;
  char prompt[PROMPT_LEN+1];
 
  get_search();
  if (*item_to_look_for!='\0' && !aborted) {
    get_replace();
    if (aborted) return;
    enable_abort();
    while (find_item(cursor_x_pos,FALSE) && !aborted) {
      copyin(current_dsp_line, current, FALSE);
      counts += proc_replace(item_to_look_for, replace_item,
                             current_dsp_line,*cursor_x_pos);
      *cursor_x_pos+=strlen(replace_item);
      copyout(current_dsp_line, current);
      write_dsp_line(current, cursor_level);
    }
    disable_abort();
    sprintf(prompt,F_SUBSTITUTIONS,counts);
    message(prompt,TRUE);
  }
}
/*}}}  */
/*{{{  its_search*/
#ifdef STD_C
void its_search(int *cursor_x_pos, bool reverse)
#else
void its_search(cursor_x_pos,reverse) int *cursor_x_pos; bool reverse;
#endif
{
  /*{{{  variables*/
  char prompt[PROMPT_LEN+1];
  char *i_ptr;
  int  i_lg=strlen(item_to_look_for);
  char *its_item=item_to_look_for+i_lg;
  int  x[80],y[80];
  bool found=TRUE;
  int k=0;
  TOKEN c=O_NOP;
  bool first=TRUE;
  /*}}}  */
 
  /*{{{  init search, prompt*/
  x[0]= *cursor_x_pos;
  y[0]=get_line_no();
  while (++k <= i_lg) {x[k]=x[0];y[k]=y[0]; }
  strcpy(prompt,"I-Search: ");
  i_ptr=prompt+strlen(prompt);
  /*}}}  */
  find_field(*cursor_x_pos);
  /*{{{  handle commands*/
  while (c!=O_BREAK && !aborted && c!=O_RETURN) {
    /*{{{  store position in stack*/
    x[i_lg] = *cursor_x_pos;
    y[i_lg] = get_line_no();
    /*}}}  */
    /*{{{  show position and prompt*/
    if (reverse) prompt[0]='R'; else prompt[0]='I';
    message(prompt,FALSE);
    if (!found) printf(M_FAILED);
    find_field(*cursor_x_pos);
    /*}}}  */
    /*{{{  handle command*/
    c=hide_key(FALSE);
    switch (c) {
      /*{{{  O_DELETE O_DEL_CHAR_R*/
      case O_DEL_CHAR_R:
      case O_DELETE: {
        if (i_lg) {
          if (first) {
            i_lg=0;
            *(its_item=item_to_look_for)='\0';
          } else {
            *--i_ptr = *--its_item ='\0';
            *cursor_x_pos=x[--i_lg];
            if (y[i_lg+1]!=y[i_lg]) find_element(y[i_lg],SCREEN_LEN/2);
            found=TRUE;
          }
        }
        break;
      }
      /*}}}  */
      /*{{{  O_RETURN*/
      case O_RETURN: {
        break;
      }
      /*}}}  */
      /*{{{  O_FIND         O_DOWN O_RIGHT*/
      case O_DOWN:
      case O_RIGHT:
      case O_FIND: {
        reverse=FALSE;
        if (first) {i_ptr += i_lg;strcat(prompt,item_to_look_for);}
        if (i_lg) {
          ++(*cursor_x_pos);
          if (!(found=find_item(cursor_x_pos,reverse)))
            --(*cursor_x_pos);
        }
        break;
      }
      /*}}}  */
      /*{{{  O_FIND_REVERSE O_UP   O_LEFT*/
      case O_UP:
      case O_LEFT:
      case O_FIND_REVERSE: {
        reverse=TRUE;
        if (first) {i_ptr+=i_lg;strcat(prompt,item_to_look_for);}
        if (i_lg) found=find_item(cursor_x_pos,reverse=TRUE);
        break;
      }
      /*}}}  */
      /*{{{  ascii*/
      default: {
        if (first) {
          i_lg=0;
          *(its_item=item_to_look_for)='\0';
        }
        if (c>=O_NOP) break;
        *i_ptr++ = c;
        *i_ptr='\0';
        *its_item++ = c;
        *its_item='\0';
        x[++i_lg] = *cursor_x_pos;
        y[i_lg]=get_line_no();
        found=find_item(cursor_x_pos,reverse);
        break;
      }
      /*}}}  */
    }
    /*}}}  */
    first=FALSE;
  }
  /*}}}  */
  if (c!=O_RETURN) { find_element(y[0],SCREEN_LEN/2); *cursor_x_pos=x[0]; }
  no_message();
}
/*}}}  */
