/* heredoc.c: heredoc slurping is done here */

#include "rc.h"
#include "lex.h"
#include "utils.h"
#include "nalloc.h"
#include "heredoc.h"
#include "tree.h"
#include "input.h"
#include "hash.h"
#include "glom.h"

struct Hq {
	Node *doc;
	char *name;
	Hq *n;
	boolean quoted;
} *hq;

static boolean dead = FALSE;

/*
 * read in a heredocument. A clever trick: skip over any partially matched end-of-file
 * marker storing only the number of characters matched. If the whole marker is matched,
 * return from readheredoc(). If only part of the marker is matched, copy that part into
 * the heredocument.
 *
 * BUG: if the eof string contains a newline, the state can get confused, and the
 * heredoc may continue past where it should.  on the other hand, /bin/sh seems to
 * never get out of its readheredoc() when the heredoc string contains a newline
 */

static char *readheredoc(char *eof) {
	int c;
	char *t, *buf, *bufend;
	unsigned char *s;
	SIZE_T bufsize;

	t = buf = nalloc(bufsize = 512);
	bufend = &buf[bufsize];
	dead = FALSE;

#define	RESIZE(extra) { \
		char *nbuf; \
		bufsize = bufsize * 2 + extra; \
		nbuf = nalloc(bufsize); \
		memcpy(nbuf, buf, t - buf); \
		t = nbuf + (t - buf); \
		buf = nbuf; \
		bufend = &buf[bufsize]; \
	}
	
	for (;;) {
		print_prompt2();
		for (s = (unsigned char *) eof; (c = gchar()) == *s; s++)
			;
		if (*s == '\0' && (c == '\n' || c == EOF)) {
			*t++ = '\0';
			return buf;
		}
		if (s != (unsigned char *) eof) {
			SIZE_T len = s - (unsigned char *) eof;
			if (t + len >= bufend)
				RESIZE(len);
			memcpy(t, eof, len);
			t += len;
		}
		for (;; c = gchar()) {
			if (c == EOF) {
				yyerror("EOF inside heredoc");
				dead = TRUE;
				return NULL;
			}
			if (t + 1 >= bufend)
				RESIZE(0);
			*t++ = c;
			if (c == '\n')
				break;
		}
	}
}

/* parseheredoc -- turn a heredoc with variable references into a node chain */

static Node *parseheredoc(char *s) {
	int c = *s;
	Node *result = NULL;

	while (TRUE) {
		Node *node;

		switch (c) {
		default: {
			char *begin = s;
			while ((c = *s++) != '\0' && c != '$')
				;
			*--s = '\0';
			node = newnode(QWORD, begin, NULL);
			break;
		}
		case '$': {
			char *begin = ++s, *var;
			c = *s++;
			if (c == '$') {
				node = newnode(QWORD, "$", NULL);
				c = *s;
			} else {
				int len = 0;
				do
					len++;
				while (!dnw[c = *(unsigned char *) s++]);
				if (c == '^')
					c = *s;
				else
					s--;
				var = nalloc(len + 1);
				var[len] = '\0';
				memcpy(var, begin, len);
				node = newnode(rFLAT, newnode(rWORD, var, NULL));
			}
			break;
		}
		case '\0':
			return result;
		}

		result = (result == NULL) ? node : newnode(CONCAT, result, node);
	}
}

/* read in heredocs when yyparse hits a newline. called from yyparse */

int heredoc(int end) {
	Hq *here;

	if ((here = hq) != NULL) {
		hq = NULL;
		if (end) {
			yyerror("EOF on command line with heredoc");
			return FALSE;
		}
		do {
			Node *n = here->doc;
			char *s = readheredoc(here->name);
			if (dead)
				return FALSE;
			n->u[2].p = here->quoted ? newnode(QWORD, s, NULL) : parseheredoc(s);
			n->u[0].i = HERESTRING;
		} while ((here = here->n) != NULL);
	}
	return TRUE;
}

/* queue pending heredocs into a queue. called from yyparse */

int qdoc(Node *name, Node *n) {
	Hq *new, **prev;

	if (name->type != rWORD && name->type != QWORD) {
		yyerror("eof-marker must be a single literal word");
		flushu();
		return FALSE;
	}

	for (prev = &hq; (new = *prev) != NULL; prev = &new->n)
		;
	*prev = new = nnew(Hq);

	new->name = name->u[0].s;
	new->quoted = (name->type == QWORD);
	new->doc = n;
	new->n = NULL;
	return TRUE;
}
