/* origami.h - contains some definitions and a lot of prototypes */

/* this must be included first! */
#include "short.h"
 
#include "../common/os.h"
#include "../common/keys.h"
 
#define PRIVATE static
#define PUBLIC
 
/*{{{  constants*/
#define MAX_FIELD_SIZE  256
#define SEARCHLEN        40
#define PROMPT_LEN      128
#define END_OF_LINE     '$'
/*{{{  display-data*/
#define TITLE_LINE      (screen_lines-1)
#define MESSAGE_LINE    screen_lines
#define OVERFLOW_LINE   MESSAGE_LINE
#define SCREEN_LEN      (screen_lines-2)
#define START           1
#define LEN             W_dx
#define SHIFT           W_dx/3
/*}}}  */
/*{{{  fold-tags*/
#define NOT_FOLD          0
#define START_FILED       0x81
#define START_FOLD        0x82
#define END_FOLD          0x83
#define START_OPEN_FOLD   0x86
#define START_ENTER_FILED 0x87
#define START_ENTER_FOLD  0x88
/*}}}  */
/*{{{  language*/
#define NO_LANGUAGES 8   /*(inc User-defined)+tds*/
#define fortran_     6
#define c_typ        3
/*}}}  */
/*{{{  macros*/
#define MACRO_COUNT (MAX_MACRO+1)
#define MAX_MACRO_LENGTH 512
#define M_DEPTH          64
/*}}}  */
#define MARKER_LEN      5
#define version         "1.5.8"
#define user_mode_lg    9
#define FileNotFound 2
#define EndOfFile 3
#define FileWriteError 4
#define MIN_ALARMTIME 2
#define MAX_ALARMTIME 32
#define NORM_ALARMTIME 8
/*}}}  */
/*{{{  types*/
/*{{{  typedef TITLE_OP*/
typedef enum {
  SHOW=0,
  SET_DM,
  SET_EM,
  SET_CF,
  SET_VIEW,
  SET_CHANGE,
  SET_ARG,
  SET_OVER,
  SET_AUTO,
  RESET_DM,
  RESET_EM,
  RESET_CF,
  RESET_VIEW,
  RESET_CHANGE,
  RESET_ARG,
  RESET_OVER,
  RESET_AUTO
} TITLE_OP;
/*}}}  */
typedef unsigned char uchar;
typedef uchar buffer[1024];
typedef char *language_comment[NO_LANGUAGES + 1];
/*{{{  element*/
typedef struct element {
  char *strng;
  struct element *prec, *next, *fold, *other_end;
  bool selected;
  uchar foldline, fold_close_line;
  union {
    struct {
      SHORT indent, fold_indent, fold_type, fold_contents;
    } U1;
    struct {
      int int1, int2;
    } U0;
  } UU;
} element;
/*}}}  */
typedef int key_conversion[3];
typedef key_conversion key_convert_lists[];
/*{{{  LOC_field*/
typedef struct LOC_field {
  int *field_pos;
  char *field_str;
  bool field_key;
  int ind;
} LOC_field;
/*}}}  */
/*{{{  LOC_file*/
typedef struct {
  FILE *tdsfile;
  element *current, *trailing, *S;
  char filename[_POSIX_PATH_MAX+1];
  int new_indent;
  char dsp_line[MAX_FIELD_SIZE + 1];
  char tdsfile_NAME[_POSIX_PATH_MAX+1];
} LOC_file;
/*}}}  */
/*{{{  LOC_proc*/
typedef struct LOC_proc {
  element *file_ptr, *p, *pn;
  char dsp_line[MAX_FIELD_SIZE + 1];
  FILE *tdsfile;
  char tdsfile_NAME[_POSIX_PATH_MAX+1];
} LOC_proc;
/*}}}  */
/*{{{  macro_entry*/
typedef struct {
  bool defineable;
  TOKEN *string;
} macro_entry;
/*}}}  */
/*}}}  */
 
/*{{{  STD_C*/
#ifdef STD_C
#  define P(s) s
#else
#  define P(s) ()
#endif
/*}}}  */

/* where should these functions be defined? */
extern FILE *popen P((char *, char *));
extern int pclose P((FILE *));
 
