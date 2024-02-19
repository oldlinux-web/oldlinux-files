/* walk.c: walks the parse tree. */

#include "jbwrap.h"
#include <signal.h>
#include "rc.h"
#include "utils.h"
#include "status.h"
#include "exec.h"
#include "walk.h"
#include "glom.h"
#include "hash.h"
#include "glob.h"
#include "lex.h"
#include "open.h"
#include "except.h"
#include "wait.h"

boolean cond = FALSE;

static boolean isallpre(Node *);
static boolean dofork(void);
static void dopipe(Node *);

/* Tail-recursive version of walk() */

#define WALK(x,y) { n = x; parent = y; goto top; }

/* walk the parse-tree. "obvious". */

boolean walk(Node *n, boolean parent) {
top:	if (n == NULL) {
		if (!parent)
			exit(0);
		set(TRUE);
		return TRUE;
	}

	switch (n->type) {
	case ARGS: case BACKQ: case CONCAT: case rCOUNT:
	case rFLAT: case LAPPEND: case rREDIR: case VAR:
	case VARSUB: case rWORD: case QWORD:
		exec(glob(glom(n)), parent);	/* simple command */
		break;
	case BODY:
		walk(n->u[0].p, TRUE);
		WALK(n->u[1].p, TRUE);
		break;
	case NOWAIT: {
		int pid;
		char apid[8];

		if ((pid = rc_fork()) == 0) {
			setsigdefaults();
#if defined(SIGTTOU) && defined(SIGTTIN) && defined(SIGTSTP)
			signal(SIGTTOU, SIG_IGN);	/* Berkeleyized version: put it in a new pgroup. */
			signal(SIGTTIN, SIG_IGN);
			signal(SIGTSTP, SIG_IGN);
			setpgrp(0, getpid());
#else
			signal(SIGINT, SIG_IGN);	/* traditional backgrounding procedure: ignore SIGINT */
#endif
			dup2(rc_open("/dev/null", FROM), 0);
			walk(n->u[0].p, FALSE);
			exit(getstatus());
		}

		if (interactive)
			fprint(2,"%d\n",pid);
		varassign("apid", word(sprint(apid,"%d",pid), NULL), FALSE);
		redirq = NULL; /* kill pre-redir queue */
		fifoq = NULL;
		break;
	}
	case rANDAND: {
		boolean oldcond = cond;

		cond = TRUE;
		if (walk(n->u[0].p, TRUE)) {
			cond = oldcond;
			WALK(n->u[1].p, TRUE);
		} else
			cond = oldcond;
		break;
	}
	case rOROR: {
		boolean oldcond = cond;

		cond = TRUE;
		if (!walk(n->u[0].p, TRUE)) {
			cond = oldcond;
			WALK(n->u[1].p, TRUE);
		} else
			cond = oldcond;
		break;
	}
	case rBANG:
		set(!walk(n->u[0].p, TRUE));
		break;
	case rIF: {
		boolean oldcond = cond;
		Node *true_cmd = n->u[1].p, *false_cmd = NULL;

		if (true_cmd != NULL && true_cmd->type == rELSE) {
			false_cmd = true_cmd->u[1].p;
			true_cmd = true_cmd->u[0].p;
		}
		cond = TRUE;
		if (!walk(n->u[0].p, TRUE))
			true_cmd = false_cmd; /* run the else clause */
		cond = oldcond;
		WALK(true_cmd, TRUE);
		break;
	}
	case rWHILE: {
		jbwrap j;
		boolean oldcond = cond;
		Estack e1,e2;

		cond = TRUE;

		if (!walk(n->u[0].p, TRUE)) { /* prevent spurious breaks inside test */
			cond = oldcond;
			break;
		}

		if (setjmp(j.j))
			break;

		except(BREAK, &j, &e1);
		do {
			cond = oldcond;
			except(ARENA, NULL, &e2);
			walk(n->u[1].p, TRUE);
			unexcept(); /* ARENA */
			cond = TRUE;
		} while (walk(n->u[0].p, TRUE));
		cond = oldcond;
		unexcept(); /* BREAK */
		break;
	}
	case FORIN: {
		List *l;
		jbwrap j;
		Estack e1,e2;

		if (setjmp(j.j))
			break;

		except(BREAK, &j, &e1);
		for (l = glob(glom(n->u[1].p)); l != NULL; l = l->n) {
			assign(glom(n->u[0].p), word(l->w, NULL), FALSE);
			except(ARENA, NULL, &e2);
			walk(n->u[2].p, TRUE);
			unexcept(); /* ARENA */
		}
		unexcept(); /* BREAK */
		break;
	}
	case rSUBSHELL:
		if (dofork()) {
			setsigdefaults();
			walk(n->u[0].p, TRUE);
			rc_exit(getstatus());
		}
		break;
	case ASSIGN:
		if (n->u[0].p == NULL)
			rc_error("null variable name");
		assign(glom(n->u[0].p), glob(glom(n->u[1].p)), FALSE);
		set(TRUE);
		break;
	case rPIPE:
		dopipe(n);
		break;
	case NEWFN: {
		List *l = glom(n->u[0].p);

		if (l == NULL)
			rc_error("null function name");
		while (l != NULL) {
			if (dashex)
				prettyprint_fn(2, l->w, n->u[1].p);
			fnassign(l->w, n->u[1].p);
			l = l->n;
		}
		set(TRUE);
		break;
	}
	case RMFN: {
		List *l = glom(n->u[0].p);

		while (l != NULL) {
			if (dashex)
				fprint(2, "fn %s\n", strprint(l->w, FALSE, FALSE));
			fnrm(l->w);
			l = l->n;
		}
		set(TRUE);
		break;
	}
	case rDUP:
		break; /* Null command */
	case MATCH: {
		List *a = glob(glom(n->u[0].p)), *b = glom(n->u[1].p);

		if (dashex)
			fprint(2, (a != NULL && a->n != NULL) ? "~ (%l) %l\n" : "~ %l %l\n", a, b);
		set(lmatch(a,b));
		break;
	}
	case rSWITCH: {
		List *v = glom(n->u[0].p);

		while (1) {
			do {
				n = n->u[1].p;
				if (n == NULL)
					return istrue();
			} while (n->u[0].p == NULL || n->u[0].p->type != rCASE);
			if (lmatch(v, glom(n->u[0].p->u[0].p))) {
				for (n = n->u[1].p; n != NULL && (n->u[0].p == NULL || n->u[0].p->type != rCASE); n = n->u[1].p)
					walk(n->u[0].p, TRUE);
				break;
			}
		}
		break;
	}
	case PRE: {
		List *v;

		if (n->u[0].p->type == rREDIR) {
			qredir(n->u[0].p);
			walk(n->u[1].p, TRUE);
			break;
		} else if (n->u[0].p->type == ASSIGN) {
			if (isallpre(n->u[1].p)) {
				walk(n->u[0].p, TRUE);
				walk(n->u[1].p, TRUE);
				break;
			} else {
				Estack e;
				
				v = glom(n->u[0].p->u[0].p);
				assign(v, glob(glom(n->u[0].p->u[1].p)), TRUE);
				except(VARSTACK, v->w, &e);
				walk(n->u[1].p, TRUE);
				varrm(v->w, TRUE);
				unexcept(); /* VARSTACK */
			}
		} else
			rc_error("walk: node other than assign or redir in PRE. help!");
		break;
	}
	case BRACE:
		if (n->u[1].p == NULL) {
			WALK(n->u[0].p, TRUE);
		} else if (dofork()) {
			setsigdefaults();
			walk(n->u[1].p, TRUE); /* Do redirections */
			redirq = NULL;   /* Reset redirection queue */
			walk(n->u[0].p, TRUE); /* Do commands */
			rc_exit(getstatus());
			/* NOTREACHED */
		}
		break;
	case EPILOG:
		qredir(n->u[0].p);
		if (n->u[1].p != NULL) {
			WALK(n->u[1].p, TRUE); /* Do more redirections. */
		} else {
			doredirs();	/* Okay, we hit the bottom. */
		}
		break;
	case NMPIPE:
		rc_error("named pipes cannot be executed as commands");
		/* NOTREACHED */
	default:
		rc_error("walk: unknown node; this can't happen");
		/* NOTREACHED */
	}
	return istrue();
}

