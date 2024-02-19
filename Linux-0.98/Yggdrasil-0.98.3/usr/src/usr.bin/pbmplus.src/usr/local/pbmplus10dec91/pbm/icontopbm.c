/* icontopbm.c - read a Sun icon file and produce a portable bitmap
**
** Copyright (C) 1988 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pbm.h"

static void ReadIconFile ARGS(( FILE* file, int* width, int* height, short** data ));

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    bit* bitrow;
    register bit* bP;
    int rows, cols, row, col, shortcount, mask;
    short* data;

    pbm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[iconfile]" );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    ReadIconFile( ifp, &cols, &rows, &data );

    pm_close( ifp );

    pbm_writepbminit( stdout, cols, rows, 0 );
    bitrow = pbm_allocrow( cols );

    for ( row = 0; row < rows; row++ )
	{
	shortcount = 0;
	mask = 0x8000;
	for ( col = 0, bP = bitrow; col < cols; col++, bP++ )
	    {
	    if ( shortcount >= 16 )
		{
		data++;
		shortcount = 0;
		mask = 0x8000;
		}
	    *bP = ( *data & mask ) ? PBM_BLACK : PBM_WHITE;
	    shortcount++;
	    mask = mask >> 1;
	    }
	data++;
	pbm_writepbmrow( stdout, bitrow, cols, 0 );
	}

    pm_close( stdout );
    exit( 0 );
    }

/* size in bytes of a bitmap */
#define BitmapSize(width, height) (((((width) + 15) >> 3) &~ 1) * (height))

static void
ReadIconFile( file, width, height, data )
    FILE* file;
    int* width;
    int* height;
    short** data;
    {
    char variable[81], ch;
    int status, value, i, data_length, gotsome;

    gotsome = 0;
    *width = *height = -1;
    for ( ; ; )
	{
	while ( ( ch = getc( file ) ) == ',' || ch == '\n' || ch == '\t' ||
		ch == ' ' )
	    ;
	for ( i = 0;
	      ch != '=' && ch != ',' && ch != '\n' && ch != '\t' && ch != ' ';
	      i++ )
	    {
	    variable[i] = ch;
	    ch = getc( file );
	    }
	variable[i] = '\0';

	if ( strcmp( variable, "*/" ) == 0 && gotsome )
	    break;

	if ( fscanf( file, "%d", &value ) != 1 )
	    continue;

	if ( strcmp( variable, "Width" ) == 0 )
	    {
	    *width = value;
	    gotsome = 1;
	    }
	else if ( strcmp( variable, "Height" ) == 0 )
	    {
    	    *height = value;
	    gotsome = 1;
	    }
	else if ( strcmp( variable, "Depth" ) == 0 )
    	    {
	    if ( value != 1 )
		pm_error( "invalid depth" );
	    gotsome = 1;
	    }
	else if ( strcmp( variable, "Format_version" ) == 0 )
    	    {
	    if ( value != 1 )
		pm_error( "invalid Format_version" );
	    gotsome = 1;
	    }
	else if ( strcmp( variable, "Valid_bits_per_item" ) == 0 )
    	    {
	    if ( value != 16 )
		pm_error( "invalid Valid_bits_per_item" );
	    gotsome = 1;
	    }
	}

    if ( *width <= 0 )
	pm_error( "invalid width: %d", *width );
    if ( *height <= 0 )
	pm_error( "invalid height: %d", *height );

    data_length = BitmapSize( *width, *height );
    *data = (short*) malloc( data_length );
    if ( *data == NULL )
    	pm_error( "out of memory" );
    data_length /= sizeof( short );
    
    for ( i = 0 ; i < data_length; i++ )
	{
	if ( i == 0 )
	    status = fscanf( file, " 0x%4hx", *data );
	else
	    status = fscanf( file, ", 0x%4hx", *data + i );
	if ( status != 1 )
	    pm_error( "error 4 scanning bits item" );
    	}
    }
