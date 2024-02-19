/* The <grp.h> header is used for the getgrid() and getgrnam() calls. */

#ifndef _GRP_H
#define _GRP_H

struct	group { 
  char *gr_name;		/* the name of the group */
  char *gr_passwd;		/* the group passwd */
  gid_t gr_gid;			/* the numerical group ID */
  char **gr_mem;		/* a vector of pointers to the members */
};

/* Function Prototypes. */
#ifndef _ANSI_H
#include <ansi.h>
#endif

_PROTOTYPE( struct group *getgrgid,  (int _gid)  			);
_PROTOTYPE( struct group *getgrnam, (char *_name) 			);

#ifdef _MINIX
_PROTOTYPE( void endgrent, (void)					);
_PROTOTYPE( struct group *getgrent, (void)				);
_PROTOTYPE( int setgrent, (void)					);
#endif

#endif /* _GRP_H */
