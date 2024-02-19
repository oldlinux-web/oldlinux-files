/*{{{  #includes*/
#include <sys/types.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <local/bool.h>

#define READFOLDS_C
 
#include "origami.h"
#include "macros.h"
 
/*}}}  */
 
/*{{{  P_eof*/
#ifdef STD_C
PRIVATE int P_eof(FILE *f)
#else
PRIVATE int P_eof(f)
FILE *f;
#endif
{
  int ch;

  if (feof(f)) return 1;
  if (f == stdin) return 0;    /* not safe to look-ahead on the keyboard! */
  ch = getc(f);
  if (ch == EOF) return 1;
  ungetc(ch, f);
  return 0;
}
/*}}}  */
/*{{{  normal_att*/
#ifdef STD_C
bool normal_att(element *p)
#else
bool normal_att(p)
element *p;
#endif
{
  return (p->UU.U1.fold_type == 0 && p->UU.U1.fold_contents == 1);
}
/*}}}  */
 
int bfile_no = 0, bfile_last = 0;
buffer buff;   /*should have 1 for each file*/
PRIVATE int the_pos;
 
/*{{{  get_dsp_file*/
#ifdef STD_C
PRIVATE void get_dsp_file(LOC_file *LINK)
#else
PRIVATE void get_dsp_file(LINK) LOC_file *LINK;
#endif
{
  if (filearg) {
    strcpy(LINK->filename, filearg);
    filearg=NULL;
  } else {
    fileprompt(LINK->filename);
    f_begin=f_end=f_current=0;
  }
}
/*}}}  */
/*{{{  create_node*/
#ifdef STD_C
PRIVATE void create_node(LOC_file *LINK)
#else
PRIVATE void create_node(LINK)
LOC_file *LINK;
#endif
{
  element *pt;

  proc_new(&pt);
  copy_line_to_parts(LINK->dsp_line, &pt);
  if (LINK->trailing == NULL)
  pt->UU.U1.indent = LINK->new_indent;
  join_links(LINK->current, pt);
  LINK->current = pt;
}
/*}}}  */
/*{{{  read_b*/
#ifdef STD_C
PRIVATE void read_b(uchar *b, LOC_file *LINK)
#else
PRIVATE void read_b(b, LINK)
uchar *b;
LOC_file *LINK;
#endif
{
  bfile_no++;
  if (bfile_no >= bfile_last) {
    bfile_last = fread((char *)buff, 1, sizeof(uchar) * 512, LINK->tdsfile);
    bfile_no = 0;
  }
  *b = buff[bfile_no];
}
/*}}}  */
/*{{{  read_tag*/
#ifdef STD_C
PRIVATE void read_tag(int *tag, int *value, LOC_file *LINK)
#else
PRIVATE void read_tag(tag, value, LINK)
int *tag, *value;
LOC_file *LINK;
#endif
{
  uchar b;

  *value = 0;
  do {
    read_b(&b, LINK);
    *tag = b >> 6;
    *value = ((*value) << 6) + (b & 63);
  } while (*tag == 3);
}
/*}}}  */
/*{{{  read_string*/
#ifdef STD_C
PRIVATE void read_string(char *s, LOC_file *LINK)
#else
PRIVATE void read_string(s, LINK)
char *s;
LOC_file *LINK;
#endif
{
  int i, tag, value;
  uchar b;

  read_tag(&tag, &value, LINK);
  if (tag != 0) {
    sprintf(s, "%d", value);
    return;
  }
  s[value] = '\0';
  for (i = 0; i < value; i++) {
    read_b(&b, LINK);
    s[i] = b;
  }
}
/*}}}  */
/*{{{  connect_fold*/
#ifdef STD_C
PRIVATE void connect_fold(LOC_file *LINK)
#else
PRIVATE void connect_fold(LINK)
LOC_file *LINK;
#endif
{
  if (LINK->trailing == NULL)
  return;
  LINK->S = LINK->trailing;
  join_links(LINK->current, LINK->S->other_end);
  LINK->current = LINK->S->other_end;
  LINK->current->other_end = LINK->S;
  LINK->trailing = LINK->S->fold;
  LINK->S->fold = LINK->S->next;
  LINK->S->other_end = LINK->current;
  LINK->S->foldline = LINK->current->foldline;   /*start*/
  /* _fold or _filed */
  current_ind -= LINK->S->UU.U1.fold_indent;
  LINK->current->foldline = END_FOLD;
  LINK->current = LINK->S;
}
/*}}}  */
/*{{{  start_a_fold*/
#ifdef STD_C
PRIVATE void start_a_fold(uchar start_, int indent, LOC_file *LINK)
#else
PRIVATE void start_a_fold(start_, indent, LINK)
uchar start_;
int indent;
LOC_file *LINK;
#endif
{
  copy_line_to_parts(LINK->dsp_line, &LINK->current);
  LINK->current->UU.U1.fold_indent = indent;
  current_ind += indent;
  LINK->current->fold = LINK->trailing;
  LINK->trailing = LINK->current;
  proc_new(&LINK->current->other_end);
  LINK->current->other_end->foldline = start_;
}
/*}}}  */
/*{{{  read_a_list*/
#ifdef STD_C
PRIVATE void read_a_list(int indent,LOC_file *LINK)
#else
PRIVATE void read_a_list(indent, LINK)
int indent;
LOC_file *LINK;
#endif /* STD_C */
{
  int i, tag, value;
  uchar b;
  /*{{{  ``forward'' for read_fold*/
#    ifdef STD_C
  PRIVATE void read_fold(int *, bool, LOC_file *);
#    else
  PRIVATE void read_fold();
#    endif
  /*}}}  */
 
  do {
    read_tag(&tag, &value, LINK);
    switch (tag) {
      /*{{{  record*/
      case 0: {
        LINK->dsp_line[value] = '\0';
        for (i = 0; i < value; i++) {
          read_b(&b, LINK);
          LINK->dsp_line[i] = b;
        }
        create_node(LINK);
        break;
      }
      /*}}}  */
      /*{{{  function*/
      case 2: {
      switch (value) {
          /*{{{  startfold*/
          case 2: {
            read_fold(&indent, FALSE, LINK);
            break;
          }
          /*}}}  */
          /*{{{  startfiled*/
          case 4: {
            read_fold(&indent, TRUE, LINK);
            break;
          }
          /*}}}  */
        }
        break;
      }
      /*}}}  */
    }
  } while (tag != 2 || value != 1);
}
/*}}}  */
/*{{{  read_fold*/
#ifdef STD_C
PRIVATE void read_fold(int *indent,bool filed,LOC_file *LINK)
#else
PRIVATE void read_fold(indent, filed, LINK)
int *indent;
bool filed;
LOC_file *LINK;
#endif /* STD_C */
{
  int tag, value, t, c, extra_indent;
  element *WITH;

  read_tag(&tag, &value, LINK);   /*startlist*/
  read_string(LINK->dsp_line, LINK);
  read_tag(&tag, &t, LINK);
  read_tag(&tag, &c, LINK);
  read_tag(&tag, &extra_indent, LINK);
  read_tag(&tag, &value, LINK);   /*should be endlist*/
  create_node(LINK);
  if (filed)
  start_a_fold(START_FILED, extra_indent, LINK);
  else
  start_a_fold(START_FOLD, extra_indent, LINK);
  if (filed) {
    read_string(LINK->dsp_line, LINK);
    copy_line_to_parts(LINK->dsp_line, &LINK->trailing->other_end);
  }
  else
  read_a_list(*indent + extra_indent, LINK);
  read_tag(&tag, &value, LINK);   /*endfold or endfiled*/
  connect_fold(LINK);
  WITH = LINK->current;
  WITH->UU.U1.fold_type = t;
  WITH->UU.U1.fold_contents = c;
}
/*}}}  */
/*{{{  read_tds*/
#ifdef STD_C
PRIVATE bool read_tds(LOC_file *LINK)
#else
PRIVATE bool read_tds(LINK)
LOC_file *LINK;
#endif /* STD_C */
{
  int tag, value;
 
  bfile_no = bfile_last;
  read_tag(&tag, &value, LINK);
  if (tag == 2 && value == 0) {
    dialect = NO_LANGUAGES;
    read_a_list(0, LINK);
    return(TRUE);
  }
  return(FALSE);
}
/*}}}  */

