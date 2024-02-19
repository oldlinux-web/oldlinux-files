/* atktopbm.c - convert Andrew Toolkit raster object to portable bitmap
**
** Copyright (C) 1991 by Bill Janssen
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include <stdio.h>
#include <sys/types.h>
#include "pbm.h"

void ReadATKRaster();

void
main( argc, argv )
    int argc;
    char *argv[];
    {
    FILE *ifp;
    register bit *bitrow, *bP;
    int rows, cols, row, col, charcount;
    unsigned char *data, mask;

    pbm_init ( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[raster obj]" );
    
    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    ReadATKRaster( ifp, &cols, &rows, &data );

    pm_close( ifp );

    pbm_writepbminit( stdout, cols, rows, 0 );
    bitrow = pbm_allocrow( cols );

    for ( row = 0; row < rows; ++row )
	{
	charcount = 0;
	mask = 0x80;
	for ( col = 0, bP = bitrow; col < cols; ++col, ++bP )
	    {
	    if ( charcount >= 8 )
		{
		++data;
		charcount = 0;
		mask = 0x80;
		}
	    *bP = ( *data & mask ) ? PBM_BLACK : PBM_WHITE;
	    ++charcount;
	    mask >>= 1;
	    }
	++data;
	pbm_writepbmrow( stdout, bitrow, cols, 0 );
	}

    pm_close( stdout );
    exit( 0 );
    }

/* readatkraster
**
** Routine for reading rasters in .raster form.  (BE2 rasters version 2.)
*/

/* codes for data stream */
#define WHITEZERO	'f'
#define WHITETWENTY	'z'
#define BLACKZERO	'F'
#define BLACKTWENTY	'Z'
#define OTHERZERO	0x1F

#define	WHITEBYTE	0x00
#define	BLACKBYTE	0xFF

/* error codes (copied from $ANDREW/atk/basics/common/dataobj.ch) */
/* return values from Read */
#define	dataobject_NOREADERROR	0
#define	dataobject_PREMATUREEOF	1
#define	dataobject_NOTBE2DATASTREAM 2 /* backward compatibility */
#define	dataobject_NOTATKDATASTREAM 2 /* preferred version */
#define dataobject_MISSINGENDDATAMARKER 3
#define	dataobject_OBJECTCREATIONFAILED	4
#define dataobject_BADFORMAT 5

/* ReadRow(file, row, length) 
** Reads from 'file' the encoding of bytes to fill in 'row'.  Row will be
** truncated or padded (with WHITE) to exactly 'length' bytes.
**
** Returns the code that terminated the row.  This may be
** 		'|'  	correct end of line
** 		'\0' 	if the length was satisfied (before a terminator)
** 		EOF 	if the file ended
** 		'\'  '{' 	other recognized ends. 
** The '|' is the expected end and pads the row with WHITE.
** The '\' and '{' are error conditions and may indicate the
** beginning of some other portion of the data stream.
** If the terminator is '\' or '{', it is left at the front of the input.
** '|' is gobbled up.
*/

/* macros to generate case entries for switch statement */
#define case1(v) case v
#define case4(v) case v: case (v)+1: case (v)+2: case(v)+3
#define case6(v) case4(v): case ((v)+4): case ((v)+5)
#define case8(v) case4(v): case4((v)+4)

