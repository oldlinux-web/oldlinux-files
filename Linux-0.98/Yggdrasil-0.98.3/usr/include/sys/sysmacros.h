#ifndef _SYS_SYSMACROS_H
#define _SYS_SYSMACROS_H

#define major(dev) (((unsigned) (dev))>>8)
#define minor(dev) ((dev)&0xff)
#define makedev(major,minor) ((major<<8)|minor)

#endif