/*{{{  DISPLAY_C*/
#ifndef DISPLAY_C
  extern void ClrScr P((void));
  extern void write_dsp_line P((element*, int));
  extern void title_op P((TITLE_OP));
  extern void delete_dsp_line P((int));
  extern bool on_screen P((element*,uchar*));
  extern void restore P((int));
  extern void restore_element P((uchar));
  extern void restore_or_restore_to_end P((void));
  extern void restore_to_end P((int));
  extern element *screen_start;
  extern element *screen_end;
  extern char keybinding_name[keyn_lg+1];
  extern int cursor_level, screen_end_level;
  extern int W_dy, W_oy,W_dx;
  extern void down_a_bit P((int));
  extern void up_a_bit P((int));
  extern void whole_screen_up P((void));
  extern void whole_screen_down P((void));
#endif
/*}}}  */
/*{{{  FIELDEDIT_C*/
#ifndef FIELDEDIT_C
  extern int x_shift P((int));
  extern TOKEN field_edit P((bool*, int*, char*));
  extern bool file_changed;
  extern bool browse_mode;
  extern element *part_line;
  extern bool overwrite;
#endif
/*}}}  */
/*{{{  FINDS_C*/
#ifndef FINDS_C
  extern bool find_item P((int*, bool));
  extern void get_search P((void));
  extern void find_element P((int, uchar));
  extern element *pre_find_element P((int));
  extern int line_no P((element*));
  extern int get_line_no P((void));
  extern void goto_line P((int*));
  extern void query_replace P((int*));
  extern void replace P((int*));
  extern void its_search P((int*,bool));
  extern char item_to_look_for[SEARCHLEN];
#endif
/*}}}  */
/*{{{  FOLDFILING_C*/
#ifndef FOLDFILING_C
extern void file_fold P((void));
extern void proc_language P((void));
extern void attach_file P((void));
extern void filter_buffer P((void));
extern void pipe_from_command P((void));
extern void pipe_to_command P((void));
extern void do_save_file P((void));
extern char *dump_file P((char*));
extern void save_file P((void));
extern void auto_write P((void));
extern void delay_auto_write P((void));
extern void open_new_file P((void));
extern void next_file P((int,char *argv[]));
extern void write_file P((void));
#endif
/*}}}  */
/*{{{  FOLDING_C*/
#ifndef FOLDING_C
  extern void create_list P((FILE*));
  extern bool select_forward P((void));
  extern bool select_backward P((void));
  extern void top P((void));
  extern void bottom P((void));
  extern int to_bottom P((int));
  extern int to_start P((int));
  extern void line_or_move_up P((int));
  extern bool copy_a_line P((element**));
  extern void pre_exit_fold P((void));
  extern void enter_fold P((void));
  extern void exit_fold P((void));
  extern void start_make_fold P((int));
  extern void undelete_before P((element*));
  extern void undelete_pick_before P((void));
  extern void undelete_after P((element*));
  extern void pre_remove_line P((element*));
  extern void remove_line P((element**));
  extern void delete_list P((element*, element*));
  extern void move_or_line_down P((int));
  extern void make_fold P((int*));
  extern void open_fold P((void));
  extern void close_and_open_folds P((element*));
  extern void close_fold P((void));
  extern void remove_fold P((void));
  extern void pre_open_fold P((element*));
  extern bool select_on;
  extern element *pick_head;
  extern element *pick_tail;
  extern element *current;
  extern element *head;
  extern element *tail;
  extern element *real_head;
  extern element *real_tail;
  extern element *End_of_fold;
  extern element *Start_of_fold;
  extern element *Line_after_fold;
  extern char current_dsp_line[MAX_FIELD_SIZE+1];
  extern int enter_depth, enter_depth_spaces;
#endif
/*}}}  */
/*{{{  GETTK_C*/
#ifndef GETTK_C
  extern void prompt_string P((char*));
  extern void stop_macros P((void));
  extern void ori_abort P((void));
  extern void call_auto_macro P((void));
  extern TOKEN get_arg P((void));
  extern TOKEN hide_key P((bool));
  extern TOKEN edit_key P((bool*));
  extern bool macro_tag;
  extern char user_mode[user_mode_lg+1];
  extern int m_s_depth;
  extern int *macro_int;
  extern macro_entry fix_macros[MACRO_COUNT];
  extern int auto_macro;
#endif
/*}}}  */
/*{{{  INITIALISE_C*/
#ifndef INITIALIZE_C
  extern int HostBegin P((void));
  extern void initialise P((void));
  extern char mark_mode[mark_name_lg];
#endif
/*}}}  */
/*{{{  KEYBOARD_C*/
#ifndef KEYBOARD_C
  extern void enable_echo P((bool));
  extern void switch_echo P((bool));
  extern void set_abort P((char));
  extern void enable_abort P((void));
  extern void disable_abort P((void));
  extern void init_keyboard P((void));
  extern void reset_keyboard P((void));
  extern TOKEN get_key P((void));
  extern bool echo_disabled;
