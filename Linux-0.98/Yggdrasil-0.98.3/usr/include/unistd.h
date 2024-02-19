#ifndef _UNISTD_H
#define _UNISTD_H

/* ok, this may be a joke, but I'm working on it */
#define _POSIX_VERSION		199009L

/* only root can do a chown (I think..) */
#define _POSIX_CHOWN_RESTRICTED	1

/* no pathname truncation (but see in kernel) */
#define _POSIX_NO_TRUNC		1

/* character to disable things like ^C */
#define _POSIX_VDISABLE		'\0'

#define _POSIX_JOB_CONTROL	1

/* Implemented, for whatever good it is */
#define _POSIX_SAVED_IDS	1 

#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

#ifndef NULL
#define NULL    ((void *)0)
#endif

/* access */
#define F_OK	0
#define X_OK	1
#define W_OK	2
#define R_OK	4

/* lseek */
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

/* Symbolic constants for sysconf() variables
 * defined by POSIX.1: 0-99
 */

/* ARG_MAX: Max length of argument to exec()
 * including environment data */
#define _SC_ARG_MAX		0
/* CHILD_MAX: Max # of processes per userid */
#define _SC_CHILD_MAX		1
/* Number of clock ticks per second */
#define _SC_CLK_TCK		2
/* NGROUPS_MAX: Max # of simultaneous supplementary group
 * IDs per process */
#define _SC_NGROUPS_MAX 	3
/* OPEN_MAX: Max # of files that one process can have open at
 * any one time */
#define _SC_OPEN_MAX		4
/* _POSIX_JOB_CONTROL: 1 iff supported */
#define _SC_JOB_CONTROL		5
/* _POSIX_SAVED_IDS: 1 iff supported */
#define _SC_SAVED_IDS		6

/* Symbolic constants for sysconf() variables added
 * by POSIX.1-1990: 100-199 */

/* STREAM_MAX: Max # of open stdio FILEs */
#define _SC_STREAM_MAX		100
/* TZNAME_MAX: Max length of timezone name */
#define _SC_TZNAME_MAX		101

#define _SC_VERSION		102

/* I don't have the documentations */
#if 0
/* Symbolic constants for sysconf() variables
 * added by POSIX.2: 200-299 */

/* largest ibase & obase for bc */
#define _SC_BC_BASE_MAX		200 
/* max array elements for bc */
#define _SC_BC_DIM_MAX		201
/* max scale value for bc */
#define _SC_BC_SCALE_MAX	202
#define _SC_BC_STRING_MAX	203
/* max bytes in collation element */
#define _SC_COLL_ELEM_MAX	204
#define _SC_EQUIV_CLASS_MAX	205
/* max nesting of (...) for expr */
#define _SC_EXPR_NEST_MAX	206
/* max length in bytes of input line */
#define _SC_LINE_MAX		207 
/* max file operands for paste */
#define _SC_PASTE_FILES_MAX	208 
/* max regular expressions permitted */
#define _SC_RE_DUP_MAX		209 
/* max bytes of pattern space for sed */
#define _SC_SED_PATTERN_MAX	210 
/* max bytes of message for sendto */
#define _SC_SENDTO_MAX		211 
/* max bytes of input line for sort */
#define _SC_SORT_LINE_MAX	212 
/* Current version of POSIX.2 */
#define _SC_2_VERSION		213
/* C Language Bindings Option */
#define _SC_2_C_BIND		214
/* C Development Utilities Option */
#define _SC_2_C_DEV		215
/* FORTRAN Dev. Utilities Option */
#define _SC_2_FORT_DEV		216
/* Software Dev. Utilities Option */
#define _SC_2_SW_DEV		217

/* Symbolic constants for sysconf() variables defined by
 * X/Open: 2000-2999 */

/* _XOPEN_VERSION: Issue # of XPG */
#define _SC_XOPEN_VERSION	8
/* PASS_MAX: Max # of bytes in password */
#define _SC_PASS_MAX		9
/* CLOCKS_PER_SEC: Units/sec of clock() */
#define _SC_CLOCKS_PER_SEC	2000

/* Symbolic constants for sysconf() variables
defined by OSF: 3000-3999 */

/* AES_OS_VERSION: Version of OSF/AES OS */
#define _SC_AES_OS_VERSION	3000
/* PAGE_SIZE: Software page size */
#define _SC_PAGE_SIZE		3001
/* ATEXIT_MAX: Max # of atexit() funcs */
#define _SC_ATEXIT_MAX		3002

#endif

/* Symbolic constants for pathconf() defined by POSIX.1: 0-99 */

