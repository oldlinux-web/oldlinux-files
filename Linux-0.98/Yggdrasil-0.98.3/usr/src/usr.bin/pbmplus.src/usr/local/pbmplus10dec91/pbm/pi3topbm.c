/*
 * Convert a ATARI Degas .pi3 file to a portable bitmap file.
 *
 * Author: David Beckemeyer
 *
 * This code was derived from the original gemtopbm program written
 * by Diomidis D. Spinellis.
 *
 * (C) Copyright 1988 David Beckemeyer and Diomidis D. Spinellis.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 */

#include <stdio.h>
#include "pbm.h"

/*
 * File header structure
 */
struct header {
	short		res;		/* resolution */
	short		pal[16];	/* palette */
};

void
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             debug = 0;
	FILE           *f;
	struct header   hd;
	int             x;
	int             i, k;
	int             c;
	int		rows, cols;
	bit		*bitrow;

	pbm_init( &argc, argv );

	if (argc > 1 && !strcmp(argv[1], "-d")) {
		--argc;
		argv[1] = argv[0];
		++argv;
		debug = 1;
	}

	if (argc == 1)
	    f = stdin;
	else if (argc == 2)
	    f = pm_openr( argv[1] );
	else
		pm_usage("[-d] [pi3file]");

	if (fread(&hd, sizeof hd, 1, f) != 1)
		pm_perror( "read" );

	if (debug)
		pm_message( "resolution is %d", hd.res );

	/* only handles hi-rez 640x400 */
	if (hd.res != 2)
		pm_error( "bad resolution" );

	cols = 640;
	rows = 400;
	pbm_writepbminit( stdout, cols, rows, 0 );
	bitrow = pbm_allocrow( cols );

	for (i = 0; i < rows; ++i) {
		x = 0;
		while (x < cols) {
			if ((c = getc(f)) == EOF)
				pm_error( "end of file reached" );
			for (k = 0x80; k; k >>= 1) {
				bitrow[x] = (k & c) ? PBM_BLACK : PBM_WHITE;
				++x;
			}
		}
		pbm_writepbmrow( stdout, bitrow, cols, 0 );
	}
	pm_close( f );
	pm_close( stdout );
	exit(0);
}
