/* pnmcat.c - concatenate portable anymaps
**
** Copyright (C) 1989, 1991 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pnm.h"

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE** ifp;
    register xel* newxelrow;
    xel** xelrow;
    xel* background;
    xelval* maxval;
    xelval newmaxval;
    int argn, backdefault, backblack, lrflag, tbflag, justify, nfiles;
    int* rows;
    int* cols;
    int* format;
    int newformat;
    int i, row;
    register int col;
    int newrows, newcols, new;
    char* usage = "[-white|-black] -leftright|-lr [-jtop|-jbottom] pnmfile ...\n               [-white|-black] -topbottom|-tb [-jleft|-jright] pnmfile ...";

    pnm_init( &argc, argv );

    argn = 1;
    backdefault = 1;
    lrflag = tbflag = 0;
    justify = 0;

    /* Check for flags. */
    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-white", 2 ) )
	    {
	    backdefault = 0;
	    backblack = 0;
	    }
	else if ( pm_keymatch( argv[argn], "-black", 2 ) )
	    {
	    backdefault = 0;
	    backblack = 1;
	    }
	else if ( pm_keymatch( argv[argn], "-lr", 2 ) ||
	          pm_keymatch( argv[argn], "-leftright", 2 ) )
	    lrflag = 1;
	else if ( pm_keymatch( argv[argn], "-tb", 2 ) ||
	          pm_keymatch( argv[argn], "-topbottom", 2 ) )
	    tbflag = 1;
	else if ( pm_keymatch( argv[argn], "-jtop", 3 ) )
	    justify = -1;
	else if ( pm_keymatch( argv[argn], "-jbottom", 3 ) )
	    justify = 1;
	else if ( pm_keymatch( argv[argn], "-jleft", 3 ) )
	    justify = -1;
	else if ( pm_keymatch( argv[argn], "-jright", 3 ) )
	    justify = 1;
	else if ( pm_keymatch( argv[argn], "-jcenter", 3 ) )
	    justify = 0;
	else
	    pm_usage( usage );
	++argn;
	}

    if ( lrflag && tbflag )
	pm_error( "only one of -lr and -tb may be specified" );
    if ( ! ( lrflag || tbflag ) )
	pm_error( "one of -lr or -tb must be specified" );

    if ( argn < argc )
	nfiles = argc - argn;
    else
	nfiles = 1;
    ifp = (FILE**) malloc( nfiles * sizeof(FILE*) );
    xelrow = (xel**) malloc( nfiles * sizeof(xel*) );
    background = (xel*) malloc( nfiles * sizeof(xel) );
    maxval = (xelval*) malloc( nfiles * sizeof(xelval) );
    rows = (int*) malloc( nfiles * sizeof(int) );
    cols = (int*) malloc( nfiles * sizeof(int) );
    format = (int*) malloc( nfiles * sizeof(int) );
    if ( ifp == (FILE**) 0 || xelrow == (xel**) 0 || background == (xel*) 0 ||
	 maxval == (xelval*) 0 || rows == (int*) 0 || cols == (int*) 0 ||
	 format == (int*) 0 )
	pm_error( "out of memory" );
    if ( argn < argc )
	{
	for ( i = 0; i < nfiles; ++i )
	    ifp[i] = pm_openr( argv[argn+i] );
	}
    else
	ifp[0] = stdin;

    newcols = 0;
    newrows = 0;
    for ( i = 0; i < nfiles; ++i )
	{
	pnm_readpnminit( ifp[i], &cols[i], &rows[i], &maxval[i], &format[i] );
	if ( i == 0 )
	    {
	    newmaxval = maxval[i];
	    newformat = format[i];
	    }
	else
	    {
	    if ( PNM_FORMAT_TYPE(format[i]) > PNM_FORMAT_TYPE(newformat) )
		newformat = format[i];
	    if ( maxval[i] > newmaxval )
		newmaxval = maxval[i];
	    }
	xelrow[i] = pnm_allocrow( cols[i] );
	if ( lrflag )
	    {
	    newcols += cols[i];
	    if ( rows[i] > newrows )
		newrows = rows[i];
	    }
	else
	    {
	    newrows += rows[i];
	    if ( cols[i] > newcols )
		newcols = cols[i];
	    }
	}
    for ( i = 0; i < nfiles; ++i )
	{
	/* Read first row just to get a good guess at the background. */
	pnm_readpnmrow( ifp[i], xelrow[i], cols[i], maxval[i], format[i] );
	pnm_promoteformatrow(
	    xelrow[i], cols[i], maxval[i], format[i], newmaxval, newformat );
	if (  backdefault )
	    background[i] =
		pnm_backgroundxelrow(
		    xelrow[i], cols[i], newmaxval, newformat );
	else
	    if ( backblack )
		background[i] = pnm_blackxel( newmaxval, newformat );
	    else
		background[i] = pnm_whitexel( newmaxval, newformat );
	}

    newxelrow = pnm_allocrow( newcols );

    pnm_writepnminit( stdout, newcols, newrows, newmaxval, newformat, 0 );

    if ( lrflag )
	{
	for ( row = 0; row < newrows; ++row )
	    {
	    new = 0;
	    for ( i = 0; i < nfiles; ++i )
		{
		int padtop;

		if ( justify == -1 )
		    padtop = 0;
		else if ( justify == 1 )
		    padtop = newrows - rows[i];
		else
		    padtop = ( newrows - rows[i] ) / 2;
		if ( row < padtop || row >= padtop + rows[i] )
		    {
		    for ( col = 0; col < cols[i]; ++col )
			newxelrow[new+col] = background[i];
		    }
		else
		    {
		    if ( row != padtop )	/* first row already read */
			{
			pnm_readpnmrow(
			    ifp[i], xelrow[i], cols[i], maxval[i], format[i] );
			pnm_promoteformatrow(
			    xelrow[i], cols[i], maxval[i], format[i],
			    newmaxval, newformat );
			}
		    for ( col = 0; col < cols[i]; ++col )
			newxelrow[new+col] = xelrow[i][col];
		    }
		new += cols[i];
		}
	    pnm_writepnmrow( stdout, newxelrow, newcols, newmaxval, newformat, 0 );
	    }
	}
    else
	{
	int padleft;

	new = 0;
	i = 0;
	if ( justify == -1 )
	    padleft = 0;
	else if ( justify == 1 )
	    padleft = newcols - cols[i];
	else
	    padleft = ( newcols - cols[i] ) / 2;

	for ( row = 0; row < newrows; ++row )
	    {
	    if ( row - new >= rows[i] )
		{
		new += rows[i];
		++i;
		if ( i >= nfiles )
		    pm_error( "shouldn't happen" );
		if ( justify == -1 )
		    padleft = 0;
		else if ( justify == 1 )
		    padleft = newcols - cols[i];
		else
		    padleft = ( newcols - cols[i] ) / 2;
		}
	    if ( row - new > 0 )
		{
		pnm_readpnmrow(
		    ifp[i], xelrow[i], cols[i], maxval[i], format[i] );
		pnm_promoteformatrow(
		    xelrow[i], cols[i], maxval[i], format[i],
		    newmaxval, newformat );
		}
	    for ( col = 0; col < padleft; ++col )
		newxelrow[col] = background[i];
	    for ( col = 0; col < cols[i]; ++col )
		newxelrow[padleft+col] = xelrow[i][col];
	    for ( col = padleft + cols[i]; col < newcols; ++col )
		newxelrow[col] = background[i];
	    pnm_writepnmrow( stdout, newxelrow, newcols, newmaxval, newformat, 0 );
	    }
	}

    for ( i = 0; i < nfiles; ++i )
	pm_close( ifp[i] );
    pm_close( stdout );

    exit( 0 );
    }
