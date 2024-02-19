/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module global.cpp

	RCS - Header:
	$Header: c:/daten/fips/source/main/RCS/global.cpp 1.4 1995/01/19 00:00:52 schaefer Exp schaefer $

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

#include <stdarg.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>

#include "version.h"
#include "global.h"

#define CTRL_C 3

global_vars global;

/* ----------------------------------------------------------------------- */
/* Initialization of global variables                                      */
/* ----------------------------------------------------------------------- */

global_vars::global_vars (void)
{
	test_mode = false;
	verbose_mode = true;
	debug_mode = false;

	drive_number_cmdline = 0;
}

global_vars::~global_vars (void)
{
	if (debug_mode) fclose (debugfile);
}

void exit_function (void)
{
	printx ("\nBye!\n");
}

void global_vars::open_debugfile (int argc,char *argv[])
{
	if ((debugfile = fopen ("fipsinfo.dbg","wt")) == NULL)
	{
		global.debug_mode = false;
		warning (true, "Can't open debug file");
	}
	else
	{
		fprintf (debugfile,"FIPS debug file\n\n");
		fprintf (debugfile,"The command was: ");
		while (argc--) fprintf (debugfile,argc ? "%s " : "%s", *argv++);
		fprintf (debugfile,"\n\nTranscript of session:\n");
	}
}

/* ----------------------------------------------------------------------- */
/* Replacement for printf - prints to screen and debugfile                 */
/* ----------------------------------------------------------------------- */

void printx (char *fmt,...)
{
	va_list ap;
	va_start (ap,fmt);
	vprintf (fmt,ap);
	if (global.debug_mode) vfprintf (global.debugfile,fmt,ap);
	va_end (ap);
}

/* ----------------------------------------------------------------------- */
/* Replacement for getch - exit when CTRL-C is pressed                     */
/* ----------------------------------------------------------------------- */

int getx (void)
{
	int character = getch();
	if (character == CTRL_C)
	{
		printx ("\n");
		exit (0);
	}
	return (character);
}

/* ----------------------------------------------------------------------- */
/* Copyright notice and version number                                     */
/* ----------------------------------------------------------------------- */

void notice (void)
{
	printx ("\nFIPS version " FIPS_VERSION ", Copyright (C) 1993/94 Arno Schaefer\n\n");
	printx ("DO NOT use FIPS in a multitasking environment like Windows, OS/2, Desqview,\n");
	printx ("Novell Task manager or the Linux DOS emulator: boot from a DOS boot disk first.\n\n");
	printx ("If you use OS/2 or a disk compressor, read the relevant sections in FIPS.DOC.\n\n");
	printx ("FIPS comes with ABSOLUTELY NO WARRANTY, see file COPYING for details\n");
	printx ("This is free software, and you are welcome to redistribute it\n");
	printx ("under certain conditions; again see file COPYING for details.\n");

	printx ("\nPress any Key\n");
	getx();
}

/* ----------------------------------------------------------------------- */
/* Hexdump binary data into a file                                         */
/* ----------------------------------------------------------------------- */

void hexwrite (byte *buffer,int number,FILE *file)
{
	for (int i=0;i<number;i++)
	{
		fprintf (file,"%02X ",*(buffer+i));
		if ((i+1)%16 == 0) fprintf (file,"\n");
		else if ((i+1)%8 == 0) fprintf (file,"- ");
	}
	fprintf (file,"\n");
}

/* ----------------------------------------------------------------------- */
/* Error Handling                                                          */
/* ----------------------------------------------------------------------- */

static void print_verbose_message (char *message)
{
	char line[256];
	int length = 0;
	FILE *error_msg_file;

	fprintf (stderr,"\n");
	if (global.debug_mode) fprintf (global.debugfile,"\n");

	if ((error_msg_file = fopen ("errors.txt","rt")) == NULL)
	{
		fprintf (stderr,"File ERRORS.TXT not found - no verbose messages available\n");
		if (global.debug_mode) fprintf (global.debugfile,"File ERRORS.TXT not found - no verbose messages available\n");
		global.verbose_mode = false;
		return;
	}

	while (message[length] != 0 && message[length] != ':') length++;

	fgets (line,255,error_msg_file);
	while (strncmp(message,line,length)) if (fgets (line,255,error_msg_file) == NULL) return;
	fgets (line,255,error_msg_file);
	while (!strncmp("  ",line,2))
	{
		fprintf (stderr,"%s",line+2);
		if (global.debug_mode) fprintf (global.debugfile,"%s",line+2);
		if (fgets (line,255,error_msg_file) == NULL) return;
	}
	fclose (error_msg_file);
}

void error (char *message,...)
{
	va_list ap;

	va_start (ap,message);

	fprintf (stderr,"\nError: ");
	vfprintf (stderr,message,ap);
	fprintf (stderr,"\n");

	if (global.debug_mode)
	{
		fprintf (global.debugfile,"\nError: ");
		vfprintf (global.debugfile,message,ap);
		fprintf (global.debugfile,"\n");
	}

	va_end (ap);

	if (global.verbose_mode) print_verbose_message (message);

	exit (-1);
}

void warning (boolean wait_key, char *message,...)
{
	va_list ap;

	va_start (ap,message);

	fprintf (stderr,"\nWarning: ");
	vfprintf (stderr,message,ap);
	fprintf (stderr,"\n");

	if (global.debug_mode)
	{
		fprintf (global.debugfile,"\nWarning: ");
		vfprintf (global.debugfile,message,ap);
		fprintf (global.debugfile,"\n");
	}

	va_end (ap);

	if (global.verbose_mode) print_verbose_message (message);

	if (wait_key)
	{
		fprintf (stderr,"\nPress any key\n");
		if (global.debug_mode) fprintf (global.debugfile,"\nPress any key\n");

		getx ();
	}
}

void infomsg (char *message,...)
{
	va_list ap;

	va_start (ap,message);

	fprintf (stderr,"\nInfo: ");
	vfprintf (stderr,message,ap);
	fprintf (stderr,"\n");

	if (global.debug_mode)
	{
		fprintf (global.debugfile,"\nInfo: ");
		vfprintf (global.debugfile,message,ap);
		fprintf (global.debugfile,"\n");
	}

	va_end (ap);

	if (global.verbose_mode) print_verbose_message (message);

	fprintf (stderr,"\nPress any key\n");
	if (global.debug_mode) fprintf (global.debugfile,"\nPress any key\n");

	getx ();
}
