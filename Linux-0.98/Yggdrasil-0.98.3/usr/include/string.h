#ifndef _STRING_H_
#define _STRING_H_

#include <traditional.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {                                    /* for C++ V2.0 */
#endif

/* bsd stuffs */
extern int	bcmp _ARGS ((const void*, const void*, int));
extern void	bcopy _ARGS ((const void *__s1, void *__s2, int n));
extern void	bzero _ARGS ((void *__s, int __n));
extern char*	index _ARGS ((const char*, int));
extern char*	rindex _ARGS ((const char*, int));

extern int	ffs _ARGS ((int i));

extern void*	memccpy _ARGS ((void*, const void*, int, size_t));
extern void*	memcpy _ARGS ((void*, const void*, size_t));
extern void*	memmove _ARGS ((void*, const void*, size_t));
extern int	memcmp _ARGS ((const void*, const void*, size_t));
extern void*	memset _ARGS ((void*, int, size_t));
extern void*	memchr _ARGS ((const void*, int, size_t));

extern int	strcasecmp _ARGS ((const char*, const char*));
extern char*	strcat _ARGS ((char*, const char*));
extern char*	strchr _ARGS ((const char*, int));
extern int	strcmp _ARGS ((const char*, const char*));
extern int	strcoll _ARGS ((const char*, const char*));
extern char*	strcpy _ARGS ((char*, const char*));
extern size_t	strcspn _ARGS ((const char*, const char*));
extern char*	strdup _ARGS ((const char*));
extern size_t	strlen _ARGS ((const char*));
extern int	strncasecmp _ARGS ((const char*, const char*, size_t));
extern char*	strncat _ARGS ((char*, const char*, size_t));
extern int	strncmp _ARGS ((const char*, const char*, size_t));
extern char*	strncpy _ARGS ((char*, const char*, size_t));
extern char*	strpbrk _ARGS ((const char*, const char*));
extern char*	strrchr _ARGS ((const char*, int));
extern char*	strsignal _ARGS ((int __signo));
extern size_t	strspn _ARGS ((const char*, const char*));
extern char*	strstr _ARGS ((const char*, const char *));
extern char*	strtok _ARGS ((char*, const char*));
extern size_t	strxfrm _ARGS ((char*, const char*, size_t));

extern char*	strerror _ARGS ((int __errno));
extern void	perror _ARGS ((const char *__s));

extern void	swab _ARGS ((const void *__from, void *__to,
			size_t __nbytes));

#ifdef __cplusplus
}
#endif

#if defined(__GNUC__) && defined(__OPTIMIZE__)

#define ffs(i)	__builtin_ffs((i))

#endif  /* GCC and optimizing.  */

#endif
