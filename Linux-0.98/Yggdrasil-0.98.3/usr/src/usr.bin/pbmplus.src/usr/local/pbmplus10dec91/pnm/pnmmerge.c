/* pnmmerge.c - wrapper program for PNM
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

#include <stdio.h>
#include "pnm.h"

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    register char* cp;

again:
    if ( ( cp = rindex( argv[0], '/' ) ) != (char*) 0 )
	++cp;
    else
	cp = argv[0];
    if ( strcmp( cp, "pnmmerge" ) == 0 )
	{
	++argv;
	--argc;
	goto again;
	}

#define TRY(s,m) { if ( strcmp( cp, s ) == 0 ) exit( m( argc, argv ) ); }

    TRY("pnmarith", pnmarith_main);
    TRY("pnmcat", pnmcat_main);
    TRY("pnmconvol", pnmconvol_main);
    TRY("pnmcrop", pnmcrop_main);
    TRY("pnmcut", pnmcut_main);
    TRY("pnmdepth", pnmdepth_main);
    TRY("pnmenlarge", pnmenlarge_main);
    TRY("pnmfile", pnmfile_main);
    TRY("pnmflip", pnmflip_main);
    TRY("pnmgamma", pnmgamma_main);
    TRY("pnminvert", pnminvert_main);
    TRY("pnmnoraw", pnmnoraw_main);
    TRY("pnmpaste", pnmpaste_main);
    TRY("pnmrotate", pnmrotate_main);
    TRY("pnmscale", pnmscale_main);
    TRY("pnmshear", pnmshear_main);
    TRY("pnmtile", pnmtile_main);
    TRY("pnmtops", pnmtops_main);
    TRY("pnmtorast", pnmtorast_main);
#ifdef LIBTIFF
    TRY("pnmtotiff", pnmtotiff_main);
#endif /*LIBTIFF*/
    TRY("pnmtoxwd", pnmtoxwd_main);
    TRY("rasttopnm", rasttopnm_main);
#ifdef LIBTIFF
    TRY("tifftopnm", tifftopnm_main);
#endif /*LIBTIFF*/
    TRY("xwdtopnm", xwdtopnm_main);

    (void) fprintf(
	stderr, "pnmmerge: \"%s\" is an unknown PNM program!\n", cp );
    exit( 1 );
    }