/*{{{  find_dialect*/
#ifdef STD_C
PRIVATE bool find_dialect(char *ss)
#else
PRIVATE bool find_dialect(ss)
char *ss;
#endif /* STD_C */
{
  char s[MAX_FIELD_SIZE + 1];
  int i;
  bool b;
  char STR1[MAX_FIELD_SIZE + 1];

  b = TRUE;
  strcpy(s, ss);
  i = first_char(s);
  if (i > strlen(s)) {
    dialect = 0;
    return b;
  }
  b = FALSE;
  strcpy(s, copy_to_end(STR1, s, i));
  trailing_spaces(s);
  for (i = 1; i < NO_LANGUAGES; i++) {
    if ((strlen(s) == strlen(dialect_start[i])) &&
    (strpos2(dialect_start[i], s, 1) == 1)) {
      dialect = i;
      b = TRUE;
    }
  }
  return b;
}
/*}}}  */
/*{{{  get_the_pos*/
#ifdef STD_C
PRIVATE void get_the_pos(char *str,LOC_file *LINK)
#else
PRIVATE void get_the_pos(str, LINK)
char *str;
LOC_file *LINK;
#endif /* STD_C */
{
  int i;
  char STR1[MAX_FIELD_SIZE + MAX_FIELD_SIZE + 1];
  char STR2[MAX_FIELD_SIZE + 1];

  the_pos = strpos2(LINK->dsp_line, str, 1);
  if (the_pos <= 0) return;
  if (the_pos==1)
    *STR1='\0';
  else
    sprintf(STR1, "%.*s", the_pos - 1, LINK->dsp_line);
  if (!find_dialect(STR1)) {the_pos = 0;return;}
  strcpy(LINK->dsp_line,
  copy_to_end(STR2, LINK->dsp_line, the_pos + strlen(str)));
  i = first_char(LINK->dsp_line);
  if (i <= strlen(LINK->dsp_line))
  strcpy(LINK->dsp_line, copy_to_end(STR2, LINK->dsp_line, i));
  i = strlen(LINK->dsp_line) - strlen(dialect_end[dialect]);
  if (i > 0)
    LINK->dsp_line[i] = '\0';
  else
    LINK->dsp_line[0] = '\0';
  the_pos -= strlen(dialect_start[dialect]);
}
/*}}}  */
/*{{{  tabbed_out*/
#ifdef STD_C
PRIVATE char *tabbed_out(char *Result, char *instr)
#else
PRIVATE char *tabbed_out(Result, instr)
char *Result, *instr;
#endif
{
  uchar j, i;
  uchar FORLIM;
  char STR1[PROMPT_LEN+1];
  PRIVATE int tab_size = 0;

  j = 0;
  FORLIM = strlen(instr);
  for (i = 0; i < FORLIM; i++) {
    if (instr[i] == '\t') {
      while (tab_size <= 0)
      sscanf(readprompt(STR1, M_TABSIZE, 10), "%d", &tab_size);
      do { j++; Result[j - 1] = ' ';} while (j % tab_size != 0);
    } else
      { j++; Result[j - 1] = instr[i];}
  }
  Result[j] = '\0';
  return Result;
}
/*}}}  */
 
