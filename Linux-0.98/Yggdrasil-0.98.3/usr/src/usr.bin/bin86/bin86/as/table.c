/*
 *   bin86/as/table.c
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* table.c - symbol table handler for assembler */

#include "const.h"
#include "type.h"
#include "globvar.h"
#include "scan.h"

#define hconv(ch) ((unsigned char) (ch) - 0x41)	/* better form for hashing */

#ifdef I80386
# ifdef MNSIZE
EXTERN char bytesizeops[];
# endif
#endif
EXTERN char ops[];
EXTERN char page1ops[];
EXTERN char page2ops[];
EXTERN char regs[];
#ifdef I80386
EXTERN char typesizes[];
#endif

#ifdef DEBUG
unsigned nhash;
unsigned nlookup;
unsigned nsym;
unsigned nx[30];
FORWARD void printchain P((void));
#endif

FORWARD void install P((register char *keyptr, unsigned data));

PUBLIC void inst_keywords()
{
    install(regs, REGBIT);
#ifdef I80386
    install(typesizes, SIZEBIT);
#endif
    install(ops, 0);
    install(page1ops, PAGE1);
    install(page2ops, PAGE2);
#ifdef I80386
# ifdef MNSIZE
    install(bytesizeops, PAGE1 | PAGE2);
# endif
#endif
}

PRIVATE void install(keyptr, data)
register char *keyptr;
unsigned data;
{
    char lowcasebuf[20];
    unsigned namelength;
    char *nameptr;
    char *namend;
    register struct sym_s *symptr;

    while (*keyptr != 0)
    {
	namelength = *keyptr++;
	lineptr = (symname = keyptr) + namelength;
	for (nameptr = lowcasebuf, namend = lowcasebuf + namelength;
	     nameptr < namend;)
	{
	    if (*keyptr < 'A' || *keyptr > 'Z')
		*nameptr++ = *keyptr++;
	    else
		*nameptr++ = *keyptr++ + ('a' - 'A');
	}
	symptr = lookup();
	symptr->type = MNREGBIT;
	symptr->data = data;
	symptr->value_reg_or_op.op.routine = *keyptr;
	symptr->value_reg_or_op.op.opcode = keyptr[1];
	lineptr = (symname = lowcasebuf) + namelength;
	symptr = lookup();
	symptr->type = MNREGBIT;
	symptr->data = data;
	symptr->value_reg_or_op.op.routine = *keyptr;
	symptr->value_reg_or_op.op.opcode = keyptr[1];
	keyptr += 2;
    }
}

/* Lookup() searches symbol table for the string from symname to lineptr - 1.
 * If string is not found and ifflag is TRUE, string is added to table, with
 *	type = 0
 *	data = inidata (RELBIT | UNDBIT, possibly with IMPBIT | SEGM)
 * Returns pointer to symbol entry (NULL if not found and not installed)
 * unless symbol table overflows, when routine aborts.
 */

PUBLIC struct sym_s *lookup()
{
    struct sym_s **hashptr;
    register char *nameptr;
    register struct sym_s *symptr;
    register unsigned hashval;
    register unsigned length;
#ifdef DEBUG
    int tries;

    ++nlookup;
    tries = 0;
#endif

    /* Hash function is a weighted xor of 1 to 4 chars in the string.
     * This works seems to work better than looking at all the chars.
     * It is important that the function be fast.
     * The string comparision function should also be fast and it helps
     * if it is optimized for mostly identical comparisions.
     * The multiplication by MULTIPLIER should compile as a shift.
     */

#define MULTIPLIER (SPTSIZ / (1 << USEFUL_BITS_IN_ASCII))
#define USEFUL_BITS_IN_ASCII 6

    nameptr = lineptr;
    length = nameptr - symname;
    if (length <= 3)
    {
	if (length <= 2)
	    hashval  = hconv(nameptr[-1]) * MULTIPLIER;
	else
	    hashval  = hconv(nameptr[-2]) * MULTIPLIER,
	    hashval ^= hconv(nameptr[-1]);
    }
    else
	hashval  = hconv(nameptr[-(length / 2)]) * MULTIPLIER,
	hashval ^= hconv(nameptr[-2]) << 2,
	hashval ^= hconv(nameptr[-1]);
    nameptr = symname;
    if ((symptr = *(hashptr = spt +
			      (hashval ^ (hconv(nameptr[0]) << 1)) % SPTSIZ))
	!= NULL)
    {
	do
	{
#ifdef DEBUG
	    if (tries != 0)
		--nx[tries];
	    ++tries;
	    if (tries < sizeof nx / sizeof nx[0])
		++nx[tries];
	    if (tries >= 5)
		printchain(hashptr - spt)
#endif
	    if ((unsigned char) length != symptr->length)
		continue;
	    if (memcmp(symptr->name, nameptr, length) == 0)
		return symptr;
	}
	while ((symptr = symptr->next) != NULL);

	/* Calculate last non-NULL hash ptr.
	 * This is faster than keeping hashptr up to date in previous loop
	 * since most lookups are successful and hash ptr is not needed.
	 */
	do
	{
	    symptr = *hashptr;
	    hashptr = &symptr->next;
	}
	while (symptr->next != NULL);
    }
    if (!ifflag)
	return NULL;
    align(heapptr);
    if (heapptr >= heapend)
	fatalerror(SYMOV);
#ifdef DEBUG
    ++nsym;
    if (hashptr >= spt && hashptr < spt + SPTSIZ)
	++nhash;
#endif
    *hashptr = symptr = (struct sym_s *) heapptr;
    symptr->type = 0;
    symptr->data = inidata;
    symptr->length = length;
    symptr->value_reg_or_op.value = (unsigned) (symptr->next = NULL);
    heapptr = symptr->name;
    do
	*heapptr++ = *nameptr++;
    while (--length != 0);
    *heapptr++ = 0;
    return symptr;
}

#ifdef DEBUG

static void printchain(hashval)
unsigned hashval;
{
    register struct sym_s *symptr;

    printf("%04x ", hashval);
    for (symptr = spt[hashval]; symptr != NULL; symptr = symptr->next)
	printf("%s ", symptr->name);
    printf("\n");
}

#endif

PUBLIC void statistics()
{
#ifdef DEBUG
    int i;
    int weight;

    for (i = 0; i < SPTSIZ; ++i)
	printchain(i);
    printf("nhash = %d, nsym = %d, nlookup = %d nx =\n", nhash, nsym, nlookup);
    weight = 0;
    for (i = 0; i < 30; ++i) 
    {
	printf("%5d", nx[i]);
	weight += nx[i] * i;
    }
    printf("\n");
    printf("weight = %d%d\n", w;
#endif
}
