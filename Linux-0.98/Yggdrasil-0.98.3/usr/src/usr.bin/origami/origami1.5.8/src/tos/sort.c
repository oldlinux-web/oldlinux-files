/*{{{  about this file*/
/*
   SORT.C - sort data from stdin to stdout
   this is part of the ORIGAMI ST distribution.
   KEYBIND sorts the output by piping it through 'sort'. As
   most shells do not come with a sort utility, I had to write this
   one.
   File size is limited to 10000 lines.
 
   It compiles well under Turbo C, but it should compile with nearly every
   ANSI compiler. The qsort() library function is required.

   You may distribute this freely in source or binary form. This program is
   distributed as-is and without any warranty, expressed or implied.

   Michael Schwingen
   Ahornstrasse 36
   W-5100 Aachen
*/
/*}}}  */
/*{{{  includes*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
/*}}}  */
/*{{{  defines*/
#define TRUE 1
#define FALSE 0
#define MAX_LINELEN 1000
#define MAX_LINES 10000
/*}}}  */

/*{{{  prototypes*/
int getline(char *line,int maxlen,FILE *fp);
long read_file(char **tab, long max, FILE *fp);
void write_file(char **tab, long max, FILE *fp);
int compare(char **elem1, char **elem2);
/*}}}  */

char name[256];  /* the filename */
char **buftab;   /* pointer to pointers to the text lines */

/*{{{  int main(int argc, char *argv[])*/
int main(int argc, char *argv[])
{
  long numlines;
  FILE *outfile = stdout;
 
  /*{{{  handle "-o filename", open file*/
  if (argc == 3 && strcmp(argv[1],"-o") == 0)
  {
    outfile = fopen(argv[2],"w+");
    if (outfile == NULL)
    {
      fprintf(stderr,"sort: error: cannout open file %s for output\n",argv[2]);
      exit(1);
    }
  }
  /*}}}  */
  /*{{{  allocate enough memory for pointer buffer*/
  buftab = (char **) malloc(sizeof(char *) * MAX_LINES);
  if(buftab==0)
  {
    fprintf(stderr,"sort: error: not enough memory !");
    exit(1);
  }
  /*}}}  */

  numlines = read_file(buftab,MAX_LINES,stdin);

  qsort(buftab,numlines,sizeof(char *),compare);

  write_file(buftab,numlines,outfile);
  /*{{{  if not stdout: close output file*/
    if (outfile != stdout)
      fclose(outfile);
  /*}}}  */
  return 0;
}
/*}}}  */

/*{{{  int getline(char *line,int maxlen,FILE *fp)*/
int getline(char *line,int maxlen,FILE *fp)
{
  register unsigned char flag;
  while( --maxlen && (*line=getc(fp))!= EOF && *line!='\n')
    if(*line!='\015' && *line!='\012') line++;
  flag=(*line!=EOF);
  *line=0;
  return flag;
}
/*}}}  */

/*{{{  long read_file(char **tab, long max, FILE *fp)*/
long read_file(char **tab, long max, FILE *fp)
{
  char line[MAX_LINELEN];
  long linenr=0;

  while(--max && getline(line,MAX_LINELEN-1,fp))
  {
    tab[linenr] = strdup(line);
    if(tab[linenr++] == NULL)
    {
      fprintf(stderr,"sort: error: not enough memory !");
      exit(1);
    }
  }
  return linenr;
}
/*}}}  */

/*{{{  void write_file(char **tab, long lines, FILE *fp)*/
void write_file(char **tab, long lines, FILE *fp)
{
  long i;
  for(i=0;i<lines;i++)
  {
    fputs(*tab++,fp);
    putc('\n',fp);
  }
}
/*}}}  */

/*{{{  int compare(char **elem1, char **elem2)*/
int compare(char **elem1, char **elem2)
{
  register char *c1,*c2;
	int result;
	  
  c1=*elem1;
  c2=*elem2;
  while (*c1 && *c2)
  {
/*    if ((result = (toupper(*c1) - toupper(*c2))) != 0)*/
    if ((result = (*c1 - *c2)) != 0)
      return result;
    c1++;
    c2++;
  }
	return *c1-*c2;
}
/*}}}  */
