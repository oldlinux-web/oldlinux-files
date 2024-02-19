/* nice.c: copywrite (92) Peter MacDonald: Distribute freely, don't restrict */
#include <sys/types.h>
#include <signal.h>
#include <string.h>

/* link nice to renice to change running processes priorities. */

int is_nice;  

void usage() 
{ 
  if (is_nice)
    puts("usage: nice [-n] command"); 
  else  
    puts("usage: renice [-n] pid"); 
  exit(-1); 
}

int renice(int priority, int pid)
{
  puts("renice system call not yet implemented");
  return(0);
}

int main(int argc, char *argv[])
{ 
  int priority = 10;
  
  is_nice = (strcmp(*argv+strlen(*argv)-6,"renice"));
  
  if ((argc>1) && (argv[1][0] == '-'))
  { priority = atoi(argv[1]+1);
    if ((priority>19) || (priority<-20))
      usage();
    argc--;
    argv++;  
  }  
    
  if (argc<2)
    usage();
    
  if (!is_nice)
    if (renice(priority,atoi(argv[1])))
      usage();
    else
      return(0);  
      
  if (nice(priority))
    usage();

  execvp(argv[1], argv+1);
  puts("can not execute");
  exit(-1);
}  
