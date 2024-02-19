/*
 * Bawk C actions compiler
 */
#include <ctype.h>
#include <stdio.h>
#include "bawk.h"

act_compile( actbuf )
char    *actbuf;        /* where tokenized actions are compiled into */
{
        Where = ACTION;
        return stmt_compile( actbuf );
}

pat_compile( actbuf )
char    *actbuf;        /* where tokenized actions are compiled into */
{
        Where = PATTERN;
        return stmt_compile( actbuf );
}

stmt_compile( actbuf )
char    *actbuf;        /* where tokenized actions are compiled into */
{
        /*
         * Read and tokenize C actions from current input file into the
         * action buffer.  Strip out comments and whitespace in the
         * process.
         */
        char    *actptr,        /* actbuf pointer */
                *cp,            /* work pointer */
                buf[MAXLINELEN];/* string buffer */
        int     braces,         /* counts '{}' pairs - return when 0 */
                parens,         /* counts '()' pairs */
                i,              /* temp */
                c;              /* current input character */

        braces = parens = 0;
        actptr = actbuf;
        while ( (c = getcharacter()) != -1 )
        {
                /*
                 * Skip over spaces, tabs and newlines
                 */
                if ( c==' ' || c=='\t' || c=='\n' )
                        continue;
                if ( c=='#' )
                {
                        /*
                         * Skip comments.  Comments start with a '#' and
                         * end at the next newline.
                         */
                        while ( (c = getcharacter()) != -1 && c!='\n' )
                                ;
                        continue;
                }

                if ( c=='{' )
                {
                        if ( Where==PATTERN )
                        {
                                /*
                                 * We're compiling a pattern. The '{' marks
                                 * the beginning of an action statement.
                                 * Push the character back and return.
                                 */
                                ungetcharacter( '{' );
                                break;
                        }
                        else
                        {
                                /*
                                 * We must be compiling an action statement.
                                 * '{'s mark beginning of action or compound
                                 * statements.
                                 */
                                ++braces;
                                *actptr++ = T_LBRACE;
                        }
                }
                else if ( c=='}' )
                {
                        *actptr++ = T_RBRACE;
                        if ( ! --braces )
                                /*
                                 * Found the end of the action string
                                 */
                                break;
                }
                else if ( c=='(' )
                {
                        ++parens;
                        *actptr++ = T_LPAREN;
                }
                else if ( c==')' )
                {
                        if ( --parens < 0 )
                                error( "mismatched '()'", ACT_ERROR );
                        *actptr++ = T_RPAREN;
                }
                else if ( c==',' && !braces && !parens && Where==PATTERN )
                {
                        /*
                         * found a comma outside of any braces or parens-
                         * this must be a regular expression seperator.
                         */
                        ungetcharacter( ',' );
                        break;
                }

                /*
                 * Check if it's a regular expression:
                 */
                else if ( c=='/' )
                {
                        /*
                         * A '/' inside a pattern string starts a regular
                         * expression.  Inside action strings, a '/' is
                         * the division operator.
                         */
                        if ( Where == PATTERN )
                                goto dopattern;
                        else
                                *actptr++ = T_DIV;
                }
                else if ( c=='@' )
                {
dopattern:
                        /*
                         * Within action strings, only the '@' may be used to
                         * delimit regular expressions
                         */
                        *actptr++ = T_REGEXP;
                        ungetcharacter( c );
                        actptr += re_compile( actptr );
                }

                /*
                 * symbol, string or constant:
                 */
                else if ( alpha( c ) )
                {
                        /*
                         * It's a symbol reference. Copy the symbol into
                         * string buffer.
                         */
                        cp = buf;
                        do
                                *cp++ = c;
                        while ( (c=getcharacter()) != -1 && alphanum( c ) );
                        ungetcharacter( c );
                        *cp = 0;
                        /*
                         * Check if a keyword, builtin function or variable.
                         */
                        if ( c = iskeyword( buf ) )
                                *actptr++ = c;
                        else if ( i = isfunction( buf ) )
                        {
                                *actptr++ = T_FUNCTION;
                                *(int *) ALIGN(actptr) = i;
                                actptr += sizeof( int )/sizeof( char );
                        }
                        else
                        {
                                /*
                                 * It's a symbol name.
                                 */
                                *actptr++ = T_VARIABLE;
                                if ( !(cp = (char *) findvar( buf )) )
                                        cp = (char *) addvar( buf );
                                *(char **) ALIGN(actptr) = cp;
                                actptr += sizeof( char * )/sizeof( char );
                        }
                }

                else if ( c == '"' )
                {
                        /*
                         * It's a string constant
                         */
                        *actptr++ = T_STRING;
                        actptr = str_compile( actptr, '"' );
                }
                else if ( c == '\'' )
                {
                        /*
                         * It's a character constant
                         */
                        *actptr++ = T_CONSTANT;
                        str_compile( buf, '\'' );
                        *(INT *) ALIGN(actptr) = *buf;
                        actptr += sizeof( INT )/sizeof( char );
                }

                else if ( isdigit( c ) )
                {
                        /*
                         * It's a numeric constant
                         */
                        *actptr++ = T_CONSTANT;
                        cp = buf;
                        do
                                *cp++ = c;
                        while ( (c=getcharacter()) != -1 && isdigit( c ) );
                        ungetcharacter( c );
                        *cp = 0;
                        *(INT *) ALIGN(actptr) = atoi( buf );
                        actptr += sizeof( INT )/sizeof( char );
                }

                /*
                 * unary operator:
                 */
                else if ( c == '$' )
                        *actptr++ = T_DOLLAR;

                /*
                 * or binary operator:
                 */
                else if ( c == '=' )
                {
                        if ( (c=getcharacter()) == '=' )
                                *actptr++ = T_EQ;
                        else
                        {
                                ungetcharacter( c );
                                *actptr++ = T_ASSIGN;
                        }
                }

                else if ( c == '!' )
                {
                        if ( (c=getcharacter()) == '=' )
                                *actptr++ = T_NE;
                        else
                        {
                                ungetcharacter( c );
                                *actptr++ = T_LNOT;
                        }
                }

                else if ( c == '<' )
                {
                        if ( (c=getcharacter()) == '<' )
                                *actptr++ = T_SHL;
                        else if ( c == '=' )
                                *actptr++ = T_LE;
                        else
                        {
                                ungetcharacter( c );
                                *actptr++ = T_LT;
                        }
                }

                else if ( c == '>' )
                {
                        if ( (c=getcharacter()) == '>' )
                                *actptr++ = T_SHR;
                        else if ( c == '=' )
                                *actptr++ = T_GE;
                        else
                        {
                                ungetcharacter( c );
                                *actptr++ = T_GT;
                        }
                }

                else if ( c == '&' )
                {
                        if ( (c=getcharacter()) == '&' )
                                *actptr++ = T_LAND;
                        else
                        {
                                ungetcharacter( c );
                                *actptr++ = T_AND;
                        }
                }

                else if ( c == '|' )
                {
                        if ( (c=getcharacter()) == '|' )
                                *actptr++ = T_LIOR;
                        else
                        {
                                ungetcharacter( c );
                                *actptr++ = T_IOR;
                        }
                }
                else if ( c == '+' )
                {
                        if ( (c=getcharacter()) == '+' )
                                *actptr++ = T_INCR;
                        else
                        {
                                ungetcharacter( c );
                                *actptr++ = T_ADD;
                        }
                }

                else if ( c == '-' )
                {
                        if ( (c=getcharacter()) == '-' )
                                *actptr++ = T_DECR;
                        else
                        {
                                ungetcharacter( c );
                                *actptr++ = T_SUB;
                        }
                }

                /*
                 * punctuation
                 */
                else if ( instr( c, "[](),;*/%+-^~" ) )
                        *actptr++ = c;

                else
                {
                        /*
                         * Bad character in input line
                         */
                        error( "lexical error", ACT_ERROR );
                }

                if ( actptr >= Workbuf + MAXWORKBUFLEN )
                        error( "action too long", MEM_ERROR );
        }
        if ( braces || parens )
                error( "mismatched '{}' or '()'", ACT_ERROR );

        *actptr++ = T_EOF;

        return actptr - actbuf;
}

