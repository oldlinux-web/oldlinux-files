/*{{{  #includes*/
#include <sys/types.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
 
#include <local/bool.h>

#define FOLDING_C
 
#include "origami.h"
#include "macros.h"
/*}}}  */
 
/*{{{  define pre_open_fold*/
#define pre_pre_open_fold(q)  {check_fold(q); \
          Line_after_fold->prec = End_of_fold; \
          Start_of_fold->next = Start_of_fold->fold; \
          Start_of_fold->foldline = START_OPEN_FOLD;}
/*}}}  */
 
/*{{{  variables*/
PRIVATE int head_line_no, cur_line_no;
PRIVATE element *select_ptr;
PRIVATE int makefold_indent;
bool select_on = FALSE;
element *pick_head,
        *pick_tail,
        *current,
        *head,
        *tail,
        *real_head,
        *real_tail,
        *End_of_fold,
        *Start_of_fold,
        *Line_after_fold;

char current_dsp_line[MAX_FIELD_SIZE+1];
int enter_depth = 0,
    enter_depth_spaces = 0;
/*}}}  */
 
/*{{{  to_start*/
#ifdef STD_C
int to_start(int m)
#else
int to_start(m)
int m;
#endif
{
  int dist_to_start;
  element *p;

  dist_to_start = 1;
  p = current;
  while (p != head->next && dist_to_start <= m) {
    dist_to_start++;
    p = p->prec;
  }
  return dist_to_start;
}
/*}}}  */
/*{{{  to_bottom*/
#ifdef STD_C
int to_bottom(int m)
#else
int to_bottom(m)
int m;
#endif
{
  int dist_to_end;
  element *p;

  dist_to_end = 0;
  p = current;
  while (p != tail && dist_to_end <= m) {
    dist_to_end++;
    p = p->next;
  }
  return dist_to_end;
}
/*}}}  */
/*{{{  select_forward*/
#ifdef STD_C
bool select_forward(void)
#else
bool select_forward()
#endif
{
  if (current == tail)
    return FALSE;
  else
    return (current->next->selected);
}
/*}}}  */
/*{{{  select_backward*/
#ifdef STD_C
bool select_backward(void)
#else
bool select_backward()
#endif
{
  if (current == head->next)
    return FALSE;
  else
    return (current->prec->selected);
}
/*}}}  */
/*{{{  move_or_line_down*/
#ifdef STD_C
void move_or_line_down(int from_line)
#else
void move_or_line_down(from_line)
int from_line;
#endif
{
  if (current == tail) {
    vmessage(M_BOTTOM);
    return;
  }
  if (select_on && select_forward()) current->selected = FALSE;
  current = current->next;
  if (cursor_level > from_line && screen_end != tail)
    restore_element(SCREEN_LEN/2);
  else {
    cursor_level++;
    if (dirty) restore(1);
  }
  if (select_on) current->selected = TRUE;
}
/*}}}  */
/*{{{  line_or_move_up*/
#ifdef STD_C
void line_or_move_up(int to_line)
#else
void line_or_move_up(to_line)
int to_line;
#endif
{
  if (current->prec == head) {
    vmessage(M_TOP);
    return;
  }
  if (select_on && select_backward()) current->selected = FALSE;
  current = current->prec;
  if (screen_start->prec != head && cursor_level < to_line)
    restore_element(SCREEN_LEN/2);
  else {
    cursor_level--;
    if (dirty) restore(1);
  }
  if (select_on) current->selected = TRUE;
}
/*}}}  */
/*{{{  sub_move_up*/
#ifdef STD_C
PRIVATE void sub_move_up(void)
#else
PRIVATE void sub_move_up()
#endif
{
  if (current->prec == head)
    return;
  if (select_on && select_backward())
    current->selected = FALSE;
  current = current->prec;
  current->selected = select_on;
}
/*}}}  */
/*{{{  sub_move_down*/
#ifdef STD_C
PRIVATE void sub_move_down(void)
#else
PRIVATE void sub_move_down()
#endif
{
  if (current == tail)
    return;
  if (select_on && select_forward())
    current->selected = FALSE;
  current = current->next;
  current->selected = select_on;
}
/*}}}  */
/*{{{  top*/
#ifdef STD_C
void top(void)
#else
void top()
#endif
{
  int dist_to_top, far_;

  far_ = cursor_level + 4;
  dist_to_top = to_start(far_);
  if (dist_to_top < far_) {
    while (current != head->next)
      move_up();
    return;
  }
  if (select_on) {
    while (current != head->next)
      sub_move_up();
  } else
    current = head->next;
  restore_element(4);
}
/*}}}  */
/*{{{  bottom*/
#ifdef STD_C
void bottom(void)
#else
void bottom()
#endif
{
  int dist_to_bottom, far_;

  far_ = SCREEN_LEN - cursor_level + 4;
  dist_to_bottom = to_bottom(far_);
  if (dist_to_bottom < far_) {
    while (current != tail)
      move_down();
    return;
  }
  if (select_on) {
    while (current != tail)
      sub_move_down();
  }
  else
    current = tail;
  restore_element(4);
}
/*}}}  */
/*{{{  insert_link_before*/
#ifdef STD_C
PRIVATE void insert_link_before(element *ptr_to_new)
#else
PRIVATE void insert_link_before(ptr_to_new)
element *ptr_to_new;
#endif
{
  element *ptr_to_prec;

  ptr_to_new->UU.U1.indent = insert_indent_of(current);
  ptr_to_prec = current->prec;
  if (screen_start == current)
    screen_start = ptr_to_new;
  join_links(ptr_to_prec, ptr_to_new);
  join_links(ptr_to_new, current);
  current = ptr_to_new;
}
/*}}}  */
/*{{{  copy_line*/
#ifdef STD_C
bool copy_a_line(element **to_ptr)
#else
bool copy_a_line(to_ptr)
element **to_ptr;
#endif
{
  element *p, *q, *last, *fold_ptr;
  bool filed;

  filed = FALSE;
  p = current;
  while (!filed && p != current->next) {
    if (p->foldline == START_FILED) filed = TRUE;
    move_on(&p);
  }
  if (filed) {
    message(M_NO_FF_COPY,TRUE);
    return (FALSE);
  }
  p = current;
  last = NULL;
  fold_ptr = NULL;
  while (p != current->next) {
    proc_new(&q);
    if (p == current) *to_ptr = q;
    *q = *p;
    if (p->strng) {
        q->strng=malloc(strlen(p->strng)+1);
        strcpy(q->strng, p->strng);
    }
    q->prec = last;
    if (last != NULL) {
      last->next = q;
      if (last->foldline == START_FOLD) last->fold = q;
      if (last->foldline == END_FOLD) {
        q->prec = last->other_end;
        q->prec->next = q;
      }
    }
    if (q->foldline == START_FOLD) {
      q->other_end = fold_ptr;
      fold_ptr = q;
    }
    move_on(&p);
    if (q->foldline == END_FOLD) {
      q->other_end = fold_ptr;
      fold_ptr = fold_ptr->other_end;
      q->other_end->other_end = q;
    }
    last = q;
  }
  return (!filed);
}
/*}}}  */
/*{{{  pre_exit_fold*/
#ifdef STD_C
void pre_exit_fold(void)
#else
void pre_exit_fold()
#endif
{
  element *p, *WITH;

  enter_depth--;
  Start_of_fold = head->next;
  End_of_fold = tail;
  Line_after_fold = tail->next;
  tail = End_of_fold->fold;
  WITH = Start_of_fold;
  WITH->foldline = START_FOLD;
  head = WITH->fold;
  WITH->fold = WITH->next;
  WITH->next = Line_after_fold;
  Line_after_fold->prec = Start_of_fold;
  /*restore indent*/
  WITH->UU.U1.indent = End_of_fold->UU.U1.fold_indent;
  enter_depth_spaces -= total_indent(Start_of_fold);
  current = Start_of_fold;
  check_fold(current);
  p = Start_of_fold->fold;
  while (p->foldline != END_FOLD) {
    if (p->foldline == START_OPEN_FOLD)
      close_fold_at(p);
    p = p->next;
  }
}
/*}}}  */
/*{{{  pre_open_fold*/
#ifdef STD_C
void pre_open_fold(element *q)
#else
void pre_open_fold(q)
element *q;
#endif
{
  element *p;
  int i;

  pre_pre_open_fold(q);
  p = Start_of_fold;
  i = total_indent(p);
  p->UU.U1.indent = i;
  do {
    p = p->next;
    p->UU.U1.indent += i;
  } while (p != End_of_fold);
}
/*}}}  */
/*{{{  pre_enter_fold*/
#ifdef STD_C
PRIVATE void pre_enter_fold(element *cur)
#else
PRIVATE void pre_enter_fold(cur)
element *cur;
#endif
{   /*rent*/
  element *WITH;

  check_fold(cur);
  Line_after_fold->prec = End_of_fold;
  enter_depth++;
  WITH = Start_of_fold;
  enter_depth_spaces += total_indent(Start_of_fold);
  WITH->next = WITH->fold;
  WITH->foldline = START_ENTER_FOLD;
  WITH->fold = head;
  head = WITH->prec;
  End_of_fold->fold = tail;
  tail = End_of_fold;
  /*save indent*/
  End_of_fold->UU.U1.fold_indent = Start_of_fold->UU.U1.indent;
  Start_of_fold->UU.U1.indent = 0;
}
/*}}}  */
/*{{{  enter_folds_to*/
#ifdef STD_C
PRIVATE void enter_folds_to(element *qq)
#else
PRIVATE void enter_folds_to(qq)
element *qq;
#endif
{
  element *q, *old_q, *save_q;

  do {
    save_q = NULL;
    q = qq;
    while (q != head) {
      old_q = q;
      q = q->prec;
      if (q->foldline == START_FOLD && q->fold == old_q)
  save_q = q;
    }
    if (save_q != NULL)
      pre_enter_fold(save_q);
  } while (save_q != NULL);
}
/*}}}  */
/*{{{  erase_current_link*/
#ifdef STD_C
PRIVATE void erase_current_link(void)
#else
PRIVATE void erase_current_link()
#endif
{
  element *prec_ptr, *p;

  if (current == tail)
    return;
  p = current;
  prec_ptr = p->prec;
  current = p->next;
  proc_dispose(p);
  join_links(prec_ptr, current);
}
/*}}}  */
/*{{{  delete_list*/
#ifdef STD_C
void delete_list(element *from_ptr, element *to_ptr)
#else
void delete_list(from_ptr, to_ptr)
element *from_ptr, *to_ptr;
#endif
{
  element *p, *q;

  q = from_ptr;   /*head^.next^.next*/
  while (q != to_ptr) {   /*tail*/
    p = q;
    q = q->next;
    proc_dispose(p);
  }
  proc_dispose(to_ptr);
}
/*}}}  */
/*{{{  enter_fold*/
#ifdef STD_C
void enter_fold(void)
#else
void enter_fold()
#endif
{
  element *p;
  char filename[_POSIX_PATH_MAX+1];
 
  /*{{{  normal fold -> enter*/
  if (current->foldline == START_FOLD) {
    current->fold_close_line = cursor_level;
    pre_enter_fold(current);
    current = current->next;
    restore_element(4);
    return;
  }
  /*}}}  */
  /*{{{  no filed fold -> error*/
  if (current->foldline != START_FILED) {
    message(M_NO_SIMPLE_FOLD,TRUE);
    return;
  }
  /*}}}  */
  if (!normal_att(current)) {
    message(M_NO_TEXT,TRUE);
    return;
  }
  /*{{{  no list enter*/
  if (current->fold != current->other_end) {
    message(M_NO_ENTER_LIST_FF,TRUE);
    return;
  }
  /*}}}  */
  /*{{{  test if can open the file*/
  line_of(filename, current->other_end);
  if (access(filename,R_OK) == -1) {
    err_message(M_CANTOPEN,filename,TRUE);
    return;
  }
  /*}}}  */
  /*{{{  read the filed fold*/
  head_line_no = line_no(head->next);
  cur_line_no = line_no(current);
  p = current;
  total_save(NULL);
  if (file_changed && !yes(M_NO_SAVE_CANCEL)) return;
  real_tail->UU.U0.int1 = head_line_no;
  real_tail->UU.U0.int2 = cur_line_no;
  real_tail->fold_close_line = cursor_level;
  current = p;
  current->foldline = START_FOLD;   /*to allow copy*/
  /*always TRUE*/
  if (copy_a_line(&p)) current = p;
  delete_list(head->next->next, tail->prec);   /*except for tail/head*/
  current->foldline = START_ENTER_FILED;
  real_tail = current->fold;
  current->next = real_tail;
  head = real_head->next;
  join_links(head, current);
  real_head = head;
  join_links(real_tail, tail);
  tail = real_tail;   /*ie file_ptr*/
  insert_file(tail, current, tail, TRUE, NULL);
  current = current->next;
  restore_element(4);
  title_op(SHOW);
  /*}}}  */
  delay_auto_write();
  call_auto_macro();
}
/*}}}  */
/*{{{  exit_fold*/
#ifdef STD_C
void exit_fold(void)
#else
void exit_fold()
#endif
{
  element *WITH;
 
  /*{{{  normal fold -> exit*/
  if (tail != real_tail) {
    pre_exit_fold();
    restore_element(current->fold_close_line);
    return;
  }
  /*}}}  */
  /*{{{  at top -> error*/
  if (real_tail->next == NULL) {
    message(M_TOP,TRUE);
    return;
  }
  /*}}}  */
  total_save(NULL);
  if (file_changed && !yes(M_NO_SAVE_CANCEL)) return;
  /*{{{  read the calling file*/
  real_tail = tail->next;
  delete_list(head->next, tail);
  real_head = head->prec;
  WITH = real_tail;
  head_line_no = WITH->UU.U0.int1;
  WITH->UU.U0.int1 = 0;
  cur_line_no = WITH->UU.U0.int2 - head_line_no;
  WITH->UU.U0.int2 = 0;
  tail = real_tail;
  head = real_head;
  current = head->next;   /*ie file_ptr*/
  insert_file(tail, current, tail, TRUE, NULL);
  current = pre_find_element(head_line_no);
  enter_folds_to(current);
  find_element(cur_line_no, real_tail->fold_close_line);
  title_op(SHOW);
  /*}}}  */
  delay_auto_write();
  call_auto_macro();
}
/*}}}  */
/*{{{  undelete_before*/
#ifdef STD_C
void undelete_before(element *ptr_to_new)
#else
void undelete_before(ptr_to_new)
element *ptr_to_new;
#endif
{
  insert_link_before(ptr_to_new);
  if (cursor_level < SCREEN_LEN) {
    down_a_bit(cursor_level);
  }
  else delete_dsp_line(SCREEN_LEN);
  write_dsp_line(current, cursor_level);
  if (screen_end_level == SCREEN_LEN)
    screen_end = screen_end->prec;
  else
    screen_end_level++;
  if (cursor_level > SCREEN_LEN - 2 && screen_end != tail)
    whole_screen_up();
  if (dirty) restore(cursor_level);
}
/*}}}  */
/*{{{  undelete_pick_before*/
#ifdef STD_C
void undelete_pick_before(void)
#else
void undelete_pick_before()
#endif
{
  element *p, *sf, *ef;

  proc_new(&ef);
  join_links(pick_tail, ef);
  p = pick_head->next;
  proc_new(&sf);
  join_links(pick_head, sf);
  join_links(sf, p);
  sf->foldline = START_FOLD;
  sf->fold = sf->next;
  sf->other_end = ef;
  ef->foldline = END_FOLD;
  ef->other_end = sf;
  undelete_before(sf);
  pick_tail = pick_head;
}
/*}}}  */
/*{{{  undelete_after*/
#ifdef STD_C
void undelete_after(element *ptr_to_new)
#else
void undelete_after(ptr_to_new)
element *ptr_to_new;
#endif
{
  element *ptr_to_next;


  /*note current<>tail*/
  ptr_to_next = current->next;
  join_links(current, ptr_to_new);
  join_links(ptr_to_new, ptr_to_next);
  if (cursor_level == SCREEN_LEN) {
    screen_end = screen_end->prec;
    whole_screen_up();
  }
  else {
    if (cursor_level == SCREEN_LEN - 1) {
      delete_dsp_line(SCREEN_LEN);
      write_dsp_line(ptr_to_new, SCREEN_LEN);
      if (screen_end_level == SCREEN_LEN) {
        screen_end = screen_end->prec;
        whole_screen_up();
      }
      else screen_end_level++;
    }
    else {
      if (screen_end_level == SCREEN_LEN) screen_end = screen_end->prec;
      else screen_end_level++;
      down_a_bit(cursor_level + 1);
      write_dsp_line(ptr_to_new, cursor_level + 1);
    }
  }
  move_down();
}
/*}}}  */
/*{{{  pre_remove_line*/
#ifdef STD_C
void pre_remove_line(element **p)
#else
void pre_remove_line(p)
element **p;
#endif
{
  element *prec_ptr;

  *p = current;
  prec_ptr = current->prec;
  if (screen_start == current)
    screen_start = current->next;
  current = current->next;
  join_links(prec_ptr, current);
}
/*}}}  */
/*{{{  remove_line*/
#ifdef STD_C
void remove_line(element **p)
#else
void remove_line(p)
element **p;
#endif
{
  if (!(current->foldline == NOT_FOLD || filed_or_fold(current))) return;
  if (current == tail) return;
  if (*p != NULL) proc_dispose(*p);
  pre_remove_line(p);
  if (cursor_level < SCREEN_LEN)
    up_a_bit(cursor_level);
  else
    delete_dsp_line(SCREEN_LEN);
  if (screen_end != tail) {
    screen_end = screen_end->next;
    write_dsp_line(screen_end, SCREEN_LEN);
  } else {
    screen_end_level--;
    whole_screen_down();
  }
  if (dirty) restore(cursor_level);
}
/*}}}  */
/*{{{  start_make_fold*/
#ifdef STD_C
void start_make_fold(int ind)
#else
void start_make_fold(ind)
int ind;
#endif
{
  element *p;
 
  if (entered(current))
    return;
  proc_new(&p);
  p->fold_close_line = cursor_level;
  makefold_indent = ind - 1;
  if (makefold_indent < current->UU.U1.indent)
    makefold_indent = current->UU.U1.indent;
  spaces(line_buffer,(int)(makefold_indent - current->UU.U1.indent));
  strcat(line_buffer,fold_open_str);
  copy_line_to_parts(line_buffer, &p);
  undelete_before(p);
  select_on = TRUE;
  select_ptr = current;
  current->selected = TRUE;
  move_down();
}
/*}}}  */
/*{{{  make_fold*/
#ifdef STD_C
void make_fold(int *cursor_x_pos)
#else
void make_fold(cursor_x_pos)
int *cursor_x_pos;
#endif
{
  /*{{{  variables*/
  element *p;
  int open_count, ind, i;
  bool indented_enough;
  int FORLIM;
  element *WITH;
  uchar on_screen_line;
  /*}}}  */
 
  if (!select_backward()) {
    message(M_NO_BACK_FOLDING,TRUE);
    return;
  }
  p = select_ptr->next;
  indented_enough = TRUE;
  open_count = 0;
  while (p != current && open_count >= 0 && indented_enough) {
    /*{{{  check indentation >= makefold_indent*/
    if (open_count == 0) {
      if (p->foldline == START_OPEN_FOLD) indented_enough = (makefold_indent <= p->UU.U1.indent);
      if (filed_or_fold(p)) indented_enough = (makefold_indent <= total_indent(p));
      if (p->foldline == NOT_FOLD) {
        copy_parts_to_line(p, current_dsp_line);
        FORLIM = makefold_indent - p->UU.U1.indent;
        for (i = 0; i < FORLIM; i++) {
          if(current_dsp_line[i] == '\0') break; /* blank lines are ok */
          indented_enough = (indented_enough && current_dsp_line[i] == ' ');
        }
      }
    }
    /*}}}  */
    /* check not across open fold boundaries,
     * ie no of open folds=no of end folds
     */
    if (p->foldline == START_OPEN_FOLD) open_count++;
    if (p->foldline == END_FOLD) open_count--;
    p = p->next;
  }
  if (!(indented_enough && open_count == 0)) {
    message(M_FOLDERR,TRUE);
    return;
  }
  p = select_ptr->next;
  ind = makefold_indent - select_ptr->UU.U1.indent;
  while (p != current) {
    WITH = p;
    if (WITH->foldline == START_OPEN_FOLD) close_fold_at(p);
    if (filed_or_fold(p)) WITH->UU.U1.fold_indent -= ind;
    if (WITH->foldline == NOT_FOLD) {
      copy_line_to_parts(copy_to_end(line_buffer, p->strng, ind + 1), &p);
    }
    WITH->UU.U1.indent = 0;
    p = p->next;
  }
  current->selected = FALSE;
  proc_new(&p);
  insert_link_before(p);
  p = select_ptr;
  while (p->selected) {
    p->selected = FALSE;
    p = p->next;
  }
  End_of_fold = current;
  Line_after_fold = End_of_fold->next;
  Start_of_fold = select_ptr;
  Start_of_fold->fold = Start_of_fold->next;
  Start_of_fold->other_end = End_of_fold;
  End_of_fold->other_end = Start_of_fold;
  Line_after_fold->prec = Start_of_fold;
  Start_of_fold->next = Line_after_fold;
  Start_of_fold->foldline = START_FOLD;
  Start_of_fold->UU.U1.indent = makefold_indent - ind;
  Start_of_fold->UU.U1.fold_indent = ind;
  End_of_fold->foldline = END_FOLD;
  End_of_fold->UU.U1.indent = 0;
  current = Start_of_fold;
  select_on = FALSE;
  copy_line_to_parts("", &End_of_fold);
  copy_line_to_parts("", &Start_of_fold);
  *cursor_x_pos = total_indent(current) + 6;
  if (on_screen(current,&on_screen_line))
    current->fold_close_line = on_screen_line;
  restore_element(current->fold_close_line);
  /* if on_screen and enough lines to fill screen */
  /*   then restore_from(on_screen_line);         */
}
/*}}}  */
/*{{{  open_fold*/
#ifdef STD_C
void open_fold(void)
#else
void open_fold()
#endif
{
  if (current->foldline != START_FOLD) {
    message(M_NO_SIMPLE_FOLD,TRUE);
    return;
  }
  current->fold_close_line = cursor_level;
  pre_open_fold(current);
  restore_to_end(cursor_level);
}
/*}}}  */
/*{{{  close_fold*/
#ifdef STD_C
void close_fold(void)
#else
void close_fold()
#endif
{
  element *p;

  p = current;
  if (p->foldline == END_FOLD)
    p = p->other_end;
  while (p != head && p->foldline != START_OPEN_FOLD) {
    if (p->foldline == END_FOLD)
      p = p->other_end->prec;
    else
      p = p->prec;
  }
  if (p != head) {
    current = p;
    close_fold_at(p);
    restore_or_restore_to_end();
    return;
  }
  if (tail != real_tail)
    message(M_USE_EXIT,TRUE);
  else
    vmessage(M_TOP);
}
/*}}}  */
/*{{{  remove_fold*/
#ifdef STD_C
void remove_fold(void)
#else
void remove_fold()
#endif
{
  element *new_, *p;
  int ind, f_ind;
  element *WITH;

  if (current->foldline != START_FOLD) {
    message(M_NO_SIMPLE_FOLD,TRUE);
    return;
  }
  ind = current->UU.U1.indent;
  f_ind = current->UU.U1.fold_indent;
  pre_pre_open_fold(current);
  new_ = Start_of_fold->next;
  p = new_;
  while (p != End_of_fold) {
    WITH = p;
    WITH->UU.U1.indent = ind;
    if (WITH->foldline == NOT_FOLD) {
      spaces(line_buffer,f_ind);
      strcat(line_buffer,p->strng);
      copy_line_to_parts(line_buffer, &p);
    } else
      WITH->UU.U1.fold_indent += f_ind;
    p = p->next;
  }
  if (new_ == End_of_fold) new_ = Line_after_fold;
  current = End_of_fold;
  erase_current_link();
  current = Start_of_fold;
  erase_current_link();
  current = new_;
  if (screen_start == Start_of_fold) {
    restore_element(4);
    return;
  }
  if (new_ == Line_after_fold)
    restore_or_restore_to_end();
  else
    restore_to_end(cursor_level);
}
/*}}}  */
/*{{{  create_list*/
#ifdef STD_C
void create_list(FILE *pipe)
#else
void create_list(pipe) FILE *pipe;
#endif
{
  element *p;
 
  proc_new(&head);
  head->prec = NULL;
  proc_new(&p);
  proc_new(&tail);
  tail->next = NULL;
  join_links(head, p);
  join_links(p, tail);
  p->foldline = START_ENTER_FILED;
  p->other_end = tail;
  tail->other_end = p;
  tail->foldline = END_FOLD;
  insert_file((element *)NULL, p, tail, TRUE, pipe);
  current = p->next;
  real_head = head;
  real_tail = tail;
}
/*}}}  */
