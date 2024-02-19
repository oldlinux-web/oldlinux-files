/* open.c: to insulate <fcntl.h> from the rest of rc. */

#include <fcntl.h>
#include "lex.h"
#include "open.h"

/* prototype for open() follows. comment out if necessary */

/*extern int open(const char *, int,...);*/
extern void rc_error(const char *);

/*
   Opens a file with the necessary flags. Assumes the following
   declaration for enum redirtype:

	enum redirtype {
		FROM, CREATE, APPEND, HEREDOC, HERESTRING
	};
*/

static const int mode_masks[] = {
	/* read */	O_RDONLY,
	/* create */	O_TRUNC | O_CREAT | O_WRONLY,
	/* append */	O_APPEND | O_CREAT | O_WRONLY
};

int rc_open(const char *name, enum redirtype m) {
	if ((unsigned int) m >= (sizeof(mode_masks)/sizeof(int)))
		rc_error("bad mode passed to rc_open");

	return open(name, mode_masks[m], 0644);
}
