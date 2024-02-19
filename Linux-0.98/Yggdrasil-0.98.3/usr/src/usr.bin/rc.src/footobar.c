/*
   footobar.c: a collection of functions to convert internal representations of
   variables and functions to external representations, and vice versa
*/

#include "rc.h"
#include "utils.h"
#include "lex.h"
#include "footobar.h"
#include "nalloc.h"
#include "input.h"
#include "list.h"

#define FSCHAR '\1'
#define FSSTRING "\1"

static char *getenvw(char *, boolean);
static void funcat(char *);
static void strtree(Node *);

static char *fun;
static SIZE_T funsize, funpos;

#ifdef PROTECT_ENV
/* octalize -- protect an exported name from brain-dead shells */

static char hexchar[] = "0123456789abcdef";

static char *octalize(char *name) {
	int c;
	static char namebuf[1000];
	char *s = namebuf;

	while (1) {
		c = *(unsigned char *) name++;
		if (c == '\0') {
			*s++ = '\0';
			return namebuf;
		}
		if (!dnw[c] && c != '*') {
			if (c != '_')
				*s++ = c;
			else if (*name != '_')
				*s++ = c;
			else
				goto fallthrough;
		} else {
	fallthrough:	*s++ = '_';
			*s++ = '_';
			*s++ = hexchar[(c >> 4) & 0xf];
			*s++ = hexchar[c & 0xf];
		}
	}
}
#endif

/* a specialized strcat, used in strtree */

static void funcat(char *s) {
	SIZE_T l = strlen(s);
	char *new;

	if (l + funpos > funsize) {
		new = nalloc(funsize *= 2);
		memcpy(new, fun, funpos);
		new[funpos] = 0;
		fun = new;
	}
	strcpy(fun + funpos, s);
	funpos += l;
}

/* used to turn a function in Node * form into something we can export to the environment */

char *fun2str(char *name, Node *s) {
#ifdef PROTECT_ENV
	name = octalize(name);
#endif
	fun = nalloc(funsize = 512);
	funpos = 0;
	funcat("fn_");
	funcat(name);
	funcat("={");
	strtree(s);
	funcat("}");
	return ecpy(fun); /* put into malloc space */
}

/* ptree is used by whatis in order to print the definition of a function to the terminal */

char *ptree(Node *s) {
	fun = nalloc(funsize = 512);
	funpos = 0;
	fun[0] = 0;
	strtree(s);
	return fun;
}

/* save some code space by gathering this operation in a function */

static void catredir(int i) {
	switch (i) {
	case CREATE: funcat(">"); break;
	case APPEND: funcat(">>"); break;
	case HEREDOC: funcat("<<"); break;
	case HERESTRING: funcat("<<<"); break;
	case FROM: funcat("<"); break;
	}
}

/* convert a function in Node * form into something rc can parse (and humans can read?) */

