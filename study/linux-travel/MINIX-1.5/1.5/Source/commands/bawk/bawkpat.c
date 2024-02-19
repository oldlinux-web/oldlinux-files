/*
 * Bawk regular expression compiler/interpreter
 */
#include <ctype.h>
#include <stdio.h>
#include "bawk.h"

re_compile( patbuf )
char    *patbuf;                /* where to put compiled pattern */
{
        /*
         * Compile a regular expression from current input file
         * into the given pattern buffer.
         */
        int     c,              /* Current character         */
                o;              /* Temp                      */
        char    *patptr,        /* destination string pntr   */
                *lp,            /* Last pattern pointer      */
                *spp,           /* Save beginning of pattern */
                delim,          /* pattern delimiter         */
                *cclass();      /* Compile class routine     */

        lp = patptr = patbuf;
        delim = getcharacter();

        while ( (c = getcharacter()) != -1 && c != delim )
        {
                /*
                 * STAR, PLUS and MINUS are special.
                 */
                if (c == '*' || c == '+' || c == '-') {
                        if (patptr == patbuf ||
                                  (o=patptr[-1]) == BOL ||
                                  o == EOL ||
                                  o == STAR ||
                                  o == PLUS ||
                                  o == MINUS)
                                error( "illegal occurrance op", RE_ERROR );
                        *patptr++ = ENDPAT;
                        *patptr++ = ENDPAT;
                        spp = patptr;           /* Save pattern end     */
                        while (--patptr > lp)   /* Move pattern down... */
                                *patptr = patptr[-1];   /* one byte     */
                        *patptr =   (c == '*') ? STAR :
                                (c == '-') ? MINUS : PLUS;
                        patptr = spp;           /* Restore pattern end  */
                        continue;
                }
                /*
                 * All the rest.
                 */
                lp = patptr;                    /* Remember start       */
                switch(c) {

                case '^':
                        *patptr++ = BOL;
                        break;

                case '$':
                        *patptr++ = EOL;
                        break;

                case '.':
                        *patptr++ = ANY;
                        break;

                case '[':
                        patptr = cclass( patptr );
                        break;

                case ':':
                        if ( (c=getcharacter()) != -1 )
                        {
                                switch( tolower( c ) )
                                {

                                case 'a':
                                        *patptr++ = ALPHA;
                                        break;

                                case 'd':
                                        *patptr++ = DIGIT;
                                        break;

                                case 'n':
                                        *patptr++ = NALPHA;
                                        break;

                                case ' ':
                                        *patptr++ = PUNCT;
                                        break;

                                default:
                                        error( "unknown ':' type", RE_ERROR );

                                }
                        }
                        else
                                error( "no ':' type", RE_ERROR );
                        break;

                case '\\':
                        c = getcharacter();

                default:
                        *patptr++ = CHAR;
                        *patptr++ = c;
                }
        }
        *patptr++ = ENDPAT;
        *patptr++ = 0;                  /* Terminate string     */

#ifdef DEBUG
        if ( Debug>1 )
        {
                for ( lp=patbuf; lp<patptr; ++lp )
                {
                        switch ( c = *lp )
                        {
                        case CHAR:      printf("char "); break;
                        case BOL:       printf("bol "); break;
                        case EOL:       printf("eol "); break;
                        case ANY:       printf("any "); break;
                        case CLASS:     printf("class(%d) ", *++lp); break;
                        case NCLASS:    printf("notclass(%d) ",*++lp); break;
                        case STAR:      printf("star "); break;
                        case PLUS:      printf("plus "); break;
                        case MINUS:     printf("minus "); break;
                        case ALPHA:     printf("alpha "); break;
                        case DIGIT:     printf("digit "); break;
                        case NALPHA:    printf("notalpha "); break;
                        case PUNCT:     printf("punct "); break;
                        case RANGE:     printf("range "); break;
                        case ENDPAT:    printf("endpat "); break;
                        default:        printf("<%c> ", c); break;
                        }
                }
                printf( "\n" );
        }
#endif

        return patptr - patbuf;
}

