#ifndef _SYS_PARAM_H
#define _SYS_PARAM_H

#include <linux/fs.h>

#define HZ 100
#define EXEC_PAGESIZE 4096
#define getdtablesize() (NR_OPEN-1)

#define NGROUPS		32	/* Max number of groups per user */
#define NOGROUP		-1

#define MAXHOSTNAMELEN 8

#endif
