/*{{{  #includes*/
#include <sys/types.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <local/bool.h>

#define FOLDFILING_C

#include "origami.h"
#include "macros.h"
/*}}}  */
 
/*{{{  delay_auto_write*/
#ifdef STD_C
void delay_auto_write(void)
#else
void delay_auto_write()
#endif
{
  if (auto_save) set_alarm(0);
}
/*}}}  */
/*{{{  proc_language*/
#ifdef STD_C
void proc_language(void)
#else
void proc_language()
#endif
{
  int  i;
  char lang_prompt[PROMPT_LEN+1];
  char command_str[PROMPT_LEN+1];
 
  *lang_prompt = '\0';
  for (i = 0; i <= NO_LANGUAGES; i++) {
    strcat(lang_prompt, language_string[i]);
    strcat(lang_prompt, " ");
  }
  readprompt(command_str, lang_prompt, 1);
  if (command_str[0] != '\0')
    for (i = 0; i <= NO_LANGUAGES; i++)
      if (toupper(command_str[0]) == *language_string[i]) {
        dialect = i;
        break;
      }
  if (dialect == NO_LANGUAGES - 1) {
    readprompt(dialect_start[dialect],M_CO_START, 3);
    readprompt(dialect_end[dialect],M_CO_END, 3);
  }
  title_op(SHOW);
}
/*}}}  */
/*{{{  file_fold*/
#ifdef STD_C
void file_fold(void)
#else
void file_fold()
#endif
{
  bool can_open;
  int olddia;
  char newfilename[_POSIX_PATH_MAX+1];
  char prompt[PROMPT_LEN+1];
 
  if (filed_or_fold(current)) {
    if (normal_att(current)) {
      if (current->foldline == START_FOLD) {
        /*{{{  file*/
        FILE *fp;
        char *s;
        
        check_fold(current);
        copy_parts_to_line(Start_of_fold, newfilename);
        /*{{{  get filename to newfilename*/
        s=newfilename;
        while (*s && *s!=' ') s++;
        *s='\0';
        /*}}}  */
        if (access(newfilename, F_OK|R_OK|W_OK)==0) {
          sprintf(prompt,F_TCFF,"toggle-create-fild-fold",newfilename);
          can_open = yes(prompt);
        } else if ((fp=fopen(newfilename,WRITE))!=NULL) {
          fclose(fp);
          can_open=TRUE;
        } else
          can_open=FALSE;
        if (can_open) {
          copy_line_to_parts(newfilename, &End_of_fold);
          write_fold(End_of_fold, Start_of_fold, NULL);
          if (Start_of_fold->fold != End_of_fold)
          delete_list(Start_of_fold->fold, End_of_fold->prec);
          Start_of_fold->fold = End_of_fold;
          End_of_fold->prec = Start_of_fold;
          Start_of_fold->foldline = START_FILED;
        } else {
          sprintf(prompt,F_NO_OPEN, newfilename);
          message(prompt,TRUE);
        }
        /*}}}  */
      } else {
        /*{{{  unfile*/
        olddia=dialect;
        pre_open_fold(current);
        if (Start_of_fold->next == End_of_fold)
        insert_file(End_of_fold, Start_of_fold, End_of_fold, FALSE, NULL);
        close_fold_at(current);
        dialect=olddia;
        /*}}}  */
      }
      write_dsp_line(current, cursor_level);
    }
  }
}
/*}}}  */
/*{{{  attach_file*/
#ifdef STD_C
void attach_file(void)
#else
void attach_file()
#endif
{
  char filename[_POSIX_PATH_MAX+1];
  char prompt[_POSIX_PATH_MAX+20];
 
  if (current->foldline == START_FOLD) {
    if (normal_att(current)) {
      check_fold(current);
      if (Start_of_fold->fold == End_of_fold) {
        copy_parts_to_line(Start_of_fold, filename);
        if (access(filename,R_OK)==0) {
          copy_line_to_parts(filename, &End_of_fold);
          Start_of_fold->foldline = START_FILED;
          write_dsp_line(current, cursor_level);
        } else {
          sprintf(prompt, F_F_FAILED, filename);
          message(prompt,TRUE);
        }
      }
    }
  }
}
/*}}}  */
/*{{{  filter_buffer*/
#ifdef STD_C
void filter_buffer(void)
#else
void filter_buffer()
#endif
{
  /*{{{  variables*/
  char filename[_POSIX_PATH_MAX+1];
  char filtername[_POSIX_PATH_MAX+1];
  char pipename[_POSIX_PATH_MAX+1];
  FILE *pipeout,*pipein;
  int line;
  /*}}}  */
 
  /*{{{  errors*/
  if (dialect==NO_LANGUAGES) {
    message(M_NOT_IF_TDS,TRUE);
    return;
  }
  /*}}}  */
  /*{{{  prepare filtering*/
  while (tail != real_tail) {exit_fold();message(M_EX_NFF,TRUE);}
  line=get_line_no();
  readprompt(filtername,"filter-buffer",PROMPT_LEN);
  if (*filtername=='\0' || aborted) return;
  /*}}}  */
  /*{{{  write to pipe*/
  line_of(filename,real_tail);
  /*{{{  open pipe for output the file*/
  if ((pipeout=fopen(filename,WRITE))==NULL) {
    err_message(M_NO_OUTFILE,filename,TRUE);
    return;
  }
  /*}}}  */
  message(M_WRITING,TRUE);
  total_save(pipeout);
  /*{{{  close*/
  if (fclose(pipeout)<0) {
    err_message(M_CLOSE_FAILED,filename,TRUE);
    return;
  }
  /*}}}  */
  /*}}}  */
  /*{{{  read from pipe*/
  /*{{{  open the pipe for filtered file*/
  sprintf(pipename,"%s <%s",filtername,filename);
  if ((pipein=popen(pipename,"r"))==NULL) {
    err_message(M_CANTOPEN,"pipe",TRUE);
    return;
  }
  /*}}}  */
  /*{{{  read the filtered file*/
  current=pre_find_element(1);
  close_and_open_folds(current);
  while (current!=real_tail) pre_remove_line(current);
  message(M_READING,TRUE);
  insert_file((element *)NULL,current->prec, current, FALSE, pipein);
  /*}}}  */
  if (pclose(pipein)<0) err_message(M_CLOSE_FAILED,"pipe",TRUE);
  /*{{{  display it at old position*/
  file_changed=TRUE;
  title_op(SET_CHANGE);
  find_element(line,4);
  /*}}}  */
  message(M_READ,TRUE);
  /*}}}  */
  return;
}
/*}}}  */
/*{{{  pipe_from_command*/
#ifdef STD_C
void pipe_from_command(void)
#else
void pipe_from_command()
#endif
{
  /*{{{  variables*/
  char command[_POSIX_PATH_MAX+1];
  FILE *pipein;
  int olddia=dialect;
  /*}}}  */
 
  /*{{{  errors*/
  if (dialect==NO_LANGUAGES) {
    message(M_NOT_IF_TDS,TRUE);
    return;
  }
  /*}}}  */
  /*{{{  open pipe*/
  readprompt(command,"pipe-from-command",PROMPT_LEN);
  if (*command=='\0' || aborted) return;
  if ((pipein=popen(command,"r"))==NULL) {
    err_message(M_CANTOPEN,"pipe",TRUE);
    return;
  }
  /*}}}  */
  message(M_READING,TRUE);
  insert_file((element *)NULL,current->prec, current, FALSE, pipein);
  dialect=olddia;
  if (pclose(pipein)<0)
    err_message(M_CLOSE_FAILED,"pipe",TRUE);
  else
    message(M_READ,TRUE);
}
/*}}}  */
/*{{{  pipe_to_command*/
#ifdef STD_C
void pipe_to_command(void)
#else
void pipe_to_command()
#endif
{
  /*{{{  variables*/
  char command[_POSIX_PATH_MAX+1];
  FILE *pipeout;
  bool ch=file_changed;
  /*}}}  */
 
  /*{{{  errors*/
  if (tail!=real_tail) {
    message(M_NO_TOP,TRUE);
    return;
  } else if (dialect==NO_LANGUAGES) {
    message(M_NOT_IF_TDS,TRUE);
    return;
  }
  /*}}}  */
  /*{{{  open pipe*/
  readprompt(command,"pipe-to-command",PROMPT_LEN);
  if (*command=='\0' || aborted) return;
  if ((pipeout=popen(command,"w"))==NULL) {
    err_message(M_CANTWRITE,"pipe",TRUE);
    return;
  }
  /*}}}  */
  message(M_WRITING,TRUE);
  total_save(pipeout);
  /*{{{  maybe reset cf-Flag*/
  if (ch) {
    file_changed=TRUE;
    title_op(SET_CF);
  }
  /*}}}  */
  if (pclose(pipeout)<0)
    err_message(M_CLOSE_FAILED,"pipe",TRUE);
  else
    message(M_WRITTEN,TRUE);
}
/*}}}  */
/*{{{  do_save_file*/
#ifdef STD_C
void do_save_file(void)
#else
void do_save_file()
#endif
{
  if (!file_changed) {
    message(M_NO_CHANGE,TRUE);
    return;
  }
  while (tail != real_tail) exit_fold();
  total_save(NULL);
}
/*}}}  */
/*{{{  dump_file*/
#ifdef STD_C
char *dump_file(char *name)
#else
char *dump_file(name) char *name;
#endif
{
  if (!file_changed) return name;
  while (tail != real_tail) exit_fold();
  strcpy(name,"oricoreXXXXXX");
  mktemp(name);
  copy_line_to_parts(name, &new_file_ptr);
  fold_out=FALSE;
  write_fold(new_file_ptr, real_head->next, NULL);
  return(name);
}
/*}}}  */
/*{{{  save_file*/
#ifdef STD_C
void save_file(void)
#else
void save_file()
#endif
{
  if (!file_changed) {
    message(M_NO_CHANGE,TRUE);
  } else if (tail == real_tail) {
    total_save(NULL);
    title_op(SHOW);
    delay_auto_write();
  } else
    message(M_NO_TOP,TRUE);
}
/*}}}  */
/*{{{  auto_write*/
#ifdef STD_C
void auto_write(void)
#else
void auto_write()
#endif
{
  if (file_changed && tail == real_tail) {
    total_save(NULL);
    title_op(SHOW);
    message(M_AUTO_SAVED,TRUE);
  } else if (file_changed)
    message(M_NO_TOP_AUTO_SAVE,TRUE);
  do_auto_save=FALSE;
  set_alarm(0);
}
/*}}}  */
/*{{{  open_new_file*/
#ifdef STD_C
void open_new_file(void)
#else
void open_new_file()
#endif
{
  if (file_changed && yes(M_CHANGED_SAVE)) total_save(NULL);
  if (aborted) return;
  delete_list(real_head, real_tail);
  read_list(NULL);
  file_changed=FALSE;
  title_op(RESET_CF);
  delay_auto_write();
  call_auto_macro();
}
/*}}}  */
/*{{{  next_file*/
#ifdef STD_C
void next_file(int of,char *argv[])
#else
void next_file(of,argv) int of;char *argv[];
#endif
{
  f_current += of;
  if (f_current>f_end || f_current<f_begin || f_begin==0 || f_end==0) {
    f_current -= of;
    message(of>0 ? M_NO_NEXT : M_NO_PREV,TRUE);
    return;
  }
  filearg=argv[f_current];
  open_new_file();
}
/*}}}  */
/*{{{  write_file*/
#ifdef STD_C
void write_file(void)
#else
void write_file()
#endif
{
  char newfilename[_POSIX_PATH_MAX+1];
 
  if (tail == real_tail) {
    fileprompt(newfilename);
    if (*newfilename != '\0' && !aborted) {
      copy_line_to_parts(newfilename, &new_file_ptr);
      if (!fold_in && (dialect < NO_LANGUAGES))
        fold_out=yes("listing-mode");
      else
        fold_out=FALSE;
      write_fold(new_file_ptr, real_head->next, NULL);
      fold_out=FALSE;
      title_op(SHOW);
    }
  } else
    message(M_NO_TOP,TRUE);
}
/*}}}  */