char *
cclass( patbuf )
char    *patbuf;        /* destination pattern buffer */
{
        /*
         * Compile a class (within [])
         */
        char    *patptr,        /* destination pattern pointer */
                *cp;            /* Pattern start     */
        int     c,              /* Current character */
                o;              /* Temp              */

        patptr = patbuf;

        if ( (c = getcharacter()) == -1 )
                error( "class terminates badly", RE_ERROR );
        else if ( c == '^')
        {
                /*
                 * Class exclusion, for example: [^abc]
                 * Swallow the "^" and set token type to class exclusion.
                 */
                o = NCLASS;
        }
        else
        {
                /*
                 * Normal class, for example: [abc]
                 * push back the character and set token type to class
                 */
                ungetcharacter( c );
                o = CLASS;
        }
        *patptr++ = o;

        cp = patptr;    /* remember where byte count is */
        *patptr++ = 0;  /* and initialize byte count */
        while ( (c = getcharacter()) != -1 && c!=']' )
        {
                o = getcharacter();             /* peek at next char */
                if (c == '\\')                  /* Store quoted chars */
                {
                        if ( o == -1) /* Gotta get something */
                                error( "class terminates badly", RE_ERROR );
                        *patptr++ = o;
                }
                else if ( c=='-' && (patptr-cp)>1 && o!=']' && o != -1 )
                {
                        c = patptr[-1];         /* Range start     */
                        patptr[-1] = RANGE;     /* Range signal    */
                        *patptr++ = c;          /* Re-store start  */
                        *patptr++ = o;          /* Store end char  */
                }
                else
                {
                        *patptr++ = c;          /* Store normal char */
                        ungetcharacter( o );
                }
        }
        if (c != ']')
                error( "unterminated class", RE_ERROR );
        if ( (c = (patptr - cp)) >= 256 )
                error( "class too large", RE_ERROR );
        if ( c == 0 )
                error( "empty class", RE_ERROR );
        *cp = c;                /* fill in byte count */

        return patptr;
}

match( line, pattern )
char    *line;          /* line to match */
char    *pattern;       /* pattern to match */
{
        /*
         * Match the current line (in Linebuf[]), return 1 if it does.
         */
        char    *l;             /* Line pointer       */
        char    *pmatch();
        char    *next;
        int     matches;

        matches = 0;
        for (l = line; *l; l++)
        {
                if ( next = pmatch(line, l, pattern) )
                {
                        l = next - 1;
                        ++matches;
#ifdef DEBUG
                        if ( Debug )
                                printf( "match!\n" );
#endif
                }
        }

        return matches;
}

char *
pmatch(linestart, line, pattern)
char    *linestart;     /* start of line to match */
char    *line;          /* (partial) line to match      */
char    *pattern;       /* (partial) pattern to match   */
{
        char    *l;     /* Current line pointer         */
        char    *p;     /* Current pattern pointer      */
        char    c;      /* Current character            */
        char    *e;     /* End for STAR and PLUS match  */
        int     op;     /* Pattern operation            */
        int     n;      /* Class counter                */
        char    *are;   /* Start of STAR match          */

        l = line;

#ifdef DEBUG
        if (Debug > 1)
                printf("pmatch(\"%s\")\n", line);
#endif

        p = pattern;
        while ((op = *p++) != ENDPAT) {

#ifdef DEBUG
                if (Debug > 1)
                        printf("byte[%d] = 0%o, '%c', op = 0%o\n",
                                        l-line, *l, *l, op);
#endif

                switch(op) {

                case CHAR:
                        if ( *l++ != *p++)
                                return 0;
                        break;

                case BOL:
                        if (l != linestart)
                                return 0;
                        break;

                case EOL:
                        if (*l != '\0')
                                return 0;
                        break;

                case ANY:
                        if (*l++ == '\0')
                                return 0;
                        break;

                case DIGIT:
                        if ((c = *l++) < '0' || (c > '9'))
                                return 0;
                        break;

                case ALPHA:
                        c = tolower( *l++ );
                        if (c < 'a' || c > 'z')
                                return 0;
                        break;

                case NALPHA:
                        c = tolower(*l++);
                        if (c >= 'a' && c <= 'z')
                                break;
                        else if (c < '0' || c > '9')
                                return 0;
                        break;

                case PUNCT:
                        c = *l++;
                        if (c == 0 || c > ' ')
                                return 0;
                        break;

                case CLASS:
                case NCLASS:
                        c = *l++;
                        n = *p++ & 0377;
                        do {
                                if (*p == RANGE) {
                                        p += 3;
                                        n -= 2;
                                        if (c >= p[-2] && c <= p[-1])
                                                break;
                                }
                                else if (c == *p++)
                                        break;
                        } while (--n > 1);
                        if ((op == CLASS) == (n <= 1))
                                return 0;
                        if (op == CLASS)
                                p += n - 2;
                        break;

                case MINUS:
                        e = pmatch(linestart,l,p);/* Look for a match    */
                        while (*p++ != ENDPAT); /* Skip over pattern   */
                        if (e)                  /* Got a match?        */
                                l = e;          /* Yes, update string  */
                        break;                  /* Always succeeds     */

                case PLUS:                      /* One or more ...     */
                        if ((l = pmatch(linestart,l,p)) == 0)
                                return 0;       /* Gotta have a match  */
                case STAR:                      /* Zero or more ...    */
                        are = l;                /* Remember line start */
                        while (*l && (e = pmatch(linestart,l,p)))
                                l = e;          /* Get longest match   */
                        while (*p++ != ENDPAT); /* Skip over pattern   */
                        while (l >= are) {      /* Try to match rest   */
                                if (e = pmatch(linestart,l,p))
                                        return e;
                                --l;            /* Nope, try earlier   */
                        }
                        return 0;               /* Nothing else worked */

                default:
                        fprintf( stderr, "bad op code %d\n", op );
                        error( "can't happen -- match", RE_ERROR );
                }
        }
        return l;
}
