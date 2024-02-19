/* Copyright (C) 1992 Aladdin Enterprises.  All rights reserved.
   Distributed by Free Software Foundation, Inc.

This file is part of Ghostscript.

Ghostscript is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.  Refer
to the Ghostscript General Public License for full details.

Everyone is granted permission to copy, modify and redistribute
Ghostscript, but only under the conditions described in the Ghostscript
General Public License.  A copy of this license is supposed to have been
given to you along with Ghostscript so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies.  */

/* echogs.c */
/* 'echo'-like utility */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>		/* for ctime */

/*
 * Usage:
	echogs [-w[b] file | -a[b] file] [-n]
	  (-D | -x hexstring | -q string | -s | -i | -r file)*
	  [-] string*
 * Echoes string(s), or the binary equivalent of hexstring(s).
 * If -w, writes to file; if -a, appends to file; if neither,
 * writes to stdout.  -wb and -ab open the file in binary mode.
 * If -n, does not append a newline to the output.  -s writes a space.
 * -D means insert the date and time.
 * -i means read from stdin, treating each line as an argument.
 * -r means read from a named file in the same way.
 * -X means treat any following literals as hex rather than string data.
 * - alone means treat the rest of the line as literal data,
 * even if the first string begins with a -.
 * Inserts spaces automatically between the trailing strings,
 * but nowhere else; in particular,
	echogs -q a b
 * writes 'ab', in contrast to
	echogs -q a -s b
 * which writes 'a b'.
 *
 * This program exists solely to get around omissions, problems, and
 * incompatibilities in the various shells and utility environments
 * that Ghostscript must deal with.  Don't count on it staying the same
 * from one Ghostscript release to another!
 */

main(argc, argv)
    int argc;
    char *argv[];
{	FILE *out = stdout;
	FILE *in;
	char *fmode;
	char *fname = 0;
	int newline = 1;
	int interact = 0;
#define LINESIZE 1000
	char line[LINESIZE];
	char sw = 0, sp = 0, hexx = 0;
	char **argp = argv + 1;
	int nargs = argc - 1;
	if ( nargs > 0 && (*argp)[0] == '-' &&
	      ((*argp)[1] == 'w' || (*argp)[1] == 'a')
	   )
	{	if ( nargs < 2 ) return 1;
		fmode = *argp + 1;
		fname = argp[1];
		argp += 2, nargs -= 2;
	}
	if ( nargs > 0 && !strcmp(*argp, "-n") )
	{	newline = 0;
		argp++, nargs--;
	}
	if ( fname != 0 )
	{	out = fopen(fname, fmode);
		if ( out == 0 ) return 1;
	}
	while ( 1 )
	{	char *arg;
		if ( interact )
		{	if ( fgets(line, LINESIZE, in) == NULL )
			{	interact = 0;
				if ( in != stdin ) fclose(in);
				continue;
			}
			/* Remove the terminating \n. */
			line[strlen(line) - 1] = 0;
			arg = line;
		}
		else
		{	if ( nargs == 0 ) break;
			arg = *argp;
			argp++, nargs--;
		}
		if ( sw == 0 && arg[0] == '-' )
		{	sp = 0;
			switch ( arg[1] )
			{
			case 'q':		/* literal string */
			case 'r':		/* read from a file */
			case 'x':		/* hex string */
				sw = arg[1];
				break;
			case 's':		/* write a space */
				fputc(' ', out);
				break;
			case 'i':		/* read interactively */
				interact = 1;
				in = stdin;
				break;
			case 'D':		/* insert date/time */
			{	long t;
				char str[26];
				time(&t);
				strcpy(str, ctime(&t));
				str[24] = 0;	/* remove \n */
				fputs(str, out);
			}	break;
			case 'X':		/* treat literals as hex */
				hexx = 1;
				break;
			case 0:			/* just '-' */
				sw = '-';
				break;
			}
		}
		else
		  switch ( sw )
		{
		case 0:
		case '-':
			if ( hexx ) goto xx;
			if ( sp ) fputc(' ', out);
			fputs(arg, out);
			sp = 1;
			break;
		case 'q':
			sw = 0;
			fputs(arg, out);
			break;
		case 'r':
			sw = 0;
			in = fopen(arg, "r");
			if ( in == NULL ) exit(1);
			interact = 1;
			break;
		case 'x':
xx:		{	char *xp;
			unsigned int xchr = 1;
			for ( xp = arg; *xp; xp++ )
			{	char ch = *xp;
				if ( !isxdigit(ch) ) return 1;
				xchr <<= 4;
				xchr += (isdigit(ch) ? ch - '0' :
					 (isupper(ch) ? tolower(ch) : ch)
					  - 'a' + 10);
				if ( xchr >= 0x100 )
				{	fputc(xchr & 0xff, out);
					xchr = 1;
				}
			}
		}	sw = 0;
			break;
		}
	}
	if ( newline ) fputc('\n', out);
	if ( out != stdout ) fclose(out);
	return 0;
}
