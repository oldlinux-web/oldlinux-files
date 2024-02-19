#include <errno.h>

extern char *sys_errlist[1];
extern int sys_nerr;

int perror() {}

int main()

{
  char *s;
  int i;
  
  s = sys_errlist[0];
  i = sys_nerr;
  
  return 0;
}
