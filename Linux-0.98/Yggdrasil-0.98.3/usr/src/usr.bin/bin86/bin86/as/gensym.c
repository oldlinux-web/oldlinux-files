/*
 *   bin86/as/gensym.c
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* gensym.c - generate symbol table for assembler */

#include "const.h"
#include "type.h"
#include "flag.h"
#include "file.h"
#include "globvar.h"

FORWARD int printsym P((register struct sym_s *symptr, unsigned column));
FORWARD void sort P((struct sym_s **array, struct sym_s **top,
		     bool_pt nameflag));

/* sort labels in symbol table on name and value */
/* if listing, write human-readable table to list file */
/* if symbol file, write machine-readable tables to it */
/* pointers become relative to start of file */

PUBLIC void gensym()
{
    unsigned column;
    struct sym_s **copyptr;
    struct sym_s **copytop;
    register struct sym_s **hashptr;
    unsigned label_count;	/* number of labels */
    unsigned labels_length;	/* length of all label strings */
    unsigned label_stringptr;	/* offset of label str from start of file */
    register struct sym_s *symptr;
    struct sym_s **symlptr;	/* start of symbol output list */

    labels_length = label_count = 0;

    /* make copy of all relavant symbol ptrs on heap */
    /* original ptrs can now be modified, but need to be an array for sort */

    align(heapptr);
    for (hashptr = spt, symlptr = copyptr = (struct sym_s **) heapptr;
	 hashptr < spt_top;)
	if ((symptr = *hashptr++) != NULL)
	    do
		if (!(symptr->type & (MACBIT | MNREGBIT | VARBIT)))
		{
		    if (copyptr >= (struct sym_s **) heapend)
		    {
			heapptr = (char *) copyptr;
			error(SYMOUTOV);	/* avoid recursive fatalerror */
			listline();	/* the main job is OK if here */
			goto sort_symbols;
		    }
		    *copyptr++ = symptr;
		    ++label_count;
		    labels_length += symptr->length + 3; /* 3 for type, value */
		}
	    while ((symptr = symptr->next) != NULL);

sort_symbols:
    sort(symlptr, copyptr, TRUE);	/* sort on name */
    heapptr = (char *) (copytop = copyptr);
    if (list.global)
    {
	innum = lstfil;
	writenl();
	writesn("Symbols:");
	for (copyptr = symlptr, column = 0; copyptr < copytop;)
	    column = printsym(*copyptr++, column);
	if (column != 0)
	    writenl();
    }
    if ((innum = symfil) != 0)
    {
	writew(mapnum);
	label_count *= 2;	/* now length of ptr table (2 bytes per ptr) */
	label_stringptr = label_count + 6;
				/* offset to current string in symbol file */
				/* 6 is length of header */
	labels_length += label_stringptr;
	/* offset to ptr table sorted on value */
	writew(labels_length + label_count);
	/* total length of symbol file */
	writew(label_count);
	for (copyptr = symlptr; copyptr < copytop;)
	{
	    symptr = *copyptr++;
	    writew((unsigned)
		   (symptr->next = (struct sym_s *) label_stringptr));
				/* reuse "next" to record string position */
	    label_stringptr += symptr->length + 3;
	}
	for (copyptr = symlptr; copyptr < copytop;)
	{
	    symptr = *copyptr++;
	    writew((unsigned) symptr->value_reg_or_op.value);
	    writec(symptr->type);
	    write(innum, symptr->name, symptr->length - 1);
	    writec(symptr->name[symptr->length - 1] | 0x80);
	}
	sort(symlptr, copyptr, FALSE);
	/* sort on value */
	for (copyptr = symlptr; copyptr < copytop;)
	{
	    symptr = *copyptr++;
	    writew((unsigned) symptr->next);	/* now has string position */
	}
    }
}

/* print symbol nicely formatted for given column */

PRIVATE int printsym(symptr, column)
register struct sym_s *symptr;
unsigned column;
{
    unsigned char length;
    register struct sym_listing_s *listptr;
    char *outname;
    char *symname;

    listptr = (struct sym_listing_s *) heapptr;
    memset((char *) listptr, ' ', SYMLIS_LEN);
    listptr->nullterm = 0;
    if ((length = symptr->length) > SYMLIS_NAMELEN)
    {
	outname = listptr->name;
	outname[length = SYMLIS_NAMELEN] = '+';
    }
    else
	outname = (listptr->name + SYMLIS_NAMELEN) - length;
    symname = symptr->name;
    do
	*outname++ = *symname++;
    while (--length != 0);
    listptr->ar[0] = symptr->data & RELBIT ? 'R' : 'A';
    listptr->segm[0] = (symptr->data & SEGM) + '0';
    if (symptr->type & COMMBIT)
	listptr->cein[0] = 'C';
    else if (symptr->type & ENTBIT)
	listptr->cein[0] = 'N';
    else if (symptr->type & EXPBIT)
	listptr->cein[0] = 'E';
    else if (symptr->data & IMPBIT)
	listptr->cein[0] = 'I';
#if SIZEOF_OFFSET_T > 2
    build_2hex_number((unsigned) (symptr->value_reg_or_op.value >> 16),
		      listptr->value);
#endif
    build_2hex_number((unsigned) symptr->value_reg_or_op.value,
		      listptr->value);
    writes((char *) listptr);
    if ((column += SYMLIS_LEN) > (80 - SYMLIS_LEN))
    {
	writenl();
	column = 0;
    }
    return column;
}

/* shell sort symbols */

PRIVATE void sort(array, top, nameflag)
struct sym_s **array;
struct sym_s **top;
bool_pt nameflag;
{
    int gap;
    int i;
    int j;
    register struct sym_s **left;
    register struct sym_s **right;
    int size;
    struct sym_s *swap;

    size = top - array;
    /* choose gaps according to Knuth V3 p95 */
    for (gap = 1, i = 4; (j = 3 * i + 1) < size; gap = i, i = j)
	;
    do
    {
	for (j = gap; j < size; ++j)
	    for (i = j - gap; i >= 0; i -= gap)
	    {
		left = &array[i];
		right = &array[i + gap];
		if ((bool_t) nameflag)
		{
		    if (strcmp((*left)->name, (*right)->name) <= 0)
			break;
		}
		else if ((unsigned) (*left)->value_reg_or_op.value <=
			 (*right)->value_reg_or_op.value)
		    break;
		swap = *left;
		*left = *right;
		*right = swap;
	    }
    }
    while ((gap /= 3) != 0);
}
