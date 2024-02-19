/* pgmmerge.c - wrapper program for PGM
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
#include "pgm.h"

void
main( argc, argv )
int argc;
char *argv[];
    {
    register char *cp;

again:
    if ( ( cp = rindex( argv[0], '/' ) ) != (char*) 0 )
	++cp;
    else
	cp = argv[0];
    if ( strcmp( cp, "pgmmerge" ) == 0 )
	{
	++argv;
	--argc;
	goto again;
	}

#define TRY(s,m) { if ( strcmp( cp, s ) == 0 ) exit( m( argc, argv ) ); }

    TRY("fitstopgm", fitstopgm_main);
    TRY("fstopgm", fstopgm_main);
    TRY("hipstopgm", hipstopgm_main);
    TRY("lispmtopgm", lispmtopgm_main);
    TRY("pgmbentley", pgmbentley_main);
    TRY("pgmcrater", pgmcrater_main);
    TRY("pgmedge", pgmedge_main);
    TRY("pgmenhance", pgmenhance_main);
    TRY("pgmhist", pgmhist_main);
    TRY("pgmnorm", pgmnorm_main);
    TRY("pgmoil", pgmoil_main);
    TRY("pgmramp", pgmramp_main);
    TRY("pgmtexture", pgmtexture_main);
    TRY("pgmtopbm", pgmtopbm_main);
    TRY("pgmtofits", pgmtofits_main);
    TRY("pgmtofs", pgmtofs_main);
    TRY("pgmtolispm", pgmtolispm_main);
    TRY("psidtopgm", psidtopgm_main);
    TRY("rawtopgm", rawtopgm_main);

    (void) fprintf(
	stderr, "pgmmerge: \"%s\" is an unknown PGM program!\n", cp );
    exit( 1 );
    }
