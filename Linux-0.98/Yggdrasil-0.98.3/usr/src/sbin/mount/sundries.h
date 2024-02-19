/* $Header: /usr/src/mount/RCS/sundries.h,v 1.1 1992/09/06 13:30:53 root Exp root $ */


#include <sys/types.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <mntent.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fstab.h"


#define streq(s, t)	(strcmp ((s), (t)) == 0)


#define MOUNTED_LOCK	"/etc/mtab~"
#define _PATH_FSTAB	"/etc/fstab"
#define LOCK_BUSY	3

/* File pointer for /etc/mtab.  */
extern FILE *F_mtab;

/* File pointer for lock.  */
extern FILE *F_lock;

/* String list data structure.  */ 
typedef struct string_list
{
  char *hd;
  struct string_list *tl;
} *string_list;

#define car(p) ((p) -> hd)
#define cdr(p) ((p) -> tl)

string_list cons (char * const a, const string_list);

/* Quiet compilation with -Wmissing-prototypes.  */
int main (int argc, char **argv);

/* From mount_call.c.  */
int mount5 (const char *, const char *, const char *, int, void *);

/* Functions in sundries.c that are used in mount.c and umount.c  */ 
void block_signals (int how);
char *canonicalize (const char *path);
void close_mtab (void);
void volatile die (int errcode, const char *fmt, ...);
void error (const char *fmt, ...);
void lock_mtab (void);
int matching_type (const char *type, string_list types);
void open_mtab (const char *mode);
string_list parse_types (char *types);
void unlock_mtab (void);
void *xmalloc (size_t size);
char *xstrdup (const char *s);

#ifdef HAVE_MOUNT5

#define mount5(special, dir, type, flags, data) \
  mount (special, dir, type, 0xC0ED0000 | (flags), data)

#else /* !HAVE_MOUNT5 */

/* Compatibility with 4 arg mount(2) used before the 0.96c-pl1 kernel.  */

#define mount5(special, dir, type, flags, data) \
  mount (special, dir, type, flags);

#endif /* !HAVE_MOUNT5 */
