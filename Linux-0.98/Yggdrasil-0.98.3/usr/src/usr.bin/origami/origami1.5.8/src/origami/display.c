/*{{{  includes*/
#include <sys/types.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
 
#include <local/bool.h>

#define DISPLAY_C
 
#include "origami.h"
#include "macros.h"
/*}}}  */
 
/*{{{  title-strings*/
#define VIEW_TAG ",View"
#define sVIEW_TAG ",V"
#define DM_TAG ",DefMac"
#define sDM_TAG ",D"
#define CF_TAG ",Fold"
#define sCF_TAG ",F"
#define EM_TAG ",ExMac"
#define sEM_TAG ",E"
#define ARG_TAG ",ArgRep"
#define sARG_TAG ",A"
#define OVER_TAG ",Over"
#define sOVER_TAG ",O"
#define AUTO_TAG ",AutoSave"
#define sAUTO_TAG ",S"
char *titleformat="=%c Origami %s (%s%s%s%s%s%s%s%s%s%s) == %s ";
char *titleend="=";
/*}}}  */
 
/*{{{  variables*/
element *screen_start,
        *screen_end;
int cursor_level = 1,
    screen_end_level = 1,
    W_dy,
    W_oy,
    W_dx;
char keybinding_name[keyn_lg+1];
/*}}}  */
 
/*{{{  ClrScr*/
#ifdef STD_C
void ClrScr(void)
#else
void ClrScr()
#endif
{
  gotoxy(1, W_oy);
  ClrFullScr();
}
/*}}}  */
/*{{{  delete_dsp_line*/
#ifdef STD_C
void delete_dsp_line(int level)
#else
void delete_dsp_line(level) int level;
#endif
{
  gotoxy(1,level);
  clreol();
}
/*}}}  */
/*{{{  on_screen*/
#ifdef STD_C
bool on_screen(element *p, uchar *on_screen_line)
#else
bool on_screen(p,on_screen_line)
element *p;
uchar *on_screen_line;
#endif
{
  element *q, *old_q;
  bool on_scr;
  uchar line;

  line = 0;
  q = screen_start;
  on_scr = FALSE;
  do {
    line++;
    old_q = q;
    if (q == p) {
      *on_screen_line = line;
      on_scr = TRUE;
    }
    q = q->next;
  } while (old_q != screen_end);
  on_scr = (on_scr && *on_screen_line > 1 && *on_screen_line < SCREEN_LEN);
  return on_scr;
}
/*}}}  */
/*{{{  line_refresh*/
#ifdef STD_C
PRIVATE void line_refresh(int level,element *ptr)
#else
PRIVATE void line_refresh(level, ptr)
int level;
element *ptr;
#endif
{
  copyin(line_buffer, ptr, FALSE);
  gotoxy(1, level);
  if (strlen(line_buffer) >= LEN) {
    line_buffer[LEN-1]=END_OF_LINE;
    line_buffer[LEN]='\0';
    fputs(line_buffer, stdout);
  } else {
    fputs(line_buffer, stdout);
    clreol();
  }
}
/*}}}  */
/*{{{  write_dsp_line*/
#ifdef STD_C
void write_dsp_line(element *ptr,int level)
#else
void write_dsp_line(ptr, level)
element *ptr;
int level;
#endif
{
  line_refresh(level, ptr);
}
/*}}}  */
 
