/*
	getttyent, getttynam, setttyent, endttyent for linux
	by qpliu@phoenix.princeton.edu 1992
	$Id$
 */

#include <stdio.h>
#include <string.h>
#include <ttyent.h>

struct ttyent *
getttynam (const char *nam)
{
	struct ttyent *ent;
	setttyent ();
	while (ent = getttyent ())
		if (!strcmp(ent->ty_name, nam)) break;
	return ent;
}
