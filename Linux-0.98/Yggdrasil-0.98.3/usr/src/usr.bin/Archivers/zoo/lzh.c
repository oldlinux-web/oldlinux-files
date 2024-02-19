/* $Id: lzh.c,v 1.15 91/07/06 19:18:51 dhesi Exp $ */
/*
lzh compression and uncompression interface module
*/

#include "options.h"
#include "zoo.h"
#include "ar.h"
#include "errors.i"

FILE *arcfile;

extern void prterror();

extern char *out_buf_adr;			/* address of buffer */

int lzh_encode(infile, outfile)
FILE *infile;
FILE *outfile;
{
	extern void encode();
	encode(infile, outfile);
	return 0;
}

/*
lzh_decode decodes its input and sends it to output.
Should return error status or byte count, but currently
returns 0.
*/

#undef COUNT_BYTES		/* define for debugging */

int lzh_decode(infile, outfile)
FILE *infile;
FILE *outfile;
{
	int n;
	extern int decoded;
#ifdef COUNT_BYTES
	int bytes_decoded = 0;		/*debug*/ /* count bytes after decoding */
#endif

	arcfile = infile;					/* stream to be decoded */

	decode_start();
	while (!decoded) {
		n = decode((uint) DICSIZ, out_buf_adr); /* n = count of chars decoded */
#ifdef COUNT_BYTES
		bytes_decoded += n;	/*debug*/
#endif
#ifdef CHECK_BREAK
		check_break();
#endif
		fwrite_crc(out_buf_adr, n, outfile);
#ifdef SHOW_DOTS
		(void) putc('.', stderr);
		(void) fflush(stderr);
#endif
	}
#ifdef COUNT_BYTES
	(void) fprintf(stderr, "bytes decoded = %d\n", bytes_decoded);
#endif
	return 0;
}
