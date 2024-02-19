/*
 *   bin86/as/express.c
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* express.c - expression handler for assembler */

#include "const.h"
#include "type.h"
#include "address.h"
#include "globvar.h"
#include "scan.h"
#include "source.h"

FORWARD void experror P((error_pt errnum));
FORWARD void expundefined P((void));
FORWARD void simple2 P((void));
FORWARD void simple P((void));
FORWARD void term P((void));
FORWARD void factor2 P((void));

PUBLIC void absexpres()
{
    expres();
    chkabs();
}

/* check lastexp.data is abs */

PUBLIC void chkabs()
{
    if (lastexp.data & RELBIT)
    {
	if (pass != 0)
	    error(ABSREQ);
	expundefined();
    }
}

PRIVATE void experror(errnum)
error_pt errnum;
{
    error(errnum);
    expundefined();
}

PRIVATE void expundefined()
{
    lastexp.data = FORBIT | UNDBIT;
}

PUBLIC void nonimpexpres()
{
    expres();
    if (lastexp.data & IMPBIT)
	experror(NONIMPREQ);
}

/* generate relocation error if pass 2, make lastexp.data forward&undefined */

PUBLIC void showrelbad()
{
    if (pass != 0)
	error(RELBAD);
    expundefined();
}

PUBLIC void symabsexpres()
{
    getsym();
    absexpres();
}

PUBLIC void symexpres()
{
    getsym();
    expres();
}

/*
  expres() parses expression = simple expression [op simple expression],
  where op is =, < or >.
  Parameters: sym, number in number, identifier from symname to lineptr - 1.
  Returns value in lastexp.
*/

PUBLIC void expres()
{
    offset_t leftoffset;

    simple();
    leftoffset = lastexp.offset;
    if (sym == EQOP)
    {
	simple2();
	if (leftoffset == lastexp.offset)
	    lastexp.offset = -1;
	else
	    lastexp.offset = 0;
    }
    else if (sym == LESSTHAN)
    {
	/* context-sensitive, LESSTHAN really means less than here */
	simple2();
	if (leftoffset < lastexp.offset)
	    lastexp.offset = -1;
	else
	    lastexp.offset = 0;
    }
    else if (sym == GREATERTHAN)
    {
	/* context-sensitive, GREATERTHAN really means greater than here */
	simple2();
	if (leftoffset > lastexp.offset)
	    lastexp.offset = -1;
	else
	    lastexp.offset = 0;
    }
}

/* get symbol and 2nd simple expression, check both rel or both abs */

PRIVATE void simple2()
{
    unsigned char leftdata;

    leftdata = lastexp.data;
    getsym();
    simple();
    if ((leftdata | lastexp.data) & IMPBIT ||
	(leftdata ^ lastexp.data) & (RELBIT | SEGM))
	showrelbad();
    else
	lastexp.data = (leftdata & lastexp.data) & ~(RELBIT | SEGM);
}

/*
  simple() parses simple expression = [+-] term {op term},
  where op is +, -, or \ (OR).
*/

PRIVATE void simple()
{
    offset_t leftoffset;
    unsigned char leftdata;

    if (sym == ADDOP || sym == SUBOP)
	lastexp.data = lastexp.offset = 0;
    else
	term();
    while (TRUE)
    {
	leftoffset = lastexp.offset;
	leftdata = lastexp.data;
	if (sym == ADDOP)
	{
	    getsym();
	    term();
	    if (leftdata & lastexp.data & RELBIT)
		showrelbad();	/* rel + rel no good */
	    else
		lastexp.data |= leftdata;
	    lastexp.offset += leftoffset;
	}
	else if (sym == SUBOP)
	{
	    getsym();
	    term();
	    /* check not abs - rel or rel - rel with mismatch */
	    if (lastexp.data & RELBIT &&
		(!(leftdata & RELBIT) ||
		 (leftdata | lastexp.data) & IMPBIT ||
		 (leftdata ^ lastexp.data) & (RELBIT | SEGM)))
		showrelbad();
	    else
		lastexp.data = ((leftdata | lastexp.data) & ~(RELBIT | SEGM))
			     | ((leftdata ^ lastexp.data) &  (RELBIT | SEGM));
	    lastexp.offset = leftoffset - lastexp.offset;
	}
	else if (sym == OROP)
	{
	    getsym();
	    term();
	    lastexp.data |= leftdata;
	    chkabs();		/* both must be absolute */
	    lastexp.offset |= leftoffset;
	}
	else
	    return;
    }
}

/* term() parses term = factor {op factor}, where op is *, /, &, <<, or >>. */

