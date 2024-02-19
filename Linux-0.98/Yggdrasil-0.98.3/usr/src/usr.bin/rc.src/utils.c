/* utils.c: general utility functions like fprint, ealloc etc. */

#include <stdarg.h>
#include <errno.h>
#include "jbwrap.h"
#include <signal.h>
#include "rc.h"
#include "utils.h"
#include "nalloc.h"
#include "status.h"
#include "input.h"
#include "except.h"
#include "lex.h"	/* import char nw[]; used by strprint to see if it needs to quote a word */
#include "walk.h"
#include "footobar.h"
#include "glom.h"
#include "hash.h"
#include "wait.h"

static void dprint(va_list, char *, char *);
static int n2u(char *, int);

/* exception handlers */

void pr_error(char *s) {
	if (s != NULL) {
		if (interactive)
			fprint(2,"%s\n",s);
		else
			fprint(2,"line %d: %s\n", lineno - 1, s);
	}
}

void rc_error(char *s) {
	pr_error(s);
	set(FALSE);
	redirq = NULL;
	cond = FALSE; /* no longer inside conditional */
	empty_fifoq();
	rc_raise(ERROR);
}

void sig(int s) {
	signal(SIGINT, sig); /* some unices require re-signaling */

	if (errno == EINTR)
		return; /* allow wait() to complete */

	fprint(2,"\n"); /* this is the newline you see when you hit ^C while typing a command */
	redirq = NULL;
	cond = FALSE;
	empty_fifoq();
	rc_raise(ERROR);
}

/* our perror */

void uerror(char *s) {
	extern int sys_nerr;
	extern char *sys_errlist[];

	if (errno > sys_nerr)
		return;

	if (s != NULL)
		fprint(2,"%s: %s\n",s,sys_errlist[errno]);
	else
		fprint(2,"%s\n",sys_errlist[errno]);
}

/* printing functions */

void fprint(int fd, char *f,...) {
        va_list ap;
	char str[FPRINT_SIZE];

	va_start(ap,f);
	dprint(ap, str, f);
	va_end(ap);
	writeall(fd,str,strlen(str));
}

char *sprint(char *b, char *f,...) {
	va_list ap;

	va_start(ap, f);
	dprint(ap, b, f);
	va_end(ap);
	return b;
}

static void dprint(va_list ap, char *strbuf, char *f) {
	int i;

	for (i = 0; *f != '\0'; f++) {
		if (*f != '%') {
			strbuf[i++] = *f;
			continue; /* avoid an ugly extra level of indentation */
		}
		switch (*++f) {
		case 'a': {
			char **a = va_arg(ap, char **);

			if (*a == NULL)
				break;
			strcpy(strbuf + i, *a);
			i += strlen(*a);
			while (*++a != NULL) {
				strbuf[i++] = ' ';
				strcpy(strbuf + i, *a);
				i += strlen(*a);
			}
			break;
		}
		case 'c':
			strbuf[i++] = va_arg(ap, int);
			break;
		case 'd': case 'o': {
			int v = va_arg(ap, int);
			int j = 0;
			int base = (*f == 'd' ? 10 : 8);
			char num[16];

			if (v == 0)
				num[j++] = '0';
			while (v != 0) {
				num[j++] = (v % base) + '0';
				v /= base;
			}
			while (--j >= 0)
				strbuf[i++] = num[j];
			break;
		}
		case 's': {
			char *s = va_arg(ap, char *);
			while (*s != '\0')
				strbuf[i++] = *s++;
				break;
		}
		case 'l': {
			List *s, *l = va_arg(ap, List *);

			if (l == NULL) {
				strbuf[i++] = '(';
				strbuf[i++] = ')';
				break;
			}
			for (s = l; s != NULL; s = s->n) {
				strcpy(strbuf + i, strprint(s->w, FALSE, TRUE));
				while (strbuf[i] != '\0')
					i++;
				if (s->n != NULL)
					strbuf[i++] = ' ';
			}
			break;
		}
		default: /* on format error, just print the bad format */
			strbuf[i++] = '%';
			/* FALLTHROUGH */
		case '%':
			strbuf[i++] = *f;
		}
	}
	strbuf[i] = '\0';
}

/* prints a string in rc-quoted form. e.g., a string with spaces in it must be quoted */

char *strprint(char *s, int quotable, int metaquote) { /* really boolean, but y.tab.c includes utils.h */
	SIZE_T i,j;
	char *t;

	if (*s == '\0')
		return "''";

	for (i = 0; s[i] != '\0'; i++)
		if (nw[(unsigned char) s[i]] == 1 || (metaquote && (s[i] == '*' || s[i] == '?' || s[i] == '[')))
			quotable = TRUE;

	if (!quotable)
		return s; /* If the string doesn't need quotes, return */

	for (i = j = 0; s[i] != '\0'; i++, j++)
		if (s[i] == '\'')
			j++;

	t = nalloc(j + 3);

	t[0] = '\'';

	for (j = 1, i = 0; s[i] != '\0'; i++, j++) {
		t[j] = s[i];
		if (s[i] == '\'')
			t[++j] = '\'';
	}

	t[j++] = '\'';
	t[j] = '\0';

	return t;
}

/* ascii -> unsigned conversion routines. -1 indicates conversion error. */

static int n2u(char *s, int base) {
	int i;

	for (i = 0; *s != '\0'; s++) {
		/* small hack with unsigned ints -- one compare for range test */
		if (((unsigned int) *s) - '0' >= (unsigned int) base)
			return -1;
		i = (i * base) + (*s - '0');
	}
	return i;
}

/* decimal -> uint */

int a2u(char *s) {
	return n2u(s, 10);
}

/* octal -> uint */

int o2u(char *s) {
	return n2u(s, 8);
}

/* memory allocation functions */

void *ealloc(SIZE_T n) {
	void *p = malloc(n);

	if (p == NULL) {
		uerror("malloc");
		rc_exit(1);
	}

	return p;
}

void *erealloc(void *p, SIZE_T n) {
	p = realloc(p, n);

	if (p == NULL) {
		uerror("realloc");
		rc_exit(1);
	}

	return p;
}

void efree(void *p) {
	if (p != NULL)
		free(p);
}

/* useful functions */

/* The last word in portable ANSI: a strcmp wrapper for qsort */

int starstrcmp(const void *s1, const void *s2) {
	return strcmp(*(char **)s1, *(char **)s2);
}

/* tests to see if pathname begins with "/", "./", or "../" */

int isabsolute(char *path) {
	return path[0] == '/' || (path[0] == '.' && (path[1] == '/' || (path[1] == '.' && path[2] == '/')));
}

/* write a given buffer allowing for partial writes from write(2) */

void writeall(int fd, char *buf, SIZE_T remain) {
	int i;

	for (i = 0; remain > 0; buf += i, remain -= i)
		if ((i = write(fd, buf, remain)) <= 0)
			return; /* abort silently on errors in write() */
}

/* clear out z bytes from character string s */

void clear(char *s, SIZE_T z) {
	while (z != 0)
		s[--z] = 0;
}

/* zero out the fifo queue, removing the fifos from /tmp as you go (also prints errors arising from signals) */

void empty_fifoq() {
	int sp;

	while (fifoq != NULL) {
		rc_wait4(fifoq->pid, &sp);
		statprint(sp);
		unlink(fifoq->name);
		fifoq = fifoq->n;
	}
}

SIZE_T strarraylen(char **a) {
	SIZE_T i;

	for (i = 0; *a != NULL; a++)
		i += strlen(*a) + 1;

	return i;
}
