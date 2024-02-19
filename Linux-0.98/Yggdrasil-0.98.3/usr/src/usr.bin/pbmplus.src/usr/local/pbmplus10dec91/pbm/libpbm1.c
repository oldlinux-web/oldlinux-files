/* libpbm1.c - pbm utility library part 1
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
#include "version.h"
#include "libpbm.h"
#if __STDC__
#include <stdarg.h>
#else /*__STDC__*/
#include <varargs.h>
#endif /*__STDC__*/


/* Forward routines. */

#if defined(NEED_VFPRINTF1) || defined(NEED_VFPRINTF2)
int vfprintf ARGS(( FILE* stream, char* format, va_list args ));
#endif /*NEED_VFPRINTF*/


/* Variable-sized arrays. */

char*
pm_allocrow( cols, size )
    int cols;
    int size;
    {
    register char* itrow;

    itrow = (char*) malloc( cols * size );
    if ( itrow == (char*) 0 )
	pm_error( "out of memory allocating a row" );
    return itrow;
    }

void
pm_freerow( itrow )
    char* itrow;
    {
    free( itrow );
    }


char**
pm_allocarray( cols, rows, size )
    int cols, rows;
    int size;
    {
    char** its;
    int i;

    its = (char**) malloc( rows * sizeof(char*) );
    if ( its == (char**) 0 )
	pm_error( "out of memory allocating an array" );
    its[0] = (char*) malloc( rows * cols * size );
    if ( its[0] == (char*) 0 )
	pm_error( "out of memory allocating an array" );
    for ( i = 1; i < rows; ++i )
	its[i] = &(its[0][i * cols * size]);
    return its;
    }

void
pm_freearray( its, rows )
    char** its;
    int rows;
    {
    free( its[0] );
    free( its );
    }


/* Case-insensitive keyword matcher. */

int
pm_keymatch( str, keyword, minchars )
    char* str;
    char* keyword;
    int minchars;
    {
    register int len;

    len = strlen( str );
    if ( len < minchars )
	return 0;
    while ( --len >= 0 )
	{
	register char c1, c2;

	c1 = *str++;
	c2 = *keyword++;
	if ( c2 == '\0' )
	    return 0;
	if ( isupper( c1 ) )
	    c1 = tolower( c1 );
	if ( isupper( c2 ) )
	    c1 = tolower( c2 );
	if ( c1 != c2 )
	    return 0;
	}
    return 1;
    }


/* Log base two hacks. */

int
pm_maxvaltobits( maxval )
    int maxval;
    {
    if ( maxval <= 1 )
	return 1;
    else if ( maxval <= 3 )
	return 2;
    else if ( maxval <= 7 )
	return 3;
    else if ( maxval <= 15 )
	return 4;
    else if ( maxval <= 31 )
	return 5;
    else if ( maxval <= 63 )
	return 6;
    else if ( maxval <= 127 )
	return 7;
    else if ( maxval <= 255 )
	return 8;
    else if ( maxval <= 511 )
	return 9;
    else if ( maxval <= 1023 )
	return 10;
    else if ( maxval <= 2047 )
	return 11;
    else if ( maxval <= 4095 )
	return 12;
    else if ( maxval <= 8191 )
	return 13;
    else if ( maxval <= 16383 )
	return 14;
    else if ( maxval <= 32767 )
	return 15;
    else if ( (long) maxval <= 65535L )
	return 16;
    else
	pm_error( "maxval of %d is too large!", maxval );
    }

int
pm_bitstomaxval( bits )
    int bits;
    {
    return ( 1 << bits ) - 1;
    }


/* Initialization. */

static char* progname;
static int showmessages;