PRIVATE void term()
{
    offset_t leftoffset;

    factor();
    while (TRUE)
    {
	leftoffset = lastexp.offset;
	if (sym == STAR)
	{
	    /* context-sensitive, STAR means multiplication here */
	    factor2();
	    lastexp.offset *= leftoffset;
	}
	else if (sym == SLASH)
	{
	    /* context-sensitive, SLASH means division here */
	    factor2();
	    lastexp.offset = leftoffset / lastexp.offset;
	}
	else if (sym == ANDOP)
	{
	    factor2();
	    lastexp.offset &= leftoffset;
	}
	else if (sym == SLOP)
	{
	    factor2();
	    lastexp.offset = leftoffset << lastexp.offset;
	}
	else if (sym == SROP)
	{
	    factor2();
	    lastexp.offset = leftoffset >> lastexp.offset;
	}
	else
	    return;
    }
}

/* get symbol and 2nd or later factor, check both abs */

PRIVATE void factor2()
{
    unsigned char leftdata;

    leftdata = lastexp.data;
    getsym();
    factor();
    lastexp.data |= leftdata;
    chkabs();
}

/*
  factor() parses factor = number | identifier | * | (expression) | ! factor,
  ! is complementation. Returns value in lastexp.offset, possible flags
  IMPBIT, FORBIT, RELBIT and UNDBIT in lastexp.data, and segment in SEGM
  part of lastexp.data, and lastexp.sym at imported symbol if IMPBIT.
  If the factor is an identifier, LOOKUP is used to get its value
  (so the ident is installed in the symbol table if necessary, with
  default flags inidata). If the identifier is not a label,
  (could be imported, or later in the program), its FORBIT is set.
  The expression FORBIT, IMPBIT, RELBIT, UNDBIT and SEGM are then
  taken from the identifier.
*/

PUBLIC void factor()
{
    switch (sym)
    {
    case SLASH:
	/* context-sensitive, SLASH means a hex number here */
	context_hexconst();
    case INTCONST:
	lastexp.data = 0;	/* absolute & not forward or undefined */
	lastexp.offset = number;
	getsym();
	return;
    case IDENT:
	{
	    register struct sym_s *symptr;

	    symptr = gsymptr;
	    if (symptr->type & (MNREGBIT | MACBIT))
		experror(symptr->type & MACBIT ? MACUID :
			 symptr->data & REGBIT ? REGUID : MNUID);
	    else
	    {
		if (!(symptr->type & (LABIT | VARBIT)))
		{
		    symptr->data |= FORBIT;
		    lastexp.sym = symptr;
		}
		if (pass == 0)
		{
		    lastexp.data = symptr->data &
			(FORBIT | RELBIT | UNDBIT | SEGM);
				/* possible flags for pass 1 */
		    lastexp.offset = symptr->value_reg_or_op.value;
		}
		else
		{
		    if ((lastexp.data = symptr->data) & IMPBIT)
			lastexp.offset = 0;	/* value != 0 for commons */
						/* OK even if UNDBIT */
		    else
		    {
			lastexp.offset = symptr->value_reg_or_op.value;
			if (lastexp.data & UNDBIT)
			    experror(UNBLAB);
		    }
		}
	    }
	    getsym();
	    return;
	}
    case LBRACKET:
	if (!asld_compatible)
	    break;		/* error, LPAREN is the grouping symbol */
	getsym();
	expres();
	if (sym != RBRACKET)
	    error(RBEXP);
	else
	    getsym();
	return;
    case LPAREN:
	if (asld_compatible)
	    break;		/* error, LBRACKET is the grouping symbol */
	getsym();
	expres();
	if (sym != RPAREN)
	    error(RPEXP);
	else
	    getsym();
	return;
    case NOTOP:
	getsym();
	factor();
	chkabs();
	lastexp.offset = ~lastexp.offset;
	return;
    case STAR:
	/* context-sensitive, STAR means location counter here */
	lastexp.offset = lc;
	if ((lastexp.data = lcdata) & UNDBIT && pass != 0)
	    experror(UNBLAB);
	getsym();
	return;
    }
    experror(FACEXP);
}

/*
  string compare for IFC/ELSEIFC
  expects (<string1>,<string2>)
  returns logical value in lastexp
*/

PUBLIC void scompare()
{
    /* prepare flags for OK, lastexp.offset for error */
    lastexp.data = lastexp.offset = 0;
    if (sym != LPAREN)
	experror(LPEXP);
    else
    {
	register char *string1;
	register char *string2;

	for (string2 = string1 = lineptr; *string2 != ')'; ++string2)
	    if (*string2 == 0 || *string2 == ')')
	    {
		symname = string2;
		experror(COMEXP);
		return;
	    }
	while (*string1++ == *string2++)
	    ;
	if (string2[-1] == ')')
	{
	    if (string1[-1] == ',')
		lastexp.offset = TRUE;	/* else leave FALSE */
	}
	else			/* FALSE, keep reading to verify syntax */
	    for (; *string2 != ')'; ++string2)
		if (*string2 == 0 || *string2 == ',')
		{
		    symname = string2;
		    experror(RPEXP);
		}
    }
}
