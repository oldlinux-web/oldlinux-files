/*
 * chown-	Change the owner and/or group ID of a file.
 *
 * Author:	Bert Reuling (bert@kyber.UUCP) 
 *
 * Revision History:
 *	1.1  89/09/25 bert@kyber.UUCP 		Initial revision
 *	     89/10/04 waltje@kyber.UUCP		Adapted to MINIX Style Sheet
 */
#include <sys/types.h>
#include <ctype.h>
#include <grp.h>
#include <pwd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>

#ifndef TRUE
#	define TRUE  1
#	define FALSE 0
#endif

static char rcsid[] = "$Header: chown.c,v 1.1 89/10/04 20:45:00 bert Exp $";


main(argc, argv)
int argc;
char *argv[];
{
  int i, uid, gid, Chown, status = 0;
  char *pgmname, *uids, *gids;
  struct passwd *pwd;
  struct group *grp;
  struct stat st;

  if ((pgmname = strrchr(argv[0], '/')) != NULL) pgmname++;
    else pgmname = argv[0];
  if (strcmp(pgmname, "chown") == 0) Chown = TRUE;
    else if (strcmp(pgmname, "chgrp") == 0) Chown = FALSE;
           else {
      		 (void) fprintf(stderr, "%s: should be named \"chown\" or \"chgrp\"\n", argv[0]);
	         (void) exit(-1);
   	   }
  if (argc < 3) {
	(void) fprintf(stderr,"Usage: %s %s file ...\n", pgmname, (Chown) ? "owner" : "group");
      	(void) exit(1);
  }
  if ((gids = strchr(argv[1], '.')) != NULL) {
  	*gids++ = '\0';
      	uids = argv[1];
  } else {
          if (Chown) {
         	uids = argv[1];
         	gids = NULL;
      	  } else {
         	  uids = NULL;
         	  gids = argv[1];
      	    }
  }
  if (uids == NULL) pwd = NULL;
    else {
          if (isdigit(*uids)) pwd = getpwuid(atoi(uids));
      	    else pwd = getpwnam(uids);
          if (pwd == NULL) {
         	(void) fprintf(stderr, "%s: unknown user id %s\n", pgmname, uids);
         	(void) exit(-1);
      	  }
    }
  if (gids == NULL) grp = NULL;
    else {
          if (isdigit(*gids)) grp = getgrgid(atoi(gids));
            else grp = getgrnam(gids);
          if (grp == NULL) {
         	(void) fprintf(stderr, "%s: unknown group: %s\n", pgmname, gids);
         	(void) exit(-1);
      	  }
    }
  for (i = 2; i < argc; i++) {
  	if (stat(argv[i], &st) != -1) {
        uid = (pwd == NULL) ? st.st_uid : pwd->pw_uid;
        gid = (grp == NULL) ? st.st_gid : grp->gr_gid;
        if (chown(argv[i], uid, gid) == -1) {
        	(void) fprintf(stderr,"%s: not changed\n", argv[i]);
            	status++;
        }
#ifdef _MINIX
        /*
         * chown(2) should do this ...
         */
        if (getuid() != 0) {
		st.st_mode &= ~S_ISUID;
            	st.st_mode &= ~S_ISGID;
            	if (chmod(argv[i], st.st_mode) == -1) {
               		(void) fprintf(stderr, "%s: mode not changed\n", argv[i]);
               		status++;
            	}
        }
#endif
     } else {
             (void) perror(argv[i]);
             status++;
       }
  }
  (void) exit(status);
}
