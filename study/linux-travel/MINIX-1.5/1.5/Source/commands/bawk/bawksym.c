/*
 * Bawk C actions builtin functions, variable declaration, and
 * stack management routines.
 */
#include <ctype.h>
#include <stdio.h>
#include "bawk.h"

#define MAXARGS         10      /* max # of arguments to a builtin func */
#define F_PRINT         1
#define F_PRINTF        2
#define F_GETLINE       3
#define F_STRLEN        4
#define F_STRCPY        5
#define F_STRCMP        6
#define F_TOUPPER       7
#define F_TOLOWER       8
#define F_MATCH         9
#define F_NEXTFILE      10

isfunction( s )
char *s;
{
        /*
         * Compare the string "s" to a list of builtin functions
         * and return its (non-zero) token number.
         * Return zero if "s" is not a function.
         */
        if ( !strcmp(s, "print") )
                return F_PRINT;
        if ( !strcmp( s, "printf" ) )
                return F_PRINTF;
        if ( !strcmp( s, "getline" ) )
                return F_GETLINE;
        if ( !strcmp( s, "strlen" ) )
                return F_STRLEN;
        if ( !strcmp( s, "strcpy" ) )
                return F_STRCPY;
        if ( !strcmp( s, "strcmp" ) )
                return F_STRCMP;
        if ( !strcmp( s, "toupper" ) )
                return F_TOUPPER;
        if ( !strcmp( s, "tolower" ) )
                return F_TOLOWER;
        if ( !strcmp( s, "match" ) )
                return F_MATCH;
        if ( !strcmp( s, "nextfile" ) )
                return F_NEXTFILE;
        return 0;
}

iskeyword( s )
char *s;
{
        /*
         * Compare the string "s" to a list of keywords and return its
         * (non-zero) token number.  Return zero if "s" is not a keyword.
         */
        if ( !strcmp( s, "char" ) )
                return T_CHAR;
        if ( !strcmp( s, "int" ) )
                return T_INT;
        if ( !strcmp( s, "if" ) )
                return T_IF;
        if ( !strcmp( s, "else" ) )
                return T_ELSE;
        if ( !strcmp( s, "while" ) )
                return T_WHILE;
        if ( !strcmp( s, "break" ) )
                return T_BREAK;

        if ( !strcmp( s, "NF" ) )
                return T_NF;
        if ( !strcmp( s, "NR" ) )
                return T_NR;
        if ( !strcmp( s, "FS" ) )
                return T_FS;
        if ( !strcmp( s, "RS" ) )
                return T_RS;
        if ( !strcmp( s, "FILENAME" ) )
                return T_FILENAME;
        if ( !strcmp( s, "BEGIN" ) )
                return T_BEGIN;
        if ( !strcmp( s, "END" ) )
                return T_END;
        return 0;
}

function( funcnum )
{
        int argc, j;
        char lpar;
        DATUM args[ MAXARGS ];

        argc = 0;
        if ( Token==T_LPAREN )
        {
                lpar = 1;
                getoken();
        }
        else
                lpar = 0;
        /*
         * If there are any arguments, evaluate them and copy their values
         * to a local array.  Clear the array first.
         */

        for (j = 0; j < MAXARGS; j++) args[j].ival = 0;

        if ( Token!=T_RPAREN && Token!=T_EOF )
        {
                for ( ;; )
                {
                        expression();
                        if ( argc<MAXARGS )
                                args[ argc++ ].ival = popint();
                        else
                                popint();
                        if ( Token==T_COMMA )
                                getoken();
                        else
                                break;
                }
        }
        if ( lpar )
        {
        	if ( Token!=T_RPAREN )
                	error( "missing ')'", ACT_ERROR );
	        else
	                getoken();
	}

        switch ( funcnum )
        {
        case F_PRINT:   /* quick and simple string printing */
                pushint( (INT)pprint( args ) );
                break;
        case F_PRINTF:  /* just like the real printf() function */
                pushint( (INT)pprntf( args[0].dptr, &args[1] ) );
                break;
        case F_GETLINE:
                /*
                 * Get the next line of input from the current input file
                 * and parse according to the current field seperator.
                 * Don't forget to free up the previous line's words first...
                 */
                while ( Fieldcount )
                        free( Fields[ --Fieldcount ] );
                pushint( (INT)getline() );
                Fieldcount = parse( Linebuf, Fields, Fieldsep );
                break;
        case F_STRLEN:  /* calculate length of string argument */
                pushint( (INT)strlen( args[0].dptr ) );
                break;
        case F_STRCPY:  /* copy second string argument to first string */
                pushint( (INT)strcpy( args[0].dptr, args[1].dptr ) );
                break;
        case F_STRCMP:  /* compare two strings */
                pushint( (INT)strcmp( args[0].dptr, args[1].dptr ) );
                break;
        case F_TOUPPER: /* convert the character argument to upper case */
                pushint( (INT)toupper( args[0].ival ) );
                break;
        case F_TOLOWER: /* convert the character argument to lower case */
                pushint( (INT)tolower( args[0].ival ) );
                break;
        case F_MATCH:   /* match a string argument to a regular expression */
                pushint( (INT)match( args[0].dptr, args[1].dptr ) );
                break;
        case F_NEXTFILE:/* close current input file and process next file */
                pushint( (INT)endfile() );
                break;
        default:        /* oops! */
                error( "bad function call", ACT_ERROR );
        }
}

