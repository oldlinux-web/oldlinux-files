/* var.c: provide "public" functions for adding and removing variables from the symbol table */

#include "rc.h"
#include "utils.h"
#include "hash.h"
#include "list.h"
#include "footobar.h"
#include "nalloc.h"
#include "status.h"
#include "glom.h"
#include "wait.h"

#ifdef READLINE /* need to reset readline() every time TERM or TERMCAP changes */
extern void rl_reset_terminal(char *);
#endif

static void colonassign(char *, List *, boolean);
static void listassign(char *, List *, boolean);
static int hasalias(char *);

static char *const aliases[] = {
	"home", "HOME", "path", "PATH", "cdpath", "CDPATH"
};

/* assign a variable in List form to a name, stacking if appropriate */

void varassign(char *name, List *def, boolean stack) {
	Variable *new;
	List *newdef = listcpy(def); /* important to do the listcpy first; get_var_place() frees old values */

	new = get_var_place(name, stack);
	new->def = newdef;
	new->extdef = NULL;

#ifdef READLINE /* need to reset readline() every time TERM or TERMCAP changes */
	if (interactive && streq(name, "TERM") || streq(name, "TERMCAP"))
		rl_reset_terminal(NULL);
#endif
}

/* assign a variable in string form. Check to see if it is aliased (e.g., PATH and path) */

boolean varassign_string(char *extdef) {
	char *name = get_name(extdef);
	Variable *new;
	int i;
	static boolean aliasset[arraysize(aliases)] = {
		FALSE, FALSE, FALSE, FALSE, FALSE, FALSE
	};

	if (name == NULL)
		return FALSE; /* add it to bozo env */

	i = hasalias(name);
	if (i >= 0) {
		aliasset[i] = TRUE;
		if ((i & 1 == 0) && aliasset[i^1])
			return TRUE; /* don't alias variables that are already set in upper case */
	}
	new = get_var_place(name, FALSE);
	new->def = NULL;
	new->extdef = ealloc(strlen(extdef) + 1);
	strcpy(new->extdef, extdef);
	if (hasalias(name) != -1)
		alias(name, varlookup(name), FALSE);
	return TRUE;
}

/*
   Return a List based on a name lookup. If the list is in external (string) form,
   convert it to internal (List) form. Treat $n (n is an integer) specially as $*(n).
   Also check to see if $status is being dereferenced. (we lazily evaluate the List
   associated with $status)
*/

List *varlookup(char *name) {
	Variable *look;
	List *ret, *l;
	int sub;

	if (streq(name, "status"))
		return sgetstatus();
	if (streq(name, "apids"))
		return sgetapids();

	if (*name != '\0' && (sub = a2u(name)) != -1) { /* handle $1, $2, etc. */
		for (l = varlookup("*"); l != NULL && sub != 0; --sub)
			l = l->n;
		if (l == NULL)
			return NULL;
		ret = nnew(List);
		ret->w = l->w;
		ret->m = NULL;
		ret->n = NULL;
		return ret;
	}

	look = lookup_var(name);

	if (look == NULL)
		return NULL; /* not found */
	if (look->def != NULL)
		return look->def;
	if (look->extdef == NULL)
		return NULL; /* variable was set to null, e.g., a=() echo foo */

	ret = parse_var(name, look->extdef);

	if (ret == NULL) {
		look->extdef = NULL;
		return NULL;
	}
	return look->def = ret;
}

/* lookup a variable in external (string) form, converting if necessary. Used by makeenv() */

char *varlookup_string(char *name) {
	Variable *look;

	look = lookup_var(name);

	if (look == NULL)
		return NULL;
	if (look->extdef != NULL)
		return look->extdef;
	if (look->def == NULL)
		return NULL;
	return look->extdef = list2str(name, look->def);
}

/* remove a variable from the symtab. "stack" determines whether a level of scoping is popped or not */

void varrm(char *name, boolean stack) {
	int i = hasalias(name);

	if (streq(name, "*") && !stack) { /* when assigning () to $*, we want to preserve $0 */
		varassign("*", varlookup("0"), FALSE);
		return;
	}

	delete_var(name, stack);
	if (i != -1)
		delete_var(aliases[i^1], stack);
}

/* assign a value (List) to a variable, using array "a" as input. Used to assign $* */

void starassign(char *dollarzero, char **a, boolean stack) {
	List *s, *var;

	var = nnew(List);
	var->w = dollarzero;

	if (*a == NULL) {
		var->n = NULL;
		varassign("*", var, stack);
		return;
	}

	var->n = s = nnew(List);

	while (1) {
		s->w = *a++;
		if (*a == NULL) {
			s->n = NULL;
			break;
		} else {
			s->n = nnew(List);
			s = s->n;
		}
	}
	varassign("*", var, stack);
}

/* (ugly name, huh?) assign a colon-separated value to a variable (e.g., PATH) from a List (e.g., path) */

static void colonassign(char *name, List *def, boolean stack) {
	char *colondef;
	List dud;
	SIZE_T deflen;
	List *r;

	if (def == NULL) {
		varassign(name, NULL, stack);
		return;
	}

	deflen = listlen(def) + 1; /* one for the null terminator */

	colondef = nalloc(deflen);
	strcpy(colondef, def->w);

	for (r = def->n; r != NULL; r = r->n) {
		strcat(colondef, ":");
		strcat(colondef, r->w);
	}

	dud.w = colondef;
	dud.n = NULL;
	varassign(name, &dud, stack);
}

/* assign a List variable (e.g., path) from a colon-separated string (e.g., PATH) */

static void listassign(char *name, List *def, boolean stack) {
	List *val, *r;
	char *v, *w;

	if (def == NULL) {
		varassign(name, NULL, stack);
		return;
	}

	v = def->w;

	r = val = enew(List);

	while((w = strchr(v,':')) != NULL) {
		*w = '\0';
		r->w = ecpy(v);
		*w = ':';
		v = w + 1;
		r->n = enew(List);
		r = r->n;
	}
	r->w = ecpy(v);
	r->n = NULL;

	varassign(name, val, stack);
}

/* check to see if a particular variable is aliased; return -1 on failure, or the index */

static int hasalias(char *name) {
	int i;

	for (i = 0; i < arraysize(aliases); i++)
		if (streq(name, aliases[i]))
			return i;
	return -1;
}

/* alias a variable to its lowercase equivalent. function pointers are used to specify the conversion function */

void alias(char *name, List *s, boolean stack) {
	int i = hasalias(name);
	static void (*vectors[])(char *, List *, boolean) = {
		varassign, varassign, colonassign, listassign, colonassign, listassign
	};

	if (i != -1)
		vectors[i](aliases[i^1], s, stack); /* xor hack to reverse case of alias entry */
}

void prettyprint_var(int fd, char *name, List *s) {
	static char *keywords[] = {
		"if", "in", "fn", "for", "else", "switch", "while", "case"
	};
	char buf[10], *newname = NULL;
	int i;

	if (s == NULL) {
		fprint(fd, "%s=()\n", strprint(name, FALSE, FALSE));
		return;
	}

	if (streq(name, "*")) {
		s = s->n;
		if (s == NULL)
			return; /* Don't print $0, and if $* is not set, skip it */
	}

	for (i = 0; i < arraysize(keywords); i++)
		if (streq(keywords[i], name)) {
			newname = buf;
			newname[0] = '\'';
			strcpy(newname + 1, name);
			strcat(newname, "\'");
			break;
		}

	fprint(fd, s->n == NULL ? "%s=%l\n" : "%s=(%l)\n", newname == NULL ? strprint(name, FALSE, FALSE) : newname, s);
}
