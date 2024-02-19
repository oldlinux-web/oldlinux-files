/*{{{  #includes*/
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#include <local/bool.h>

#define ORIEDT_C

#include "origami.h"
#include "macros.h"
/*}}}  */
 
/*{{{  variables*/
PRIVATE int dispose_size = 0;
bool fold_out=FALSE,
     fold_in=FALSE;
element *dispose_head,
        *dispose_tail;
int first_edit_pos,
    pre_edit_pos;
/*}}}  */
 
/*{{{  pad_                  fill string s from position to end with token*/
#ifdef STD_C
void pad_(char *s, int ch, int l)
#else
void pad_(s, ch, l)
char *s;
int ch, l;
#endif
{
  int i;

  i = strlen(s);
  s[l] = '\0';
  for (; i < l; i++)
    s[i] = (char)ch;
}
/*}}}  */
/*{{{  spaces                fill a string with spaces*/
#ifdef STD_C
char *spaces(char *Result, int l)
#else
char *spaces(Result, l)
char *Result;
int l;
#endif
{
  Result=Result+l;
  *(Result--)='\0';
  while (l--) *(Result--)=' ';
  return(Result+1);
}
/*}}}  */
/*{{{  copy_line_to_parts*/
#ifdef STD_C
void copy_line_to_parts(char *line, element **parts)
#else
void copy_line_to_parts(line, parts) char *line; element **parts;
#endif
{
  if ((*parts)->strng!=NULL) free((*parts)->strng);
  (*parts)->strng = malloc(strlen(line)+1);
  strcpy((*parts)->strng, line);
}
/*}}}  */
/*{{{  copy_parts_to_parts*/
#ifdef STD_C
void copy_parts_to_parts(element *in_parts,element ** parts)
#else
void copy_parts_to_parts(in_parts, parts) element *in_parts, **parts;
#endif
{
  if ((*parts)->strng!=NULL) free((*parts)->strng);
  (*parts)->strng = malloc(strlen((in_parts)->strng)+1);
  strcpy((*parts)->strng, (in_parts)->strng);
}
/*}}}  */
/*{{{  first_char*/
#ifdef STD_C
int first_char(char *s)
#else
int first_char(s)
char *s;
#endif /* STD_C */
{
  int l=1;
 
  while (*s==' ') {s++;l++;}
  return(l);
}
/*}}}  */
/*{{{  last_char*/
#ifdef STD_C
int last_char(char *s)
#else
int last_char(s)
char *s;
#endif /* STD_C */
{
  int i=strlen(s);
  char *x=s+i;
 
  while (*(x--) == ' ' && i-- > 0);
  return i;
}
/*}}}  */
/*{{{  trailing_spaces*/
#ifdef STD_C
void trailing_spaces(char *s)
#else
void trailing_spaces(s)
char *s;
#endif /* STD_C */
{
  char *c;

  if (*s == 0) return;
  c = s + strlen(s) - 1;
  while(c>s && *c == ' ') c--;
  *++c = 0;
}
/*}}}  */
/*{{{  join_links*/
#ifdef STD_C
void join_links(element *p,element *q)
#else
void join_links(p, q)
element *p, *q;
#endif /* STD_C */
{
  p->next = q;
  q->prec = p;
  if (filed_or_fold(p)) p->other_end->next = q;
}
/*}}}  */
/*{{{  move_on*/
#ifdef STD_C
void move_on(element **p)
#else
void move_on(p)
element **p;
#endif /* STD_C */
{
  if ((*p)->foldline == START_FOLD) *p = (*p)->fold; else *p = (*p)->next;
}
/*}}}  */
/*{{{  insert_indent_of*/
#ifdef STD_C
int insert_indent_of(element *p)
#else
int insert_indent_of(p)
element *p;
#endif /* STD_C */
{
  if (p->foldline == START_OPEN_FOLD)
    return (p->UU.U1.indent - p->UU.U1.fold_indent);
  else
    return (p->UU.U1.indent);
}
/*}}}  */
/*{{{  close_fold_at*/
#ifdef STD_C
void close_fold_at(element *ptr)
#else
void close_fold_at(ptr)
element *ptr;
#endif /* STD_C */
{
  element *p;
  int i;
 
  p = ptr;
  /*{{{  start fold ?*/
  if (p->foldline == START_OPEN_FOLD) {
    p->foldline = START_FOLD;
    p->fold = p->next;
    p->next = p->other_end->next;
    p->next->prec = p;
  }
  /*}}}  */
  i = p->UU.U1.fold_indent;
  p->UU.U1.indent -= i;
  i += p->UU.U1.indent;
  p = p->fold;
  /*{{{  search matching end-fold*/
  while (p->foldline != END_FOLD) {
    if (p->foldline == START_OPEN_FOLD) close_fold_at(p);
    p->UU.U1.indent -= i;
    p = p->next;
  }
  /*}}}  */
  p->UU.U1.indent -= i;
}
/*}}}  */
/*{{{  lines_within*/
#ifdef STD_C
int lines_within(element *p)
#else
int lines_within(p)
element *p;
#endif /* STD_C */
{
  element *q;
  int l;

  if (p->foldline != START_FOLD) {l = 1;return l;}
  l = 0;
  q = p;
  while (q != p->next) {l++;move_on(&q);}
  return l;
}
/*}}}  */
/*{{{  proc_dispose*/
#ifdef STD_C
void proc_dispose(element *p)
#else
void proc_dispose(p) element *p;
#endif
{
  dispose_size += lines_within(p);
  join_links(dispose_tail, p);
  dispose_tail = p;
}
/*}}}  */
/*{{{  proc_new*/
#ifdef STD_C
void proc_new(element **p)
#else
void proc_new(p)
element **p;
#endif /* STD_C */
{
  element *q;
 
  if (dispose_head != dispose_tail) {
    *p = dispose_head->next;
    if (filed_or_fold(*p)) {
      q = (*p)->other_end;
      (*p)->foldline = NOT_FOLD;
      q->foldline = NOT_FOLD;
      if (dispose_tail == *p) dispose_tail = q;
      (*p)->next = (*p)->fold;
    }
    if (dispose_tail == *p)
      dispose_tail = dispose_head;
    else
      join_links(dispose_head, (*p)->next);
    dispose_size--;
    if ((*p)->strng) free((*p)->strng);
  } else
    *p = malloc(sizeof(element));
  **p = *null_item_ptr;
  (*p)->strng=malloc(1); *((*p)->strng)='\0';
}
/*}}}  */
/*{{{  proc_to_edit_pos*/
#ifdef STD_C
void proc_to_edit_pos(void)
#else
void proc_to_edit_pos()
#endif /* STD_C */
{
  element *WITH;

  WITH = current;
  pre_edit_pos = WITH->UU.U1.indent;
  first_edit_pos = pre_edit_pos + 1;
  if (filed_or_fold(current)) {
    pre_edit_pos = first_edit_pos + WITH->UU.U1.fold_indent;
    first_edit_pos = pre_edit_pos + MARKER_LEN;
  }
  if (WITH->foldline == START_OPEN_FOLD || WITH->foldline == START_ENTER_FOLD) {
    pre_edit_pos = 0;
    first_edit_pos += MARKER_LEN;
  }
  if (WITH->foldline == END_FOLD || WITH->foldline == START_ENTER_FILED) {
    pre_edit_pos = 0;
    first_edit_pos = MAX_FIELD_SIZE+1;
  }
}
/*}}}  */
/*{{{  proc_from_edit_pos*/
#ifdef STD_C
void proc_from_edit_pos(void)
#else
void proc_from_edit_pos()
#endif /* STD_C */
{
  element *WITH;

  WITH = current;
  if (filed_or_fold(current))
    WITH->UU.U1.fold_indent =
      strpos2(current_dsp_line, fold_line_str, 1) - WITH->UU.U1.indent - 1;
}
/*}}}  */
/*{{{  copyin*/
#ifdef STD_C
void copyin(char *line,element *p,bool full)
#else
void copyin(line, p, full) char *line; element *p; bool full;
#endif
{
  char fold_str[17];
  int  ind,null_line=0;
 
  ind = p->UU.U1.indent;
  copy_parts_to_line(p, line);
  if (p->foldline == END_FOLD) *line = '\0';
  if (full) trailing_spaces(line);
  if (p->foldline != NOT_FOLD) {
    /*{{{  fold*/
    /*{{{  mark foldbegin*/
    if (filed_or_fold(p)) {
      if (fold_out)
        strcpy(fold_str,"");
      else {
        if (full && (p->foldline != START_FILED || p->UU.U1.fold_type != 1))
          strcpy(fold_str, fold_open_str);
        else
          strcpy(fold_str, fold_line_str);
        if (p->foldline == START_FILED)
          strcat(fold_str, "F ");
        else
          strcat(fold_str, "  ");
      }
      ind += p->UU.U1.fold_indent;
    }
    /*}}}  */
    if (dialect >= 0) {
      /*{{{  language mark*/
      switch (p->foldline) {
        /*{{{  begin fold*/
        case START_OPEN_FOLD:
        case START_ENTER_FOLD: {
          if (!fold_out) sprintf(fold_str, "%s  ", fold_open_str);
          break;
        }
        /*}}}  */
        /*{{{  start filed*/
        case START_ENTER_FILED: {
          if (!fold_out) sprintf(fold_str, "%sF ", fold_open_str);
          break;
        }
        /*}}}  */
        /*{{{  end fold*/
        case END_FOLD: {
          if (!fold_out)
            strcpy(fold_str, fold_close_str);
          else
            null_line++;
          break;
        }
        /*}}}  */
      }
      if (full) pad_(fold_str,' ', 5); else pad_(fold_str,'\0', 5);
      if (full) strinsert(dialect_start[dialect], fold_str, 1);
      /*}}}  */
    } else
      *fold_str = '\0';
    /*}}}  */
  } else
    *fold_str = '\0';
  spaces(line1_buffer,ind);
  strcat(line1_buffer,fold_str);
  strinsert(line1_buffer,line,1);
  if (p->foldline != NOT_FOLD && full  && dialect >= 0)
    strcat(line, dialect_end[dialect]);
  if (null_line) *line=0;
}
/*}}}  */
/*{{{  copyout*/
#ifdef STD_C
void copyout(char *line,element *p)
#else
void copyout(line, p)
char *line;
element *p;
#endif /* STD_C */
{
  int i;
  element *WITH;

  WITH = p;
  if (WITH->foldline == NOT_FOLD)
    i = WITH->UU.U1.indent;
  else
    i = WITH->UU.U1.indent + MARKER_LEN;
  if (filed_or_fold(p)) i += WITH->UU.U1.fold_indent;
  copy_line_to_parts(copy_to_end(line1_buffer, line, i + 1), &p);
}
/*}}}  */