/* FIX FvK: was INT */
char *str_compile( str, delim )
char *str, delim;
{
        /*
         * Compile a string from current input file into the given string
         * buffer.  Stop when input character is the delimiter in "delim".
         * Returns a pointer to the first character after the string.
         */
        int c;
        char buf[ MAXLINELEN ];

        while ( (c = getcharacter()) != -1 && c != delim)
        {
                if ( c  == '\\' )
                {
                        switch ( c = getcharacter() )
                        {
                        case -1: goto err;
                        case 'b': c = '\b'; break;
                        case 'n': c = '\n'; break;
                        case 't': c = '\t'; break;
                        case 'f': c = '\f'; break;
                        case 'r': c = '\r'; break;
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                                *buf = c;
                                for ( c=1; c<3; ++c )
                                {
                                        if ( (buf[c]=getcharacter()) == -1 )
                                                goto err;
                                }
                                buf[c] = 0;
                                sscanf( buf, "%o", &c );
                                break;
                        case '\n':
                                if ( getcharacter() == -1 )
                                        goto err;
                        default:
                                if ( (c = getcharacter()) == -1 )
                                        goto err;
                        }
                }
                *str++ = c;
        }
        *str++ = 0;

        return str;
err:
        sprintf( buf, "missing %c delimiter", delim );
        error( buf, 4 );
}


getoken()
{
        char *cp;
        int i;

        switch ( Token = *Actptr++ )
        {
        case T_STRING:
        case T_REGEXP:
                Value.dptr = Actptr;
                Actptr += strlen( Actptr ) + 1;
                break;
        case T_VARIABLE:
                Value.dptr = *(char **) ALIGN(Actptr);
                Actptr += sizeof( char * )/sizeof( char );
                break;
        case T_FUNCTION:
                Value.ival = *(int *) ALIGN(Actptr);
                Actptr += sizeof( int )/sizeof( char );
                break;
        case T_CONSTANT:
                Value.ival = *(INT *) ALIGN(Actptr);
                Actptr += sizeof( INT )/sizeof( char );
                break;
        case T_EOF:
                --Actptr;
        default:
                Value.dptr = 0;
        }

#ifdef DEBUG
        if ( Debug > 1 )
                printf( "Token='%c' (0x%x), Value=%d\n",
                        Token,Token,Value.ival );
#endif

        return Token;
}
