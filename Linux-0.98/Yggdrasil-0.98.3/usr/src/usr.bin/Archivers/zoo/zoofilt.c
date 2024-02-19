/* derived from: zoofilt.c 1.8 88/01/30 23:47:05 */

#ifndef LINT
static char sccsid[]="@(#) $Id: zoofilt.c,v 1.5 91/07/09 01:54:15 dhesi Exp $";
#endif

/*
(C) Copyright 1988 Rahul Dhesi -- All rights reserved
(C) Copyright 1991 Rahul Dhesi -- All rights reserved

Filter mode -- compress or decompress standard input and write
to standard output.
*/

#include "options.h"

#ifdef FILTER

#include "zooio.h"
#include "errors.i"
#include "zoofns.h"

/* action */
#define COMPRESS		0
#define UNCOMPRESS	1

#define FTAG	((unsigned int) 0x5a32)	/* magic number */

extern unsigned int crccode;

int rdint PARMS((unsigned int *));	/* read an unsigned int */
int wrint PARMS((unsigned int));	/* write an unsigned int */

/* global variable used to pass two bytes (CRC value) back from lzd to here */
unsigned int filt_lzd_word;

void zoofilt (option)
char *option;
{
	int choice;											/* what to do -- [de]compress */
	unsigned int filetag;							/* tag stored in input */
	int stat1, stat2, stat3;						/* status codes */
	int use_lzh = 0;									/* use lzh instead */
	extern lzc(), lzh_encode();					/* possible encoders */
	extern lzd(), lzh_decode();					/* and decoders */

	while (*++option) {
		switch (*option) {
			case 'c':	choice = COMPRESS;    break;
			case 'u':	choice = UNCOMPRESS;  break;
			case 'h':	use_lzh = 1; break;
			default:
         	prterror ('f', inv_option, *option);	/* fatal error -- abort */
      }
	}

	crccode = 0;	/* needed whether compressing or uncompressing */

	switch (choice) {
		case COMPRESS:
			stat1 = wrint (FTAG);
			stat2 = (use_lzh ? lzh_encode : lzc) (STDIN, STDOUT);
			stat3 = wrint (crccode);
			if (stat1 == 0 && stat2 == 0 && stat3 == 0)
				zooexit (0);
			else {
				fprintf (stderr, "Zoo: FATAL: Compression error.\n");
				zooexit (1);
			}
			break;
		case UNCOMPRESS:
			stat1 = rdint (&filetag);
			if (stat1 != 0 || filetag != FTAG)
				zooexit (1);
			stat2 = (use_lzh ? lzh_decode : lzd) (STDIN, STDOUT);
			if (stat2 == 0 && filt_lzd_word == crccode)
				zooexit (0);
			else {
				fprintf (stderr, "Zoo: FATAL: Uncompression error.\n");
				zooexit (1);
			}
			break;
	}
} /* zoofilt */

#endif /* FILTER */
