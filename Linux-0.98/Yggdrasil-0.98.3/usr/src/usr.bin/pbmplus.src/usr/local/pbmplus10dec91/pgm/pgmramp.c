/* pgmramp.c - generate a grayscale ramp
**
** Copyright (C) 1989 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pgm.h"

void
main( argc, argv )
int argc;
char *argv[];
    {
    gray *grayrow;
    register gray *gP;
    int rows, cols, rowso2, colso2, row;
    register int col;
    int ramptype;
#define RT_LR 1
#define RT_TB 2
#define RT_RECT 3
#define RT_ELLIP 4
    char *usage = "-lr|-tb|-rectangle|-ellipse <width> <height>";

    pgm_init( &argc, argv );

    if ( argc != 4 )
	pm_usage( usage );

    if ( pm_keymatch( argv[1], "-lr", 2 ) )
	ramptype = RT_LR;
    else if ( pm_keymatch( argv[1], "-tb", 2 ) )
	ramptype = RT_TB;
    else if ( pm_keymatch( argv[1], "-rectangle", 2 ) )
	ramptype = RT_RECT;
    else if ( pm_keymatch( argv[1], "-ellipse", 2 ) )
	ramptype = RT_ELLIP;
    else
	pm_usage( usage );
    
    if ( sscanf( argv[2], "%d", &cols ) != 1 )
	pm_usage( usage );
    if ( sscanf( argv[3], "%d", &rows ) != 1 )
	pm_usage( usage );

    colso2 = cols / 2;
    rowso2 = rows / 2;

    pgm_writepgminit( stdout, cols, rows, PGM_MAXMAXVAL, 0 );
    grayrow = pgm_allocrow( cols );

    for ( row = 0; row < rows; ++row )
	{
	for ( col = 0, gP = grayrow; col < cols; ++col, ++gP )
	    {
	    switch ( ramptype )
		{
		case RT_LR:
		*gP = col * PGM_MAXMAXVAL / ((cols == 1) ? 1 : (cols - 1));
		break;

		case RT_TB:
		*gP = row * PGM_MAXMAXVAL / ((rows == 1) ? 1 : (rows - 1));
		break;

		case RT_RECT:
		{
		float r, c;
		r = abs( rowso2 - row ) / (float) rowso2;
		c = abs( colso2 - col ) / (float) colso2;
		*gP = PGM_MAXMAXVAL - ( r + c ) / 2.0 * PGM_MAXMAXVAL;
		}
		break;

		case RT_ELLIP:
		{
		float r, c, v;
		r = abs( rowso2 - row ) / (float) rowso2;
		c = abs( colso2 - col ) / (float) colso2;
		v = r * r + c * c;
		if ( v < 0.0 ) v = 0.0;
		else if ( v > 1.0 ) v = 1.0;
		*gP = PGM_MAXMAXVAL - v * PGM_MAXMAXVAL;
		}
		break;

		default:
		pm_error( "can't happen" );
		}
	    }
	pgm_writepgmrow( stdout, grayrow, cols, PGM_MAXMAXVAL, 0 );
	}

    pm_close( stdout );
    exit( 0 );
    }
