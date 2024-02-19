#include "jbwrap.h"
#include <stdarg.h>
#include "rc.h"
#include "utils.h"
#include "except.h"
#include "status.h"
#include "hash.h"
#include "input.h"
#include "nalloc.h"

/*
   a return goes back stack frames to the last return. A break does not. A signal
   goes to the last interactive level.
*/

static Estack *estack;

/* add an exception to the input stack. */

void except(enum except e, void *jb, Estack *ex) {
	ex->prev = estack;
	estack = ex;

	switch (estack->e = e) {
	case ARENA:
		estack->b = newblock();
		break;
	case ERROR:
	case BREAK:
	case RETURN:
		estack->interactive = interactive;
		estack->jb = (jbwrap *) jb;
		break;
	case VARSTACK:
		estack->name = (char *) jb;
		break;
	}
}

/* remove an exception, restore last interactive value */

void unexcept() {
	if (estack->e == ERROR)
		interactive = estack->interactive;
	else if (estack->e == ARENA)
		restoreblock(estack->b);
	estack = estack->prev;
}

/*
   Raise an exception. The rules are pretty complicated: you can return from a loop inside a
   function, but you can't break from a function inside of a loop. On errors, rc_raise() goes back
   to the LAST INTERACTIVE stack frame. If no such frame exists, then rc_raise() exits the shell.
   This is what happens, say, when there is a syntax error in a noninteractive shell script. While
   traversing the exception stack backwards, rc_raise() also removes input sources (closing
   file-descriptors, etc.) and pops instances of $* that have been pushed onto the variable stack
   (e.g., for a function call).
*/

void rc_raise(enum except e) {
	if (e == ERROR && rc_pid != getpid())
			exit(1); /* child processes exit on an error/signal */

	for (; estack != NULL; estack = estack->prev)
		if (estack->e != e) {
			if (e == BREAK && estack->e != ARENA)
				rc_error("break outside of loop");
			else if (e == RETURN && estack->e == ERROR) /* can return from loops inside functions */
				rc_error("return outside of function");
			if (estack->e == VARSTACK)
				varrm(estack->name, TRUE);
			else if (estack->e == ARENA)
				restoreblock(estack->b);
		} else {
			if (e == ERROR && !estack->interactive) {
				popinput();
			} else {
				jbwrap *j = estack->jb;

				interactive = estack->interactive;
				estack = estack->prev;
				longjmp(j->j, 1);
			}
		}
	rc_exit(1); /* top of exception stack */
}
