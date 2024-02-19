/* id - return uid and gid		Author: John J. Marco */


/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/* 		----- id.c -----					*/
/* Id - get real and effective user id and group id			*/
/* Author: John J. Marco						*/
/*	   pa1343@sdcc15.ucsd.edu					*/
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>

main()
{
  struct passwd *pwd;
  struct passwd *getpwuid();
  struct group *grp;
  struct group *getgrgid();
  int uid, gid, euid, egid;

  uid = getuid();
  gid = getgid();
  euid = geteuid();
  egid = getegid();

  if ((pwd = getpwuid(uid)) == (struct passwd *) NULL)
	printf("%s%d%s", "uid=", uid, " ");
  else
	printf("%s%d%s%s%s", "uid=", uid, "(", pwd->pw_name, ") ");

  if ((grp = getgrgid(gid)) == (struct group *) NULL)
	printf("%s%d%s", "gid=", gid, " ");
  else
	printf("%s%d%s%s%s", "gid=", gid, "(", grp->gr_name, ") ");

  if (uid != euid)
	if ((pwd = getpwuid(euid)) != (struct passwd *) NULL)
		printf("%s%d%s%s%s", "euid=", euid, "(", pwd->pw_name, ") ");
	else
		printf("%s%d%s", "euid=", euid, " ");

  if (gid != egid)
	if ((grp = getgrgid(egid)) != (struct group *) NULL)
		printf("%s%d%s%s%s", "egid=", egid, "(", grp->gr_name, ") ");
	else
		printf("%s%d%s", "egid=", egid, " ");

  printf("\n");
}