/*{{{  title_op*/
#ifdef STD_C
void title_op(TITLE_OP op)
#else
void title_op(op)
TITLE_OP op;
#endif
{
  /*{{{  variables*/
  char filename[_POSIX_PATH_MAX+1];
  char statline[MAX_FIELD_SIZE+1];
  char language[20];
  int tail;
  static bool dm_tag=FALSE;
  static bool em_tag=FALSE;
  static bool cf_tag=FALSE;
  static bool view_tag=FALSE;
  static bool change_tag=FALSE;
  static bool arg_tag=FALSE;
  static bool over_tag=FALSE;
  static bool auto_tag=FALSE;
  /*}}}  */
 
  /*{{{  handle title-set/reset arguments*/
  switch(op) {
    case SHOW: break;
    /*{{{  SETs*/
    case SET_DM:     dm_tag=TRUE; break;
    case SET_EM:     em_tag=TRUE; break;
    case SET_CF:     cf_tag=TRUE; break;
    case SET_CHANGE: change_tag=TRUE; break;
    case SET_ARG:    arg_tag=TRUE; break;
    case SET_VIEW:   view_tag=TRUE; break;
    case SET_OVER:   over_tag=TRUE; break;
    case SET_AUTO:   auto_tag=TRUE; break;
    /*}}}  */
    /*{{{  RESETs*/
    case RESET_DM:     dm_tag=FALSE; break;
    case RESET_EM:     em_tag=FALSE; break;
    case RESET_CF:     cf_tag=FALSE; break;
    case RESET_CHANGE: change_tag=FALSE; break;
    case RESET_ARG:    arg_tag=FALSE; break;
    case RESET_VIEW:   view_tag=FALSE; break;
    case RESET_OVER:   over_tag=FALSE; break;
    case RESET_AUTO:   auto_tag=FALSE; break;
    /*}}}  */
  }
  /*}}}  */
  if (real_tail == real_head)
    *filename = '\0';
  else
    line_of(filename, real_tail);
  /*{{{  get correct language-mark*/
  strcpy(language,language_string[dialect]);
  if (dialect == NO_LANGUAGES-1)
  {
    strcat(language," ");
    strcat(language,dialect_start[dialect]);
    strcat(language," ");
    strcat(language,dialect_end[dialect]);
  }
  /*}}}  */
  /*{{{  data -> statline*/
  sprintf(statline,titleformat,
          change_tag ? '*' : '=',
          version,
          keybinding_name,
          language,
          view_tag ? VIEW_TAG : "",
          dm_tag ? DM_TAG : "",
          em_tag ? EM_TAG : "",
          cf_tag ? CF_TAG : "",
          arg_tag ? ARG_TAG : "",
          over_tag ? OVER_TAG : "",
          auto_tag ? AUTO_TAG : "",
          user_mode,
          filename);
  /*}}}  */
  tail=W_dx-strlen(statline);
  /*{{{  if to long, try short tags*/
  if (tail<0) {
    /*{{{  data -> statline*/
    sprintf(statline,titleformat,
            change_tag ? '*' : '=',
            version,
            keybinding_name,
            language,
            view_tag ? sVIEW_TAG : "",
            dm_tag ? sDM_TAG : "",
            em_tag ? sEM_TAG : "",
            cf_tag ? sCF_TAG : "",
            arg_tag ? sARG_TAG : "",
            over_tag ? sOVER_TAG : "",
            auto_tag ? sAUTO_TAG : "",
            user_mode,
            filename);
    /*}}}  */
    tail=W_dx-strlen(statline);
  }
  /*}}}  */
  statline[W_dx-2*sg]='\0';
  while (tail-->0) strcat(statline,titleend);
  gotoxy(1,TITLE_LINE);
  standout();
  fputs(statline,stdout);
  standend();
}
/*}}}  */
/*{{{  restore*/
#ifdef STD_C
void restore(int from)
#else
void restore(from)
int from;
#endif
{
  element *p=screen_start;
  int h;
 
  for (h = 1; h <= screen_end_level; h++) {
    /*{{{  display if under from*/
    if (h >= from) write_dsp_line(p, h);
    /*}}}  */
    p = p->next;
  }
  if (screen_end_level < SCREEN_LEN) {
    W_oy=screen_end_level + 1; W_dy=SCREEN_LEN;
    ClrScr();
    full_window();
    title_op(SHOW);
  }
  dirty=FALSE;
#  ifdef TOS
    st_redraw();
#  endif
}
/*}}}  */
/*{{{  restore_to_end*/
#ifdef STD_C
void restore_to_end(int from)
#else
void restore_to_end(from)
int from;
#endif
{  /*if select_on then set select separately*/
  screen_end = current;
  screen_end_level = cursor_level;
  while (screen_end_level < SCREEN_LEN && screen_end != tail) {
    screen_end_level++;
    screen_end = screen_end->next;
  }
  restore(from);
}
/*}}}  */
/*{{{  restore_element*/
#ifdef STD_C
void restore_element(uchar close_line)
#else
void restore_element(close_line) unsigned char close_line;
#endif
{  /*if select_on then set select separately*/
  int level, dist_to_end;
  int dist_to_start;

  dist_to_start = to_start((int)(SCREEN_LEN - 1));
  dist_to_end = to_bottom((int)(SCREEN_LEN - 2));
  cursor_level = close_line;
  if (cursor_level < 2) cursor_level = 2;
  if (cursor_level >= SCREEN_LEN) cursor_level = SCREEN_LEN - 1;
  if (dist_to_start < cursor_level) cursor_level = dist_to_start;
  while (cursor_level+dist_to_end < SCREEN_LEN && dist_to_start > cursor_level)
    cursor_level++;
  screen_start = current;
  for (level = cursor_level - 1; level >= 1; level--)
    screen_start = screen_start->prec;
  restore_to_end(1);
}
/*}}}  */
/*{{{  restore_or_restore_to_end*/
#ifdef STD_C
void restore_or_restore_to_end(void)
#else
void restore_or_restore_to_end()
#endif
{
  element *p;

  if (screen_start != head->next) {
    restore_element(SCREEN_LEN/2);
    return;
  }
  p = head->next;
  cursor_level = 1;
  while (p != current) {
    cursor_level++;
    p = p->next;
  }
  restore_to_end(cursor_level);
}
/*}}}  */

/*{{{  up_a_bit*/
#ifdef STD_C
void up_a_bit(int from)
#else
void up_a_bit(from) int from;
#endif
{
  W_oy=from; W_dy=SCREEN_LEN;
  if (from!=TITLE_LINE-1) {
    gotoxy(1,W_oy); DelLine();
    gotoxy(1,TITLE_LINE-1); insLine();
  }
  gotoxy(1,W_oy);full_window();
}
/*}}}  */
/*{{{  down_a_bit*/
#ifdef STD_C
void down_a_bit(int from)
#else
void down_a_bit(from) int from;
#endif
{
  W_oy=from; W_dy=SCREEN_LEN;
  if (from!=TITLE_LINE-1) {
    gotoxy(1,W_oy); insLine();
    gotoxy(1,TITLE_LINE); DelLine();
  }
  gotoxy(1,W_oy); full_window();
}
/*}}}  */
/*{{{  whole_screen_up*/
#ifdef STD_C
void whole_screen_up(void)
#else
void whole_screen_up()
#endif
{
  if (screen_start == tail) return;
  up_a_bit(1);
  screen_start = screen_start->next;
  cursor_level--;
  if (screen_end != tail) {
    screen_end = screen_end->next;
    write_dsp_line(screen_end, SCREEN_LEN);
  } else
    screen_end_level--;
  if (dirty) restore(1);
}
/*}}}  */
/*{{{  whole_screen_down*/
#ifdef STD_C
void whole_screen_down(void)
#else
void whole_screen_down()
#endif
{
  if (screen_start == head->next) return;
  down_a_bit(1);
  screen_start = screen_start->prec;
  cursor_level++;
  write_dsp_line(screen_start, 1);
  if (screen_end_level == SCREEN_LEN)
    screen_end = screen_end->prec;
  else
    screen_end_level++;
  if (dirty) restore(1);
}
/*}}}  */
