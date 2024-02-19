#include <stdlib.h>

/*
 * These I liked writing. More library routines like these. Linus
 */

#undef abs
#undef labs

int abs(int n)
{
	return (n<0)?-n:n;
}

long labs(long n)
{
	return (n<0)?-n:n;
}
