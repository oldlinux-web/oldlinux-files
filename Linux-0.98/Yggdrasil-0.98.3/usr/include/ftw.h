/*
**  <FTW.H>
**  Header values for the third parameter to the user-supplied function
**  for ftw().
**
**  $Header: ftw.h,v 1.1 87/12/29 21:34:34 rsalz Exp $
*/

#ifndef _FTW_H
#define _FTW_H

#include <traditional.h>
#include <sys/stat.h>

#define FTW_NS		100	/* Something stat(2) failed on	*/
#define FTW_DNR		200	/* Something opendir(3) failed on*/
#define FTW_F		300	/* A normal file		*/
#define FTW_D		400	/* A directory			*/

#ifdef _cplusplus
extern "C" {
#endif

typedef int (*__ftw_fn_t)(const char *, const struct stat *, int);

extern int ftw _ARGS ((const char *__path, __ftw_fn_t __fn,
			int __depth));


#ifdef _cplusplus
}
#endif

#endif /* _FTW_H */
