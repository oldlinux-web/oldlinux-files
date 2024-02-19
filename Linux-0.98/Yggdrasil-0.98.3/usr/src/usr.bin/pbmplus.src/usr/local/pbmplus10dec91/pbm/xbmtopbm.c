/* xbmtopbm.c - read an X bitmap file and produce a portable bitmap
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

static void ReadBitmapFile ARGS(( FILE* stream, int* widthP, int* heightP, char** dataP ));

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    bit* bitrow;
    register bit* bP;
    int rows, cols, row, col, charcount;
    char* data;
    char mask;

    pbm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[bitmapfile]" );
    
    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    ReadBitmapFile( ifp, &cols, &rows, &data );

    pm_close( ifp );

    pbm_writepbminit( stdout, cols, rows, 0 );
    bitrow = pbm_allocrow( cols );

    for ( row = 0; row < rows; ++row )
	{
	charcount = 0;
	mask = 1;
	for ( col = 0, bP = bitrow; col < cols; ++col, ++bP )
	    {
	    if ( charcount >= 8 )
		{
		++data;
		charcount = 0;
		mask = 1;
		}
	    *bP = ( *data & mask ) ? PBM_BLACK : PBM_WHITE;
	    ++charcount;
	    mask = mask << 1;
	    }
	++data;
	pbm_writepbmrow( stdout, bitrow, cols, 0 );
	}

    pm_close( stdout );
    exit( 0 );
    }

#define MAX_LINE 500

static void
ReadBitmapFile( stream, widthP, heightP, dataP )
    FILE* stream;
    int* widthP;
    int* heightP;
    char** dataP;
    {
    char line[MAX_LINE], name_and_type[MAX_LINE];
    char* ptr;
    char* t;
    int version10, raster_length, v;
    register int bytes, bytes_per_line, padding;
    register int c1, c2, value1, value2;
    int hex_table[256];

    *widthP = *heightP = -1;

    for ( ; ; )
	{
	if ( fgets( line, MAX_LINE, stream ) == NULL )
	    pm_error( "EOF / read error" );
	if ( strlen( line ) == MAX_LINE - 1 )
	    pm_error( "line too long" );

	if ( sscanf( line, "#define %s %d", name_and_type, &v ) == 2 )
	    {
	    if ( ( t = rindex( name_and_type, '_' ) ) == NULL )
		t = name_and_type;
	    else
		++t;
	    if ( ! strcmp( "width", t ) )
		*widthP = v;
	    else if ( ! strcmp( "height", t ) )
		*heightP = v;
	    continue;
	    }
	
	if ( sscanf( line, "static short %s = {", name_and_type ) == 1 ) /* } */
	    {
	    version10 = 1;
	    break;
	    }
	if ( sscanf( line, "static char %s = {", name_and_type ) == 1 ) /* } */
	    {
	    version10 = 0;
	    break;
	    }
	}
 
    if ( *widthP == -1 )
	pm_error( "invalid width" );
    if ( *heightP == -1 )
	pm_error( "invalid height" );

    padding = 0;
    if ( ((*widthP % 16) >= 1) && ((*widthP % 16) <= 8) && version10 )
	padding = 1;

    bytes_per_line = (*widthP+7)/8 + padding;
    
    raster_length =  bytes_per_line * *heightP;
    *dataP = (char*) malloc( raster_length );
    if ( *dataP == (char*) 0 )
	pm_error( "out of memory" );

    /* Initialize hex_table. */
    for ( c1 = 0; c1 < 256; ++c1 )
	hex_table[c1] = 256;
    hex_table['0'] = 0;
    hex_table['1'] = 1;
    hex_table['2'] = 2;
    hex_table['3'] = 3;
    hex_table['4'] = 4;
    hex_table['5'] = 5;
    hex_table['6'] = 6;
    hex_table['7'] = 7;
    hex_table['8'] = 8;
    hex_table['9'] = 9;
    hex_table['A'] = 10;
    hex_table['B'] = 11;
    hex_table['C'] = 12;
    hex_table['D'] = 13;
    hex_table['E'] = 14;
    hex_table['F'] = 15;
    hex_table['a'] = 10;
    hex_table['b'] = 11;
    hex_table['c'] = 12;
    hex_table['d'] = 13;
    hex_table['e'] = 14;
    hex_table['f'] = 15;

    if ( version10 )
	for ( bytes = 0, ptr = *dataP; bytes < raster_length; bytes += 2 )
	    {
	    while ( ( c1 = getc( stream ) ) != 'x' )
	        if ( c1 == EOF )
		    pm_error( "EOF / read error" );
	    c1 = getc( stream );
	    c2 = getc( stream );
	    if ( c1 == EOF || c2 == EOF )
		pm_error( "EOF / read error" );
	    value1 = ( hex_table[c1] << 4 ) + hex_table[c2];
	    if ( value1 >= 256 )
		pm_error( "syntax error" );
	    c1 = getc( stream );
	    c2 = getc( stream );
	    if ( c1 == EOF || c2 == EOF )
		pm_error( "EOF / read error" );
	    value2 = ( hex_table[c1] << 4 ) + hex_table[c2];
	    if ( value2 >= 256 )
		pm_error( "syntax error" );
	    *ptr++ = value2;
	    if ( ( ! padding ) || ( ( bytes + 2 ) % bytes_per_line ) )
	        *ptr++ = value1;
	    }
    else
	for ( bytes = 0, ptr = *dataP; bytes < raster_length; ++bytes )
	    {
	    /*
	    ** Skip until digit is found.
	    */
	    for ( ; ; )
		{
		c1 = getc( stream );
		if ( c1 == EOF )
		    pm_error( "EOF / read error" );
		value1 = hex_table[c1];
		if ( value1 != 256 )
		    break;
		}
	    /*
	    ** Loop on digits.
	    */
	    for ( ; ; )
		{
		c2 = getc( stream );
		if ( c2 == EOF )
		    pm_error( "EOF / read error" );
		value2 = hex_table[c2];
		if ( value2 != 256 )
		    {
		    value1 = (value1 << 4) | value2;
		    if ( value1 >= 256 )
			pm_error( "syntax error" );
		    }
		else if ( c2 == 'x' || c2 == 'X' )
		    if ( value1 == 0 )
			continue;
		    else pm_error( "syntax error" );
	        else break;
	        }
	    *ptr++ = value1;
	    }
    }
