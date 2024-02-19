/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module cmdl_arg.cpp

	RCS - Header:
	$Header: c:/daten/fips/source/main/RCS/cmdl_arg.cpp 1.4 1995/01/19 00:00:51 schaefer Exp schaefer $

	Copyright (C) 1993 Arno Schaefer

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


	Report problems and direct all questions to:

	schaefer@rbg.informatik.th-darmstadt.de
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "global.h"
#include "getopt.h"


/* ----------------------------------------------------------------------- */
/* Replacement for atoi                                                    */
/* ----------------------------------------------------------------------- */

static int atoint (char *string)
{
	long int value = 0;
	while (isdigit (*string))
	{
		value = value * 10 + (*string - '0');
		if (value > 32767) return (-1);
		string++;
	}
	if (*string != '\0') return (-1);
	return (int) value;
}


/* ----------------------------------------------------------------------- */
/* Usage instructions                                                      */
/* ----------------------------------------------------------------------- */

static void usage (void)
{
	printf ("\nFIPS [-t] [-d] [-h|-?] [-n<num>]:\n\n");
	printf ("-t        : test mode (no writes to disk)\n");
	printf ("-d        : debug mode\n");
	printf ("-h/-?     : this help page\n");
	printf ("-n<num>   : select drive <num> - valid values: 128 to 255\n");
}


/* ----------------------------------------------------------------------- */
/* Process commandline parameters                                          */
/* ----------------------------------------------------------------------- */

void evaluate_argument_vector (int argc, char *argv[])
{
	int c;

	opterr = 0;

	while ((c = getopt (argc, argv, ":htdn:")) >= 0)
	{
		switch (c)
		{
			case 't':
				global.test_mode = true;
				break;
			case 'd':
				global.debug_mode = true;
				break;
			case 'h':
				usage ();
				exit (1);
			case 'n':
				global.drive_number_cmdline = atoint (optarg);

				if
				(
					global.drive_number_cmdline < 0x80
					|| global.drive_number_cmdline > 0xff
				)
				{
					fprintf
					(
						stderr,
						"\nInvalid argument: %s\n",
						optarg
					);

					usage ();
					exit (-1);
				}
				break;
			case ':':
				fprintf
				(
					stderr,
					"\nSwitch %c requires an argument\n",
					optopt
				);
				usage ();
				exit (-1);
			case '?':
				if (optopt != '?')
					fprintf
					(
						stderr,
						"\nInvalid Commandline Parameter: %s\n",
						argv[optind - 1]
					);

				usage ();
				exit (-1);
		} /* switch */

	} /* while */
}