#endif
/*}}}  */
/*{{{  KEYTAB_C*/
#ifndef KEYTAB_C
  extern void set_key_top P((KEY*));
  extern bool valid_key P((bool*,TOKEN*));
  extern bool valid_field_key P((bool*,TOKEN*));
  extern bool valid_screen_key P((bool*,TOKEN*));
#endif
/*}}}  */
/*{{{  LOOP_C*/
#ifndef LOOP_C
extern int dialect;
extern char line_buffer[MAX_FIELD_SIZE+1];
extern char line1_buffer[MAX_FIELD_SIZE+1];
extern void main_editor_loop P((char **, int));
#endif
/*}}}  */
/*{{{  MAIN_C*/
#ifndef MAIN_C
extern char keybase[_POSIX_PATH_MAX+1];
extern bool c_hash_front;
extern char *filearg;
extern bool split_line;
extern bool auto_save;
extern bool do_auto_save;
extern bool verbose;
extern bool aborted;
#ifdef SIGWINCH
extern bool win_changed;
#endif
extern bool interrupt_restore;
extern element *null_item_ptr;
extern element *new_file_ptr;
extern int current_ind;
extern language_comment dialect_start;
extern language_comment dialect_end;
extern char *language_string[NO_LANGUAGES + 1];
extern char fold_open_str[tag_length];
extern char fold_close_str[tag_length];
extern char fold_file_str[tag_length];
extern char fold_line_str[tag_length];
extern int f_begin;
extern int f_end;
extern int f_current;
#endif
/*}}}  */
/*{{{  MESSAGES_C*/
#ifndef MESSAGES_C
  extern char F_NO_OPEN[];
  extern char F_F_FAILED[];
  extern char F_UNABLE[];
  extern char M_NOT_BOUND[];
  extern char M_CANTOPEN[];
  extern char M_CANTWRITE[];
  extern char F_WRITTEN[];
  extern char F_WRITING[];
  extern char F_TERMA[];
  extern char F_TERMB[];
  extern char F_LOADING[];
  extern char F_SUBSTITUTIONS[];
  extern char F_TCFF[];
  extern char M_ABORTED[];
  extern char M_ANY[];
  extern char M_ANY_Q[];
  extern char M_ARG[];
  extern char M_AUTO_SAVED[];
  extern char M_BOTTOM[];
  extern char M_BYE[];
  extern char M_CHANGED_LEAVE[];
  extern char M_CHANGED_SAVE[];
  extern char M_CLOSE_FAILED[];
  extern char M_CO_END[];
  extern char M_CO_START[];
  extern char M_DEF_EXE[];
  extern char M_END[];
  extern char M_ERR_ARG[];
  extern char M_EXE_EXE[];
  extern char M_ERR_MACROPLACE[];
  extern char M_ERR_PO[];
  extern char M_EX_NFF[];
  extern char M_FAILED[];
  extern char M_FILENAME[];
  extern char M_FILE_NOT_FOUND[];
  extern char M_FIX_EXE[];
  extern char M_FOLDERR[];
  extern char M_GOTO[];
  extern char M_KEY_BUFF_FULL[];
  extern char M_LONG_LINE[];
  extern char M_MAIL_S[];
  extern char M_MAIL_1[];
  extern char M_MAIL_2[];
  extern char M_NOFOLDKEY[];
  extern char M_NOT_IF_TDS[];
  extern char M_NO_AUTO[];
  extern char M_NO_BACK_FOLDING[];
  extern char M_NO_BIND[];
  extern char M_NO_CHANGE[];
  extern char M_NO_DEFINE[];
  extern char M_NO_ENTER_LIST_FF[];
  extern char M_NO_FF_COPY[];
  extern char M_NO_HELP[];
  extern char M_NO_HOME[];
  extern char M_NO_KTAB[];
  extern char M_NO_MARKS[];
  extern char M_NO_MEMORY[];
  extern char M_NO_OUTFILE[];
  extern char M_NO_PICK[];
  extern char M_NO_PROG[];
  extern char M_NO_SAVE_CANCEL[];
  extern char M_NO_SHELL_VAR[];
  extern char M_NO_SIMPLE_FOLD[];
  extern char M_NO_TERM[];
  extern char M_NO_TERMCAP[];
  extern char M_NO_TEXT[];
  extern char M_NO_TOP[];
  extern char M_NO_TOP_AUTO_SAVE[];
  extern char M_NO_UNDEL[];
  extern char M_Q_R_PROMPT[];
  extern char M_READING[];
  extern char M_READ[];
  extern char M_REPLACE[];
  extern char M_SEARCH[];
  extern char M_SHELL_CREATED[];
  extern char M_SH_COMMAND[];
  extern char M_STACK_OVER[];
  extern char M_TABSIZE[];
  extern char M_TOP[];
  extern char M_USE_EXIT[];
  extern char M_VIEW[];
  extern char M_WHICH_FIX[];
  extern char M_WHICH_MODE[];
  extern char M_WIN_CHANGED[];
  extern char M_WRITING[];
  extern char M_WRITTEN[];
  extern char NO;
  extern char YES;
  extern char AB;
  extern char DO_AB;
  extern char DO_REST;
  extern char M_NO_NEXT[];
  extern char M_NO_PREV[];
