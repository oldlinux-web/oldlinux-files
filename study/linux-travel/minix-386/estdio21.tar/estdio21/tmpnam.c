/*                               t m p n a m                               */

#include "stdiolib.h"

#define L_tmpdir	(L_tmpnam-L_pid-L_sep-L_seq-1) /* length of P_tmpdir */
#define L_pid		8		/* length of pid code */
#define L_sep		1		/* length of separator */
#define L_seq		4		/* length of the sequence */

/*LINTLIBRARY*/

char *tmpnam F1(char *, s)

{
  static char P_tmpdir[L_tmpdir+1] = {'/','t','m','p','/',0}; /* tmpdir */
  static char name[L_tmpnam];		/* default place */
  static time_t seq = 0;		/* sequence number */
  static pid_t pid = -1;		/* process id */
  struct stat sbuf;			/* stat buffer */
  register char *p;			/* name generator */
  unsigned int v;			/* conversion value */
  register int d;			/* digits to generate */
  int j;				/* counter */

  if (pid < 0)
    pid = getpid();
  if (seq == 0)
    seq = time((time_t *) 0);

  do {

/* Generate name backwards */
    if ((p = s) == 0)
      p = name;
    p += L_tmpnam-1;
    *p = 0;

/* Fill in last characters */
    v = (unsigned int) seq;
    d = L_seq;
    j = 2;
    for (;;) {
      do {
	*--p = "0123456789abcdef"[v & 0xf];
	v >>= 4;
      } while (--d);

      if (--j == 0)
	break;

      p[-1] = p[0];
      *p--  = '.';

      v = (unsigned int) pid;
      d = L_pid;
    }

/* Fill in directory prefix */
    s = &P_tmpdir[L_tmpdir];
    d = L_tmpdir;
    do
      *--p = *--s;
    while (--d);

/* Advance for next name */
    seq++;

  } while (stat(p, &sbuf) >= 0);

  return p;
}
