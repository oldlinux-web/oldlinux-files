/* pnmconvol.c - general MxN convolution on a portable anymap
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
    FILE* cifp;
    FILE* ifp;
    xel** cxels;
    xel** xelbuf;
    xel* outputrow;
    xel x;
    int argn, crows, ccols, cformat, ccolso2, crowso2;
    int rows, cols, format, newformat, crow, row;
    register int ccol, col;
    xelval cmaxval, maxval;
    xelval g;
    float** gweights;
    float gsum;
    xelval r, b;
    float** rweights;
    float** bweights;
    float rsum, bsum;
    char* usage = "<convolutionfile> [pnmfile]";

    pnm_init( &argc, argv );

    argn = 1;

    if ( argn == argc )
	pm_usage( usage );
    cifp = pm_openr( argv[argn] );
    ++argn;

    if ( argn != argc )
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    pnm_pbmmaxval = PNM_MAXMAXVAL;  /* use larger value for better results */

    /* Read in the convolution matrix. */
    cxels = pnm_readpnm( cifp, &ccols, &crows, &cmaxval, &cformat );
    pm_close( cifp );
    if ( ccols % 2 != 1 || crows % 2 != 1 )
	pm_error(
	 "the convolution matrix must have an odd number of rows and columns" );
    ccolso2 = ccols / 2;
    crowso2 = crows / 2;

    pnm_readpnminit( ifp, &cols, &rows, &maxval, &format );
    if ( cols < ccols || rows < crows )
	pm_error(
	    "the image is smaller than the convolution matrix" );

    newformat = max( PNM_FORMAT_TYPE(cformat), PNM_FORMAT_TYPE(format) );
    if ( PNM_FORMAT_TYPE(cformat) != newformat )
	pnm_promoteformat( cxels, ccols, crows, cmaxval, cformat, cmaxval, newformat );
    if ( PNM_FORMAT_TYPE(format) != newformat )
        {
        switch ( PNM_FORMAT_TYPE(newformat) )
            {
            case PPM_TYPE:
            if ( PNM_FORMAT_TYPE(format) != newformat )
                pm_message( "promoting to PPM" );
            break;
            case PGM_TYPE:
            if ( PNM_FORMAT_TYPE(format) != newformat )
                pm_message( "promoting to PGM" );
            break;
            }
        }

    /* Set up the normalized weights. */
    rweights = (float**) pm_allocarray( ccols, crows, sizeof(float) );
    gweights = (float**) pm_allocarray( ccols, crows, sizeof(float) );
    bweights = (float**) pm_allocarray( ccols, crows, sizeof(float) );
    rsum = gsum = bsum = 0;
    for ( crow = 0; crow < crows; ++crow )
	for ( ccol = 0; ccol < ccols; ++ccol )
	    {
	    switch ( PNM_FORMAT_TYPE(format) )
		{
		case PPM_TYPE:
		rsum += rweights[crow][ccol] =
		    ( PPM_GETR(cxels[crow][ccol]) * 2.0 / cmaxval - 1.0 );
		gsum += gweights[crow][ccol] =
		    ( PPM_GETG(cxels[crow][ccol]) * 2.0 / cmaxval - 1.0 );
		bsum += bweights[crow][ccol] =
		    ( PPM_GETB(cxels[crow][ccol]) * 2.0 / cmaxval - 1.0 );
		break;

		default:
		gsum += gweights[crow][ccol] =
		    ( PNM_GET1(cxels[crow][ccol]) * 2.0 / cmaxval - 1.0 );
		break;
		}
	    }
    switch ( PNM_FORMAT_TYPE(format) )
	{
	case PPM_TYPE:
	if ( rsum < 0.9 || rsum > 1.1 || gsum < 0.9 || gsum > 1.1 ||
	     bsum < 0.9 || bsum > 1.1 )
	    pm_message(
		"WARNING - this convolution matrix is biased" );
	break;

	default:
	if ( gsum < 0.9 || gsum > 1.1 )
	    pm_message(
		 "WARNING - this convolution matrix is biased" );
	break;
	}

    /* Allocate space for one convolution-matrix's worth of rows, plus
    ** a row output buffer. */
    xelbuf = pnm_allocarray( cols, crows );
    outputrow = pnm_allocrow( cols );

    pnm_writepnminit( stdout, cols, rows, maxval, newformat, 0 );

    /* Read in one convolution-matrix's worth of image, less one row. */
    for ( row = 0; row < crows - 1; ++row )
	{
	pnm_readpnmrow( ifp, xelbuf[row], cols, maxval, format );
	if ( PNM_FORMAT_TYPE(format) != newformat )
	    pnm_promoteformatrow(
		xelbuf[row], cols, maxval, format, maxval, newformat );
	/* Write out just the part we're not going to convolve. */
	if ( row < crowso2 )
	    pnm_writepnmrow( stdout, xelbuf[row], cols, maxval, newformat, 0 );
	}

    /* Now the rest of the image - read in the row at the end of
    ** xelbuf, and convolve and write out the row in the middle.
    */
    for ( ; row < rows; ++row )
	{
	pnm_readpnmrow( ifp, xelbuf[row % crows], cols, maxval, format );
	if ( PNM_FORMAT_TYPE(format) != newformat )
	    pnm_promoteformatrow(
		xelbuf[row % crows], cols, maxval, format, maxval, newformat );

	for ( col = 0; col < cols; ++col )
	    if ( col < ccolso2 || col >= cols - ccolso2 )
		outputrow[col] = xelbuf[(row - crowso2) % crows][col];
	    else
		{
                switch ( PNM_FORMAT_TYPE(format) )
                    {
                    case PPM_TYPE:
		    rsum = gsum = bsum = 0.0;
		    for ( crow = 0; crow < crows; ++crow )
			for ( ccol = 0; ccol < ccols; ++ccol )
			    {
			    x = xelbuf[(row+1+crow) % crows][col-ccolso2+ccol];
			    rsum += PPM_GETR( x ) * rweights[crow][ccol];
			    gsum += PPM_GETG( x ) * gweights[crow][ccol];
			    bsum += PPM_GETB( x ) * bweights[crow][ccol];
			    }
		    if ( rsum < 0.0 ) r = 0;
		    else if ( rsum > maxval ) r = maxval;
		    else r = rsum + 0.5;
		    if ( gsum < 0.0 ) g = 0;
		    else if ( gsum > maxval ) g = maxval;
		    else g = gsum + 0.5;
		    if ( bsum < 0.0 ) b = 0;
		    else if ( bsum > maxval ) b = maxval;
		    else b = bsum + 0.5;
		    PPM_ASSIGN( outputrow[col], r, g, b );
                    break;

                    default:
		    gsum = 0.0;
		    for ( crow = 0; crow < crows; ++crow )
			for ( ccol = 0; ccol < ccols; ++ccol )
			    {
			    x = xelbuf[(row+1+crow) % crows][col-ccolso2+ccol];
			    gsum += PNM_GET1( x ) * gweights[crow][ccol];
			    }
		    if ( gsum < 0.0 ) g = 0;
		    else if ( gsum > maxval ) g = maxval;
		    else g = gsum + 0.5;
		    PNM_ASSIGN1( outputrow[col], g );
                    break;
                    }
		}

	pnm_writepnmrow( stdout, outputrow, cols, maxval, newformat, 0 );
	}
    pm_close( ifp );

    /* Now write out the remaining unconvolved rows in xelbuf. */
    for ( ; row < rows + crowso2; ++row )
	pnm_writepnmrow(
            stdout, xelbuf[(row-crowso2) % crows], cols, maxval, newformat, 0 );

    pm_close( stdout );
    exit( 0 );
    }
