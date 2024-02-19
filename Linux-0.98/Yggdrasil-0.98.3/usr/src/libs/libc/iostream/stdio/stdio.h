#include "_stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Generate a unique file name.  */
extern char*	__stdio_gen_tempname(const char *__dir,
			const char *__pfx,
			int __dir_search, size_t *__lenptr);

#ifdef __cplusplus
}
#endif

#define __stat		stat
#define __open		open
#define __close		close
#define __getpid	getpid
#define __getegid	getegid
#define __geteuid	geteuid