static void strtree(Node *n) {
	int defaultfd;
	char b[16];

	if (n == NULL) {
		funcat("()");
		return;
	}

	switch (n->type) {
	case rDUP:
		catredir(n->u[0].i);
		if (n->u[2].i != -1)
			sprint(b, "[%d=%d]", n->u[1].i, n->u[2].i);
		else
			sprint(b, "[%d=]", n->u[1].i);
		funcat(b);
		break;
	case rWORD:
		funcat(strprint(n->u[0].s, FALSE, FALSE));
		break;
	case QWORD:
		funcat(strprint(n->u[0].s, TRUE, FALSE));
		break;
	case BACKQ:
		if (n->u[0].p != NULL && n->u[0].p->type == VAR
			&& n->u[0].p->u[0].p != NULL && n->u[0].p->u[0].p->type == rWORD
			&& streq(n->u[0].p->u[0].p->u[0].s,"ifs")) {
			funcat("`{");
		} else {
			funcat("``");
			strtree(n->u[0].p);
			funcat("{");
		}
		strtree(n->u[1].p);
		funcat("}");
		break;
	case rBANG:
		funcat("! ");
		strtree(n->u[0].p);
		break;
	case rCASE:
		funcat("case ");
		strtree(n->u[0].p);
		break;
	case CBODY:
		if (n->u[0].p != NULL) {
			strtree(n->u[0].p);
			funcat(";");
		}
		if (n->u[1].p != NULL)
			strtree(n->u[1].p);
		break;
	case NOWAIT:
		strtree(n->u[0].p);
		funcat("&");
		break;
	case rCOUNT:
		funcat("$#");
		strtree(n->u[0].p);
		break;
	case rFLAT:
		funcat("$^");
		strtree(n->u[0].p);
		break;
	case RMFN:
		funcat("fn ");
		strtree(n->u[0].p);
		break;
	case rSUBSHELL:
		funcat("@ ");
		strtree(n->u[0].p);
		break;
	case VAR:
		funcat("$");
		strtree(n->u[0].p);
		break;
	case rANDAND:
		strtree(n->u[0].p);
		funcat("&&");
		strtree(n->u[1].p);
		break;
	case ASSIGN:
		strtree(n->u[0].p);
		funcat("=");
		strtree(n->u[1].p);
		break;
	case BODY:
		if (n->u[1].p != NULL)
			funcat("{");
		strtree(n->u[0].p);
		if (n->u[1].p != NULL) {
			funcat(";");
			strtree(n->u[1].p);
			funcat("}");
		}
		break;
	case BRACE:
		funcat("{");
		strtree(n->u[0].p);
		funcat("}");
		if (n->u[1].p != NULL)
			strtree(n->u[1].p);
		break;
	case CONCAT:
		strtree(n->u[0].p);
		funcat("^");
		strtree(n->u[1].p);
		break;
	case rELSE:
		funcat("{");
		strtree(n->u[0].p);
		funcat("}else ");
		strtree(n->u[1].p);
		break;
	case EPILOG:
	case PRE:
		strtree(n->u[0].p);
		if (n->u[1].p != NULL) {
			funcat(" ");
			strtree(n->u[1].p);
		}
		break;
	case NEWFN:
		funcat("fn ");
		strtree(n->u[0].p);
		funcat(" {");
		strtree(n->u[1].p);
		funcat("}");
		break;
	case rIF:
		funcat("if(");
		strtree(n->u[0].p);
		funcat(")");
		strtree(n->u[1].p);
		break;
	case rOROR:
		strtree(n->u[0].p);
		funcat("||");
		strtree(n->u[1].p);
		break;
	case ARGS:
		strtree(n->u[0].p);
		funcat(" ");
		strtree(n->u[1].p);
		break;
	case rSWITCH:
		funcat("switch(");
		strtree(n->u[0].p);
		funcat("){");
		strtree(n->u[1].p);
		funcat("}");
		break;
	case MATCH:
		funcat("~ ");
		strtree(n->u[0].p);
		funcat(" ");
		strtree(n->u[1].p);
		break;
	case VARSUB:
		funcat("$");
		strtree(n->u[0].p);
		funcat("(");
		strtree(n->u[1].p);
		funcat(")");
		break;
	case rWHILE:
		funcat("while(");
		strtree(n->u[0].p);
		funcat(")");
		strtree(n->u[1].p);
		break;
	case LAPPEND:
		funcat("(");
		strtree(n->u[0].p);
		funcat(" ");
		strtree(n->u[1].p);
		funcat(")");
		break;
	case FORIN:
		funcat("for(");
		strtree(n->u[0].p);
		funcat(" in ");
		strtree(n->u[1].p);
		funcat(")");
		strtree(n->u[2].p);
		break;
	case rPIPE:
		funcat("{");
		strtree(n->u[2].p);
		if (n->u[0].i == 1) {
			if (n->u[1].i == 0)
				sprint(b, "}|{");
			else
				sprint(b, "}|[1=%d]{", n->u[1].p);
		} else {
			if (n->u[1].i == 0)
				sprint(b, "}|[%d]{", n->u[0].p);
			else
				sprint(b, "}|[%d=%d]{", n->u[0].i, n->u[1].i);
		}
		funcat(b);
		strtree(n->u[3].p);
		funcat("}");
		break;
	case NMPIPE:
		defaultfd = (n->u[0].i == CREATE || n->u[0].i == APPEND);
		catredir(n->u[0].i);
		if (n->u[1].i != defaultfd) {
			sprint(b, "[%d]{", n->u[1].i);
			funcat(b);
		} else
			funcat("{");
		strtree(n->u[2].p);
		funcat("}");
		break;
	case rREDIR:
		defaultfd = (n->u[0].i == CREATE || n->u[0].i == APPEND);
		catredir(n->u[0].i);
		if (n->u[1].i != defaultfd) {
			sprint(b, "[%d]", n->u[1].i);
			funcat(b);
		}
		strtree(n->u[2].p);
		break;
 	}
}

