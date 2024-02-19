/*
 *   bin86/as/macro.c
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* macro.c - expand macros for assembler */

#include "const.h"
#include "type.h"
#include "globvar.h"
#include "scan.h"
#undef EXTERN
#define EXTERN
#include "macro.h"

/*
  Enter macro: stack macro and get its parameters.
  Parameters form a linked list of null-terminated strings of form
  next:string. The first string is the macro number in 4 bytes.
*/

PUBLIC void entermac(symptr)
struct sym_s *symptr;
{
    if (maclevel >= MAXMAC)
	error(MACOV);
    else if (macpar + 2 > macptop)
	error(PAROV);		/* no room for 0th param */
				/* (2 structs to fit it!) */
    else
    {
	char ch;
	struct schain_s *param1;
	register char *reglineptr;
	register char *stringptr;

	++maclevel;
	(--macstak)->text = (char *) symptr->value_reg_or_op.value;
	macstak->parameters = param1 = macpar;
	param1->next = NULL;
	*(stringptr = build_number(++macnum, 3, param1->string)) = 0;
	macpar = (struct schain_s *) (stringptr + 1);
				/* TODO: alignment */
	getsym();
	if (sym != LPAREN)
	    return;		/* no other params */
	reglineptr = lineptr;
	stringptr = macpar->string;
	while (TRUE)
	{
	    if (stringptr >= (char *) macptop)
	    {
		symname = reglineptr;
		error(PAROV);
		return;
	    }
	    ch = *reglineptr++;
	    if (ch == '/')
		/* escaped means no special meaning for slash, comma, paren */
		ch = *reglineptr++;
	    else if (ch == ',' || ch == ')')
	    {
		if (stringptr >= (char *) macptop)
		{
		    symname = reglineptr;
		    error(PAROV);	/* no room for null */
		    return;
		}
		*stringptr = 0;
		param1->next = macpar;	/* ptr from previous */
		(param1 = macpar)->next = NULL;
					/* this goes nowhere */
		macpar = (struct schain_s *) (stringptr + 1);
					/* but is finished OK - TODO align */
		stringptr = macpar->string;
		if (ch == ')')
		    return;
		continue;
	    }
	    if ((*stringptr++ = ch) == 0)
	    {
		symname = reglineptr;
		error(RPEXP);
		return;
	    }
	}
    }
}

/* MACRO pseudo-op */

PUBLIC void pmacro()
{
    bool_t saving;
    bool_t savingc;
    struct sym_s *symptr;

    saving =			/* prepare for bad macro */
	savingc = FALSE;	/* normally don't save comments */
    macload = TRUE;		/* show loading */
    if (label != NULL)
	error(ILLAB);
    else if (sym != IDENT)
	error(LABEXP);
    else
    {
	symptr = gsymptr;
	if (symptr->type & MNREGBIT)
	    error(LABEXP);
	else if (symptr->type & LABIT || symptr->data & FORBIT)
	    error(RELAB);
	else if (pass == 0 || symptr->type & REDBIT)
				/* copy on pass 0, also pass 1 if redefined */
	{
	    saving = TRUE;
	    if (symptr->type & MACBIT)
		symptr->type |= REDBIT;
	    else
		symptr->type |= MACBIT;
	    symptr->data = UNDBIT;	/* undefined till end */
	    symptr->value_reg_or_op.value = (unsigned) heapptr;
					/* beginning of store for macro */
					/* value s.b. (char *) */
	    getsym_nolookup();		/* test for "C" */
	    if (sym == IDENT && lineptr == symname + 1 && *symname == 'C')
		savingc = TRUE;
	}
    }
    while (TRUE)
    {
	skipline();
	listline();
	readline();
	if (!macload)
	    break;		/* macload cleared to show eof */
	getsym_nolookup();
	if (sym == IDENT)
	{
	    if (lineptr == symname + 4 && strncmp(symname, "MEND", 4) == 0)
		break;
	}
	else if (sym != MACROARG)
	{
	    if (!savingc)
		continue;	/* don't save comment */
	}
	if (!saving)
	    continue;
	{
	    register char *reglineptr;
	    register char *regheapptr;

	    reglineptr = linebuf;
	    regheapptr = heapptr;
	    do
	    {
		if (regheapptr >= heapend)
		{
		    heapptr = regheapptr;
		    fatalerror(SYMOV);	/* won't fit */
		}
	    }
	    while ((*regheapptr++ = *reglineptr++) != EOLCHAR);
	    heapptr = regheapptr;
	}
    }
    macload = FALSE;
    if (saving)
    {
	*heapptr++ = ETB;
	symptr->data = 0;
    }
}
