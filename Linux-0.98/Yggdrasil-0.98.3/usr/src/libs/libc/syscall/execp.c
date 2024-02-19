#define __LIBRARY__
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#define execve  libc_execve
#include <unistd.h>
#undef execve

static char tmp_path[PATH_MAX+1];

static inline
_syscall3(int,execve,const char *,file,char **,argv,char **,envp)

static int execvep(const char * file, char ** argv, char ** envp)
{
	char * path, * tmp;
	int len;
	struct stat st;

#if 0
	if (*file=='/' || !*file || !(path=getenv("PATH")))
#else
	/* This routine isn't very good.   Without this change it searched
	 * the path for "./foo" and "foo/bar".  See the Minix execlp.c by
	 * Terence Holm and Bruce Evans for other tricky points.  Using
	 * access is almost guaranteed to be wrong.
	 */
	if (strchr(file,'/') || !*file || !(path=getenv("PATH")))
#endif
		return execve(file,argv,envp);
	while (*path) {
		tmp=tmp_path;
		len=0;
		while (len<PATH_MAX && *path && *path!=':') {
			*(tmp++) = *(path++);
			len++;
		}
		if (*path==':')
			path++;
		if (!len || tmp[-1]!='/') {
			*(tmp++) = '/';
			len++;
		}
		if (len>=PATH_MAX)
			continue;
		strncpy(tmp,file,PATH_MAX-len);
		if (stat(tmp_path,&st))
			continue;
		else
			if (!(st.st_mode & S_IFREG))
				continue;
		if (!access(tmp_path,1))
			return execve(tmp_path,argv,envp);
	}
	errno = ENOEXEC;
	return -1;
}

int execvp(const char * file, char ** argv)
{
	return execvep(file,argv,__environ);
}

int execlp(const char * file, char * arg0, ...)
{
	return execvep(file,&arg0,__environ);
}