VARIABLE *
findvar( s )
char *s;
{
        /*
         * Search the symbol table for a variable whose name is "s".
         */
        VARIABLE *pvar;
        int i;
        char name[ MAXVARLEN ];

        i = 0;
        while ( i < MAXVARLEN && alphanum( *s ) )
                name[i++] = *s++;
        if ( i<MAXVARLEN )
                name[i] = 0;

        for ( pvar = Vartab; pvar<Nextvar; ++pvar )
        {
                if ( !strncmp( pvar->vname, name, MAXVARLEN ) )
                        return pvar;
        }
        return NULL;
}

VARIABLE *
addvar( name )
char *name;
{
        /*
         * Add a new variable to symbol table and assign it default
         * attributes (int name;)
         */
        int i;

        if ( Nextvar <= Vartab + MAXVARTABSZ )
        {
                i = 0;
                while ( i<MAXVARLEN && alphanum( *name ) )
                        Nextvar->vname[i++] = *name++;
                if ( i<MAXVARLEN )
                        Nextvar->vname[i] = 0;

                Nextvar->vclass = 0;
                Nextvar->vsize = WORD;
                Nextvar->vlen = 0;
                /*
                 * Allocate some new room
                 */
                Nextvar->vptr = getmem((unsigned int) WORD );
                fillmem( Nextvar->vptr, WORD, 0 );
        }
        else
                error( "symbol table overflow", MEM_ERROR );

        return Nextvar++;
}

declist()
{
        /*
         * Parse a "char" or "int" statement.
         */
        char type;

        type = Token;
        getoken();
        decl( type );
        while ( Token==T_COMMA )
        {
                getoken();
                decl( type );
        }
        if ( Token==T_SEMICOLON )
                getoken();
}

VARIABLE *
decl( type )
{
        /*
         * Parse an element of a "char" or "int" declaration list.
         * The function stmt_compile() has already entered the variable
         * into the symbol table as an integer, this routine simply changes
         * the symbol's class, size or length according to the declaraction.
         * WARNING: The interpreter depends on the fact that pointers are
         * the same length as int's.  If your machine uses long's for
         * pointers either change the code or #define int long (or whatever).
         */
        char class, size;
        int len;
        unsigned oldsize, newsize;
        VARIABLE *pvar;

        if ( Token==T_MUL )
        {
                /*
                 * it's a pointer
                 */
                getoken();
                pvar = decl( type );
                ++pvar->vclass;
        }
        else if ( Token==T_VARIABLE )
        {
                /*
                 * Simple variable so far.  The token value (in the global
                 * "Value" variable) is a pointer to the variable's symbol
                 * table entry.
                 */
                pvar = (VARIABLE *)Value.dptr;
                getoken();
                class = 0;
                /*
                 * Compute its length
                 */
                if ( Token==T_LBRACKET )
                {
                        /*
                         * It's an array.
                         */
                        getoken();
                        ++class;
                        /*
                         * Compute the dimension
                         */
                        expression();
                        if ( Token!=T_RBRACKET )
                                error( "missing ']'", ACT_ERROR );
                        getoken();
                        len = popint();
                }
                else
                        /*
                         * It's a simple variable - array length is zero.
                         */
                        len = 0;

                size = (type==T_CHAR) ? BYTE : WORD;

                newsize = (len ? len : 1) * size;
                oldsize = (pvar->vlen ? pvar->vlen : 1) * pvar->vsize;
                if ( newsize != oldsize )
                {
                        /*
                         * The amount of storage needed for the variable
                         * has changed - free up memory allocated initially
                         * and reallocate for new size.
                         */
                        free( pvar->vptr );
                        pvar->vptr = getmem( newsize );
                }
                /*
                 * Now change the variable's attributes.
                 */
                pvar->vclass = class;
                pvar->vsize = size;
                pvar->vlen = len;
        }
        else
                syntaxerror();

        return pvar;
}

