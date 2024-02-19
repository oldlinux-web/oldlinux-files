/* kill.c : copyright (92) Peter MacDonald: Distribute freely, don't restrict*/

#include <sys/types.h>
#include <signal.h>
#include <strings.h>

#define ERR 2
const char *usagestr = "usage: kill [-signal] pid ...\n";

void oops(void)
{ write(ERR,usagestr,strlen(usagestr));
  exit(1);
}   

void oops2(char *str1, char *str2)
{
  write(ERR,str1,strlen(str1));
  write(ERR,str2,strlen(str2));
  write(ERR,"\n",1);
  exit(1);
}   

char *itoa(int num)
{ int sign = 0; 
  static char buf[15];
  char *cp = buf+sizeof(buf)-1;
  if (num<0)
  { sign = 1;
    num = -num;
  }
  do
  { 
    *cp-- = '0'+num%10; 
    num /= 10;
  } while (num);
  if (sign)
    *cp-- = '-';
  return(cp+1);
}

const char *signames[] =  {
	"HUP", "INT", "QUIT", "ILL", "TRAP", "ABRT", "UNUSED", 
	"FPE", "KILL", "USR1", "SEGV", "USR2", "PIPE", "ALRM", "TERM", 
	"STKFLT", "CHLD", "CONT", "STOP", "TSTP", "TTIN", "TTOU", 0 };

int main(int argc, char *argv[])
{
  int i, signum = SIGTERM;
  
  if (argc < 2)
    oops();
  if ((argc>1) && ('-' == *argv[1]))
  {
    if ('?' == argv[1][1])
      for (i=0; i<NSIG; i++)
        if (!signames[i])
        { write(ERR,"\n",1);
          exit(0);
        }
        else
        { write(ERR,signames[i],strlen(signames[i]));
          write(ERR," ", 1);
          if (i==10)
            write(ERR,"\n",1);
        }
          
    for (i=0; i<NSIG; i++)
      if (!signames[i])
      { i = NSIG; 
        break;
      }
      else
        if (!strcmp(signames[i],argv[1]+1))
          break;
         
    if (i < NSIG)
      signum = i+1;
    else
      if ((!(signum = atoi(argv[1]+1))) || (signum<=0) || (signum>NSIG))
        oops();
    argc--;
    argv++;    
  }      

  while (--argc)
  { 
    argv++;
    if ((i = atoi(*argv)) <= 3)
      oops();
    if (kill(i,signum))
      oops2( "kill: no such process ", itoa(i));
  }
  return(0);
}
      

