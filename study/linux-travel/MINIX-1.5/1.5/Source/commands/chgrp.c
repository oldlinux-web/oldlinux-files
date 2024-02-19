/* chgrp  -  change group		 Author: Bert Reuling */

/* This program is also chown.c */
/*
 * 89Dec07 Blayne Puklich Modified so that it checks its BASENAME
 *		rather than the whole argv[0] to see who it should be.
 *		Added slash(), which comes from dosread.c.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

char *slash( /* char *str */ );

main(argc, argv)
int argc;
char *argv[];
{
  int i, uid, gid, owner, status = 0;
  struct passwd *pwd, *getpwuid(), *getpwnam();
  struct group *grp, *getgrgid(), *getgrnam();
  struct stat st;
  char *name = slash(argv[0]);

  if (strcmp(name, "chown") == 0)
	owner = TRUE;
  else if (strcmp(name, "chgrp") == 0)
	owner = FALSE;
  else {
	fprintf(stderr, "%s: should be named \"chown\" or \"chgrp\"\n", name);
	exit(1);
  }

  if (argc < 3) {
	fprintf(stderr, 
		 "Usage: %s %s file ...\n", name, (owner) ? "owner" : "group");
	exit(1);
  }
  if (isdigit(argv[1][0])) {
	if (owner)
		pwd = getpwuid(atoi(argv[1]));
	else
		grp = getgrgid(atoi(argv[1]));
  } else {
	if (owner)
		pwd = getpwnam(argv[1]);
	else
		grp = getgrnam(argv[1]);
  }

  if (owner && (pwd == (struct passwd *) 0)) {
	fprintf(stderr, "%s: unknown user\n", name);
	exit(1);
  }
  if (!owner && (grp == (struct group *) 0)) {
	fprintf(stderr, "%s: unknown group\n", name);
	exit(1);
  }
  for (i = 2; i < argc; i++) {
	if (stat(argv[i], &st) != -1) {
		if (owner) {
			uid = pwd->pw_uid;	/* new owner */
			gid = st.st_gid;	/* old group */
		} else {
			uid = st.st_uid;	/* old owner */
			gid = grp->gr_gid;	/* new group */
		}
		if (chown(argv[i], uid, gid) == -1) {
			fprintf(stderr, "%s: not changed\n", argv[i]);
			status++;
		}
	} else {
		perror(argv[i]);
		status++;
	}
  }
  exit(status);
}

/* Stolen from dosread.c 89Dec07 Blayne Puklich */

char *slash(str)
register char *str;
{
  register char *result = str;

  while (*str)
	if (*str++ == '/') result = str;

  return result;
}
