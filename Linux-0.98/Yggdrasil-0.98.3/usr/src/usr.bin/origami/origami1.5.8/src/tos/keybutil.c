#include <stdlib.h>

void newexit (int status)
{
  exit(status);
}

int shellwait = 0;
/*{{{  void newsystem(char *command) - prepare & do system call*/
void newsystem(char *command)
{
  system(command);
}
/*}}}  */