static long
ReadRow(file, row, length)
	register FILE *file;		/* where to get them from */
	register unsigned char *row;	/* where to put bytes */
	register long length;	/* how many bytes in row must be filled */
{
	/* Each input character is processed by the central loop.  There are 
	** some input codes which require two or three characters for
	** completion; these are handled by advancing the state machine.
	** Errors are not processed; instead the state machine is reset
	** to the Ready state whenever a character unacceptable to the
	** current state is read.
	*/
	enum stateCode {
	    Ready, 		/* any input code is allowed */
	    HexDigitPending,	/* have seen the first of a hex digit pair */
	    RepeatPending, 	/* repeat code has been seen:
				   must be followed by two hex digits */
	    RepeatAndDigit};	/* have seen repeat code and its first
				   following digit */
	enum stateCode InputState;	/* current state */
	register c;		/* the current input character */
	register long repeatcount = 0;	/* current repeat value */
	register long hexval;	/* current hex value */
	long pendinghex = 0;	/* the first of a pair of hex characters */
	
	/* We cannot exit when length becomes zero because we need to check 
	** to see if a row ending character follows.  Thus length is checked
	** only when we get a data generating byte.  If length then is
	** zero, we ungetc the byte.
	*/

	InputState = Ready;
	while ((c=getc(file)) != EOF) switch (c) {

	case8(0x0):
	case8(0x8):
	case8(0x10):
	case8(0x18):
	case1(' '):
		/* control characters and space are legal and ignored */
		break;
	case1(0x40):	/* '@' */
	case1(0x5B):	/* '[' */
	case4(0x5D):	/*  ']'  '^'  '_'  '`' */
	case4(0x7D):	/* '}'  '~'  DEL  0x80 */
	default:		/* all above 0x80 */
		/* error code:  Ignored at present.  Reset InputState. */
		InputState = Ready;
		break;

	case1(0x7B):	/* '{' */
	case1(0x5C):	/* '\\' */
		/* illegal end of line:  exit anyway */
		ungetc(c, file);	/* retain terminator in stream */
		/* DROP THROUGH */
	case1(0x7C):	/* '|' */
		/* legal end of row: may have to pad  */
		while (length-- > 0)
			*row++ = WHITEBYTE;
		return c;
	
	case1(0x21):
	case6(0x22):
	case8(0x28):
		/* punctuation characters: repeat byte given by two
		** succeeding hex chars
		*/
		if (length <= 0) {
			ungetc(c, file);
			return('\0');
		}
		repeatcount = c - OTHERZERO;
		InputState = RepeatPending;
		break;

	case8(0x30):
	case8(0x38):
		/* digit (or following punctuation)  -  hex digit */
		hexval = c - 0x30;
		goto hexdigit;
	case6(0x41):
		/* A ... F    -  hex digit */
		hexval = c - (0x41 - 0xA);
		goto hexdigit;
	case6(0x61):
		/* a ... f  - hex digit */
		hexval = c - (0x61 - 0xA);
		goto hexdigit;

	case8(0x67):
	case8(0x6F):
	case4(0x77):
		/* g ... z   -   multiple WHITE bytes */
		if (length <= 0) {
			ungetc(c, file);
			return('\0');
		}
		repeatcount = c - WHITEZERO;
		hexval = WHITEBYTE;
		goto store;
	case8(0x47):
	case8(0x4F):
	case4(0x57):
		/* G ... Z   -   multiple BLACK bytes */
		if (length <= 0) {
			ungetc(c, file);
			return('\0');
		}
		repeatcount = c - BLACKZERO;
		hexval = BLACKBYTE;
		goto store;

hexdigit:
		/* process a hex digit.  Use InputState to determine
			what to do with it. */
		if (length <= 0) {
			ungetc(c, file);
			return('\0');
		}
		switch(InputState) {
		case Ready:
			InputState = HexDigitPending;
			pendinghex = hexval << 4;
			break;
		case HexDigitPending:
			hexval |= pendinghex;
			repeatcount = 1;
			goto store;
		case RepeatPending:
			InputState = RepeatAndDigit;
			pendinghex = hexval << 4;
			break;
		case RepeatAndDigit:
			hexval |= pendinghex;
			goto store;
		}
		break;

store:
		/* generate byte(s) into the output row 
			Use repeatcount, depending on state.  */
		if (length < repeatcount) 
			/* reduce repeat count if it would exceed
				available space */
			repeatcount = length;
		length -= repeatcount;	/* do this before repeatcount-- */
		while (repeatcount-- > 0)
				*row++ = hexval;
		InputState = Ready;
		break;

	} /* end of while( - )switch( - ) */
	return EOF;
}
#undef case1
#undef case4
#undef case6
#undef case8

void
ReadATKRaster(file, rwidth, rheight, destaddr)
     FILE *file;
     unsigned char **destaddr;
     int *rwidth, *rheight;
{
	register unsigned char *byteaddr;	/* where to store next row */
	register long row, rowlen;	/* count rows;  byte length of row */
	long version, options, xscale, yscale;
	long xoffset, yoffset, subwidth, subheight;
	char keyword[6];
	long discardid, objectid;     /* id read for the incoming pixel image */
	long tc;			/* temp */
	long width, height;		/* dimensions of image */
	long result;

	if (fscanf(file, "\\begindata{raster,%ld", &discardid) != 1
				|| getc(file) != '}' || getc(file) != '\n')
	  pm_error ("input file not Andrew raster object");

	fscanf(file, " %d ", &version);
	if (version < 2) 
	  pm_error ("version too old to parse");

	/* ignore all these features: */
	fscanf(file, " %u %ld %ld %ld %ld %ld %ld",  
		&options, &xscale, &yscale, &xoffset, 
		&yoffset, &subwidth, &subheight);

	/* scan to end of line in case this is actually something beyond V2 */
	while (((tc=getc(file)) != '\n') && (tc != '\\') && (tc != EOF)) {}

	/* read the keyword */
	fscanf(file, " %5s", keyword);
	if (strcmp(keyword, "bits") != 0)
	  pm_error ("keyword is not bits!");

	fscanf(file, " %d %d %d ", &objectid, &width, &height);

	if (width < 1 || height < 1 || width > 1000000 || height > 1000000) 
	  pm_error ("bad width or height");

	*rwidth = width;
	*rheight = height;
	rowlen = (width + 7) / 8;
	*destaddr = (unsigned char *) malloc (sizeof(unsigned char) * height *
rowlen);
	for (row = 0;   row < height;   row++)
	  {
	    long c;

	    c = ReadRow(file, *destaddr + (row * rowlen), rowlen);
	    if (c != '|')
	      {
		if (c == EOF)
		  pm_error ("premature EOF");
		else
		  pm_error ("bad format");
		break;
	      }
	  }
	while (! feof(file) && getc(file) != '\\') {};	/* scan for \enddata */
	if (fscanf(file, "enddata{raster,%d", &discardid) != 1
	    || getc(file) != '}' || getc(file) != '\n')
	  pm_error ("missing end-of-object marker");
}