/* LINK_MAX: Max # of links to a single file */
#define _PC_LINK_MAX		0
/* MAX_CANON: Max # of bytes in a terminal canonical input line */
#define _PC_MAX_CANON		1
/* MAX_INPUT: Max # of bytes allowed in a terminal input queue */ 
#define _PC_MAX_INPUT		2
/* NAME_MAX: Max # of bytes in a filename */
#define _PC_NAME_MAX		3
/* PATH_MAX: Max # of bytes in a pathname */
#define _PC_PATH_MAX		4
/* PIPE_BUF: Max # of bytes for which pipe writes are atomic */ 
#define _PC_PIPE_BUF		5
/* _POSIX_CHOWN_RESTRICTED: 1 iff only a privileged process can
 * use chown() */
#define _PC_CHOWN_RESTRICTED	6
/* _POSIX_NO_TRUNC: 1 iff an error is detected when
 * exceeding NAME_MAX */
#define _PC_NO_TRUNC		7
/* _POSIX_VDISABLE: character setting which disables TTY local
 * editing characters */
#define _PC_VDISABLE		8

#if 0

/* Conformance and options for POSIX.2 */

/* 1989 (1989), POSIX.2, Draft 9 (09)L */
#define _POSIX2_VERSION 198909L
#define _POSIX2_C_BIND		1	/* Always on */
#define _POSIX2_C_DEV		-1
#define _POSIX2_FORT_DEV	-1
#define _POSIX2_SW_DEV		-1
#endif

/* Path on which all POSIX.2 utilities can be found */

#define CS_PATH	  "/bin:/usr/bin:"

/* Symbolic constants for confstr() defined by POSIX.2: 200-299 */

/* Search path that finds all POSIX.2 utils */
#define _CS_PATH	200


#include <traditional.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <utime.h>
#include <signal.h>

#ifdef __LIBRARY__
#include <linux/unistd.h>
#endif

extern char **environ;

