/* list.c: routines for manipulating the List type */

#include "rc.h"
#include "utils.h"
#include "list.h"

/*
   These list routines assign meta values of null to the resulting lists;
   it is impossible to glob with the value of a variable unless this value
   is rescanned with eval---therefore it is safe to throw away the meta-ness
   of the list.
*/

/* free a list from malloc space */

void listfree(List *p) {
	if (p == NULL)
		return;
	listfree(p->n);
	efree(p->w);
	efree(p);
}

/* copy of list in malloc space (for storing a variable) */

List *listcpy(List *s) {
	List *r;

	if (s == NULL)
		return NULL;

	r = enew(List);
	r->w = ecpy(s->w);
	r->m = NULL;
	r->n = listcpy(s->n);

	return r;
}

/* length of list */

SIZE_T listlen(List *s) {
	SIZE_T size;

	for (size = 0; s != NULL; s = s->n)
		size += strlen(s->w) + 1;

	return size;
}

/* number of elements in list */

int listnel(List *s) {
	int nel;

	for (nel = 0; s != NULL; s = s->n)
		nel++;

	return nel;
}
