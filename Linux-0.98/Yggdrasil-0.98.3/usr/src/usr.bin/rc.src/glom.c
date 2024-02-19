/* glom.c: builds an argument list out of words, variables, etc. */

#include <sys/types.h>
#include <sys/stat.h>
#ifndef S_IFIFO
#ifndef DEVFD
#define NOCMDARG
#endif
#endif
#include "rc.h"
#include "utils.h"
#include "nalloc.h"
#include "glom.h"
#include "hash.h"
#include "walk.h"
#include "status.h"
#include "exec.h"
#include "lex.h"
#include "open.h"
#include "list.h"
#include "wait.h"

static List *backq(Node *, Node *);
static List *bqinput(List *, int);
static List *count(List *);
static List *mkcmdarg(Node *);

Rq *redirq = NULL;
Fq *fifoq = NULL;

List *word(char *w, char *m) {
	List *s;

	if (w == NULL)
		return NULL;

	s = nnew(List);
	s->w = w;
	s->m = m;
	s->n = NULL;
	return s;
}

/*
   Append list s2 to list s1 by copying s1 and making the new copy
   point at s2.
*/

List *append(List *s1, List *s2) {
	List *r, *top;

	if (s1 == NULL)
		return s2;
	if (s2 == NULL)
		return s1;

	r = top = nnew(List);
	while (1) {
		r->w = s1->w;
		r->m = s1->m;
		if ((s1 = s1->n) == NULL)
			break;
		r = r->n = nnew(List);
	}

	r->n = s2;

	return top;
}

List *concat(List *s1, List *s2) {
	int n1, n2;
	SIZE_T y,z;
	List *n, *s;

	if (s1 == NULL)
		return s2;
	if (s2 == NULL)
		return s1;

	n1 = listnel(s1);
	n2 = listnel(s2);

	if (n1 != n2 && n1 != 1 && n2 != 1)
		rc_error("bad concatenation");

	n = s = nnew(List);

	while (1) {
		z = strlen(s1->w) + strlen(s2->w) + 1;
		n->w = nalloc(z);
		strcpy(n->w,s1->w);
		strcat(n->w,s2->w);
		if (s1->m == NULL && s2->m == NULL) {
			n->m = NULL;
		} else {
			n->m = nalloc(z);
			y = strlen(s1->w);
			if (s1->m == NULL)
				clear(n->m, y);
			else
				memcpy(n->m, s1->m, y);
			if (s2->m == NULL)
				clear(n->m + y, strlen(s2->w));
			else
				memcpy(n->m + y, s2->m, strlen(s2->w));
			n->m[z] = 0;
		}
		if (n1 > 1)
			s1 = s1->n;
		if (n2 > 1)
			s2 = s2->n;
		if (s1 == NULL || s2 == NULL || (n1 == 1  && n2 == 1)) {
			n->n = NULL;
			return s;
		}
		n->n = nnew(List);
		n = n->n;
	}
}

List *varsub(List *v, List *subs) {
	int i,j;
	int n;
	List *r,*s;
	List *top,*cat;

	n = listnel(v);

	top = cat = NULL;

	for (s = subs; s != NULL; s = s->n) {
		i = a2u(s->w);
		if (i < 1)
			rc_error("bad subscript");
		if (i <= n) {
			for (j = 1, r = v; j != i; j++, r = r->n)
				; /* loop until r == v(i) */
			if (top == NULL) {
				top = cat = nnew(List);
			} else {
				cat->n = nnew(List);
				cat = cat->n;
			}
			cat->w = r->w;
			cat->m = r->m;
		}
	}

	if (top == NULL)
		return NULL;

	cat->n = NULL;
	return top;
}

List *flatten(List *s) {
	List *r;

	if (s == NULL || s->n == NULL)
		return s;

	r = nnew(List);
	r->w = nalloc(listlen(s) + 1);
	r->m = NULL; /* flattened lists come from variables, so no meta */
	r->n = NULL;

	strcpy(r->w, s->w);

	do {
		s = s->n;
		strcat(r->w, " ");
		strcat(r->w, s->w);
	} while (s->n != NULL);

	return r;
}

static List *count(List *l) {
	List *s = nnew(List);
	char buf[16];

	s->w = sprint(buf, "%d", listnel(l));
	s->w = ncpy(s->w);
	s->n = NULL;
	s->m = NULL;
	return s;
}