void
pm_init( argcP, argv )
    int* argcP;
    char* argv[];
    {
    int argn, i;

    /* Extract program name. */
    progname = rindex( argv[0], '/');
    if ( progname == NULL )
	progname = argv[0];
    else
	++progname;

    /* Check for any global args. */
    showmessages = 1;
    for ( argn = 1; argn < *argcP; ++argn )
	{
	if ( pm_keymatch( argv[argn], "-quiet", 6 ) )
	    {
	    showmessages = 0;
	    }
	else if ( pm_keymatch( argv[argn], "-version", 7 ) )
	    {
	    pm_message( "Version of %s", PBMPLUS_VERSION );
#ifdef BSD
	    pm_message( "BSD defined" );
#endif /*BSD*/
#ifdef SYSV
	    pm_message( "SYSV defined" );
#endif /*SYSV*/
#ifdef MSDOS
	    pm_message( "MSDOS defined" );
#endif /*MSDOS*/
#ifdef PBMPLUS_RAWBITS
	    pm_message( "PBMPLUS_RAWBITS defined" );
#endif /*PBMPLUS_RAWBITS*/
#ifdef PBMPLUS_BROKENPUTC1
	    pm_message( "PBMPLUS_BROKENPUTC1 defined" );
#endif /*PBMPLUS_BROKENPUTC1*/
#ifdef PBMPLUS_BROKENPUTC2
	    pm_message( "PBMPLUS_BROKENPUTC2 defined" );
#endif /*PBMPLUS_BROKENPUTC2*/
#ifdef PGM_BIGGRAYS
	    pm_message( "PGM_BIGGRAYS defined" );
#endif /*PGM_BIGGRAYS*/
#ifdef PPM_PACKCOLORS
	    pm_message( "PPM_PACKCOLORS defined" );
#endif /*PPM_PACKCOLORS*/
#ifdef DEBUG
	    pm_message( "DEBUG defined" );
#endif /*DEBUG*/
#ifdef NEED_VFPRINTF1
	    pm_message( "NEED_VFPRINTF1 defined" );
#endif /*NEED_VFPRINTF1*/
#ifdef NEED_VFPRINTF2
	    pm_message( "NEED_VFPRINTF2 defined" );
#endif /*NEED_VFPRINTF2*/
#ifdef RGB_DB
	    pm_message( "RGB_DB=\"%s\"", RGB_DB );
#endif /*RGB_DB*/
#ifdef LIBTIFF
	    pm_message( "LIBTIFF defined" );
#endif /*LIBTIFF*/
	    exit( 0 );
	    }
	else
	    continue;
	for ( i = argn + 1; i <= *argcP; ++i )
	    argv[i - 1] = argv[i];
	--(*argcP);
	}
    }

void
pbm_init( argcP, argv )
    int* argcP;
    char* argv[];
    {
    pm_init( argcP, argv );
    }


/* Error handling. */

void
pm_usage( usage )
    char* usage;
    {
    fprintf( stderr, "usage:  %s %s\n", progname, usage );
    exit( 1 );
    }

void
pm_perror( reason )
    char* reason;
    {
    extern char* sys_errlist[];
    extern int errno;
    char* e;

    e = sys_errlist[errno];

    if ( reason != 0 && reason[0] != '\0' )
	pm_error( "%s - %s", reason, e );
    else
	pm_error( "%s", e );
    }

