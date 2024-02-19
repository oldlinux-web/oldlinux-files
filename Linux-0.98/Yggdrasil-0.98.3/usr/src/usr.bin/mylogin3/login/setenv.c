/* setenv seems to be some post 4.3 BSD
   feature, I guess the third parameter
   tells whether to overwrite an eventual
   existing environment value...
*/

#include <memory.h>
#include <stdlib.h>

void *malloc();

void setenv(var,val,overwrite)
  char *var,*val;
  int overwrite;
  { char *p;
    if(!overwrite && getenv(var))
      return;
    p=malloc(strlen(var)+strlen(val)+2);
    sprintf(p,"%s=%s",var,val);
    putenv(p);
  }
