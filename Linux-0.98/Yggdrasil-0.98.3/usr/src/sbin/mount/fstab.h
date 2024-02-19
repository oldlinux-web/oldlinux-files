/* The fsent(3) routines are obsoleted by mntent(3).  I use them for
   convenience.  Since the implementation uses mntent(3), be very
   careful with the static buffers returned.
   $Header: /usr/src/mount/RCS/fstab.h,v 1.1 1992/09/06 13:30:53 root Exp root $ */

#ifndef _FSTAB_H
#include <stdio.h>
#include <mntent.h>

#define _PATH_FSTAB	"/etc/fstab"

/* Translate fsent(3) stuff into mntent(3) stuff.
   In general this won't work, but it's good enough here.  */
#define fstab mntent
#define fs_type mnt_type
#define fs_spec mnt_fsname
#define FSTAB_SW MNTTYPE_SWAP

struct fstab *getfsent (void);
struct fstab *getfsspec (const char *spec);
struct fstab *getfsfile (const char *file);
int setfsent (void);
void endfsent (void);

#endif /* _FSTAB_H */