void assign(List *s1, List *s2, boolean stack) {
	List *val = s2;

	if (s1 == NULL)
		rc_error("null variable name");
	if (s1->n != NULL)
		rc_error("multi-word variable name");
	if (*s1->w == '\0')
		rc_error("zero-length variable name");
	if (a2u(s1->w) != -1)
		rc_error("numeric variable name");
	if (strchr(s1->w, '=') != NULL)
		rc_error("'=' in variable name");
	if (s1->w[0] == '*' && s1->w[1] == '\0')
		val = append(varlookup("0"), s2); /* preserve $0 when * is assigned explicitly */


	if (s2 != NULL || stack) {
		if (dashex)
			prettyprint_var(2, s1->w, val);
		varassign(s1->w, val, stack);
		alias(s1->w, val, stack);
	} else {
		if (dashex)
			prettyprint_var(2, s1->w, NULL);
		varrm(s1->w, stack);
	}
}

/*
   The following two functions are by the courtesy of Paul Haahr,
   who could not stand the incompetence of my own backquote implementation.
*/

#define BUFSIZE	((SIZE_T) 1000)

static List *bqinput(List *ifs, int fd) {
	char *end, *bufend, *s;
	List *r, *top, *prev;
	SIZE_T remain, bufsize;
	char isifs[256];
	int n;
	int state; /* a simple FSA is used to read in data */

	clear(isifs, sizeof isifs);
	isifs['\0'] = 1;
	for (r = ifs; r != NULL; r = r->n)
		for (s = r->w; *s != '\0'; s++)
			isifs[*(unsigned char *)s] = 1;

	remain = bufsize = BUFSIZE;
	top = r = nnew(List);
	r->w = end = nalloc(bufsize + 1);
	r->m = NULL;
	state = 0;
	prev = NULL;

	while (1) {
		if (remain == 0) { /* is the string bigger than the buffer? */
			SIZE_T m = end - r->w;
			char *buf;

			while (bufsize < m + BUFSIZE)
				bufsize *= 2;
			buf = nalloc(bufsize + 1);
			memcpy(buf, r->w, m);
			r->w = buf;
			end = &buf[m];
			remain = bufsize - m;
		}

		n = read(fd, end, remain);

		if (n <= 0) {
			if (n == -1) {
				uerror("backquote read");
				rc_error(NULL);
			}
	/* break */	break;
		}

		remain -= n;

		for (bufend = &end[n]; end < bufend; end++)
			switch (state) {
			case 0:
				if (!isifs[*(unsigned char *)end]) {
					state = 1;
					r->w = end;
				}
				break;
			case 1:
				if (isifs[*(unsigned char *)end]) {
					state = 0;
					*end = '\0';
					prev = r;
					r = r->n = nnew(List);
					r->w = end+1;
					r->m = NULL;
				}
				break;
			}
	}

	if (state == 1) { /* terminate last string */
		*end = '\0';
		r->n = NULL;
	} else {
		if (prev == NULL) /* no input at all? */
			top = NULL;
		else
			prev->n = NULL; /* else terminate list */
	}
		
	return top;
}

static List *backq(Node *ifsnode, Node *n) {
	int p[2], pid, sp;
	List *result, *ifs;

	if (n == NULL)
		return NULL;

	if (pipe(p) < 0) {
		uerror("pipe");
		rc_error(NULL);
	}

	if ((pid = rc_fork()) == 0) {
		setsigdefaults();
		dup2(p[1],1);
		close(p[0]);
		close(p[1]);
		redirq = NULL;
		fifoq = NULL;
		walk(n, FALSE);
		exit(getstatus());
	}

	ifs = glom(ifsnode);
	close(p[1]);
	result = bqinput(ifs, p[0]);
	close(p[0]);
	rc_wait4(pid, &sp);
	statprint(sp);
	return result;
}

void qredir(Node *n) {
	Rq *next;

	if (redirq == NULL) {
		next = redirq = nnew(Rq);
	} else {
		for (next = redirq; next->n != NULL; next = next->n)
			;
		next->n = nnew(Rq);
		next = next->n;
	}

	next->r = n;
	next->n = NULL;
}

