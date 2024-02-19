/* This is a dummy zip.h to allow crypt.c from Zip to compile for unzip */

#define SKIP_TIME_H   /* temporary bugfix for VMS gcc compiler (may be */
#include "unzip.h"    /*  necessary for other compilers--edit unzip.h  */
#undef SKIP_TIME_H    /*  appropriately) */

#define local
#ifdef __STDC__
#  define PROTO
#endif
typedef unsigned short ush;
typedef unsigned long ulg;
#if defined(UTS)   /* may be necessary for other old compilers, too */
   typedef char *voidp;
#else /* !UTS */
   typedef void *voidp;
#endif /* ?UTS */
typedef int extent;

extern ULONG crc_32_tab[];
#define crc32(c, b) (crc_32_tab[((int)(c) ^ (b)) & 0xff] ^ ((c) >> 8))
