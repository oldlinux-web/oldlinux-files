#ifndef _SYS_VFS_H
#define _SYS_VFS_H

#include <linux/vfs.h>
#include <traditional.h>


#ifdef __cplusplus
extern "C" {
#endif

extern int statfs _ARGS ((const char *__path, struct statfs *__buf));
extern int fstatfs _ARGS ((int __fildes, struct statfs *__buf));

#ifdef __cplusplus
}
#endif

#endif /*_SYS_VFS_H */