#ifdef NOCMDARG
static List *mkcmdarg(Node *n) {
	rc_error("named pipes are not supported");
	return NULL;
}
#else
#ifndef DEVFD
static List *mkcmdarg(Node *n) {
	int fd, pid;
	char *name, buf[32];
	List *ret = nnew(List);
	Fq *f = nnew(Fq);
	static int fifonumber = 0;

	name = sprint(buf,"/tmp/rc%d.%d", getpid(), fifonumber++);
	name = ncpy(name);

	if (mknod(name, S_IFIFO | 0644, 0) < 0) {
		uerror("mknod");
		return NULL;
	}

	if ((pid = rc_fork()) == 0) {
		setsigdefaults();
		/* fd = rc_open(name, CREATE); */
		fd = rc_open(name, (n->u[0].i != FROM) ? FROM : CREATE); /* stupid hack */
		if (fd < 0) {
			uerror("open");
			exit(1);
		}
		if (dup2(fd, (n->u[0].i == FROM)) < 0) { /* same stupid hack */
			uerror("dup2");
			exit(1);
		}
		close(fd);
		redirq = NULL;
		fifoq = NULL;
		walk(n->u[2].p, FALSE);
		exit(getstatus());
	}

	f->pid = pid;
	f->name = name;
	f->n = fifoq;
	fifoq = f;

	ret->w = name;
	ret->m = NULL;
	ret->n = NULL;

	return ret;
}
#else
static List *mkcmdarg(Node *n) {
	char *name, buf[32];
	List *ret = nnew(List);
	int p[2];

	if (pipe(p) < 0) {
		uerror("pipe");
		return NULL;
	}

	if (rc_fork() == 0) {
		setsigdefaults();

		if (dup2(p[n->u[0].i == FROM], n->u[0].i == FROM) < 0) { /* stupid hack */
			uerror("dup2");
			exit(1);
		}
		close(p[n->u[0].i != FROM]);

		redirq = NULL;
		walk(n->u[2].p, FALSE);
		exit(getstatus());
	}

	name = sprint(buf, "/dev/fd/%d", p[n->u[0].i != FROM]);
	name = ncpy(name); /* ncpy evaluates the expression twice */
	ret->w = name;
	ret->m = NULL;
	ret->n = NULL;

	close(p[n->u[0].i == FROM]);

	return ret;
}
#endif /* DEVFD */
#endif /* !NOCMDARG */

List *glom(Node *n) {
	Node *words;
	List *v, *first, *last;
	boolean dollarstar;

	if (n == NULL)
		return NULL;

	switch (n->type) {
	case ARGS:
	case LAPPEND:
		words = n->u[0].p;
		last = NULL;
		while (words != NULL && (words->type == ARGS || words->type == LAPPEND)) {
			if (words->u[1].p != NULL && words->u[1].p->type != rWORD && words->u[1].p->type != QWORD)
				break;
			first = glom(words->u[1].p);
			if (first != NULL) {
				first->n = last;
				last = first;
			}
			words = words->u[0].p;
		}
		v = append(glom(words), last); /* force left to right evaluation */
		return append(v, glom(n->u[1].p));
	case BACKQ:
		return backq(n->u[0].p,n->u[1].p);
	case CONCAT:
		first = glom(n->u[0].p); /* force left-to-right evaluation */
		return concat(first, glom(n->u[1].p));
	case rDUP:
	case rREDIR:
		qredir(n);
		return NULL;
	case rWORD: case QWORD:
		return word(n->u[0].s,n->u[1].s);
	case NMPIPE:
		return mkcmdarg(n);
	default:
		break;
	}

	/*
           the next three operations depend on the left-child of glom
	   to be a variable name. Therefore they are all treated here.
	   (previously each function looked up and checked the validity
	   of a variable name)
	*/

	v = glom(n->u[0].p);
	if (v == NULL)
		rc_error("null variable name");
	if (v->n != NULL)
		rc_error("multi-word variable name");
	if (*v->w == '\0')
		rc_error("zero-length variable name");

	dollarstar = (v->w[0] == '*' && v->w[1] == '\0');
	v = varlookup(v->w);
	if (dollarstar)
		v = v->n;

	switch (n->type) {
	default:
		fprint(2,"glom: this can't happen\n");
		exit(1);
		/* NOTREACHED */
	case rCOUNT:
		return count(v);
	case rFLAT:
		return flatten(v);
	case VAR:
		return v;
	case VARSUB:
		return varsub(v, glom(n->u[1].p));
	}

}