#ifdef __cplusplus
extern "C" {
#endif

/* XXX - several non-POSIX functions here, and POSIX functions that are
 * supposed to be declared elsewhere.  Non-promotion of short types in
 * prototypes may cause trouble.  Arg names should be prefixed by
 * underscores.
 */

#ifdef __STDC__
extern volatile	void _exit (int __status);
#else
extern void _exit ();
#endif

/* XXX - short type */
extern int	access _ARGS ((const char* __filename, mode_t __mode));
extern int	acct _ARGS ((const char* __filename));
/* XXX - POSIX says unsigned alarm (unsigned sec) */
extern int	alarm _ARGS ((int __sec));
extern int	brk _ARGS ((void* __end_data_segment));
extern void*	sbrk _ARGS ((ptrdiff_t __increment));
extern int	chdir _ARGS ((const char* __filename));
/* XXX - short type */
extern int	chmod _ARGS ((const char* __filename, mode_t __mode));
/* XXX - short type */
extern int	chown _ARGS ((const char* __filename, uid_t __owner,
			gid_t __group));
extern int	chroot _ARGS ((const char* __filename));
extern int	close _ARGS ((int __fildes));
/* XXX - short type */
extern int	creat _ARGS ((const char* __filename, mode_t __mode));
extern int	dup _ARGS ((int __fildes));
extern int	dup2 _ARGS ((int __oldfd, int __newfd));
extern int	execl _ARGS ((const char* __pathname,
			const char* __arg0, ...));
extern int	execle _ARGS ((const char * __pathname,
			const char * __arg0, ...));
extern int	execlp _ARGS ((const char* __file,
			const char* __arg0, ...));
extern int	execv _ARGS ((const char* __pathname,
			char* const __argv []));
extern int	execve _ARGS ((const char* __filename,
			char* const __argv [], char* const __envp []));
extern int	execvp _ARGS ((const char* __file,
			char* const __argv []));
extern int	fchmod _ARGS ((int __fildes, mode_t __mode));
extern int	fchown _ARGS ((int __fildes, uid_t __owner, gid_t __group));
extern int	fcntl _ARGS ((int __fildes, int __cmd, ...));
extern pid_t	fork _ARGS ((void));
extern int	fstat _ARGS ((int __fildes, struct stat* __stat_buf));
extern int	ftruncate _ARGS ((int __fildes, size_t __length));
extern gid_t	getegid _ARGS ((void));
extern uid_t	geteuid _ARGS ((void));
extern gid_t	getgid _ARGS ((void));
extern int	getgroups _ARGS ((int __gidsetlen, gid_t *__gidset));
extern int	gethostname _ARGS ((char *__name, int __len));
extern pid_t	getpgrp _ARGS ((void));
extern pid_t	getpid _ARGS ((void));
extern int	getppid _ARGS ((void));
extern uid_t	getuid _ARGS ((void));
extern int	ioctl _ARGS ((int __fildes, int __cmd, ...));
extern int	ioperm  _ARGS ((unsigned long __from,
			unsigned long __num, int __turn_on));
extern int	link _ARGS ((const char* __filename1,
			const char* __filename2));
extern off_t	lseek _ARGS ((int __fildes, off_t __offset,
			int __origin));
extern int	lstat _ARGS ((const char* __filename,
			struct stat* __stat_buf));
extern int	mkdir _ARGS ((const char *__path, mode_t __mode));
/* XXX - shorts */
extern int	mknod _ARGS ((const char* __filename, mode_t __mode,
			dev_t __dev));
extern caddr_t	mmap _ARGS ((caddr_t __addr, size_t __len, int __prot,
			int __flags, int __fd, off_t __off));
extern int	mount _ARGS ((const char* __specialfile,
			const char* __dir, const char* __filesystemype,
			unsigned long __rwflag, const void *__data));
extern int	munmap _ARGS ((caddr_t __addr, size_t __len));
extern int	nice _ARGS ((int __val));
extern int	open _ARGS ((const char* __filename, int __flag, ...));
extern int	pause _ARGS ((void));
extern int	pipe _ARGS ((int* __fildes));
extern int	profil _ARGS ((char *__buf, int __bufsiz, int __offset,
			int __scale));
extern int	ptrace _ARGS ((int __request, int __pid, int __addr,
			int __data));
/* XXX**2 - POSIX says unsigned count */
extern int	read _ARGS ((int __fildes, char* __buf,
			off_t __count));
extern int	readlink _ARGS ((const char *__path, char *__buf,
			int __bufsiz));
extern int	reboot _ARGS ((int __magic, int __magic_too,
			int __flag));
extern int	rename _ARGS ((const char *__from, const char *__to));
extern int	rmdir _ARGS ((const char *__path));
extern int	setgid _ARGS ((gid_t __gid));	/* XXX - short type */
extern int	setgroups _ARGS ((int __gidsetlen, gid_t *__gidset));
extern int	sethostname _ARGS ((char* __name, int __len));
extern int	setpgid _ARGS ((pid_t __pid, pid_t __pgid));
extern int	setpgrp _ARGS ((void));
extern int	setregid _ARGS ((gid_t __rgid, gid_t __egid));
extern int	setreuid _ARGS ((uid_t __ruid, uid_t __euid));
extern pid_t	setsid _ARGS ((void));
extern int	setuid _ARGS ((uid_t __uid));	/* XXX - short type */
extern int	stat _ARGS ((const char* __filename,
			struct stat* __stat_buf));
extern int	symlink _ARGS ((const char *__name1,
			const char *__name2));
extern int	sync _ARGS ((void));
extern int	truncate _ARGS ((const char *__path, size_t __length));
extern int	ulimit _ARGS ((int __cmd, long __limit));
extern mode_t	umask _ARGS ((mode_t __mask));
extern int	umount _ARGS ((const char* __specialfile));
extern int	uname _ARGS ((struct utsname* __name));
extern int	unlink _ARGS ((const char* __filename));
extern int	ustat _ARGS ((dev_t __dev, struct ustat* __ubuf));
extern pid_t	vfork _ARGS ((void));
/* XXX**2 - POSIX says unsigned count */
extern int	write _ARGS ((int __fildes, const char* __buf,
			off_t __count));


extern long	sysconf _ARGS ((int __name));
extern long	pathconf _ARGS ((const char *__path, int __name));
extern long	fpathconf _ARGS ((int __fildes, int __name));
extern size_t	confstr _ARGS ((int __name, char *__buf, size_t __len));


extern char*	crypt _ARGS((const char *__key, const char *__salt));
extern void	encrypt _ARGS((char __block[64], int __edflag));
extern char*	getcwd _ARGS((char * __buf, size_t __size));
extern size_t	getdtablesize  _ARGS((void));
extern char*	getlogin _ARGS((void));
extern size_t	getpagesize  _ARGS((void));
extern char*	getpass _ARGS((const char *__prompt));
extern char*	getwd _ARGS((char *__pathname));
extern int	isatty _ARGS((int __fildes));
extern char*	mktemp _ARGS((char *__template));
extern void	setkey _ARGS((const char *__key));
extern unsigned int
		sleep _ARGS((unsigned int __seconds));
extern int	swapon _ARGS ((const char * __specialfile));
extern int	swapoff _ARGS ((const char * __specialfile));
extern char*	ttyname _ARGS((int __fildes));
extern int	uselib _ARGS ((const char *__filename));
extern void	usleep _ARGS((unsigned long __usec));

extern int	iopl _ARGS((int __level));

#ifdef __cplusplus
}                                               /* for C++ V2.0 */
#endif

#endif
