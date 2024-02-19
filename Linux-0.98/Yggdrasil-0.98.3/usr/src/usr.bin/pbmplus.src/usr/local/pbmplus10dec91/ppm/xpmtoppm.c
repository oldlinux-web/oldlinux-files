/* xpmtoppm.c - read an X11 pixmap file and produce a portable pixmap
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

#include "ppm.h"

static void ReadXPMFile ARGS((
    FILE* stream, int* widthP, int* heightP, int* ncolorsP,
    int* chars_per_pixelP, pixel** colorsP, int** dataP ));
static void getline ARGS(( char* line, int size, FILE* stream ));

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    pixel* pixrow;
    pixel* colors;
    register pixel* pP;
    int rows, cols, ncolors, chars_per_pixel, row;
    register int col;
    int* data;
    register int* ptr;

    ppm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[xpmfile]" );
    
    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    ReadXPMFile(
	ifp, &cols, &rows, &ncolors, &chars_per_pixel, &colors, &data );

    pm_close( ifp );

    ppm_writeppminit( stdout, cols, rows, (pixval) PPM_MAXMAXVAL, 0 );
    pixrow = ppm_allocrow( cols );

    for ( row = 0, ptr = data; row < rows; ++row )
	{
	for ( col = 0, pP = pixrow; col < cols; ++col, ++pP, ++ptr )
	    *pP = colors[*ptr];
	ppm_writeppmrow( stdout, pixrow, cols, (pixval) PPM_MAXMAXVAL, 0 );
	}

    pm_close( stdout );

    exit( 0 );
    }

#define MAX_LINE 500

static void
ReadXPMFile( stream, widthP, heightP, ncolorsP, chars_per_pixelP, colorsP, dataP )
    FILE* stream;
    int* widthP;
    int* heightP;
    int* ncolorsP;
    int* chars_per_pixelP;
    pixel** colorsP;
    int** dataP;
    {
    char line[MAX_LINE], str1[MAX_LINE], str2[MAX_LINE];
    char* t1;
    char* t2;
    int format, v, datasize;
    int* ptr;
    int* ptab;
    register int i, j;

    *widthP = *heightP = *ncolorsP = *chars_per_pixelP = format = -1;

    /* Read the initial defines. */
    for ( ; ; )
	{
	getline( line, sizeof(line), stream );

	if ( sscanf( line, "#define %s %d", str1, &v ) == 2 )
	    {
	    if ( ( t1 = rindex( str1, '_' ) ) == NULL )
		t1 = str1;
	    else
		++t1;
	    if ( ! strcmp( t1, "format" ) )
		format = v;
	    else if ( ! strcmp( t1, "width" ) )
		*widthP = v;
	    else if ( ! strcmp( t1, "height" ) )
		*heightP = v;
	    else if ( ! strcmp( t1, "ncolors" ) )
		*ncolorsP = v;
	    else if ( ! strcmp( t1, "pixel" ) )
		*chars_per_pixelP = v;
	    }
	else if ( ! strncmp( line, "static char", 11 ) )
	    {
	    if ( ( t1 = rindex( line, '_' ) ) == NULL )
		t1 = line;
	    else
		++t1;
	    break;
	    }
	}
    if ( format == -1 )
	pm_error( "missing or invalid format" );
    if ( format != 1 )
	pm_error( "can't handle XPM version %d", format );
    if ( *widthP == -1 )
	pm_error( "missing or invalid width" );
    if ( *heightP == -1 )
	pm_error( "missing or invalid height" );
    if ( *ncolorsP == -1 )
	pm_error( "missing or invalid ncolors" );
    if ( *chars_per_pixelP == -1 )
	pm_error( "missing or invalid chars_per_pixel" );
    if ( *chars_per_pixelP > 2 )
	pm_message(
	    "warning, chars_per_pixel > 2 uses a lot of memory" );

    /* If there's a monochrome color table, skip it. */
    if ( ! strncmp( t1, "mono", 4 ) )
	{
	for ( ; ; )
	    {
	    getline( line, sizeof(line), stream );
	    if ( ! strncmp( line, "static char", 11 ) )
		break;
	    }
	}

    /* Allocate space for color table. */
    if ( *chars_per_pixelP <= 2 )
	{
	/* Up to two chars per pixel, we can use an indexed table. */
	v = 1;
	for ( i = 0; i < *chars_per_pixelP; ++i )
	    v *= 256;
	*colorsP = ppm_allocrow( v );
	}
    else
	{
	/* Over two chars per pixel, we fall back on linear search. */
	*colorsP = ppm_allocrow( *ncolorsP);
	ptab = (int*) malloc( *ncolorsP * sizeof(int) );
	}

    /* Read color table. */
    for ( i = 0; i < *ncolorsP; ++i )
	{
	getline( line, sizeof(line), stream );

	if ( ( t1 = index( line, '"' ) ) == NULL )
	    pm_error( "error scanning color table" );
	if ( ( t2 = index( t1 + 1, '"' ) ) == NULL )
	    pm_error( "error scanning color table" );
	if ( t2 - t1 - 1 != *chars_per_pixelP )
	    pm_error(
		"wrong number of chars per pixel in color table" );
	strncpy( str1, t1 + 1, t2 - t1 - 1 );
	str1[t2 - t1 - 1] = '\0';

	if ( ( t1 = index( t2 + 1, '"' ) ) == NULL )
	    pm_error( "error scanning color table" );
	if ( ( t2 = index( t1 + 1, '"' ) ) == NULL )
	    pm_error( "error scanning color table" );
	strncpy( str2, t1 + 1, t2 - t1 - 1 );
	str2[t2 - t1 - 1] = '\0';

	v = 0;
	for ( j = 0; j < *chars_per_pixelP; ++j )
	    v = ( v << 8 ) + str1[j];
	if ( *chars_per_pixelP <= 2 )
	    /* Index into table. */
	    (*colorsP)[v] = ppm_parsecolor( str2, (pixval) PPM_MAXMAXVAL );
	else
	    {
	    /* Set up linear search table. */
	    (*colorsP)[i] = ppm_parsecolor( str2, (pixval) PPM_MAXMAXVAL );
	    ptab[i] = v;
	    }
	}
 
    /* Read pixels. */
    for ( ; ; )
	{
	getline( line, sizeof(line), stream );
	if ( ! strncmp( line, "static char", 11 ) )
	    break;
	}
    datasize = *widthP * *heightP;
    *dataP = (int*) malloc( datasize * sizeof(int) );
    if ( *dataP == 0 )
	pm_error( "out of memory" );
    i = 0;
    ptr = *dataP;
    for ( ; ; )
	{
	getline( line, sizeof(line), stream );

	/* Find the open quote. */
	if ( ( t1 = index( line, '"' ) ) == NULL )
	    pm_error( "error scanning pixels" );
	++t1;

	/* Handle pixels until a close quote or the end of the image. */
	while ( *t1 != '"' )
	    {
	    v = 0;
	    for ( j = 0; j < *chars_per_pixelP; ++j )
		v = ( v << 8 ) + *t1++;
	    if ( *chars_per_pixelP <= 2 )
		/* Index into table. */
		*ptr++ = v;
	    else
		{
		/* Linear search into table. */
		for ( j = 0; j < *ncolorsP; ++j )
		    if ( ptab[j] == v )
			goto gotit;
		pm_error( "unrecognized pixel in line \"%s\"", line );
	    gotit:
		*ptr++ = j;
		}
	    ++i;
	    if ( i >= datasize )
		return;
	    }
	}
    }

static void
getline( line, size, stream )
    char* line;
    int size;
    FILE* stream;
    {
    if ( fgets( line, MAX_LINE, stream ) == NULL )
	pm_error( "EOF / read error" );
    if ( strlen( line ) == MAX_LINE - 1 )
	pm_error( "line too long" );
    }