#endif
/*}}}  */
/*{{{  MISC_C*/
#ifndef MISC_C
extern char *completebase P((char*,char*));
extern char *fileprompt P((char[_POSIX_PATH_MAX+1]));
extern char base_name[];
extern void put_vars P((int));
#endif
/*}}}  */
/*{{{  ORIEDT_C*/
#ifndef ORIEDT_C
  extern void pad_ P((char*, TOKEN, int));
  extern char *spaces P((char*, int));
  extern void copy_line_to_parts P((char*, element**));
  extern void copy_parts_to_parts P((element*, element**));
  extern void proc_dispose P((element*));
  extern int first_char P((char*));
  extern int last_char P((char*));
  extern void trailing_spaces P((char*));
  extern void join_links P((element*, element*));
  extern void move_on P((element**));
  extern int insert_indent_of P((element*));
  extern void close_fold_at P((element*));
  extern int lines_within P((element*));
  extern void proc_new P((element**));
  extern void proc_to_edit_pos P((void));
  extern void proc_from_edit_pos P((void));
  extern void copyin P((char*, element*, bool));
  extern void copyout P((char*, element*));
  extern bool fold_out;
  extern bool fold_in;
  extern element *dispose_head;
  extern element *dispose_tail;
  extern int first_edit_pos, pre_edit_pos;
#endif
/*}}}  */
/*{{{  PROMPT_C*/
#ifndef PROMPT_C
  extern bool help P((bool));
  extern char *readprompt P((char*, char*, int));
  extern bool yes P((char*));
  extern void message P((char*, bool));
  extern void vmessage P((char*));
  extern void err_message P((char*,char*,bool));
  extern void no_message P((void));
#endif
/*}}}  */
/*{{{  SCREEN_C*/
#ifndef SCREEN_C
  extern int full_window P((void));
  extern int gotoxy P((int, int));
  extern int clreol P((void));
  extern int insLine P((void));
  extern int DelLine P((void));
  extern int ClrFullScr P((void));
  extern int standout P((void));
  extern int standend P((void));
  extern void init_terminal P((void));
  extern void reset_terminal P((void));
  extern int get_terminal_capability P((void));
  extern screen_lines;
  extern bool dirty;
  extern int sg;
#endif
/*}}}  */
/*{{{  SIGNALS_C*/
#ifndef SIGNALS_C
extern void exit_origami P((int));
extern void init_signals P((void));
extern void enable_sig_abort P((void));
extern void disable_sig_abort P((void));
extern void got_alarm P((void));
extern void set_alarm P((int));
extern void reset_alarm P((void));
extern void bktoshell P((void));
extern int alarm_time;
extern char general_abort_key;
#endif
/*}}}  */
/*{{{  STRING_C*/
#ifndef STRING_C
  extern char *strrstr P((char*, char*));
  extern void strinsert P((char*, char*, int));
  extern int strpos2 P((char*, char*, int));
  extern char *strsub P((char*, char*, int, int));
  extern int proc_replace P((char*, char*, char*, int));
#endif
/*}}}  */
/*{{{  READFOLDS_C*/
#ifndef READFOLDS_C
  extern buffer buff;
  extern int bfile_no;
  extern bool normal_att P((element*));
  extern void insert_file P((element*, element*, element*, bool, FILE*));
#endif
/*}}}  */
/*{{{  WRITEFOLDS_C*/
#ifndef WRITEF_C
  extern bool write_fold P((element*, element*, FILE*));
  extern void total_save P((FILE*));
#endif
/*}}}  */
 
#undef P
