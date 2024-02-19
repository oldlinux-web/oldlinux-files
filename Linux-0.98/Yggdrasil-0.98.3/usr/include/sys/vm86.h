#ifndef _SYS_VM86_H
#define _SYS_VM86_H

#include <linux/vm86.h>
#include <traditional.h>


#ifdef __cplusplus
extern "C" {
#endif

extern vm86(struct vm86_struct * __info);

#ifdef __cplusplus
}
#endif

#endif /*_SYS_VM86_H */
