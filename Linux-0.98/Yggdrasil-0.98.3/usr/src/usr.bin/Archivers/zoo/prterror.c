#ifndef LINT
/* @(#) prterror.c 2.8 88/01/31 18:48:17 */
static char sccsid[]="@(#) prterror.c 2.8 88/01/31 18:48:17";
#endif /* LINT */

/*
The contents of this file are hereby released to the public domain.

                                 -- Rahul Dhesi 1986/11/14

*/

#include "options.h"
#ifndef	OK_STDIO
#include <stdio.h>
#define	OK_STDIO
#endif
#include "various.h"
#include "zooio.h"
#include "zoofns.h"

#ifdef NEEDCTYP
# include <ctype.h>	/* for isdigit() */
#endif

#ifdef STDARG
# include <stdarg.h>
#else
# ifdef VARARGS
#  include <varargs.h>
# else
#  include "MUST DEFINE STDARG OR VARARGS"
# endif
#endif

#ifdef NEED_VPRINTF
static int zvfprintf();
#endif

/* General error handler.  Input format:

   parameter 1:  'w', 'e', or 'f'.

      'm':  message
      'M':  message without preceding identification
      'w':  WARNING
      'e':  ERROR
      'f':  FATAL
      'F':  FATAL but program doesn't exist immediately

   All text printed is preceded by "Zoo:  " or "Ooz:  "  depending
   upon conditional compilation, except in the case of 'M' messages
   which are printed without any text being added.

   For messages, the text supplied is printed if and only if the global
   variable "quiet" is zero.  Control then returns to the caller.

   For warnings, errors, and fatal errors, the variable "quiet" is used
	as follows.  Warning messages are suppressed if quiet > 1;  error
	messages are suppressed if quiet > 2.  Fatal error messages are 
	never suppressed--doing so would be a bit risky.

   For warnings and errors, the error message is preceded by the "WARNING:"
   or "ERROR".  The error message is printed and control returns to the
   caller.

   For fatal errors, the error message is preceded by "FATAL:" and an
   error message is printed.  If the option was 'f', the program exits with 
   a status of 1.  If the option was 'F', control returns to the caller and 
   it is assumed that the caller will do any cleaning up necessary and then 
   exit with an error status.

   parameter 2:  The format control string for printf.   
	remining parameters:  passed on to vprintf().

	All messages, whether informative or error, are sent to standard
	output via printf.  It might be a good idea to eventually send 'e' and
	'f' class messages to the standard error stream.  Best would be
	some way of telling if standard output and standard error are not
	the same device, so that we could always send error messages to
	standard error, and also duplicate them to standard output if 
	different from standard error.  This is one thing that VMS seems
	to be capable of doing.  There seems to be no way of doing this
	in the general case.
*/

extern int quiet;

/* These declarations must be equivalent to those in errors.i */
char no_match[] = "No files matched.\n";
char failed_consistency[] = "Archive header failed consistency check.\n";
char invalid_header[] = "Invalid or corrupted archive.\n";
char internal_error[]="Internal error.\n";
char disk_full[]      = "I/O error or disk full.\n";
char bad_directory[]  = "Directory entry in archive is invalid.\n";
char no_memory[] = "Ran out of memory.\n";
char too_many_files[] = "Some filenames ignored -- can only handle %d.\n";
char packfirst[] = "Old format archive -- please pack first with P command.\n";
char garbled[] = "Command is garbled.\n";
char start_ofs[] = "Starting at %ld (offset %ld)\n";

#ifndef OOZ
char wrong_version[]=
   "Zoo %d.%d or later is needed to fully manipulate this archive.\n";
char cant_process[] =
   "The rest of the archive (%lu bytes) cannot be processed.\n";
char option_ignored[] = "Ignoring option %c.\n";
char inv_option[] = "Option %c is invalid.\n";
char bad_crc[] = "\007Bad CRC, %s probably corrupted\n";
#endif

#ifdef OOZ
char could_not_open[] = "Could not open ";
#else
char could_not_open[] = "Could not open %s.\n";
#endif

#ifdef STDARG
void prterror(int level, char *format, ...)
#else
/*VARARGS*/
void prterror(va_alist)
va_dcl
#endif
{
	va_list args;
   char string[120];       /* local format string */
#ifdef VARARGS
	int level;
	char *format;
#endif

#ifdef STDARG
	va_start(args, format);
#else
	va_start(args);
	level = va_arg(args, int);
	format = va_arg(args, char *);
#endif

   *string = '\0';         /* get a null string to begin with */

#ifdef OOZ
   strcpy (string, "Ooz:  ");
#else
   strcpy (string, "Zoo:  ");
#endif

   switch (level) {
      case 'M': *string = '\0';                    /* fall through to 'm' */
      case 'm': if (quiet) return; break;
      case 'w': 
			if (quiet > 1) return;
			strcat (string, "WARNING:  "); break;
      case 'e': 
			if (quiet > 2) return;
			strcat (string, "ERROR:  ");   break;
      case 'F':
      case 'f': strcat (string, "FATAL:  ");   break;
      default: prterror ('f', internal_error);  /* slick recursive call */
   }

   strcat (string, format);      /* just append supplied format string */

	/* and print the whole thing */
#ifdef NEED_VPRINTF
	(void) zvfprintf(stdout, string, args);
#else
   (void) vprintf(string, args);
#endif
	fflush (stdout);

   if (level == 'f')       /* and abort on fatal error 'f' but not 'F' */
      zooexit (1);
}


