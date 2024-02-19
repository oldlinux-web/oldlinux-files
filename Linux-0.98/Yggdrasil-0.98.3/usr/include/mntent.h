#ifndef _MNTENT_H
#define _MNTENT_H

#include <traditional.h>

#define MNTTAB "/etc/fstab"
#define MOUNTED "/etc/mtab"

#define	MNTMAXSTR	128

#define	MNTTYPE_MINIX	"minix"		/* Minix file system. */
#define	MNTTYPE_SWAP	"swap"		/* Swap device */
#define	MNTTYPE_IGNORE	"ignore"	/* Ignore this entry */

/* generic mount options */
#define	MNTOPT_DEFAULTS	"defaults"	/* use all default opts */
#define	MNTOPT_RO	"ro"		/* read only */
#define	MNTOPT_RW	"rw"		/* read/write */
#define	MNTOPT_SUID	"suid"		/* set uid allowed */
#define	MNTOPT_NOSUID	"nosuid"	/* no set uid allowed */
#define MNTOPT_NOAUTO	"noauto"	/* don't auto mount */

/* minix specific options */
/* none defined yet */

/* swap specific options */
/* none defined yet */

#ifdef __cplusplus
extern "C" {
#endif

struct mntent{
       char  *mnt_fsname;
       char  *mnt_dir;
       char  *mnt_type;
       char  *mnt_opts;
       int   mnt_freq;
       int   mnt_passno;
};

#define __need_file
#include <stdio.h>
extern FILE *setmntent _ARGS ((const char *__filep, const char *__type));
extern struct mntent *getmntent _ARGS ((FILE *__filep));
extern int addmntent _ARGS ((FILE *__filep, const struct mntent *__mnt));
extern char *hasmntopt _ARGS ((const struct mntent *__mnt,
			const char *__opt));
extern int endmntent _ARGS ((FILE *__filep));

#ifdef __cplusplus
}
#endif

#endif /* _MNTENT_H */
