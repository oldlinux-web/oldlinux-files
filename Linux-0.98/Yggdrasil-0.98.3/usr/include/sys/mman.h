#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H

#include <traditional.h>
#include <sys/types.h>

#define PROT_READ        0x1       /* page can be read */
#define PROT_WRITE       0x2       /* page can be written */
#define PROT_EXEC        0x4       /* page can be executed */
#define PROT_NONE        0x0       /* page can not be accessed */

#define MAP_SHARED       1         /* Share changes */
#define MAP_PRIVATE      2         /* Changes are private */
#define MAP_TYPE         0xf       /* Mask for type of mapping */
#define MAP_FIXED        0x10      /* Interpret addr exactly */

#ifdef __cplusplus
extern "C" {
#endif

extern caddr_t mmap _ARGS ((caddr_t __addr, size_t __len,
		int __prot, int __flags, int __fd, off_t __off));
extern int munmap _ARGS ((caddr_t __addr, size_t __len));

#ifdef __cplusplus
}
#endif

#endif /* _SYS_MMAN_H */
