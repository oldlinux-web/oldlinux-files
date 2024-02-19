#define __LIBRARY__
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#define execve	libc_execve
#include <unistd.h>
#undef execve

static inline
_syscall3(int,execve,const char *,file,char **,argv,char **,envp)

int execv(const char * pathname, char ** argv)
{
	return execve(pathname,argv,__environ);
}

int execl(const char * pathname, char * arg0, ...)
{
/* XXX - use varargs better (starting with not declaring arg0). */

	return execve(pathname,&arg0,__environ);
}

int execle(const char * pathname, char * arg0, ...)
{
/* XXX - as above. */

	char ** argp = &arg0;

	while (*argp++ != NULL) ;

	/* Since we didn't do varargs right, we now have to do a
	 * bletcherous cast to get the right type for envp.
	 */
	return execve(pathname,&arg0,(char **) *argp);
}
