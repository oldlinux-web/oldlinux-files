/* pbmtext.c - render text into a bitmap
**
** Copyright (C) 1991 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pbm.h"
#include "pbmfont.h"

static void fix_control_chars ARGS(( char* buf ));
static void fill_rect ARGS(( bit** bits, int row0, int col0, int height, int width, bit color ));
static void copy_rect ARGS(( bit** fbits, int frow0, int fcol0, int height, int width, bit** tbits, int trow0, int tcol0 ));

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    bit** bits;
    int argn, rows, cols, row, col;
    bit** font;
    int frows, fcols;
    FILE* ifp;
    int dump;
    int char_width, char_height, char_awidth, char_aheight, vmargin, hmargin;
    int char_row0[95];
    int char_col0[95];
    char buf[5000];
    char** lp;
    int lines, maxlines, maxchars, line;
    char* cp;
    char* usage = "[-font <fontfile>] [text]";

    pbm_init( &argc, argv );

    /* Set up default parameters. */
    argn = 1;
    font = pbm_defaultfont( &fcols, &frows );
    dump = 0;

    /* Check for flags. */
    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-font", 2 ) )
	    {
	    ++argn;
	    if ( argn == argc )
		pm_usage( usage );
	    ifp = pm_openr( argv[argn] );
	    pbm_freearray( font, frows );
	    font = pbm_readpbm( ifp, &fcols, &frows );
	    pm_close( ifp );
	    }
	else if ( pm_keymatch( argv[argn], "-dump", 2 ) )
	    /* Undocumented dump flag for installing a new built-in font. */
	    dump = 1;
	else
	    pm_usage( usage );
	++argn;
	}

    if ( dump )
	{
	pbm_dumpfont( font, fcols, frows );
	exit( 0 );
	}

    maxlines = 50;
    lp = (char**) malloc( maxlines * sizeof(char*) );
    if ( lp == (char**) 0 )
	pm_error( "out of memory" );

    if ( argn < argc )
	{ /* Get text from the command line. */
	(void) strcpy( buf, argv[argn] );
	++argn;
	while ( argn < argc )
	    {
	    (void) strcat( buf, " " );
	    (void) strcat( buf, argv[argn] );
	    ++argn;
	    }
	fix_control_chars( buf );
	maxchars = strlen( buf );
	lp[0] = buf;
	lines = 1;
	}
    else
	{ /* Read text from stdin. */
	lines = 0;
	maxchars = 0;
	while ( gets( buf ) != NULL )
	    {
	    int l;

	    fix_control_chars( buf );
	    l = strlen( buf );
	    maxchars = max( maxchars, l );
	    if ( lines >= maxlines )
		{
		maxlines *= 2;
		lp = (char**) realloc( (char*) lp, maxlines * sizeof(char*) );
		if ( lp == (char**) 0 )
		    pm_error( "out of memory" );
		}
	    lp[lines] = (char*) malloc( l + 1 );
	    if ( lp[lines] == 0 )
		pm_error( "out of memory" );
	    (void) strcpy( lp[lines], buf );
	    ++lines;
	    }
	}

    pbm_dissectfont( font, frows, fcols, &char_height, &char_width,
		     &char_aheight, &char_awidth, char_row0, char_col0 );
    if ( lines == 1 )
	{
	vmargin = char_height / 2;
	hmargin = char_width;
	}
    else
	{
	vmargin = char_height;
	hmargin = 2 * char_width;
	}
    rows = 2 * vmargin + lines * char_height;
    cols = 2 * hmargin + maxchars * char_width;
    bits = pbm_allocarray( cols, rows );

    /* Fill background with the color of ' '. */
    fill_rect( bits, 0, 0, rows, cols, font[char_row0[0]][char_col0[0]] );
    
    /* Render characters. */
    for ( line = 0; line < lines; ++line )
	{
	row = vmargin + line * char_height;
	col = hmargin;

	for ( cp = lp[line]; *cp != '\0'; ++cp )
	    {
	    copy_rect( font, char_row0[*cp - ' '], char_col0[*cp - ' '],
		       char_height, char_width, bits, row, col );
	    col += char_width;
	    }
	}

    /* All done. */
    pbm_writepbm( stdout, bits, cols, rows, 0 );
    pm_close( stdout );

    exit( 0 );
    }

static void
fix_control_chars( buf )
    char* buf;
    {
    int i, j, n, l;

    for ( i = 0; buf[i] != '\0'; ++i )
	{
	if ( buf[i] == '\t' )
	    { /* Turn tabs into the right number of spaces. */
	    n = ( i + 8 ) / 8 * 8;
	    l = strlen( buf );
	    for ( j = l; j > i; --j )
		buf[j + n - i - 1] = buf[j];
	    for ( ; i < n; ++i )
		buf[i] = ' ';
	    --i;
	    }
	else if ( buf[i] < ' ' || buf[i] > '~' )
	    /* Turn other control chars into a single space. */
	    buf[i] = ' ';
	}
    }

#if __STDC__
static void
fill_rect( bit** bits, int row0, int col0, int height, int width, bit color )
#else /*__STDC__*/
static void
fill_rect( bits, row0, col0, height, width, color )
    bit** bits;
    int row0, col0, height, width;
    bit color;
#endif /*__STDC__*/
    {
    int row, col;

    for ( row = row0; row < row0 + height; ++row )
	for ( col = col0; col < col0 + width; ++col )
	    bits[row][col] = color;
    }

static void
copy_rect( fbits, frow0, fcol0, height, width, tbits, trow0, tcol0 )
    bit** fbits;
    int frow0, fcol0, height, width;
    bit** tbits;
    int trow0, tcol0;
    {
    int row, col;

    for ( row = 0; row < height; ++row )
	for ( col = 0; col < width; ++col )
	    tbits[trow0 + row][tcol0 + col] = fbits[frow0 + row][fcol0 + col];
    }
