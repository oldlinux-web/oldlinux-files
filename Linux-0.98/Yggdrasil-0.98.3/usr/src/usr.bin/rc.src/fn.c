/*
   fn.c: functions for adding and deleting functions from the symbol table.
   Support for signal handlers is also found here.
*/

#include "rc.h"
#include <signal.h>
#ifdef PROTECT_JOB
#include <sys/ioctl.h>
#endif
#include "utils.h"
#include "status.h"
#include "tree.h"
#include "hash.h"
#include "footobar.h"
#include "walk.h"
#include "nalloc.h"
#include "sigmsgs.h"
#include "builtins.h"
#include "input.h"

static void fn_handler(int);

static Node *handlers[NUMOFSIGNALS], null_function;
static boolean runexit = FALSE;

/* set signals to default values for rc. this means that interactive shells ignore SIGQUIT, etc. */

void inithandler() {
	if (interactive) {
#ifdef PROTECT_JOB
		if (dashell) {
			extern int ioctl(int, long,...);

			setpgrp(rc_pid, rc_pid);
			ioctl(2, TIOCSPGRP, &rc_pid);
		}
#endif

		signal(SIGINT, sig);
		if (!dashdee) {
			signal(SIGQUIT, SIG_IGN);
			handlers[SIGQUIT] = &null_function;
			signal(SIGTERM, SIG_IGN);
			handlers[SIGTERM] = &null_function;
		}
	}
	null_function.type = BODY;
	null_function.u[0].p = null_function.u[1].p = NULL;
}

/* only run this in a child process! resets signals to their default values */

void setsigdefaults() {
	int i;

	/*
	   General housekeeping: (setsigdefaults happens after fork(), so it's a convenient
	   place to clean up)
	*/

	interactive = FALSE;
	if (histstr != NULL) {	/* Close an open history file */
		close(histfd);
		histstr = NULL; /* But prevent re-closing of the same file-descriptor */
	}

	/* Restore signals to SIG_DFL */

	for (i = 1; i < NUMOFSIGNALS; i++) { /* signal 0 is never used (bogus) */
		if (handlers[i] != NULL) {
			handlers[i] = NULL;
			signal(i, SIG_DFL);
		}
	}
	signal(SIGINT, SIG_DFL); /* sigint is special because its handler is normally		*/
				 /* set to NULL, which doesn't get caught in the above loop	*/
	runexit = FALSE; /* No sigexit on subshells */
}

/* rc's exit. if runexit is set, run the sigexit function. */

void rc_exit(int stat) {
	static char *sigexit[2] = { "sigexit", NULL };

	if (runexit) {
		runexit = FALSE;
		funcall(sigexit);
		exit(getstatus());
	} else {
		exit(stat);
	}
}

/* the signal handler for all functions. calls walk() */

static void fn_handler(int s) {
	if (s < 0 || s >= NUMOFSIGNALS)
		rc_error("unknown signal!?");

	signal(s, fn_handler); /* sgi seems to require re-signalling */
	walk(handlers[s], TRUE);
}

/*
   assign a function in Node form. Check to see if the function is also a signal, and set the
   signal vectors appropriately.
*/

void fnassign(char *name, Node *def) {
	Node *newdef = treecpy(def == NULL ? &null_function : def, ealloc); /* important to do the treecopy first */
	Function *new = get_fn_place(name);
	int i;

	new->def = newdef;
	new->extdef = NULL;

	if (strncmp(name, "sig", sizeof "sig" - 3) == 0) { /* slight optimization */
#ifdef NOSIGCLD /* System V machines treat SIGCLD very specially */
		if (streq(name, "sigcld"))
			rc_error("can't trap SIGCLD");
#endif
		if (streq(name, "sigexit"))
			runexit = TRUE;
		for (i = 1; i < NUMOFSIGNALS; i++) /* zero is a bogus signal */
			if (streq(signals[i][0], name)) {
				if (newdef != NULL) {
					handlers[i] = newdef;
					signal(i, fn_handler);
				} else {
					handlers[i] = &null_function;
					signal(i, SIG_IGN);
				}
				break;
			}
	}
}

/* assign a function from the environment. store just the external representation */

void fnassign_string(char *extdef) {
	char *name = get_name(extdef+3); /* +3 to skip over "fn_" */
	Function *new;

	if (name == NULL)
		return;

	new = get_fn_place(name);
	new->def = NULL;
	new->extdef = ecpy(extdef);
}

/* return a function in Node form, evaluating an entry from the environment if necessary */

Node *fnlookup(char *name) {
	Function *look = lookup_fn(name);
	Node *ret;

	if (look == NULL)
		return NULL; /* not found */
	if (look->def != NULL)
		return look->def;
	if (look->extdef == NULL) /* function was set to null, e.g., fn foo {} */
		return &null_function;

	ret = parse_fn(name, look->extdef);

	if (ret == NULL) {
		efree(look->extdef);
		look->extdef = NULL;
		return &null_function;
	} else {
		return look->def = treecpy(ret, ealloc); /* Need to take it out of talloc space */
	}
}

/* return a function in string form (used by makeenv) */

char *fnlookup_string(char *name) {
	Function *look = lookup_fn(name);

	if (look == NULL)
		return NULL;
	if (look->extdef != NULL)
		return look->extdef;
	return look->extdef = fun2str(name, look->def);
}

/*
   remove a function from the symbol table. If it also defines a signal handler, restore the signal handler
   to its default value.
*/

void fnrm(char *name) {
	int i;

	for (i = 1; i < NUMOFSIGNALS; i++) /* signal 0 unused */
		if (streq(signals[i][0], name)) {
			handlers[i] = NULL;
			if (i == SIGINT)
				signal(i, sig); /* restore default signal handler for rc */
			else if ((i == SIGQUIT || i == SIGTERM) && !dashdee) {
				handlers[i] = &null_function;
				signal(i, SIG_IGN);	 /* ditto */
			} else {
				signal(i, SIG_DFL);
			}
		}

	if (streq(name, "sigexit"))
		runexit = FALSE;

	delete_fn(name);
}

void whatare_all_signals() {
	int i;

	for (i = 1; i < NUMOFSIGNALS; i++)
		if (*signals[i][0] != '\0')
			if (handlers[i] == NULL)
				fprint(1, "fn %s\n", signals[i][0]);
			else
				fprint(1, "fn %s {%s}\n", strprint(signals[i][0], FALSE, FALSE), ptree(handlers[i]));
}

void prettyprint_fn(int fd, char *name, Node *n) {
	fprint(fd, "fn %s {%s}\n", strprint(name, FALSE, FALSE), ptree(n));
}
