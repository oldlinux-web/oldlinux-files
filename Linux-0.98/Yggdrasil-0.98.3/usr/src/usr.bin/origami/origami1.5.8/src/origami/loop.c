/*{{{  #includes*/
#include <sys/types.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <local/bool.h>

#define LOOP_C

#include "origami.h"
#include "macros.h"
/*}}}  */

/*{{{  variables*/
int  dialect = 0;
char line_buffer[MAX_FIELD_SIZE + 1];
char line1_buffer[MAX_FIELD_SIZE + 1];
/*}}}  */

/*{{{  #define append_to_pick*/
#define append_to_pick() (join_links(pick_tail, pick_ptr), \
                       pick_ptr->UU.U1.indent = 0, \
                       pick_tail = pick_ptr)

/*}}}  */
/*{{{  main_editor_loop*/
#ifdef STD_C
void
main_editor_loop(char *argv[], int gotoarg)
#else
void
main_editor_loop(argv, gotoarg)
char *argv[];
int  gotoarg;
#endif
{
  /*{{{  variables*/
  TOKEN    ch;
  bool     keypad = TRUE;
  bool     quit = FALSE;
  bool     del_under = FALSE;
  int      cursor_x_pos = 1;
  element  *WITH;
  element  *move_ptr = NULL;
  element  *copy_ptr = NULL;
  element  *pick_ptr = NULL;
  element  *line_dsp_line = NULL;
  /*}}}  */

  /*{{{  startup the session*/
  initialise();
  read_list(NULL);
  if (gotoarg > 0) find_element(gotoarg, 4);
  if (auto_save) {
    title_op(SET_AUTO);
    set_alarm(0);
  }
  call_auto_macro();
  /*}}}  */
  do {
    copyin(current_dsp_line, current, FALSE);
    proc_to_edit_pos();
    /*{{{  interrupt_restore?*/
    if (interrupt_restore) {
      restore(1);
      interrupt_restore = FALSE;
    }
    /*}}}  */
    /*{{{  aborted?*/
    if (aborted) {
      aborted = FALSE;
      message(M_ABORTED, TRUE);
    }
    /*}}}  */
    /*{{{  get next editor-token*/
    if (do_auto_save && !select_on)
      ch = O_AUTO_SAVE;
#        ifdef SIGWINCH
    else if (win_changed)
      ch = O_REFRESH;
#        endif
    else
      ch = field_edit(&keypad, &cursor_x_pos, current_dsp_line);
    /*}}}  */
    proc_from_edit_pos();
    if (current->foldline != END_FOLD)
      copyout(current_dsp_line, current);
    if (valid_screen_key(&keypad, &ch)) {
      no_message();
      if (keypad) {
        switch (ch) {
           /*{{{  M_EXIT*/
           case M_EXIT:{
            char string[100];
 
            prompt_string(string);
            title_op(RESET_EM);
            if (*string != '\0')
              message(string, TRUE);
            stop_macros();
            break;
           }
           /*}}}  */
           /*{{{  M_GO_LINE*/
           case M_GO_LINE:{
             cursor_x_pos = 1;
             find_element(macro_int[get_arg()], 4);
             break;
           }
           /*}}}  */
           /*{{{  O_AUTO_FOLD*/
           case O_AUTO_FOLD:{
             if (!select_on) { /* makefold */
               start_make_fold(cursor_x_pos);  /* makefold */
               if (select_on) {
                 while (current != tail && last_char(line_of(line_buffer, current)) > 0)
                   move_down();
                 make_fold(&cursor_x_pos);
                 if (!select_on) { /* makefold */
                   WITH = current;
                   copy_parts_to_parts(WITH->fold, &current);
                   write_dsp_line(current, cursor_level);
                   move_down();
                   while (current != tail && last_char(line_of(line_buffer, current)) == 0)
                     move_down();
                 }
               }
             }
             break;
           }
           /*}}}  */
           /*{{{  O_AUTO_SAVE*/
           case O_AUTO_SAVE:{
             auto_write();
             break;
           }
           /*}}}  */
           /*{{{  O_ATTACH_FILE*/
           case O_ATTACH_FILE:{
             attach_file();
             break;
           }
           /*}}}  */
           /*{{{  O_A_AUTO_SAVE*/
           case O_A_AUTO_SAVE:{
             char time[PROMPT_LEN + 1];
           
             readprompt(time, "auto-save-interval", 10);
             if (!aborted) {
               auto_save = TRUE;
               set_alarm(atoi(time));
               title_op(SET_AUTO);
             }
             break;
           }
           /*}}}  */
           /*{{{  O_A_ECHO*/
           case O_A_ECHO:{
             switch_echo(TRUE);
             break;
           }
           /*}}}  */
           /*{{{  O_BOT_OF_FOLD*/
           case O_BOT_OF_FOLD:{
             bottom();
             break;
           }
           /*}}}  */
           /*{{{  O_BREAK*/
           case O_BREAK:{
             break;
           }
           /*}}}  */
           /*{{{  O_CLOSE_FOLD*/
           case O_CLOSE_FOLD:{
             close_fold();
             break;
           }
           /*}}}  */
           /*{{{  O_COPY*/
           case O_COPY:{
             if (  (current->foldline == NOT_FOLD ||
                    current->foldline == START_FOLD)
                 && current != tail
                 && copy_a_line(&copy_ptr))
               undelete_after(copy_ptr);
             break;
           }
           /*}}}  */
           /*{{{  O_COPY_PICK*/
           case O_COPY_PICK:{
             if (current->foldline == NOT_FOLD || current->foldline == START_FOLD) {
               pick_ptr = NULL;
               if (copy_a_line(&pick_ptr))
                 append_to_pick();
             }
             break;
           }
           /*}}}  */
           /*{{{  O_CREATE_FOLD*/
           case O_CREATE_FOLD:{
             if (!select_on) {
               if (!entered(current)) {
                 start_make_fold(cursor_x_pos);
                 title_op(SET_CF);
               }
             } else {
               make_fold(&cursor_x_pos);
               title_op(RESET_CF);
             }
             break;
           }
           /*}}}  */
           /*{{{  O_DEL_LINE*/
           case O_DEL_LINE:{
             remove_line(&line_dsp_line);
             break;
           }
           /*}}}  */
           /*{{{  O_DEL_CHAR_RIGHT/O_DELETES*/
           case O_DEL_CHAR_R:{
             move_down();
             del_under = TRUE;
           }
           case O_DELETE:{
             if (current->foldline == NOT_FOLD) {
               if (current != tail && current != head->next
                   && current->prec->foldline == NOT_FOLD) {
                 copy_parts_to_line(current, line_buffer);
                 trailing_spaces(line_buffer);
                 move_up();
                 copyin(current_dsp_line, current, FALSE);
                 trailing_spaces(current_dsp_line);
                 cursor_x_pos = strlen(current_dsp_line) + 1;
                 if ((strlen(line_buffer) + strlen(current_dsp_line))
                     <= MAX_FIELD_SIZE) {
                   copy_parts_to_line(current, current_dsp_line);
                   strcat(current_dsp_line, line_buffer);
                   copy_line_to_parts(current_dsp_line, &current);
                   write_dsp_line(current, cursor_level);
                   move_down();
                   part_line = NULL;
                   remove_line(&part_line);
                   proc_dispose(part_line);
                   move_up();
                   del_under = FALSE;
                 } else {
                   if (del_under) move_up();
                   message(M_LONG_LINE, TRUE);
                 }
                 break;
               }
             }
             if (del_under) {
               move_up();
               del_under = FALSE;
             }
             vmessage(M_ERR_PO);
             break;
           }
           /*}}}  */
           /*{{{  O_DESCRIBE_BINDINGS*/
           case O_DESCRIBE_BINDINGS:{
             if (help(FALSE)) restore(1);
             break;
           }
           /*}}}  */
           /*{{{  O_D_AUTO_SAVE*/
           case O_D_AUTO_SAVE:{
             auto_save = FALSE;
             reset_alarm();
             title_op(RESET_AUTO);
             break;
           }
           /*}}}  */
           /*{{{  O_D_ECHO*/
           case O_D_ECHO:{
             switch_echo(FALSE);
             break;
           }
           /*}}}  */
           /*{{{  O_DOWN*/
           case O_DOWN:{
             move_down();
             break;
           }
           /*}}}  */
           /*{{{  O_ENTER_FOLD*/
           case O_ENTER_FOLD:{
             cursor_x_pos = 1;
             enter_fold();
             break;
           }
           /*}}}  */
           /*{{{  O_EXIT_FOLD*/
           case O_EXIT_FOLD:{
             exit_fold();
             break;
           }
           /*}}}  */
           /*{{{  O_FILE_FOLD*/
           case O_FILE_FOLD:{
             file_fold();
             break;
           }
           /*}}}  */
           /*{{{  O_FILTER_BUFFER*/
           case O_FILTER_BUFFER:{
             put_vars(cursor_x_pos);
             filter_buffer();
             break;
           }
           /*}}}  */
           /*{{{  O_FIND/O_FIND_REVERSE*/
           case O_FIND:{
           case O_FIND_REVERSE:
             get_search();
             if (*item_to_look_for != '\0' && !aborted)
               if (!find_item(&cursor_x_pos, ch == O_FIND_REVERSE))
                 message(M_FAILED, FALSE);
             break;
           }
           /*}}}  */
           /*{{{  O_FINISH*/
           case O_FINISH:{
             do_save_file();
             quit = TRUE;
             break;
           }
           /*}}}  */
           /*{{{  O_FIRST_LINE_TO_FOLD_HEADER*/
           case O_FIRST_LINE_TO_FOLD_HEADER:{
             WITH = current;
             if (WITH->foldline == START_FOLD) {
               copy_parts_to_parts(WITH->fold, &current);
               write_dsp_line(current, cursor_level);
             }
             break;
           }
           /*}}}  */
           /*{{{  O_FOLD_INFO*/
           case O_FOLD_INFO:{
             WITH = current;
             message("Line  ", FALSE);
             printf("%d. Enter depth %d folds. %d spaces.",
                    get_line_no(),
                    enter_depth,
                    enter_depth_spaces);
             if (filed_or_fold(current))
               printf("%sA %2d %d",
                      fold_file_str,
                      WITH->UU.U1.fold_type,
                      WITH->UU.U1.fold_contents);
             if (WITH->foldline == START_FILED)
               printf(" %s", WITH->other_end->strng);
             break;
           }
           /*}}}  */
           /*{{{  O_GOTO_LINE*/
           case O_GOTO_LINE:{
             goto_line(&cursor_x_pos);
             break;
           }
           /*}}}  */
           /*{{{  O_HELP*/
           case O_HELP:{
             if (help(TRUE)) restore(1);
             break;
           }
           /*}}}  */
           /*{{{  O_INSERT_FILE*/
           case O_INSERT_FILE:{
             if (!entered(current)) {
               int old_dia = dialect;
           
               insert_file((element *) NULL, current->prec, current, FALSE, NULL);
               dialect = old_dia;
               title_op(SHOW);
               restore_element((uchar) cursor_level);
             }
             break;
           }
           /*}}}  */
           /*{{{  O_ITS_REVERSE*/
           case O_ITS_REVERSE:{
             its_search(&cursor_x_pos, TRUE);
             break;
           }
           /*}}}  */
           /*{{{  O_ITS_SEARCH*/
           case O_ITS_SEARCH:{
             its_search(&cursor_x_pos, FALSE);
             break;
           }
           /*}}}  */
           /*{{{  O_LANGUAGE*/
           case O_LANGUAGE:{
             proc_language();
             break;
           }
           /*}}}  */
           /*{{{  O_MOVE*/
           case O_MOVE:{
             if (move_ptr != NULL) {
               if (!entered(current)) {
                 undelete_before(move_ptr);
                 move_ptr = NULL;
               }
             } else
               remove_line(&move_ptr);
             break;
           }
           /*}}}  */
           /*{{{  O_NEXT_FILE*/
           case O_NEXT_FILE:{
             next_file(1, argv);
             break;
           }
           /*}}}  */
           /*{{{  O_OPEN_FOLD*/
           case O_OPEN_FOLD:{
             open_fold();
             break;
           }
           /*}}}  */
           /*{{{  O_OPEN_NEW_FILE*/
           case O_OPEN_NEW_FILE:{
             open_new_file();
             break;
           }
           /*}}}  */
           /*{{{  O_PAGE_DOWN*/
           case O_PAGE_DOWN:{
             int i = SCREEN_LEN - 2;
           
             /*{{{  skip i lines*/
             while (i-- && current != tail) {
               if (select_on && select_forward()) current->selected = FALSE;
               current = current->next;
               if (select_on) current->selected = TRUE;
             }
             /*}}}  */
             restore_element(1);
             break;
           }
           /*}}}  */
           /*{{{  O_PAGE_UP*/
           case O_PAGE_UP:{
             int i = SCREEN_LEN - 2;
           
             /*{{{  skip i lines*/
             while (i-- && current->prec != head) {
               if (select_on && select_backward()) current->selected = FALSE;
               current = current->prec;
               if (select_on) current->selected = TRUE;
             }
             /*}}}  */
             restore_element(1);
             break;
           }
           /*}}}  */
           /*{{{  O_PICK*/
           case O_PICK:{
             pick_ptr = NULL;
             remove_line(&pick_ptr);
             if (pick_ptr != NULL) append_to_pick();
             break;
           }
           /*}}}  */
           /*{{{  O_PIPE_FROM_COMMAND*/
           case O_PIPE_FROM_COMMAND:{
             if (!entered(current)) {
               put_vars(cursor_x_pos);
               pipe_from_command();
               restore_element((uchar) cursor_level);
             }
             break;
           }
           /*}}}  */
           /*{{{  O_PIPE_TO_COMMAND*/
           case O_PIPE_TO_COMMAND:{
             put_vars(cursor_x_pos);
             if (tail == real_tail)
               pipe_to_command();
             else
               message(M_NO_TOP, TRUE);
             break;
           }
           /*}}}  */
           /*{{{  O_PREV_FILE*/
           case O_PREV_FILE:{
             next_file(-1, argv);
             break;
           }
           /*}}}  */
           /*{{{  O_PUT_PICK*/
           case O_PUT_PICK:{
             if (!entered(current))
               if (pick_head != pick_tail)
                 undelete_pick_before();
               else
                 message(M_NO_PICK, TRUE);
             break;
           }
           /*}}}  */
           /*{{{  O_QUERY_REPLACE*/
           case O_QUERY_REPLACE:{
             query_replace(&cursor_x_pos);
             break;
           }
           /*}}}  */
           /*{{{  O_QUIT*/
           case O_QUIT:{
             quit = (file_changed ? yes(M_CHANGED_LEAVE) : TRUE);
             break;
           }
           /*}}}  */
           /*{{{  O_REFRESH*/
           case O_REFRESH:{
             if (get_terminal_capability()) return;
             full_window();
             ClrScr();
             restore_element(SCREEN_LEN / 2);
             title_op(SHOW);
             /*{{{  window?*/
#                          ifdef SIGWINCH
             if (win_changed) {
               win_changed = FALSE;
               vmessage(M_WIN_CHANGED);
             }
#                          endif
             /*}}}  */
             break;
           }
           /*}}}  */
           /*{{{  O_REMOVE_FOLD*/
           case O_REMOVE_FOLD:{
             remove_fold();
             break;
           }
           /*}}}  */
           /*{{{  O_REPLACE*/
           case O_REPLACE:{
             replace(&cursor_x_pos);
             break;
           }
           /*}}}  */
           /*{{{  O_RETURN*/
           case O_RETURN:{
             if (!select_on)
               if (split_line)
                 undelete_after(part_line);
               else {
                 undelete_before(part_line);
                 move_down();
               }
             break;
           }
           /*}}}  */
           /*{{{  O_SAVE_FILE*/
           case O_SAVE_FILE:{
             save_file();
             break;
           }
           /*}}}  */
           /*{{{  O_SHELL_COMMAND*/
           case O_SHELL_COMMAND:{
             char cmd[PROMPT_LEN + 1];
           
             put_vars(cursor_x_pos);
             readprompt(cmd, M_SH_COMMAND, PROMPT_LEN);
             if (*cmd == '\0') break;
             gotoxy(1, screen_lines);
             putchar('\n');
             fflush(stdout);
             reset_keyboard();
             reset_terminal();
             system(cmd);
             init_terminal();
             init_keyboard();
             message(M_END, TRUE);
             hide_key(TRUE);
             gotoxy(1, 1);
             ClrFullScr();
             title_op(SHOW);
             restore(1);
             break;
           }
           /*}}}  */
           /*{{{  O_SHELL*/
           case O_SHELL:{
             char *shell;
           
             if ((shell = getenv("SHELL")) == 0) {
               message(M_NO_SHELL_VAR, TRUE);
               break;
             }
             put_vars(cursor_x_pos);
             message(M_SHELL_CREATED, TRUE);
             gotoxy(1, screen_lines);
             putchar('\n');
             fflush(stdout);
             reset_keyboard();
             reset_terminal();
             system(shell);
             init_terminal();
             init_keyboard();
             gotoxy(1, 1);
             ClrFullScr();
             title_op(SHOW);
             restore(1);
             break;
           }
           /*}}}  */
           /*{{{  O_SUSPEND*/
           case O_SUSPEND: {
             bktoshell();
             break;
           }
           /*}}}  */
           /*{{{  O_TOP_OF_FOLD*/
           case O_TOP_OF_FOLD: {
             top();
             break;
           }
           /*}}}  */
           /*{{{  O_UNDEL_LINE*/
           case O_UNDEL_LINE:{
             if (!entered(current)) {
               if (line_dsp_line == NULL)
                 message(M_NO_UNDEL, TRUE);
               else
                 undelete_before(line_dsp_line);
               line_dsp_line = NULL;
             }
             break;
           }
           /*}}}  */
           /*{{{  O_UP*/
           case O_UP:{
             move_up();
             break;
           }
           /*}}}  */
           /*{{{  O_WRITE_FILE*/
           case O_WRITE_FILE:{
             write_file();
             break;
           }
           /*}}}  */
        }
      }
    } else
      message(M_NOFOLDKEY, TRUE);
  } while (!quit);
 
  full_window();
  message(M_BYE, TRUE);
  gotoxy(1, screen_lines);
}
/*}}}  */