#if __STDC__
void
pm_message( char* format, ... )
    {
    va_list args;

    va_start( args, format );
#else /*__STDC__*/
/*VARARGS1*/
void
pm_message( va_alist )
    va_dcl
    { /*}*/
    va_list args;
    char* format;

    va_start( args );
    format = va_arg( args, char* );
#endif /*__STDC__*/

    if ( showmessages )
	{
	fprintf( stderr, "%s: ", progname );
	(void) vfprintf( stderr, format, args );
	fputc( '\n', stderr );
	}
    va_end( args );
    }

#if __STDC__
void
pm_error( char* format, ... )
    {
    va_list args;

    va_start( args, format );
#else /*__STDC__*/
/*VARARGS1*/
void
pm_error( va_alist )
    va_dcl
    { /*}*/
    va_list args;
    char* format;

    va_start( args );
    format = va_arg( args, char* );
#endif /*__STDC__*/

    fprintf( stderr, "%s: ", progname );
    (void) vfprintf( stderr, format, args );
    fputc( '\n', stderr );
    va_end( args );
    exit( 1 );
    }

#ifdef NEED_VFPRINTF1

/* Micro-vfprintf, for systems that don't have vfprintf but do have _doprnt.
*/

int
vfprintf( stream, format, args )
    FILE* stream;
    char* format;
    va_list args;
    {
    return _doprnt( format, args, stream );
    }
#endif /*NEED_VFPRINTF1*/

#ifdef NEED_VFPRINTF2

/* Portable mini-vfprintf, for systems that don't have either vfprintf or
** _doprnt.  This depends only on fprintf.  If you don't have fprintf,
** you might consider getting a new stdio library.
*/

int
vfprintf( stream, format, args )
    FILE* stream;
    char* format;
    va_list args;
    {
    int n;
    char* ep;
    char fchar;
    char tformat[512];
    int do_long;
    int i;
    long l;
    unsigned u;
    unsigned long ul;
    char* s;
    double d;

    n = 0;
    while ( *format != '\0' )
	{
	if ( *format != '%' )
	    { /* Not special, just write out the char. */
	    (void) putc( *format, stream );
	    ++n;
	    ++format;
	    }
	else
	    {
	    do_long = 0;
	    ep = format + 1;

	    /* Skip over all the field width and precision junk. */
	    if ( *ep == '-' )
		++ep;
	    if ( *ep == '0' )
		++ep;
	    while ( isdigit( *ep ) )
		++ep;
	    if ( *ep == '.' )
		{
		++ep;
		while ( isdigit( *ep ) )
		    ++ep;
		}
	    if ( *ep == '#' )
		++ep;
	    if ( *ep == 'l' )
		{
		do_long = 1;
		++ep;
		}

	    /* Here's the field type.  Extract it, and copy this format
	    ** specifier to a temp string so we can add an end-of-string.
	    */
	    fchar = *ep;
	    (void) strncpy( tformat, format, ep - format + 1 );
	    tformat[ep - format + 1] = '\0';

	    /* Now do a one-argument fprintf with the format string we have
	    ** isolated.
	    */
	    switch ( fchar )
		{
		case 'd':
		if ( do_long )
		    {
		    l = va_arg( args, long );
		    n += fprintf( stream, tformat, l );
		    }
		else
		    {
		    i = va_arg( args, int );
		    n += fprintf( stream, tformat, i );
		    }
		break;

	        case 'o':
	        case 'x':
	        case 'X':
	        case 'u':
		if ( do_long )
		    {
		    ul = va_arg( args, unsigned long );
		    n += fprintf( stream, tformat, ul );
		    }
		else
		    {
		    u = va_arg( args, unsigned );
		    n += fprintf( stream, tformat, u );
		    }
		break;

	        case 'c':
		i = (char) va_arg( args, int );
		n += fprintf( stream, tformat, i );
		break;

	        case 's':
		s = va_arg( args, char* );
		n += fprintf( stream, tformat, s );
		break;

	        case 'e':
	        case 'E':
	        case 'f':
	        case 'g':
	        case 'G':
		d = va_arg( args, double );
		n += fprintf( stream, tformat, d );
		break;

	        case '%':
		(void) putc( '%', stream );
		++n;
		break;

		default:
		return -1;
		}

	    /* Resume formatting on the next character. */
	    format = ep + 1;
	    }
	}
    return nc;
    }
#endif /*NEED_VFPRINTF2*/


/* File open/close that handles "-" as stdin and checks errors. */

FILE*
pm_openr( name )
    char* name;
    {
    FILE* f;

    if ( strcmp( name, "-" ) == 0 )
	f = stdin;
    else
	{
#ifdef MSDOS
	f = fopen( name, "rb" );
#else /*MSDOS*/
	f = fopen( name, "r" );
#endif /*MSDOS*/
	if ( f == NULL )
	    {
	    pm_perror( name );
	    exit( 1 );
	    }
	}
    return f;
    }

FILE*
pm_openw( name )
    char* name;
    {
    FILE* f;

#ifdef MSDOS
    f = fopen( name, "wb" );
#else /*MSDOS*/
    f = fopen( name, "w" );
#endif /*MSDOS*/
    if ( f == NULL )
	{
	pm_perror( name );
	exit( 1 );
	}
    return f;
    }

void
pm_close( f )
    FILE* f;
    {
    fflush( f );
    if ( ferror( f ) )
	pm_message( "a file read or write error occurred at some point" );
    if ( f != stdin )
	if ( fclose( f ) != 0 )
	    pm_perror( "fclose" );
    }

/* Endian I/O.
*/

int
pm_readbigshort( in, sP )
    FILE* in;
    short* sP;
    {
    int c;

    if ( (c = getc( in )) == EOF )
	return -1;
    *sP = ( c & 0xff ) << 8;
    if ( (c = getc( in )) == EOF )
	return -1;
    *sP |= c & 0xff;
    return 0;
    }

#if __STDC__
int
pm_writebigshort( FILE* out, short s )
#else /*__STDC__*/
int
pm_writebigshort( out, s )
    FILE* out;
    short s;
#endif /*__STDC__*/
    {
    (void) putc( ( s >> 8 ) & 0xff, out );
    (void) putc( s & 0xff, out );
    return 0;
    }

int
pm_readbiglong( in, lP )
    FILE* in;
    long* lP;
    {
    int c;

    if ( (c = getc( in )) == EOF )
	return -1;
    *lP = ( c & 0xff ) << 24;
    if ( (c = getc( in )) == EOF )
	return -1;
    *lP |= ( c & 0xff ) << 16;
    if ( (c = getc( in )) == EOF )
	return -1;
    *lP |= ( c & 0xff ) << 8;
    if ( (c = getc( in )) == EOF )
	return -1;
    *lP |= c & 0xff;
    return 0;
    }

int
pm_writebiglong( out, l )
    FILE* out;
    long l;
    {
    (void) putc( ( l >> 24 ) & 0xff, out );
    (void) putc( ( l >> 16 ) & 0xff, out );
    (void) putc( ( l >> 8 ) & 0xff, out );
    (void) putc( l & 0xff, out );
    return 0;
    }

int
pm_readlittleshort( in, sP )
    FILE* in;
    short* sP;
    {
    int c;

    if ( (c = getc( in )) == EOF )
	return -1;
    *sP = c & 0xff;
    if ( (c = getc( in )) == EOF )
	return -1;
    *sP |= ( c & 0xff ) << 8;
    return 0;
    }

#if __STDC__
int
pm_writelittleshort( FILE* out, short s )
#else /*__STDC__*/
int
pm_writelittleshort( out, s )
    FILE* out;
    short s;
#endif /*__STDC__*/
    {
    (void) putc( s & 0xff, out );
    (void) putc( ( s >> 8 ) & 0xff, out );
    return 0;
    }

int
pm_readlittlelong( in, lP )
    FILE* in;
    long* lP;
    {
    int c;

    if ( (c = getc( in )) == EOF )
	return -1;
    *lP = c & 0xff;
    if ( (c = getc( in )) == EOF )
	return -1;
    *lP |= ( c & 0xff ) << 8;
    if ( (c = getc( in )) == EOF )
	return -1;
    *lP |= ( c & 0xff ) << 16;
    if ( (c = getc( in )) == EOF )
	return -1;
    *lP |= ( c & 0xff ) << 24;
    return 0;
    }

int
pm_writelittlelong( out, l )
    FILE* out;
    long l;
    {
    (void) putc( l & 0xff, out );
    (void) putc( ( l >> 8 ) & 0xff, out );
    (void) putc( ( l >> 16 ) & 0xff, out );
    (void) putc( ( l >> 24 ) & 0xff, out );
    return 0;
    }
