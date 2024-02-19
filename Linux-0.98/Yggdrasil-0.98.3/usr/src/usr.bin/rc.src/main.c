/* main.c: handles initialization of rc and command line options */

#include <stdarg.h>
#include "rc.h"
#include "utils.h"
#include "input.h"
#include "nalloc.h"
#include "hash.h"
#include "lex.h"
#include "open.h"
#include "tree.h"
#include "glom.h"
#include "builtins.h"
#include "parse.h"
#include "status.h"
#include "getopt.h"

boolean dashdee, dashee, dashvee, dashex, dashell, dasheye, dashen, interactive;
int rc_pid;

#define REALLYNULL ((void *) 0) /* used to terminate a vararg list with NULL */

static void assigndefault(char *,...);

void main(int argc, char *argv[], char *envp[]) {
	char *dashsee[2], pid[8], *dollarzero, *null[1];
	int c;

	dashee = dashell = dashvee = dashex = dashdee = dashen = FALSE;
	dashsee[0] = dashsee[1] = NULL;
	dollarzero = argv[0];
	rc_pid = getpid();

	dashell = (*argv[0] == '-'); /* Unix tradition */

	while ((c = getopt(argc, argv, "nleivdxc:")) != -1)
		switch (c) {
		case 'l':
			dashell = TRUE;
			break;
		case 'e':
			dashee = TRUE;
			break;
		case 'i':
			dasheye = interactive = TRUE;
			break;
		case 'v':
			dashvee = TRUE;
			break;
		case 'x':
			dashex = TRUE;
			break;
		case 'd':
			dashdee = TRUE;
			break;
		case 'c':
			dashsee[0] = optarg;
			goto quitopts;
		case 'n':
			dashen = TRUE;
			break;
		case '?':
			exit(1);
		}

quitopts:
	argv += optind;

	/* use isatty() iff -i is not set, and iff the input is not from a script or -c flag */
	if (!dasheye && dashsee[0] == NULL && *argv == NULL)
		interactive = isatty(0);

	if (dashsee[0] != NULL) { /* rc -c must always ignore signals, e.g., for "rc -c sh" */
		int i = interactive;
	
		interactive = TRUE;
		inithandler();
		interactive = i;
	} else
		inithandler();
	inithash();
	initparse();
	assigndefault("prompt", "; ", "", REALLYNULL);
	assigndefault("path", ".", "/bin", "/usr/bin", "/usr/ucb", REALLYNULL);
	assigndefault("ifs", " ", "\t", "\n", REALLYNULL);
	assigndefault("pid", sprint(pid, "%d", rc_pid), REALLYNULL);
	initenv(envp);
	initinput();
	null[0] = NULL;
	starassign(dollarzero, null, FALSE); /* assign $0 to $* */

	if (dashsee[0] != NULL) {	/* input from the -c flag? */
		if (*argv != NULL)
			starassign(dollarzero, argv, FALSE);
		pushinput(STRING, dashsee, TRUE);
	} else if (*argv != NULL) {	/* else from a file? */
		b_dot(--argv);
		rc_exit(getstatus());
	} else {			/* else stdin */
		pushinput(FD, 0);
	}

	dasheye = FALSE;
	doit(TRUE);
	rc_exit(getstatus());
}

static void assigndefault(char *name,...) {
	va_list ap;
	List *l;
	char *v;

	va_start(ap, name);

	for (l = NULL; (v = va_arg(ap, char *)) != NULL;)
		l = append(l, word(v, NULL));

	varassign(name, l, FALSE);

	if (streq(name,"path"))
		alias(name, l, FALSE);

	va_end(ap);
}
