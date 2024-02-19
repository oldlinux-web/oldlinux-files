/* status.c: functions for printing fancy status messages in rc */

#include "rc.h"
#include "utils.h"
#include "status.h"
#include "nalloc.h"
#include "walk.h"
#include "sigmsgs.h"

/* status == the wait() value of the last command in the pipeline, or the last command */

static int statuses[512];
static int pipelength = 1;

/*
   Test to see if rc's status is true. According to td, status is true if and only if every
   pipe-member has an exit status of zero.
*/

int istrue() {
	int i;

	for (i = 0; i < pipelength; i++)
		if (statuses[i] != 0)
			return FALSE;
	return TRUE;
}

/*
   Return the status as an integer. A status which has low-bits set is a signal number,
   whereas a status with high bits set is a value set from exit().
*/

int getstatus() {
	int s = statuses[0];

	return s & 0xff ? s & 0x7f : (s >> 8) & 0xff;
}

void set(boolean code) {
	setstatus((!code) << 8); /* exit status 1 == 0x100 */
}

/* take a pipeline and store the exit statuses. Check to see whether any of the children dumped core */

void setpipestatus(int stats[], int num) {
	int i;

	for (i = 0; i < (pipelength = num); i++) {
		statprint(stats[i]);
		statuses[i] = stats[i];
	}
}

/* set a simple status, as opposed to a pipeline */

void setstatus(int i) {
	pipelength = 1;
	statuses[0] = i;
	statprint(i);
}

/* print a message if termination was with a signal, and if the child dumped core. exit on error if -e is set */

void statprint(int i) {
	if (i & 0xff) {
		char *msg = ((i & 0x7f) < NUMOFSIGNALS ? signals[i & 0x7f][1] : "");

		if (i & 0x80) {
			if (*msg == '\0')
				fprint(2,"core dumped\n");
			else
				fprint(2,"%s--core dumped\n",msg);
		} else if (*msg != '\0')
			fprint(2,"%s\n",msg);
	}

	if (i != 0 && dashee && !cond)
		rc_exit(getstatus());
}

/* prepare a list to be passed back. Used whenever $status is dereferenced */

List *sgetstatus() {
	List *r;
	int i;

	for (r = NULL, i = 0; i < pipelength; i++) {
		char buf[16];
		List *q = nnew(List);
		int s = statuses[i];
		int t;

		q->n = r;
		r = q;

		if ((t = s & 0x7f) != 0) {
			if (t < NUMOFSIGNALS && *signals[t][0] != '\0')
				sprint(buf, "%s", signals[t][0]);
			else
				sprint(buf,"-%d", t); /* unknown signals are negated */
			if (s & 0x80)
				strcat(buf, "+core");
		} else {
			sprint(buf, "%d", (s >> 8) & 0xff);
		}

		r->w = ncpy(buf);
		r->m = NULL;
	}

	return r;
}

void ssetstatus(char **av) {
	int i, j, k, l;
	boolean found;

	for (l = 0; av[l] != NULL; l++)
		; /* count up array length */
	--l;

	for (i = 0; av[i] != NULL; i++) {
		j = a2u(av[i]);

		if (j >= 0) {
			statuses[l - i] = j << 8;
			continue;
		}

		found = FALSE;
		for (k = 0; k < NUMOFSIGNALS; k++) {
			if (streq(signals[k][0], av[i])) {
				statuses[l - i] = k;
				found = TRUE;
				break;
			} else {
				int len = strlen(signals[k][0]);
				if (strncmp(signals[k][0], av[i], len) == 0 && streq(av[i] + len, "+core")) {
					statuses[l - i] = k + 0x80;
					found = TRUE;
					break;
				}
			}
		}
		if (!found) {
			fprint(2, "bad return value\n");
			set(FALSE);
			return;
		}
	}
	pipelength = i;
}
