/*
 * devname.c
 *
 */

#include <sys/stat.h>
#include <string.h>

/*
 * ttynames:
 * 	ttya0 a1 a2...	virtual consoles
 *	ttys0 s1 s2...	serial lines
 *	ttyp0 p1 p2...	pty's
 */

static char *ttgrp = "abcdstuvPQRWpqrs";
static char *ttsub = "0123456789abcdef";

char *
dev_to_tty(int dev)
{
    static char tty[3];

    if (dev == -1)
	return "? ";
    if (dev == 0)
	return "co";

    tty[0] = ttgrp[(dev >> 4) & 017];
    tty[1] = ttsub[dev & 017];
    return(tty);
}

tty_to_dev(tty)
char *tty;
{
    char *p, *q;

    if (*tty == '\0') {		/* empty string: controlling tty */
	struct stat buf;
	if (fstat(0, &buf) != -1)
	    return(buf.st_rdev & 0xff);
	else
	    return -1;
    }
    if (tty[1] == '\0' && *tty >= '0' && *tty <= '9')
	return(*tty - '0');
    if (strcmp(tty, "co") == 0)
	return 0;
    if ((p = strchr(ttgrp, *tty)) != NULL &&
	(q = strchr(ttsub, tty[1])) != NULL)
	return(((p - ttgrp) << 4) | (q - ttsub));
    else
	return -1;
}
