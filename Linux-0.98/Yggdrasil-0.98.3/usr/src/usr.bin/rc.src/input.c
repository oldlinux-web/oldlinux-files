/* input.c: i/o routines for files and pseudo-files (strings) */

#include <errno.h>
#include "jbwrap.h"
#include <stdarg.h>
#include "rc.h"
#include "input.h"
#include "utils.h"
#include "walk.h"
#include "hash.h"
#include "lex.h"
#include "open.h"
#include "nalloc.h"
#include "except.h"
#include "glom.h"
#include "builtins.h"
#include "parse.h"
#include "tree.h"
#include "footobar.h"

/*
   warning, changes have been made to (fd|string)(gchar|ugchar) so that
   you cannot unget EOF more than once. lex.c never does this, so I'm
   safe, but if you unget EOF more than once, expect to be able to read
   it back only once. The reason is that EOF is an int, whereas the buffers
   are character buffers.
*/

typedef struct Input {
	enum inputtype t;
	char *ibuf;
	int fd;
	int index;
	int read;
	int lineno;
	boolean saved;
} Input;

#define BUFSIZE ((SIZE_T) 256)

#ifdef READLINE
extern char *readline(char *);
extern void add_history(char *);
static char *rlinebuf;
#endif

char *prompt, *prompt2;
boolean rcrc;
char *histstr;
int histfd;

static int dead(void);
static int fdgchar(void);
static int stringgchar(void);
static void history(void);
static void ugdead(int);

static char *inbuf;
static SIZE_T istacksize, chars_out, chars_in;
static boolean eofread = FALSE, save_lineno = TRUE;
static Input *istack, *itop;

static int (*realgchar)(void);
static void (*realugchar)(int);

int last;

int gchar() {
	if (eofread) {
		eofread = FALSE;
		return last = EOF;
	}
	return realgchar();
}

void ugchar(int c) {
	realugchar(c);
}

static int dead() {
	return last = EOF;
}

static void ugdead(int c) {
	return;
}

static void ugalive(int c) {
	if (c == EOF)
		eofread = TRUE;
	else
		inbuf[--chars_out] = c;
}

/* get the next character from a string. */

static int stringgchar() {
	return last = (inbuf[chars_out] == '\0' ? EOF : inbuf[chars_out++]);
}

/*
   read a character from a file-descriptor. If GNU readline is defined, add a newline and doctor
   the buffer to look like a regular fdgchar buffer.
*/

static int fdgchar() {
	if (chars_out >= chars_in + 2) { /* has the buffer been exhausted? if so, replenish it */
		while (1) {
#ifdef READLINE
			if (interactive && istack->fd == 0) {
				rlinebuf = readline(prompt);
				if (rlinebuf == NULL) {
					chars_in = 0;
				} else {
					if (*rlinebuf != '\0')
						add_history(rlinebuf);
					chars_in = strlen(rlinebuf) + 1;
					efree(inbuf);
					inbuf = ealloc(chars_in + 3);
					strcpy(inbuf+2, rlinebuf);
					strcat(inbuf+2, "\n");
					efree(rlinebuf);
				}
			} else
#endif
				{
				int /* (ssize_t) */ r = read(istack->fd, inbuf + 2, BUFSIZE);

				if (r < 0) {
					if (errno == EINTR)
						continue; /* Suppose it was interrupted by a signal */
					uerror("read");
					rc_exit(1);
				}

				chars_in = (SIZE_T) r;
			}
			break;
		}

		if (chars_in == 0)
			return last = EOF;

		chars_out = 2;
		if (dashvee)
			writeall(2, inbuf + 2, chars_in);
		history();
	}
	return last = inbuf[chars_out++];
}

/* set up the input stack, and put a "dead" input at the bottom, so that yyparse will always read eof */

void initinput() {
	istack = itop = ealloc(istacksize = 256 * sizeof (Input));
	istack->t = FD;
	istack->fd = -1;
	realugchar = ugalive;
}

/* push an input source onto the stack. set up a new input buffer, and set the right getchar() */