assignment()
{
        /*
         * Perform an assignment
         */
        INT ival;

        ival = popint();
        /*
         * make sure we've got an lvalue
         */
        if ( Stackptr->lvalue )
        {
                if ( Stackptr->class )
                        movemem((char *) &ival, Stackptr->value.dptr, WORD );
                else
                        movemem((char *) &ival, Stackptr->value.dptr, Stackptr->size);
                pop();
                pushint( ival );
        }
        else
                error( "'=' needs an lvalue", ACT_ERROR );
}

INT pop()
{
        /*
         * Pop the stack and return the integer value
         */
        if ( Stackptr >= Stackbtm )
                return (Stackptr--)->value.ival;
        return error( "stack underflow", ACT_ERROR );
}

push( pclass, plvalue, psize, pdatum )
char pclass, plvalue, psize;
DATUM *pdatum;
{
        /*
         * Push item parts onto the stack
         */
        if ( ++Stackptr <= Stacktop )
        {
                Stackptr->lvalue = plvalue;
                Stackptr->size = psize;
                if ( !(Stackptr->class = pclass)  &&  !plvalue )
                        Stackptr->value.ival = pdatum->ival;
                else
                        Stackptr->value.dptr = pdatum->dptr;
        }
        else
                error( "stack overflow", MEM_ERROR );
}

pushint( intvalue )
INT intvalue;
{
        /*
         * push an integer onto the stack
         */
        if ( ++Stackptr <= Stacktop )
        {
                Stackptr->lvalue =
                Stackptr->class = 0;
                Stackptr->size = WORD;
                Stackptr->value.ival = intvalue;
        }
        else
                error( "stack overflow", MEM_ERROR );
}

INT popint()
{
        /*
         * Resolve the item on the top of the stack and return it
         */
        INT intvalue;

        if ( Stackptr->lvalue )
        {
                /*
                 * if it's a byte indirect, sign extend it
                 */
                if ( Stackptr->size == BYTE && !Stackptr->class )
                        intvalue = *Stackptr->value.dptr;
                else
                {
                        /*
                         * otherwise, it's an unsigned int
                         */
                        intvalue = *(int *)Stackptr->value.dptr;
                }
                pop();
                return intvalue;
        }
        else
        {
                /*
                 * else it's an ACTUAL, just pop it
                 */
                return pop();
        }
}


pprint( args )
DATUM args[];
{
	/*
	 * execute the "print string, ..." command
	 */
	int i;

	for ( i=0; i<MAXARGS; i++ )
		if ( args[i].dptr != (char *)NULL )
			printf(" %s", args[i].dptr);
 	printf("\n");  /* AWK's default behaviour */

	return 0;
}


pprntf( fmt, args )
char *fmt;
DATUM args[];
{
	/*
	 * execute the "printf fmt, data ..." command
	 */
	char lfmt[40], *t;
	register char *s;
	int i, type, lflg;
#define LINT	0
#define SINT	1
#define	PNTR	2

	s = fmt;
	i = 0;
	while ( *s != '\0' && i < MAXARGS-1 )
	{
		if ( *s != '%' )
		{
			putc( *s++, stdout );
			continue;
		}
		t = lfmt;
		*t++ = *s++;		/* % */
		if ( *s == '-' || *s == '+' )
			*t++ = *s++;	/* sign */
		while ( *s >= '0' && *s <= '9' || *s == '*' )
			*t++ = *s++;	/* width */
		if ( *s == '.' )
			*t++ = *s++;	/* . */
		while ( *s >= '0' && *s <= '9' || *s == '*' )
			*t++ = *s++;	/* digits */

		lflg = (*s == 'l' || *s == 'L');
		if ( lflg )
			*t++ = *s++ ;
		if ( *s == 'd' || *s == 'D' || *s == 'u' || *s == 'U' ||
		     *s == 'x' || *s == 'X' || *s == 'o' || *s == 'O' )
		{
			type = lflg ? LINT : SINT;
			*t++ = *s++;
		}
		else if ( *s == 'p' || *s == 'P' || *s == 's' || *s == 'S' )
		{
			type = PNTR;
			*t++ = *s++;
		}
		else /* yuk. Better to forbid %e, %f and %g here ? */
		{
			type = SINT;
			if (*s) *t++ = *s++;
		}
		*t++ = '\0';

		switch ( type )
		{
		    case LINT:
			printf( lfmt, (long) args[i++].ival );
			break;
		    case SINT:
			printf( lfmt, (int) args[i++].ival );
			break;
		    case PNTR:
			printf( lfmt, args[i++].dptr );
			break;
		    default:
			/* impossible */
			break;
		}
	}

	return 0;
}
