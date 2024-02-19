/* adduser : add a new user account          */
/* Craig Hagan                               */
/*                                           */
/* pardon the kludge..written during         */
/* commercials of star trek tng              */
/*                                           */
/* to compile, you need crypt.o from poe-IGL */
/* gcc -O -o adduser adduser.c crypt.o       */



#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#define PASSWD_FILE "/etc/passwd"
/*#define DEBUG*/

char *crypt();

main()
{
  char foo[32],uname[32],person[32],passwd[32],dir[60],shell[60],line[100];
  unsigned int group,uid;
  int bad=0,done=0,correct=0;
  char	salt[2];
  time_t tm;

  fprintf(stderr,"password: ");
  fflush(stderr);
  fgets(passwd,32,stdin);
  if (!strcmp(passwd,""))
    exit(0);
  time(&tm);
  salt[0] = (tm & 0x0f) +	'A';
  salt[1] = ((tm & 0xf0) >> 4) + 'a';
  printf("%s",crypt(passwd,salt));
}
  
