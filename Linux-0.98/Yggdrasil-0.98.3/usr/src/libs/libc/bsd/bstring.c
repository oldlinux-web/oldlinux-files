#define __LIBRARY__
#include <strings.h>

int
bcmp(const void* s1, const void* s2, int len)
{return (len > 0 ? memcmp(s1, s2, (size_t) len) : 0);}

void
bcopy(const void* s, void* d, int n) 
{ if (n > 0) memmove (d, s, (size_t) n); }

void
bzero(void* s, int l)
{ if (l > 0) memset(s, 0, (size_t) l); }
