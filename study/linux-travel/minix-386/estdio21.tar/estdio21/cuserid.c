/*                              c u s e r i d                              */

#include "stdiolib.h"

/*LINTLIBRARY*/

char *cuserid F1(register char *, s)

{
  static char name[L_cuserid];		/* name buffer */
  struct passwd *pp;			/* point at password */

  if (s == NULL)
    s = name;
  return (pp = getpwuid(geteuid())) != NULL
         ? strncpy(s, pp->pw_name, L_cuserid)
	 : (*s = 0, s);
}
