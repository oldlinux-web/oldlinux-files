/*{{{  includes*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <local/bool.h>

#define FILEIO_C

#ifdef KEYBIND
#include "..\common\keys.h"
#include "..\common\rcformat.h"
#include "..\keybind\scanner.h"
#include "..\keybind\keybind.h"
#include "..\origami\os.h"
#else
#include "..\origami\origami.h"
#endif
/*}}}  */

char base_name[]="origami.";

static int pipemode = 0;
static char pipecmd[256];
static char pipetemp[] = "_pipetmp.tmp";
extern int shellwait;

FILE *popen(char *command, char *mode)
{
  FILE *stream;
 
  if (*mode == 'r')
  {
    sprintf(pipecmd,"%s >%s",command,pipetemp);
    shellwait = FALSE;
    system(pipecmd);
    stream = fopen(pipetemp,"r");
    if (stream != NULL)
      pipemode = 1;
    return stream;
  } else if (*mode == 'w')
  {
    stream = fopen(pipetemp,"w+");
    sprintf(pipecmd,"%s <%s",command,pipetemp);
    if (stream != NULL)
      pipemode = 2;
    return stream;
  } else
    return NULL; /* unknown mode */
}

int pclose (FILE *stream)
{
  int result;
  if (pipemode == 1) /* read from pipe */
  {
    pipemode = 0;
    result = fclose(stream);
    unlink(pipetemp);
    return result;
  }
  if (pipemode == 2) /* write to pipe */
  {
    pipemode = 0;
    result = fclose(stream);
    if (result == 0)
    {
      shellwait = FALSE;
      system(pipecmd);
    }
    unlink(pipetemp);
    return result;
  }
  return -1; /* no pipe open */
}

/*{{{  tr_fopen*/
#ifdef STD_C
FILE *tr_fopen(char *filename)
#else
FILE *tr_fopen(filename)
char *filename;
#endif
{
  FILE *f;
  f = fopen(filename,"r");
  if (f != NULL)
    setvbuf(f,NULL,_IOFBF,10240L);
  return f;
}
/*}}}  */
/*{{{  tw_fopen*/
#ifdef STD_C
FILE *tw_fopen(char *filename)
#else
FILE *tw_fopen(filename)
char *filename;
#endif
{
  FILE *f;
  char bak_name[128], *p;
 
  strcpy(bak_name,filename);
  p = bak_name + strlen(bak_name) - 1;
  while (p>bak_name && *p != '.') p--;
  if (*p=='.')
    *p=0;
  strcat(p,".BAK");
  unlink(bak_name);
  rename(filename,bak_name);
 
  f = fopen(filename,"w");
  if (f != NULL)
    setvbuf(f,NULL,_IOFBF,10240L);
  return f;
}
/*}}}  */
/*{{{  br_fopen*/
#ifdef STD_C
FILE *br_fopen(char *filename)
#else
FILE *br_fopen(filename)
char *filename;
#endif
{
  FILE *f;
  f = fopen(filename,"rb");
  if (f != NULL)
    setvbuf(f,NULL,_IOFBF,10240L);
  return f;
}
/*}}}  */
/*{{{  bw_fopen*/
#ifdef STD_C
FILE *bw_fopen(char *filename)
#else
FILE *bw_fopen(filename)
char *filename;
#endif
{
  FILE *f;
  char bak_name[128], *p;
 
  strcpy(bak_name,filename);
  p = bak_name + strlen(bak_name) - 1;
  while (p>bak_name && *p != '.') p--;
  if (*p=='.')
    *p=0;
  strcat(p,".BAK");
  unlink(bak_name);
  rename(filename,bak_name);

  f = fopen(filename,"w+b");
  if (f != NULL)
    setvbuf(f,NULL,_IOFBF,10240L);
  return f;
}
/*}}}  */