/*{{{  insert_file*/
#ifdef STD_C
void insert_file(element *file_ptr,element *prec_ptr,
                 element *next_ptr,bool remember_file,
                 FILE *pipe)
#else
void insert_file(file_ptr, prec_ptr, next_ptr, remember_file,pipe)
element *file_ptr, *prec_ptr, *next_ptr;
bool remember_file;
FILE *pipe;
#endif /* STD_C */
{
  /*{{{  local variables*/
  LOC_file V;
  bool add_c;
  int marker_pos, i, j;
  char STR1[_POSIX_PATH_MAX+11];
  char STR2[MAX_FIELD_SIZE+MAX_FIELD_SIZE+1];
  char *TEMP;
  char STR3[MAX_FIELD_SIZE + 1];
  char STR4[6];
  bool lang_def=TRUE;
  char fold_type_str[tag_length+tag_length];
  char fold_c_open_str[tag_length+1];
  char name_file_str[tag_length+1];
  char name_att_str[tag_length+1];
  char fold_c_line_str[tag_length+1];
  char fold_f_line_str[tag_length+1];
  FILE *tstorefile;
  char tstorefile_NAME[_POSIX_PATH_MAX+1];
  /*}}}  */
  /*{{{  prepare the folding-marks*/
  strcpy(fold_type_str,fold_open_str);
  strcat(fold_type_str,fold_close_str);
  strcpy(fold_c_open_str,"C");
  strcat(fold_c_open_str,fold_open_str);
  strcpy(name_file_str,fold_file_str);
  strcat(name_file_str,"F");
  strcpy(name_att_str,fold_file_str);
  strcat(name_att_str,"A");
  strcpy(fold_f_line_str,fold_line_str);
  strcat(fold_f_line_str,"F");
  /*}}}  */
 
  V.tdsfile = NULL;
  V.current = prec_ptr;
  V.new_indent = insert_indent_of(next_ptr);
  /*{{{  get and check filename*/
  if (pipe==NULL) {
    if (file_ptr == NULL)
      get_dsp_file(&V);
    else
      line_of(V.filename, file_ptr);
    if (*V.filename == '\0' && remember_file)
      exit_origami(0);
    if (access(V.filename,F_OK)==0 && access(V.filename,W_OK) == -1)
      browse_mode=TRUE;
  }
  /*}}}  */
  title_op(browse_mode ? SET_VIEW : RESET_VIEW);
  strcpy(V.tdsfile_NAME, V.filename);
  /*{{{  open tds to check, if the file is in tds-format*/
  if (pipe==NULL) {
    V.tdsfile = fopen(V.tdsfile_NAME,READ_BIN);
    if (V.tdsfile!=NULL) {
      sprintf(STR1, F_LOADING, V.filename);
      message(STR1,TRUE);
    } else
      err_message(M_CANTOPEN,V.filename,TRUE);
  } else
    V.tdsfile=NULL;
  /*}}}  */
  V.trailing = NULL;
  current_ind = 0;
  if (pipe!=NULL || (V.tdsfile!=NULL && !read_tds(&V))) {
    /*{{{  read ascii-file*/
    /*{{{  open file tstorefile*/
    if (pipe==NULL) {
      strcpy(tstorefile_NAME, V.filename);
      if ((tstorefile = fopen(tstorefile_NAME,READ))==NULL) {
        err_message(M_CANTOPEN,V.filename,TRUE);
      }
    } else
      tstorefile=pipe;
    /*}}}  */
    while (!P_eof(tstorefile)) {
      /*{{{  handle one line*/
      fgets(V.dsp_line, MAX_FIELD_SIZE + 1 - V.new_indent, tstorefile);
      TEMP = strchr(V.dsp_line, '\n');
      if (TEMP != NULL) *TEMP = 0;
      /*{{{  define a language??*/
      if (!fold_in && lang_def) {
        marker_pos = strpos2(V.dsp_line, fold_type_str, 1);
        if (marker_pos > 0) {
          trailing_spaces(V.dsp_line);
          dialect = NO_LANGUAGES - 1;
          sprintf(STR2, "%.*s", marker_pos - 1, V.dsp_line);
          strcpy(dialect_start[dialect], STR2);
          strcpy(dialect_end[dialect],
          strsub(STR2, V.dsp_line, marker_pos + 6, strlen(V.dsp_line)));
          fgets(V.dsp_line, MAX_FIELD_SIZE + 1 - V.new_indent, tstorefile);
          TEMP = strchr(V.dsp_line, '\n');
          if (TEMP != NULL) *TEMP = 0;
        }
      }
      lang_def=FALSE;
      /*}}}  */
      trailing_spaces(V.dsp_line);
      strcpy(V.dsp_line, tabbed_out(STR3, V.dsp_line));
      if (strlen(V.dsp_line) < current_ind) pad_(V.dsp_line, ' ', current_ind);
      /*{{{  comment-correction (c_hash, FORTRAN, ... )*/
      /*{{{  C... -> maybe fortran line handling (@)*/
      add_c = FALSE;
      if (toupper(V.dsp_line[0]) == 'C') {
        i = strpos2(V.dsp_line, fold_c_open_str, 1);
        j = strpos2(V.dsp_line, fold_c_line_str, 1);
        if (dialect == fortran_ && current_ind > 0 || i > 1 || j > 1)
          V.dsp_line[0] = ' ';
        if (dialect == fortran_ && i == 0 && j == 0)
          add_c = TRUE;
      }
      /*}}}  */
      /*{{{  c_hash-mode??*/
      if (c_hash_front && V.dsp_line[0]=='#') {
        char *s=V.dsp_line;
      
        *s=' ';
        while (*(++s) == ' ') ;
        *(s-1)='#';
      }
      /*}}}  */
      /*{{{  correct indentation!*/
      i = first_char(V.dsp_line);
      if (i <= current_ind)
        strcpy(V.dsp_line, copy_to_end(STR3, V.dsp_line, i));
      else
        strcpy(V.dsp_line, copy_to_end(STR3, V.dsp_line, current_ind + 1));
      /*}}}  */
      /*{{{  C add for fortran (deleted in @)*/
      if (add_c) {
        if (*V.dsp_line == '\0')
          strcpy(V.dsp_line, "C");
        else if (toupper(V.dsp_line[0]) != 'C')
          strinsert("C ", V.dsp_line, 1);
      }
      /*}}}  */
      /*}}}  */
      if (!fold_in) {
        /*{{{  if attached file name*/
        get_the_pos(name_att_str, &V);
        if (the_pos > 0) {  /*read and save attributes*/
          if (V.trailing != NULL) {
            the_pos = strpos2(V.dsp_line, " ", 1);
            if (the_pos==1)
              *STR2='\0';
            else
              sprintf(STR2, "%.*s", the_pos - 1, V.dsp_line);
            i = (sscanf(STR2, "%d", &V.trailing->UU.U1.fold_type) == 0);
            i = (sscanf(copy_to_end(STR3, V.dsp_line, the_pos), "%d",
            &V.trailing->UU.U1.fold_contents) == 0);
          }
          continue;
        }
        /*}}}  */
        /*{{{  if filed file name*/
        get_the_pos(name_file_str, &V);
        if (the_pos > 0) {  /*save filename*/
          if (V.trailing != NULL) {
            copy_line_to_parts(V.dsp_line, &V.trailing->other_end);
            if (V.trailing->UU.U1.fold_type != 0)
            connect_fold(&V);
          }
          continue;
        }
        /*}}}  */
        /*{{{  if close fold*/
        get_the_pos(fold_close_str, &V);
        if (the_pos > 0) {
          connect_fold(&V);
          continue;
        }
        /*}}}  */
      }
      create_node(&V);
      if (!fold_in) {
        sprintf(STR4, "%sF", fold_open_str);
        get_the_pos(STR4, &V);
        if (the_pos == 0) get_the_pos(fold_f_line_str, &V);
        if (the_pos > 0)
          start_a_fold(START_FILED, the_pos - 1, &V);
        else {
          get_the_pos(fold_open_str, &V);
          if (the_pos > 0) start_a_fold(START_FOLD, the_pos - 1, &V);
        }
      }
      /*}}}  */
    }
    if (pipe == NULL && tstorefile!=NULL)
      if (fclose(tstorefile)<0) err_message(M_CLOSE_FAILED,V.filename,TRUE);
    /*}}}  */
  }
  /*{{{  filename must maybe moved to internal data*/
  if (remember_file) {
    if (file_ptr == NULL) copy_line_to_parts(V.filename, &prec_ptr);
    copy_line_to_parts(V.filename, &next_ptr);
  }
  /*}}}  */
  join_links(V.current, next_ptr);
  if (V.tdsfile != NULL)
    if (fclose(V.tdsfile)<0) err_message(M_CLOSE_FAILED,V.filename,TRUE);
  V.tdsfile=NULL;
}
/*}}}  */
