#include <errno.h>
#include "rc.h"
#include "utils.h"
#include "nalloc.h"
#include "wait.h"

typedef struct Pid Pid;

static struct Pid {
	int p;
	int stat;
	boolean alive;
	Pid *n;
} *plist = NULL;

int rc_fork() {
	Pid *new = enew(Pid);
	int pid = fork();

	switch (pid) {
	case -1:
		uerror("fork");
		rc_error(NULL);
		/* NOTREACHED */
	case 0:
		return 0;
	default:
		new->p = pid;
		new->alive = TRUE;
		new->n = plist;
		plist = new;
		return pid;
	}
}

int rc_wait4(int pid, int *stat) {
	Pid *r, *prev;
	int ret;

	/* first look for a child which may already have exited */

again:	for (r = plist, prev = NULL; r != NULL; prev = r, r = r->n)
		if (r->p == pid)
			break;

	if (r == NULL) {
		errno = ECHILD; /* no children */
		uerror("wait");
		*stat = 0x100; /* exit(1) */
		return -1;
	}

	if (r->alive) {
		while (pid != (ret = wait(stat))) {
			Pid *q;

			if (ret < 0)		/* if rc is interrupted, a signal handler may have already */
				goto again;	/* wait()ed while the main rc was in limbo, so try again   */

			for (q = plist; q != NULL; q = q->n)
				if (q->p == ret) {
					q->alive = FALSE;
					q->stat = *stat;
					break;
				}
		}
	} else
		*stat = r->stat;

	if (prev == NULL)
		plist = r->n; /* remove element from head of list */
	else
		prev->n = r->n;
	efree(r);

	return pid;
}

List *sgetapids() {
	List *r;
	Pid *p;

	for (r = NULL, p = plist; p != NULL; p = p->n) {
		char buf[16];
		List *q;

		if (!p->alive)
			continue;

		sprint(buf, "%d", p->p);
		q = nnew(List);
		q->w = ncpy(buf);
		q->m = NULL;
		q->n = r;
		r = q;
	}

	return r;
}

void waitforall(int *stat) {
	while (plist != NULL)
		rc_wait4(plist->p, stat);
}
