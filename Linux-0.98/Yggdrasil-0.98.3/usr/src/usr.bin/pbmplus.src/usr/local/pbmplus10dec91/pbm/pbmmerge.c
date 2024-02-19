/* pbmmerge.c - wrapper program for PBM
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
#include "pbm.h"

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
    if ( strcmp( cp, "pbmmerge" ) == 0 )
	{
	++argv;
	--argc;
	goto again;
	}

#define TRY(s,m) { if ( strcmp( cp, s ) == 0 ) exit( m( argc, argv ) ); }

    TRY( "atktopbm", atktopbm_main );
    TRY( "brushtopbm", brushtopbm_main );
    TRY( "cmuwmtopbm", cmuwmtopbm_main );
    TRY( "g3topbm", g3topbm_main );
    TRY( "icontopbm", icontopbm_main );
    TRY( "gemtopbm", gemtopbm_main );
    TRY( "macptopbm", macptopbm_main );
    TRY( "mgrtopbm", mgrtopbm_main );
    TRY( "pbmlife", pbmlife_main );
    TRY( "pbmmake", pbmmake_main );
    TRY( "pbmmask", pbmmask_main );
    TRY( "pbmreduce", pbmreduce_main );
    TRY( "pbmtext", pbmtext_main );
    TRY( "pbmto10x", pbmto10x_main );
    TRY( "pbmtoascii", pbmtoascii_main );
    TRY( "pbmtoatk", pbmtoatk_main );
    TRY( "pbmtobbnbg", pbmtobbnbg_main );
    TRY( "pbmtocmuwm", pbmtocmuwm_main );
    TRY( "pbmtoepson", pbmtoepson_main );
    TRY( "pbmtog3", pbmtog3_main );
    TRY( "pbmtogem", pbmtogem_main );
    TRY( "pbmtogo", pbmtogo_main );
    TRY( "pbmtoicon", pbmtoicon_main );
    TRY( "pbmtolj", pbmtolj_main );
    TRY( "pbmtomacp", pbmtomacp_main );
    TRY( "pbmtomgr", pbmtomgr_main );
    TRY( "pbmtopi3", pbmtopi3_main );
    TRY( "pbmtoplot", pbmtoplot_main );
    TRY( "pbmtoptx", pbmtoptx_main );
    TRY( "pbmtox10bm", pbmtox10bm_main );
    TRY( "pbmtoxbm", pbmtoxbm_main );
    TRY( "pbmtoybm", pbmtoybm_main );
    TRY( "pbmtozinc", pbmtozinc_main );
    TRY( "pbmupc", pbmupc_main );
    TRY( "pi3topbm", pi3topbm_main );
    TRY( "xbmtopbm", xbmtopbm_main );
    TRY( "ybmtopbm", ybmtopbm_main );

    (void) fprintf(
	stderr, "pbmmerge: \"%s\" is an unknown PBM program!\n", cp );
    exit( 1 );
    }