/* convert a List to a string, separating it with ^A characters. Used for exporting variables to the environment */

char *list2str(char *name, List *s) {
	SIZE_T size;
	List *t;
	char *w;

#ifdef PROTECT_ENV
	name = octalize(name);
#endif
	size = strlen(name) + listlen(s);

	w = ealloc(size + 2);
	t = s;
	strcpy(w, name);
	strcat(w, "=");
	strcat(w, t->w);
	for (s = s->n; s != NULL; s = s->n) {
		strcat(w, FSSTRING);
		strcat(w, s->w);
	}
	return w;
}

/* convert a List to an array, for execve() */

char **list2array(List *s, boolean print) {
	char **av;
	int i;

	/* 4 == 1 for the null terminator + 2 for the fake execve() + 1 for defaulting to sh */

	av = nalloc((listnel(s) + 4) * sizeof (char *));

	av += 3; /* hide the two free spots from rc (two for #! emulation, one for defaulting to sh) */

	if (print)
		fprint(2, "%l\n", s);

	for (i = 0; s != NULL; i++) {
		av[i] = s->w;
		s = s->n;
	}
	av[i] = NULL;
	return av;
}

/* figure out the name of a variable given an environment string. copy this into malloc space */

char *get_name(char *s) {
	int c;
	SIZE_T i;
	char *r, *namebuf;
#ifdef PROTECT_ENV
	char *h1, *h2;
#endif

	for (i = 0; s[i] != '\0' && s[i] != '='; i++)
		;

	if (s[i] == '\0')
		return NULL;

	r = namebuf = ealloc(i + 1);

	while (1)
		switch (c = *s++) {
		case '=':
			*r++ = '\0';
			return namebuf;
#ifdef PROTECT_ENV
		case '_':
			if (*s == '_' && (h1 = strchr(hexchar, s[1])) != NULL && (h2 = strchr(hexchar, s[2])) != NULL) {
				*r++ = ((h1 - hexchar) << 4) | (h2 - hexchar);
				s += 3;
				break;
			}
			/* FALLTHROUGH */
#endif
		default:
			*r++ = c;
			break;
		}
}

/* get the next word from a variable's value as represented in the environment. */

static char *getenvw(char *s, boolean saw_alpha) {
	char *r;
	SIZE_T i,j;

	for (i = j = 0; s[i] != '\0' && s[i] != FSCHAR; i++)
		;

	if (i == 0)
		if(s[i] == '\0' && !saw_alpha)
			return NULL;
		else {
			r = enew(char);
			*r = '\0';
			return r;
		}

	r = ealloc(i + j + 1);

	r[i + j] = '\0';

	while (i > 0) {
		--i;
		r[i + j] = s[i];
	}
	return r;
}

/* take an environment entry for a variable (elements ^A separated) and turn it into a List */

List *parse_var(char *name, char *extdef) {
	List *r, *top;
	char *f;
	boolean saw_alpha;

	top = r = enew(List);
	extdef = strchr(extdef, '=') + 1;


	if ((f = getenvw(extdef, FALSE)) == NULL)
		return NULL;

	while (1) {
		r->w = f;
		r->m = NULL;
		extdef += strlen(f);
		if (*extdef == FSCHAR) {
			extdef++;
			saw_alpha = TRUE;
		} else {
			saw_alpha = FALSE;
		}
		if ((f = getenvw(extdef, saw_alpha)) == NULL) {
			r->n = NULL;
			break;
		}
		r = r->n = enew(List);
	}
	return top;
}

/* get an environment entry for a function and have rc parse it. */

Node *parse_fn(char *name, char *extdef) {
	Node *def;
	int i;
	char *s, old[4];

	s = strchr(extdef, '=');

	if (s == NULL)
		return NULL;

	s -= 3;

	for (i = 0; i < 4; i++)
		old[i] = s[i];
	s[0] = 'f';
	s[1] = 'n';
	s[2] = ' ';
	s[3] = 'x';

	def = parseline(s);
	for (i = 0; i < 4; i++)
		s[i] = old[i];

	if (def == NULL || def->type != NEWFN)
		return NULL;
	else
		return def->u[1].p;
}
