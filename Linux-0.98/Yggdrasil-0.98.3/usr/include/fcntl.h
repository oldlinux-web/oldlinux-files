#ifndef _FCNTL_H
#define _FCNTL_H

#include <features.h>
#include <traditional.h>
#include <sys/types.h>
#include <linux/fcntl.h>

#ifdef __cplusplus
extern  "C" {
#endif

extern int creat _ARGS ((const char * __filename, mode_t __mode));
extern int fcntl _ARGS ((int __fildes,int __cmd, ...));
extern int open _ARGS ((const char * __filename, int __flags, ...));

#ifdef __cplusplus
}
#endif

#endif
