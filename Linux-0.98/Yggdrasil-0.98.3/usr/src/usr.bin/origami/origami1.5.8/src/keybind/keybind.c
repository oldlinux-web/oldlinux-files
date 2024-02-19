
#define KEYBIND_C

/*{{{  #includes*/
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
 
#include <local/bool.h>
#include <local/argparse.h>

#include "../common/keys.h"
#include "keybind.h"
/*}}}  */
 
/*{{{  variables*/
char filebase[60];
char base_name[]=".origami";
bool sorting=FALSE,
     usage=FALSE,
     verbose=FALSE,
     int_no,
     list_keys=FALSE;
int keynumber;
int t_counter=0,
    k_counter=0,
    m_counter=0;
FILE *rc,*bind,*in,*popen();
/*}}}  */

/*{{{  completebase*/
#ifdef STD_C
char *completebase(char *s, char *t)
#else
char *completebase(s,t) char *s, *t;
#endif
{
  char *h=getenv("HOME");
 
  if (h==NULL) *s='\0';
  else
  {
    strcpy(s,h);
    strcat(s,PATH_SEP);
  }
  strcat(s,filebase);
  strcat(s,t);
  return(s);
}
/*}}}  */
 
/*{{{  display usage*/
#ifdef STD_C
void do_usage(void)
#else
void do_usage()
#endif
{
  fprintf(stderr,"Usage: keybind [-o<namebase>] [-hslv?] binding-file\n");
  fprintf(stderr,"       keybind -k<key number>\n");
}
/*}}}  */
/*{{{  write procedures*/
/*{{{  write mark-type to rc*/
#ifdef STD_C
void write_mark_rc(char *name, char marks[4][tag_length-1])
#else
void write_mark_rc(name,marks)
char *name; char marks[4][tag_length-1];
#endif
{
  int i,j;
  char *n=name;
 
  putc(RC_MARKS,rc);
  for (i=1;i<mark_name_lg;i++)
    if (*n!='\0') putc(*n++,rc); else putc('\0',rc);
  for (i=0;i<4;i++)
    for (j=0;j<(tag_length-1);j++)
      putc(marks[i][j],rc);
  m_counter++;
  if (verbose) fprintf(stderr,F_MDECLARE,name);
}
/*}}}  */
/*{{{  write break*/
#ifdef STD_C
void write_break_rc(char a)
#else
write_break_rc(a) char a;
#endif
{
  putc(RC_BREAK,rc);
  putc(a,rc);
  if (verbose)
    if (a<' ')
      fprintf(stderr,"ABORT: C-%c.\n",a+'@');
    else
      fprintf(stderr,"ABORT: %c.\n",a);
}
/*}}}  */
/*{{{  write automacro*/
#ifdef STD_C
void write_auto_rc(int x)
#else
void write_auto_rc(x)
int x;
#endif
{
  putc(RC_AUTOM,rc);
  putc(x,rc);
  if (verbose) fprintf(stderr,"automacro uses buffer %d.\n",x);
}
/*}}}  */
/*{{{  write binding_name to rc file*/
#ifdef STD_C
bool write_name_rc(char *name)
#else
bool write_name_rc(name) char *name;
#endif
{
  int i=keyn_lg;
 
  if (*name=='\0') {
    fprintf(stderr,M_NOBINDNAME);
    return(TRUE);
  }
  putc(RC_KEYNAME,rc);
  while (i--) {putc(*name<' ' ? '\0' : *name,rc);name++;}
  return(FALSE);
}
/*}}}  */
/*{{{  write one macro definition to rc file*/
#ifdef STD_C
bool write_macro_rc(readtags tag, TOKEN op, int lg, TOKEN *code)
#else
bool write_macro_rc(tag,op,lg,code) readtags tag; TOKEN op, *code; int lg;
#endif
{
  if (!(tag==RC_DEFMACRO || tag==RC_INITMACRO)) {
    fprintf(stderr,M_NOMACROTAG);
    return(TRUE);
  }
  /*{{{  verbose-mode*/
  if (verbose) {
    if (tag==RC_DEFMACRO)
      fprintf(stderr,F_STMAC,op,lg);
    else
      fprintf(stderr,F_DEMAC,op,lg);
    t_counter += lg;
  }
  /*}}}  */
  if (lg==0) return(FALSE);
  putc(tag,rc);
  putw(op,rc);
  putw(lg,rc);
  while (lg--) putw(*code++,rc);
  return(FALSE);
}
/*}}}  */
/*{{{  write one definition to bind file*/
#ifdef STD_C
void write_bind(char *name,char *code)
#else
write_bind(name,code) char *name,*code;
#endif
{
#  ifdef TOS
    if(strstr(code,"mmeta")==NULL) fprintf(bind,"%-35s %s\n",name,code);
#  else
    fprintf(bind,"%-35.*s%s\n",strlen(name),name,code);
#  endif
}
/*}}}  */
/*}}}  */
 