/* checks to see whether a subtree is all pre-command directives, i.e., assignments and redirs only */

static boolean isallpre(Node *n) {
	if (n == NULL)
		return TRUE;

	switch (n->type) {
	case PRE:
		return isallpre(n->u[1].p);
	case rREDIR: case ASSIGN: case rDUP:
		return TRUE;
	default:
		return FALSE;
	}
}

/*
   A code-saver. Forks, child returns (for further processing in walk()), and the parent
   waits for the child to finish, setting $status appropriately.
*/

static boolean dofork() {
	int pid, sp;

	if ((pid = rc_fork()) == 0)
		return TRUE;

	redirq = NULL; /* clear out the pre-redirection queue in the parent */
	fifoq = NULL;
	rc_wait4(pid, &sp);
	setstatus(sp);
	return FALSE;
}

static void dopipe(Node *n) {
	int i, j, sp, pid, fd_prev, fd_out, pids[512], stats[512], p[2];
	boolean intr;
	void (*handler)(int);
	Node *r;

	fd_prev = fd_out = 1;

	for (r = n, i = 0; r != NULL && r->type == rPIPE; r = r->u[2].p, i++) {
		if (i > 500) /* the only hard-wired limit in rc? */
			rc_error("pipe too long");

		if (pipe(p) < 0) {
			uerror("pipe");
			rc_error(NULL);
		}

		if ((pid = rc_fork()) == 0) {
			setsigdefaults();
			redirq = NULL; /* clear preredir queue */
			fifoq = NULL;
			dup2(p[0],r->u[1].i);
			if (fd_prev != 1) {
				dup2(fd_prev, fd_out);
				close(fd_prev);
			}
			close(p[0]);
			close(p[1]);
			walk(r->u[3].p, FALSE);
			exit(getstatus());
		}

		if (fd_prev != 1)
			close(fd_prev); /* parent must close all pipe fd's */
		pids[i] = pid;
		fd_prev = p[1];
		fd_out = r->u[0].i;
		close(p[0]);
	}

	if ((pid = rc_fork()) == 0) {
		setsigdefaults();
		dup2(fd_prev, fd_out);
		close(fd_prev);
		walk(r, FALSE);
		exit(getstatus());
		/* NOTREACHED */
	}

	redirq = NULL; /* clear preredir queue */
	fifoq = NULL;
	close(fd_prev);
	pids[i++] = pid;

	/* collect statuses */

	intr = FALSE;

	if ((handler = signal(SIGINT, SIG_IGN)) == SIG_DFL)
		signal(SIGINT, SIG_DFL); /* don't ignore interrupts in noninteractive mode */

	for (j = 0; j < i; j++) {
		rc_wait4(pids[j], &sp);
		stats[j] = sp;
		intr |= (sp == SIGINT);
	}

	setpipestatus(stats, i);
	signal(SIGINT, handler);
	if (intr) /* interrupted during pipe? let the signal handler deal with it */
		rc_raise(ERROR);
}
