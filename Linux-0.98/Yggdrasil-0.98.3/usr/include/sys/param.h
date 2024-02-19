#ifndef _SYS_PARAM_H
#define _SYS_PARAM_H

#include <limits.h>
#include <linux/param.h>

/* Don't change it. H.J. */
#ifdef OLD_LINUX
#undef	MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 	8	/* max length of hostname */
#endif

#if 1
#define MAXPATHLEN      1024
#define NOFILE          32
#endif

#endif
