/*
 *   bin86/ld/table.c
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* table.c - table-handler module for linker */

#include "const.h"
#include "align.h"
#include "obj.h"
#include "type.h"
#include "globvar.h"

#define GOLDEN 157		/* GOLDEN/HASHTABSIZE approx golden ratio */
#define HASHTABSIZE 256

PRIVATE struct symstruct *hashtab[HASHTABSIZE];	/* hash table */
PRIVATE char *tableptr;		/* next free spot in catchall table */
PRIVATE char *tableend;		/* ptr to spot after last in table */

FORWARD struct symstruct **gethashptr P((char *name));

/* initialise symbol table */

PUBLIC void syminit()
{
    unsigned i;

    for (i = sizeof(int) <= 2 ? 0xE000 : (unsigned) 0x38000;
	 i != 0; i -= 512)
	if ((tableptr = malloc(i)) != NULL)
	    break;
    if (tableptr == NULL)
	outofmemory();
    tableend = tableptr + i;
    for (i = 0; i < HASHTABSIZE; i++)
	hashtab[i] = NULL;
}

/* add named symbol to end of table - initialise only name and next fields */
/* caller must not duplicate names of externals for findsym() to work */

PUBLIC struct symstruct *addsym(name)
char *name;
{
    struct symstruct **hashptr;
    struct symstruct *oldsymptr;
    struct symstruct *symptr;

    hashptr = gethashptr(name);
    symptr = *hashptr;
    while (symptr != NULL)
    {
	oldsymptr = symptr;
	symptr = symptr->next;
    }
    align(tableptr);
    symptr = (struct symstruct *) tableptr;
    if ((tableptr = symptr->name + (strlen(name) + 1)) > tableend)
	outofmemory();
    symptr->modptr = NULL;
    symptr->next = NULL;
    if (name != symptr->name)
	strcpy(symptr->name, name);	/* should't happen */
    if (*hashptr == NULL)
	*hashptr = symptr;
    else
	oldsymptr->next = symptr;
    return symptr;
}

/* lookup named symbol */

PUBLIC struct symstruct *findsym(name)
char *name;
{
    struct symstruct *symptr;

    symptr = *gethashptr(name);
    while (symptr != NULL && (!(symptr->flags & (E_MASK | I_MASK)) ||
			      strcmp(symptr->name, name) != 0))
	symptr = symptr->next;
    return symptr;
}

/* convert name to a hash table ptr */

PRIVATE struct symstruct **gethashptr(name)
register char *name;
{
    register unsigned hashval;

    hashval = 0;
    while (*name)
	hashval = hashval * 2 + *name++;
    return hashtab + ((hashval * GOLDEN) & (HASHTABSIZE - 1));

/*

#asm

GOLDEN	EQU	157
HASHTABSIZE	EQU	256

	CLRB		can build value here since HASHTABSIZE <= 256
	LDA	,X
	BEQ	HASHVAL.EXIT
HASHVAL.LOOP
	ADDB	,X+
	LSLB
	LDA	,X
	BNE	HASHVAL.LOOP
	RORB
	LDA	#GOLDEN
	MUL
HASHVAL.EXIT
HASHVAL.EXIT
	LDX	#_hashtab
	ABX			discard	A - same as taking mod HASHTABSIZE
	ABX
#endasm

*/

}

/* move symbol descriptor entries to top of table (no error checking) */

PUBLIC char *moveup(nbytes)
unsigned nbytes;
{
    register char *source;
    register char *target;

    source = tableptr;
    target = tableend;
    while (nbytes--)
	*--target = *--source;
    tableptr = source;
    return tableend = target;
}

/* our version of malloc */

PUBLIC char *ourmalloc(nbytes)
unsigned nbytes;
{
    char *allocptr;

    align(tableptr);
    allocptr = tableptr;
    if ((tableptr += nbytes) > tableend)
	outofmemory();
    return allocptr;
}

/* our version of free (release from bottom of table) */

PUBLIC void ourfree(cptr)
char *cptr;
{
    tableptr = cptr;
}

/* read string from file into table at offset suitable for next symbol */

PUBLIC char *readstring()
{
    int c;
    char *s;
    char *start;

    align(tableptr);
    start = s = ((struct symstruct *) tableptr)->name;
    while (TRUE)
    {
	if (s >= tableend)
	    outofmemory();
	if ((c = readchar()) < 0)
	    prematureeof();
	if ((*s++ = c) == 0)
	    return start;
    }
    /* NOTREACHED */
}

/* release from top of table */

PUBLIC void release(cptr)
char *cptr;
{
    tableend = cptr;
}

/* allocate space for string */

PUBLIC char *stralloc(s)
char *s;
{
    return strcpy(ourmalloc((unsigned) strlen(s) + 1), s);
}
