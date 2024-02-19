#if defined(DEBUG)
#include "freeze.h"
#include "huf.h"
#include "bitio.h"

		  /*---------------------------*/
		  /*      DEBUG      MODULE    */
		  /*---------------------------*/

printcodes(mode)
{
    /*
     * Just print out codes from input file.  For debugging.
     */
    register short k, c, col = 0;

#ifdef COMPAT
	if (!mode) {
		StartHuff(N_CHAR1);
		init(Table1);
	} else
#endif
	{
		if (read_header() == EOF) {
			fprintf(stderr, "Bad header\n");
			return;
		}
		StartHuff(N_CHAR2);
		init(Table2);
	}

	InitIO();

    for (;;) {

	    if((c = DecodeChar()) == ENDOF)
		    break;
	    if (c < 256) {
		fprintf(stderr, "%5d%c", c,
			(col+=8) >= 74 ? (col = 0, '\n') : '\t' );
	    } else {
		c = c - 256 + THRESHOLD;

		k = DecodePosition();

		fprintf(stderr, "%2d-%d%c", c, k,
			(col+=8) >= 74 ? (col = 0, '\n') : '\t' );
	    }
    }
    putc( '\n', stderr );
    exit( 0 );
}

/* for pretty char printing */

char *
pr_char(c)
	register uc_t c;
{
	static char buf[5];
	register i = 4;
	buf[4] = '\0';
	if ( (isascii((int)c) && isprint((int)c) && c != '\\') || c == ' ' ) {
	    buf[--i] = c;
	} else {
	    switch( c ) {
	    case '\n': buf[--i] = 'n'; break;
	    case '\t': buf[--i] = 't'; break;
	    case '\b': buf[--i] = 'b'; break;
	    case '\f': buf[--i] = 'f'; break;
	    case '\r': buf[--i] = 'r'; break;
	    case '\\': buf[--i] = '\\'; break;
	    default:
		buf[--i] = '0' + c % 8;
		buf[--i] = '0' + (c / 8) % 8;
		buf[--i] = '0' + c / 64;
		break;
	    }
	    buf[--i] = '\\';
	}
	return &buf[i];
}
#endif
