/*{{{  #includes*/
#include <sys/types.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <local/bool.h>

#define WRITEFOLDS_C

#include "origami.h"
#include "macros.h"
/*}}}  */

/*{{{  write_b*/
#ifdef STD_C
PRIVATE void write_b(uchar b,LOC_proc *LINK)
#else
PRIVATE void write_b(b, LINK)
uchar b;
LOC_proc *LINK;
#endif /* STD_C */
{
  buff[bfile_no] = b;
  bfile_no++;
  if (bfile_no != 512)
  return;
  bfile_no = fwrite((char *)buff, 1, sizeof(uchar) * 512, LINK->tdsfile);
  bfile_no = 0;
}
/*}}}  */
/*{{{  write_tag*/
#ifdef STD_C
PRIVATE void write_tag(int tag,int value,LOC_proc *LINK)
#else
PRIVATE void write_tag(tag, value, LINK)
int tag, value;
LOC_proc *LINK;
#endif /* STD_C */
{
  if (value > 63)
  write_b((uchar)(((unsigned)value) >> 6) + 192, LINK);
  write_b((uchar)((tag << 6) + (value & 63)), LINK);
}
/*}}}  */
/*{{{  write_string*/
#ifdef STD_C
PRIVATE void write_string(char *s,LOC_proc *LINK)
#else
PRIVATE void write_string(s, LINK)
char *s;
LOC_proc *LINK;
#endif /* STD_C */
{
  int i, FORLIM;

  write_tag(0, strlen(s), LINK);
  FORLIM = strlen(s);
  for (i = 0; i < FORLIM; i++)
  write_b((uchar)s[i], LINK);
}
/*}}}  */
/*{{{  write_header*/
#ifdef STD_C
PRIVATE void write_header(LOC_proc *LINK)
#else
PRIVATE void write_header(LINK)
LOC_proc *LINK;
#endif /* STD_C */
{
  write_tag(2, 0, LINK);   /*startlist*/
  write_string(LINK->dsp_line, LINK);
  write_tag(1, (int)LINK->p->UU.U1.fold_type, LINK);
  write_tag(1, (int)LINK->p->UU.U1.fold_contents, LINK);
  write_tag(1, (int)LINK->p->UU.U1.fold_indent, LINK);
  write_tag(2, 1, LINK);   /*endlist*/
}
/*}}}  */
/*{{{  move_on_full*/
#ifdef STD_C
PRIVATE void move_on_full(element **p)
#else
PRIVATE void move_on_full(p)
element **p;
#endif /* STD_C */
{
  if (filed_or_fold(*p))
  *p = (*p)->fold;
  else
  *p = (*p)->next;
}
/*}}}  */
/*{{{  write_a_list*/
#ifdef STD_C
PRIVATE void write_a_list(LOC_proc *LINK)
#else
PRIVATE void write_a_list(LINK)
LOC_proc *LINK;
#endif /* STD_C */
{
  char STR1[MAX_FIELD_SIZE + 1];

  write_tag(2, 0, LINK);
  while (LINK->pn->foldline != END_FOLD) {
    LINK->p = LINK->pn;
    move_on_full(&LINK->pn);
    copy_parts_to_line(LINK->p, LINK->dsp_line);
    trailing_spaces(LINK->dsp_line);
    if (LINK->p->foldline == NOT_FOLD) write_string(LINK->dsp_line, LINK);
    if (LINK->p->foldline == START_FOLD || LINK->p->foldline == START_OPEN_FOLD) {
      write_tag(2, 2, LINK);
      write_header(LINK);
      write_a_list(LINK);
      write_tag(2, 3, LINK);   /*endfold*/
      continue;
    }
    if (LINK->p->foldline != START_FILED) continue;
    write_tag(2, 4, LINK);
    write_header(LINK);
    write_string((char *)line_of(STR1, LINK->p->other_end), LINK);
    write_tag(2, 5, LINK);   /*endfiled*/
    move_on_full(&LINK->pn);
  }
  write_tag(2, 1, LINK);
  if (LINK->pn->foldline == END_FOLD)
  move_on_full(&LINK->pn);
}
/*}}}  */
/*{{{  write_tds*/
#ifdef STD_C
PRIVATE bool write_tds(LOC_proc *LINK)
#else
PRIVATE bool write_tds(LINK)
LOC_proc *LINK;
#endif /* STD_C */
{
  char STR1[MAX_FIELD_SIZE + 1];
 
  strcpy(LINK->tdsfile_NAME, line_of(STR1, LINK->file_ptr));
  LINK->tdsfile = fopen(LINK->tdsfile_NAME,WRITE_BIN);
  if (LINK->tdsfile == NULL) {
    err_message(M_CANTWRITE,LINK->tdsfile_NAME,TRUE);
    return(FALSE);
  }
  bfile_no = 0;
  write_a_list(LINK);
  if (bfile_no > 0) bfile_no = fwrite((char *)buff, 1, bfile_no, LINK->tdsfile);
  if (fclose(LINK->tdsfile)<0) {
    err_message(M_CLOSE_FAILED,LINK->tdsfile_NAME,TRUE);
    return(FALSE);
  }
  sprintf(STR1,F_WRITTEN,LINK->tdsfile_NAME);
  message(STR1,TRUE);
  LINK->tdsfile = NULL;
  return(TRUE);
}
/*}}}  */
/*{{{  write_fold*/
#ifdef STD_C
bool write_fold(element *file_ptr_,element *from_ptr, FILE *pipe)
#else
bool write_fold(file_ptr_, from_ptr, pipe)
element *file_ptr_, *from_ptr;
FILE *pipe;
#endif /* STD_C */
{
  /*{{{  variables*/
  LOC_proc V;
  char filename[_POSIX_PATH_MAX+1];
  char STR1[PROMPT_LEN+1];
  char STR2[MAX_FIELD_SIZE + 1];
  char STR3[MAX_FIELD_SIZE + 1];
  FILE *tstorefile;
  char tstorefile_NAME[_POSIX_PATH_MAX+1];
  /*}}}  */
 
  V.file_ptr = file_ptr_;
  V.tdsfile = NULL;
  V.pn = from_ptr;
  move_on(&V.pn);
  current_ind = 0;
  line_of(filename, V.file_ptr);
  /*{{{  if no pipe, show writing*/
  if (pipe==NULL) {
    sprintf(STR1, F_WRITING, filename);
    message(STR1,TRUE);
  }
  /*}}}  */
  if (dialect < NO_LANGUAGES) {   /*ie not tds*/
    /*{{{  ascii-types*/
    /*{{{  open tstorefile*/
    if (pipe!=NULL)
      tstorefile=pipe;
    else
      tstorefile = fopen(filename,WRITE);
    if (tstorefile==NULL) {
      err_message(M_CANTWRITE, filename,TRUE);
      return(FALSE);
    }
    /*}}}  */
    strcpy(tstorefile_NAME, filename);
    /*{{{  maybe print language-mark-line*/
    if ((dialect == NO_LANGUAGES - 1) && !fold_out) {
      fprintf(tstorefile, "%s%s%s%s\n",
      dialect_start[dialect],
      fold_open_str,
      fold_close_str,
      dialect_end[dialect]);
    }
    /*}}}  */
    while (V.pn != from_ptr->other_end) {   /*tail*/
      /*{{{  handle one line*/
      V.p = V.pn;
      copyin(V.dsp_line, V.p, TRUE);
      trailing_spaces(V.dsp_line);
      if (current_ind > 0)
       strinsert(spaces(STR2, current_ind), V.dsp_line, 1);
      if (dialect == fortran_ && (V.p->foldline != NOT_FOLD ||
                      strpos2(line_of(STR3, V.p), "C ", 1) == 1))
        V.dsp_line[0] = 'C';
      /*{{{  maybe move #*/
      if (c_hash_front) {
        char *s=V.dsp_line;
      
        while (*(s++)==' ') ;
        if (*(--s)=='#') {*s=' ';*V.dsp_line='#';}
      }
      /*}}}  */
      fprintf(tstorefile, "%s\n", V.dsp_line);
      if (filed_or_fold(V.p)) current_ind += total_indent(V.p);
      if (filed_or_fold(V.p) || V.p->foldline == START_OPEN_FOLD) {
        if (dialect >= 0) {
          /*{{{  print correct fold-file-string*/
          sprintf(V.dsp_line, "%s%s%s",
                  spaces(STR2, current_ind),
                  dialect_start[dialect],
                  fold_file_str);
          if (dialect == fortran_) V.dsp_line[0] = 'C';
          /*}}}  */
          /*{{{  !normal_att*/
          if (!normal_att(V.p)) {
            fprintf(tstorefile, "%sA %2d %d%s\n",
            V.dsp_line, V.p->UU.U1.fold_type,
            V.p->UU.U1.fold_contents,
            dialect_end[dialect]);
          }
          /*}}}  */
          /*{{{  START_FILED*/
          if (V.p->foldline == START_FILED) {
            fprintf(tstorefile, "%sF %s%s\n",
            V.dsp_line, line_of(STR3, V.p->other_end),
            dialect_end[dialect]);
          }
          /*}}}  */
        }
        if (V.p->UU.U1.fold_type == 1 && V.p->foldline == START_FILED) {
          current_ind -= total_indent(V.p);
          V.pn = V.p->other_end;
        }
      }
      move_on_full(&V.pn);
      if (V.p->foldline == END_FOLD) {
        if (filed_or_fold(V.p->other_end))
        current_ind -= total_indent(V.p->other_end);
      }
      /*}}}  */
    }
    /*{{{  maybe close tstorefile*/
    if (pipe==NULL) {
      if (fclose(tstorefile)<0) {
        err_message(M_CLOSE_FAILED,filename,FALSE);
        return(FALSE);
      }
      sprintf(STR1, F_WRITTEN, filename);
      message(STR1,TRUE);
      tstorefile=NULL;
    }
    return(TRUE);
    /*}}}  */
    /*}}}  */
  } else
    return(write_tds(&V));
}
/*}}}  */
/*{{{  total_save*/
#ifdef STD_C
void total_save(FILE *pipe)
#else
void total_save(pipe) FILE *pipe;
#endif
{
  while (tail != real_tail) pre_exit_fold();
  if ((file_changed && !browse_mode) || pipe!=NULL ) {
    if (write_fold(real_tail, real_head->next,pipe) || pipe==NULL)
      file_changed=FALSE;
    title_op(RESET_CHANGE);
  }
}
/*}}}  */
