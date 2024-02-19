/*
 * Convert a GEM .img file to a portable bitmap file.
 *
 * Author: Diomidis D. Spinellis
 * (C) Copyright 1988 Diomidis D. Spinellis.
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
 *
 * Comments and additions should be sent to the author:
 *
 *                     Diomidis D. Spinellis
 *                     1 Myrsinis Str.
 *                     GR-145 62 Kifissia
 *                     GREECE
 *
 */

#include <assert.h>
#include "pbm.h"

/*
 * Handle systems that do CR-LF translation on reading / writing and
 * little endians (some guesswork is involved).
 */
#ifdef MSDOS
#define LITTLE_ENDIAN
#endif /*MSDOS*/
#ifdef xenix
#define LITTLE_ENDIAN
#endif /*xenix*/
#ifdef sun386
#define LITTLE_ENDIAN
#endif /*sun386*/
#ifdef vax
#define LITTLE_ENDIAN
#endif /*vax*/


/* Seek from current position */
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif /*SEEK_CUR*/

/*
 * File header structure
 */
struct header {
	short           version;/* Image file version */
	unsigned short  hlen;	/* Header length in bytes */
	unsigned short  planes;	/* Number of planes */
	unsigned short  patlen;	/* Pattern definition length (bytes) */
	unsigned short  pxlen;	/* Pixel height (microns) */
	unsigned short  pxht;	/* Pixel height (microns) */
	unsigned short  linewid;/* Scan line width (bytes) */
	unsigned short  nitems;	/* Number of scan line items */
};

/*
 * Scan item header
 */
struct line {
	char            d1, d2;	/* Should be 0 */
	char            d3;	/* Should be 0xff */
	char            count;	/* Repetition count */
};

char            pattern[256];


#ifdef LITTLE_ENDIAN
static void byteswap ARGS(( unsigned short* data, int n ));
#endif /*LITTLE_ENDIAN*/

void
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             debug = 0;
	FILE           *f;
	struct header   hd;
	int             x;
	int             i, j, k, l;
	int             c, cc, linerep;
	int		rows, cols;
	bit		*bitrow;

	pbm_init( &argc, argv );

	/* Check if the compiler alligns structures the way we want */
	if ( sizeof( struct header ) != 16 )
		pm_error( "sizeof( struct header ) != 16 ???" );

	if (argc > 1 && !strcmp(argv[1], "-d")) {
		argc--;
		argv[1] = argv[0];
		++argv;
		++debug;
	}

	if (argc != 2)
		pm_usage("[-d] <gemfile>");

	f = pm_openr( argv[1] );

	if (fread(&hd, sizeof hd, 1, f) != 1)
		pm_perror( "read" );

#ifdef LITTLE_ENDIAN
	byteswap((unsigned short *) &hd, sizeof hd / 2);
#endif /*LITTLE_ENDIAN*/

	for ( i = sizeof(struct header); i < hd.hlen; ++i )
	    (void) getc(f);

	if (debug) {
		pm_message(
			"version %d, hlen %d, planes %d, patlen %d",
		        hd.version, hd.hlen, hd.planes, hd.patlen, 0);
		pm_message(
			"pxlen %d, pxht %d, linewid %d, nitems %d",
			hd.pxlen, hd.pxht, hd.linewid, hd.nitems, 0);
	}

	cols = hd.linewid % 8 == 0 ? hd.linewid : hd.linewid + 8 - hd.linewid % 8;
	rows = hd.nitems;
	pbm_writepbminit( stdout, cols, rows, 0 );
	bitrow = pbm_allocrow( cols );

	for (i = 0; i < hd.nitems; ) {
		x = 0;
		linerep = 0;
		while (x < hd.linewid) {
			switch (c = getc(f)) {
			case 0x80:	/* Bit String */
				c = getc(f);	/* Byte count */
				if (debug)
					pm_message(
						"bit string of %d bytes",
						c );
				for (j = 0; j < c; ++j) {
					cc = getc(f);
					for (k = 0x80; k; k >>= 1) {
						bitrow[x] = (k & cc) ? PBM_BLACK : PBM_WHITE;
						++x;
					}
				}
				break;
			case 0:		/* Pattern run */
				c = getc(f);	/* Repeat count */
				if (debug)
					pm_message(
						"pattern run of %d repetitions",
						c );
                                /* line repeat */
                                if (c == 0) {
                                        c = getc(f);
                                        if (c != 0x00ff)
                                                pm_error( "badly formed line repeat" );
                                        linerep = getc(f);
                                        break;
                                }
				fread(pattern, 1, hd.patlen, f);
				for (j = 0; j < c; ++j)
					for (l = 0; l < hd.patlen; ++l)
						for (k = 0x80; k; k >>= 1) {
							bitrow[x] = (k & pattern[l]) ? PBM_BLACK : PBM_WHITE;
							++x;
						}
				break;

			default:	/* Solid run */
				if (debug)
					pm_message(
						"solid run of %d bytes %s",
						c & 0x7f,
						c & 0x80 ? "on" : "off" );
                                /* each byte had eight bits DSB */
                                l = (c & 0x80) ? PBM_BLACK : PBM_WHITE;
                                c = (c & 0x7f) * 8;
                                for (j = 0; j < c; ++j) {
                                        bitrow[x] = l;
                                        ++x;
                                }
				break;

			case EOF:	/* End of file */
				pm_error( "end of file reached" );

			}
		}
                if ( debug )
                        pm_message( "EOL" );
                if ( x != hd.linewid )
                        pm_error( "EOL beyond edge" );
		do {
			pbm_writepbmrow( stdout, bitrow, cols, 0 );
			++i;
		} while (linerep--);
	}
	pm_close( f );
	pm_close( stdout );
	exit(0);
}

#ifdef LITTLE_ENDIAN
static void
byteswap(data, n)
	unsigned short* data;
	int             n;
{
	register        i;
	char           *p, c;

	for (i = 0, p = (char *) data; i < n; ++i, ++p) {
		c = *p;
		*p = *(p + 1);
		*++p = c;
	}
}
#endif /*LITTLE_ENDIAN*/