/*{{{  argument parser table*/
#define argnum 7

ARG argtab[] =
{
  'k', INT_ARG, &keynumber,
  'o', STRING_ARG, filebase,
  's', BOOL_ARG, &sorting,
  '?', BOOL_ARG, &usage,
  'h', BOOL_ARG, &usage,
  'v', BOOL_ARG, &verbose,
  'l', BOOL_ARG, &list_keys,
};
/*}}}  */
 
/*{{{  main*/
#ifdef STD_C
void main(int argc,char **argv)
#else
main(argc,argv) int argc; char **argv;
#endif
{
  char *home=getenv("HOME"),
       rcfile[128],
       bindfile[128];
  bool ok;
 
  /*{{{  init*/
#  ifdef TOS
    if (home == NULL) home = ".";
#  endif
  
  *filebase='\0';
  init_alias();
  if (init_keytab()==NULL) {
    printf(M_NOMEMORY);
    exit(1);
  }
  /*}}}  */
  argc=argparse(argc,argv,argtab,argnum);
  /*{{{  handle missing output argument*/
  if (*filebase=='\0')
#    ifdef TOS
      strcpy(filebase,"origami.");
#    else
      strcpy(filebase,".origami");
#    endif
  /*}}}  */
  /*{{{  handle -? for usage*/
  if (usage) {
    do_usage();
    exit(0);
  }
  /*}}}  */
  /*{{{  handle -k<keynumber>*/
  if (keynumber) {
    KEYNAME *seek = bindings;
  
    while (seek->num != O_NOP)
      if ((seek->num)==(TOKEN) keynumber) {
        fprintf(stderr,"%d = %s\n",keynumber,seek->name);
        break;
      } else
        seek++;
    if (seek->num == O_NOP) {
      fprintf(stderr,F_NOKEYNUMBER,keynumber);
      exit(1);
    }
    exit(0);
  }
  /*}}}  */
  /*{{{  handle -l*/
  if (list_keys) {
    sc_list_keys();
    exit(0);
  }
  /*}}}  */
  /*{{{  open files/pipe*/
  /*{{{  open input-file*/
  if (argc<=1) {
    do_usage();
    exit(1);
  }
  if ((in=fopen(argv[1],READ))==NULL) {
    fprintf(stderr,M_NOBFILE);
    exit(1);
  }
  source=argv[1];
  read_in();
  /*}}}  */
  /*{{{  open .origamirc*/
  if ((rc=fopen(completebase(rcfile,"rc"),WRITE_BIN))==NULL) {
    fprintf(stderr,F_NORC,home,filebase);
    exit(1);
  }
  /*}}}  */
  /*{{{  open .origamibind*/
  /*{{{  if no sorting open file*/
  if (!sorting) {
    if ((bind=fopen(completebase(bindfile,"bind"),WRITE))==NULL) {
      fprintf(stderr,F_NOBIND,home,filebase);
      exit(1);
    }
  /*}}}  */
  /*{{{  if sorting open pipe*/
  } else {
    char x[128];
  
    sprintf(bindfile,"sort -o %s",completebase(x,"bind"));
    if ((bind=popen(bindfile,WRITE))==NULL) {
      fprintf(stderr,F_NOPIPE,bindfile);
      exit(1);
    }
  }
  /*}}}  */
  /*}}}  */
  /*}}}  */
  if (!(ok=process_file())) {
    /*{{{  do rest*/
    /*{{{  put #vars*/
    if (int_no) {
      putc(RC_INTS,rc);
      putw(int_no,rc);
    }
    /*}}}  */
    write_keynodes();
    putc(RC_ENDE,rc);
    /*}}}  */
  }
  /*{{{  close all files*/
  if (in!=NULL) fclose(in);
  fclose(rc);
  if (!sorting)
    fclose(bind);
  else
    pclose(bind);
  /*}}}  */
  /*{{{  maybe message*/
  if (ok) {
    fprintf(stderr,M_INCORRECTFILES);
    exit(1);
  } else if (verbose) {
    fprintf(stderr,F_INTS,int_no);
    fprintf(stderr,F_BINDS,t_counter);
    fprintf(stderr,F_SEQ,k_counter);
    fprintf(stderr,F_MARKT,m_counter);
  }
  /*}}}  */
  exit(0);
}
/*}}}  */