void pushinput(int /*enum inputtype*/ t,...) {
	SIZE_T count, idiff;
	char **a;
	va_list ap;

	va_start(ap, t);

	istack->index = chars_out;
	istack->read = chars_in;
	istack->ibuf = inbuf;
	istack->lineno = lineno;
	istack->saved = save_lineno;
	istack++;

	idiff = istack - itop;

	if (idiff >= istacksize / sizeof (Input)) {
		itop = erealloc(itop, istacksize *= 2);
		istack = itop + idiff;
	}

	istack->t = t;
	if (t == FD) {
		save_lineno = TRUE;
		istack->fd = va_arg(ap, int);
		realgchar = fdgchar;
		inbuf = ealloc(BUFSIZE + 2);
	} else {
		count = strarraylen(a = va_arg(ap, char **));
		save_lineno = va_arg(ap, boolean);
		sprint((inbuf = ealloc(count + 3)) + 2, "%a", a);
		realgchar = stringgchar;
	}

	va_end(ap);

	realugchar = ugalive;
	chars_out = 2;
	chars_in = 0;
	if (save_lineno)
		lineno = 1;
	else
		--lineno;
}

/* remove an input source from the stack. restore the right kind of getchar (string,fd) etc. */

void popinput() {
	if (istack->t == FD)
		close(istack->fd);
	efree(inbuf);

	--istack;

	realgchar = (istack->t == STRING ? stringgchar : fdgchar);

	if (istack->fd == -1) { /* top of input stack */
		realgchar = dead;
		realugchar = ugdead;
	}

	inbuf = istack->ibuf;
	chars_out = istack->index;
	chars_in = istack->read;
	if (save_lineno)
		lineno = istack->lineno;
	else
		lineno++;
	save_lineno = istack->saved;
}

/* flush input characters upto newline. Used by scanerror() */

void flushu() {
	int c;

	if (last == '\n' || last == EOF)
		return;

	while ((c = gchar()) != '\n' && c != EOF)
		; /* skip to newline */

	if (c == EOF)
		ugchar(c);
}

/* the wrapper loop in rc: prompt for commands until EOF, calling yyparse and walk() */

Node *doit(boolean execit) {
	boolean eof;
	jbwrap j;
	Estack e1, e2;

	if (dashen)
		execit = FALSE;

	setjmp(j.j);
	except(ERROR, &j, &e1);

	for (eof = FALSE; !eof;) {
		except(ARENA, NULL, &e2);

		if (dashell) {
			char *fname[3];

			fname[1] = concat(varlookup("home"),word("/.rcrc",NULL))->w;
			fname[2] = NULL;
			rcrc = TRUE;
			dashell = FALSE;
			b_dot(fname);
		}

		if (interactive) {
			List *s;

			if (fnlookup("prompt") != NULL) {
				static char *arglist[] = { "prompt", NULL };
				funcall(arglist);
			}

			if ((s = varlookup("prompt")) != NULL) {
#ifdef READLINE
				prompt = s->w;
#else
				fprint(2,"%s",s->w);
#endif
				prompt2 = (s->n == NULL ? "" : s->n->w);
			}
		}

		inityy();

		if (yyparse() == 1 && execit)
			rc_raise(ERROR);

		eof = (last == EOF); /* "last" can be clobbered during a walk() */

		if (parsetree != NULL) {
			if (execit)
				walk(parsetree, TRUE);
			else if (dashex && dashen)
				fprint(2, "%s\n", ptree(parsetree));
		}

		unexcept(); /* ARENA */
	}

	popinput();
	unexcept(); /* ERROR */
	return parsetree;
}

/* parse a function imported from the environment */

Node *parseline(char *extdef) {
	char *in[2];
	int i = interactive;
	Node *ret;

	in[0] = extdef;
	in[1] = NULL;
	interactive = FALSE;
	pushinput(STRING, in, TRUE);
	ret = doit(FALSE);
	interactive = i;
	return ret;
}

/* write last command out to a file. Check to see if $history has changed, also */

static void history() {
	List *histlist;
	SIZE_T a;

	if (!interactive)
		return;

	if ((histlist = varlookup("history")) == NULL) {
		if (histstr != NULL) {
			efree(histstr);
			close(histfd);
			histstr = NULL;
		}
		return;
	}

	if (histstr == NULL || !streq(histstr, histlist->w)) { /* open new file */
		if (histstr != NULL) {
			efree(histstr);
			close(histfd);
		}
		histstr = ecpy(histlist->w);
		histfd = rc_open(histstr, APPEND);
		if (histfd < 0) {
			uerror(histstr);
			efree(histstr);
			histstr = NULL;
			varrm("history", FALSE);
		}
	}

	/*
	   small unix hack: since read() reads only up to a newline from a terminal, then
	   presumably this write() will write at most only one input line at a time.
	*/

	for (a = 2; a < chars_in + 2; a++) { /* skip empty lines and comments in history. */
		if (inbuf[a] == '#' || inbuf[a] == '\n')
			return;
		if (inbuf[a] != ' ' && inbuf[a] != '\t')
			break;
	}
	writeall(histfd, inbuf + 2, chars_in);
}
