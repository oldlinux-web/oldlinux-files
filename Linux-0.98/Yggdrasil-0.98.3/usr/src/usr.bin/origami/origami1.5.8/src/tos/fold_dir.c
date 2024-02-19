/*{{{  about this file*/
/*

   FOLD_DIR.C - generate filed folds for all arguments
   this does the same as the shell script in the origami docs
   
   Michael Schwingen
   Ahornstrasse 36
   W-5100 Aachen
*/
/*}}}  */
/*{{{  includes*/
#include <stdio.h>
#include <stdlib.h>
#include "xarg.h"
/*}}}  */
/*{{{  defines*/
#define TRUE 1
#define FALSE 0
/*}}}  */

/*{{{  int main(int argc, char *argv[])*/
int main(int argc, char *argv[])
{
  int i;

  /*{{{  complain if wrong number of parameters*/
  if (argc<2)
  {
    puts("fold_dir: no arguments");
    exit(1);
  }
  /*}}}  */
	for (i=2;i<argc;i++)
	{
		printf("{{{F %s\n:::F %s\n}}}\n",argv[i],argv[i]);
  /*}}}  */
  }
  return 0;
}
/*}}}  */

