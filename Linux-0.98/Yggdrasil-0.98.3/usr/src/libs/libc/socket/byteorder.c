#include <ansidecl.h>
#include <gnu-stabs.h>

#include <netinet/in.h>

#undef ntohl
#undef ntohs
#undef htonl
#undef htons

unsigned long int
htonl(unsigned long int x)
{
  return __htonl (x);
}

unsigned short int
htons(unsigned short int x)
{
  return __htons (x);
}

function_alias(ntohl, htonl, unsigned long int, (x),
	       DEFUN(htonl, (x), unsigned long int))

function_alias(ntohs, htons, unsigned short int, (x),
	       DEFUN(htons, (x), unsigned short int))
