/* redir.c: code for opening files and piping heredocs after fork but before exec. */

#include "rc.h"
#include "lex.h"
#include "glom.h"
#include "glob.h"
#include "open.h"
#include "exec.h"
#include "utils.h"
#include "redir.h"
#include "hash.h"

/*
   Walk the redirection queue, and open files and dup2 to them. Also, here-documents are treated
   here by dumping them down a pipe. (this should make here-documents fast on systems with lots
   of memory which do pipes right. Under sh, a file is copied to /tmp, and then read out of /tmp
   again. I'm interested in knowing how much faster, say, shar runs when unpacking when invoked
   with rc instead of sh. On my sun4/280, it runs in about 60-75% of the time of sh for unpacking
   the rc source distribution.)
*/

void doredirs() {
	List *fname;
	int fd, p[2];
	Rq *r;

	for (r = redirq; r != NULL; r = r->n) {
		switch(r->r->type) {
		default:
			fprint(2,"%d: bad node in doredirs\n", r->r->type);
			exit(1);
			/* NOTREACHED */
		case rREDIR:
			if (r->r->u[0].i == HERESTRING) {
				fname = flatten(glom(r->r->u[2].p)); /* fname is really a string */
				if (fname == NULL) {
					close(r->r->u[1].i); /* feature? */
					break;
				}
				if (pipe(p) < 0) {
					uerror("pipe");
					exit(1);
				}
				switch (fork()) {
				case -1:
					uerror("fork");
					exit(1);
					/* NOTREACHED */
				case 0:	/* child writes to pipe */
					setsigdefaults();
					close(p[0]);
					writeall(p[1], fname->w, strlen(fname->w));
					exit(0);
					/* NOTREACHED */
				default:
					close(p[1]);
					if (dup2(p[0], r->r->u[1].i) < 0) {
						uerror("dup");
						exit(1);
					}
					close(p[0]);
				}
			} else {
				fname = glob(glom(r->r->u[2].p));
				if (fname == NULL) {
					fprint(2,"null filename in redirection\n");
					exit(1);
				}
				if (fname->n != NULL) {
					fprint(2,"multi-word filename in redirection\n");
					exit(1);
				}
				switch (r->r->u[0].i) {
				default:
					fprint(2,"doredirs: this can't happen\n");
					exit(1);
					/* NOTREACHED */
				case CREATE: case APPEND: case FROM:
					fd = rc_open(fname->w, r->r->u[0].i);
					break;
				}
				if (fd < 0) {
					uerror(fname->w);
					exit(1);
				}
				if (dup2(fd, r->r->u[1].i) < 0) {
					uerror("dup");
					exit(1);
				}
				close(fd);
			}
			break;
		case rDUP:
			if (r->r->u[2].i == -1)
				close(r->r->u[1].i);
			else if (dup2(r->r->u[2].i, r->r->u[1].i) < 0) {
				uerror("dup");
				exit(1);
			}
		}
	}
	redirq = NULL;
}
