/* bcopy.c <P>-1992. 
   We have to emulate the bcopy and bzero routines using memcpy and memset.
   Since the minix versions of these routines are copyrighted I engaged the
   enormous task of rewriting them from scratch ;)
*/

#include <string.h>

void bcopy(From, To, length)
char *From,*To;
int length;
{
  memcpy(To,From,length);
}


void bzero(To, length)
char *To;
int length;
{
  memset(To,0,length);
}
