/* tree.c: functions for manipulating parse-trees. (create, copy, delete) */

#include <stdarg.h>
#include "rc.h"
#include "tree.h"
#include "utils.h"
#include "nalloc.h"

/* make a new node, pass it back to yyparse. Used to generate the parsetree. */

Node *newnode(int /*enum nodetype*/ t,...) {
	va_list ap;
	Node *n;

	va_start(ap,t);

	switch (t) {
	default:
		fprint(2,"newnode: this can't happen\n");
		exit(1);
		/* NOTREACHED */
	case rDUP:
		n = nalloc(offsetof(Node, u[3]));
		n->u[0].i = va_arg(ap, int);
		n->u[1].i = va_arg(ap, int);
		n->u[2].i = va_arg(ap, int);
		break;
	case rWORD: case QWORD:
		n = nalloc(offsetof(Node, u[2]));
		n->u[0].s = va_arg(ap, char *);
		n->u[1].s = va_arg(ap, char *);
		break;
	case rBANG: case NOWAIT:
	case rCOUNT: case rFLAT: case RMFN: case rSUBSHELL:
	case VAR: case rCASE:
		n = nalloc(offsetof(Node, u[1]));
		n->u[0].p = va_arg(ap, Node *);
		break;
	case rANDAND: case ASSIGN: case BACKQ: case BODY: case BRACE: case CONCAT:
	case rELSE: case EPILOG: case rIF: case NEWFN: case CBODY:
	case rOROR: case PRE: case ARGS: case rSWITCH:
	case MATCH: case VARSUB: case rWHILE: case LAPPEND:
		n = nalloc(offsetof(Node, u[2]));
		n->u[0].p = va_arg(ap, Node *);
		n->u[1].p = va_arg(ap, Node *);
		break;
	case FORIN:
		n = nalloc(offsetof(Node, u[3]));
		n->u[0].p = va_arg(ap, Node *);
		n->u[1].p = va_arg(ap, Node *);
		n->u[2].p = va_arg(ap, Node *);
		break;
	case rPIPE:
		n = nalloc(offsetof(Node, u[4]));
		n->u[0].i = va_arg(ap, int);
		n->u[1].i = va_arg(ap, int);
		n->u[2].p = va_arg(ap, Node *);
		n->u[3].p = va_arg(ap, Node *);
		break;
	case rREDIR:
	case NMPIPE:
		n = nalloc(offsetof(Node, u[3]));
		n->u[0].i = va_arg(ap, int);
		n->u[1].i = va_arg(ap, int);
		n->u[2].p = va_arg(ap, Node *);
		break;
 	}
	n->type = t;
	va_end(ap);
	return n;
}

/* copy a tree to malloc space. Used when storing the definition of a function */

Node *treecpy(Node *s, void *(*alloc)(SIZE_T)) {
	Node *n;

	if (s == NULL)
		return NULL;

	switch (s->type) {
	default:
		fprint(2,"treecpy: this can't happen\n");
		exit(1);
		/* NOTREACHED */
	case rDUP:
		n = alloc(offsetof(Node, u[3]));
		n->u[0].i = s->u[0].i;
		n->u[1].i = s->u[1].i;
		n->u[2].i = s->u[2].i;
		break;
	case rWORD: case QWORD:
		n = alloc(offsetof(Node, u[2]));
		n->u[0].s = ecpy(s->u[0].s);
		if (s->u[1].s != NULL) {
			SIZE_T i = strlen(s->u[0].s);

			n->u[1].s = alloc(i);
			memcpy(n->u[1].s, s->u[1].s, i);
		} else
			n->u[1].s = NULL;
		break;
	case rBANG: case NOWAIT: case rCASE:
	case rCOUNT: case rFLAT: case RMFN: case rSUBSHELL: case VAR:
		n = alloc(offsetof(Node, u[1]));
		n->u[0].p = treecpy(s->u[0].p, alloc);
		break;
	case rANDAND: case ASSIGN: case BACKQ: case BODY: case BRACE: case CONCAT:
	case rELSE: case EPILOG: case rIF: case NEWFN: case CBODY:
	case rOROR: case PRE: case ARGS: case rSWITCH:
	case MATCH: case VARSUB: case rWHILE: case LAPPEND:
		n = alloc(offsetof(Node, u[2]));
		n->u[0].p = treecpy(s->u[0].p, alloc);
		n->u[1].p = treecpy(s->u[1].p, alloc);
		break;
	case FORIN:
		n = alloc(offsetof(Node, u[3]));
		n->u[0].p = treecpy(s->u[0].p, alloc);
		n->u[1].p = treecpy(s->u[1].p, alloc);
		n->u[2].p = treecpy(s->u[2].p, alloc);
		break;
	case rPIPE:
		n = alloc(offsetof(Node, u[4]));
		n->u[0].i = s->u[0].i;
		n->u[1].i = s->u[1].i;
		n->u[2].p = treecpy(s->u[2].p, alloc);
		n->u[3].p = treecpy(s->u[3].p, alloc);
		break;
	case rREDIR:
	case NMPIPE:
		n = alloc(offsetof(Node, u[3]));
		n->u[0].i = s->u[0].i;
		n->u[1].i = s->u[1].i;
		n->u[2].p = treecpy(s->u[2].p, alloc);
		break;
	}
	n->type = s->type;
	return n;
}

/* free a function definition that is no longer needed */

void treefree(Node *s) {
	if (s == NULL)
		return;
	switch (s->type) {
	case rDUP:
		break;
	case rWORD: case QWORD:
		efree(s->u[0].s);
		efree(s->u[1].s);
		break;
	case rBANG: case NOWAIT:
	case rCOUNT: case rFLAT: case RMFN:
	case rSUBSHELL: case VAR: case rCASE:
		treefree(s->u[0].p);
		efree(s->u[0].p);
		break;
	case rANDAND: case ASSIGN: case BACKQ: case BODY: case BRACE: case CONCAT:
	case rELSE: case EPILOG: case rIF: case NEWFN:
	case rOROR: case PRE: case ARGS: case CBODY:
	case rSWITCH: case MATCH:  case VARSUB: case rWHILE:
	case LAPPEND:
		treefree(s->u[1].p);
		treefree(s->u[0].p);
		efree(s->u[1].p);
		efree(s->u[0].p);
		break;
	case FORIN:
		treefree(s->u[2].p);
		treefree(s->u[1].p);
		treefree(s->u[0].p);
		efree(s->u[2].p);
		efree(s->u[1].p);
		efree(s->u[0].p);
		break;
	case rPIPE:
		treefree(s->u[2].p);
		treefree(s->u[3].p);
		efree(s->u[2].p);
		efree(s->u[3].p);
		break;
	case rREDIR:
	case NMPIPE:
		treefree(s->u[2].p);
		efree(s->u[2].p);
		break;
	default:
		fprint(2,"treefree: this can't happen\n");
		exit(1);
	}
}