#ifdef NEED_VPRINTF
/* Some systems don't have vprintf;  if so, we roll our own.  The following
has been adapted from a Usenet posting by Jef Poskanzer <jef@well.sf.ca.us>.

This is a portable mini-vfprintf that depends only on fprintf.

We don't call this routine vfprintf to avoid unexpected conflicts with any 
library routine of the same name, notwithstanding the fact that we will 
usually use it only when there is no conflict.  Also, even though we only 
need vprintf, the routine used here implements vfprintf.  This will allow 
future uses as needed when output is to be sent to a stream other than
stdout.  */

/* Whether to support double.  Better not to, because it may cause
math stuff to be linked in */

#undef NEED_DOUBLE

static int zvfprintf(stream, format, args)
FILE *stream;
char *format;
va_list args;
{
	char *ep;
	char fchar;
	char tformat[512];
	int do_long;		/* whether to print as long (l format suffix) */
	int do_star;		/* * used in format => get width from argument */
	int star_size;		/* size arg corresponding to "*" format */
	int i;
	long l;
	unsigned u;
	unsigned long ul;
	char *s;
#ifdef NEED_DOUBLE
	double d;
#endif

	while (*format != '\0') {
		if (*format != '%') { /* Not special, just write out the char. */
			putc(*format, stream);
			++format;
	   } else {
			do_star = 0;
			do_long = 0;
			ep = format + 1;

			/* Skip over all the field width and precision junk. */
			if (*ep == '-')
				++ep;
			if (*ep == '0')
				++ep;
			while (isdigit(*ep))
				++ep;
	      if (*ep == '.') {
				++ep;
				while (isdigit(*ep))
					++ep;
			}
			if (*ep == '#')
				++ep;
			if (*ep == '*') {
				do_star = 1;
				star_size = va_arg(args, int);	/* get * argument */
				++ep;
			}
			if (*ep == 'l') {
				do_long = 1;
				++ep;
			}

	      /* Here's the field type.  Extract it, and copy this format
	      ** specifier to a temp string so we can add an end-of-string.
	      */
	      fchar = *ep;
	      (void) strncpy(tformat, format, ep - format + 1);
	      tformat[ep - format + 1] = '\0';

	      /* Now do a one-argument printf with the format string we have 
			isolated.  If the * format was used, we will also supply the 
			additional parameter star_size, which we have already obtained 
			from the variable argument list.  */

	      switch (fchar) {
			 case 'd':
				if (do_long) {
					l = va_arg(args, long);
					if (do_star)
						(void) fprintf(stream, tformat, star_size, l);
					else
						(void) fprintf(stream, tformat, l);
				} else {
					i = va_arg(args, int);
					if (do_star)
						(void) fprintf(stream, tformat, star_size, i);
					else
						(void) fprintf(stream, tformat, i);
				}
				break;

	       case 'o':
	       case 'x':
	       case 'u':
		      if (do_long) {
					ul = va_arg(args, unsigned long);
					if (do_star)
						(void) fprintf(stream, tformat, star_size, ul);
					else
						(void) fprintf(stream, tformat, ul);
		      } else {
			      u = va_arg(args, unsigned);
					if (do_star)
						(void) fprintf(stream, tformat, star_size, u);
					else
						(void) fprintf(stream, tformat, u);
		      }
			   break;

			 case 'c':
				i = (char) va_arg(args, int);
				if (do_star)
					(void) fprintf(stream, tformat, star_size, i);
				else
					(void) fprintf(stream, tformat, i);
				break;

			 case 's':
				s = va_arg(args, char *);
				if (do_star)
					(void) fprintf(stream, tformat, star_size, s);
				else
					(void) fprintf(stream, tformat, s);
				break;

#ifdef NEED_DOUBLE
	       case 'e':
	       case 'f':
	       case 'g':
				d = va_arg(args, double);
				if (do_star)
					(void) fprintf(stream, tformat, star_size, d);
				else
					(void) fprintf(stream, tformat, d);
				break;
#endif

	       case '%':
				putc('%', stream);
				break;

			 default:
				return -1;
			}

			/* Resume formatting on the next character. */
			format = ep + 1;
		}
	}
	va_end(args);
	return 0;
}
#endif /*NEED_VPRINTF*/
