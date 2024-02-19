/* Copyright (C) 1989, 1992 Aladdin Enterprises.  All rights reserved.
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

/* gs.c */
/* Driver program for Ghostscript */
/* We have to include std.h before any file that includes <sys/types.h>. */
#include "std.h"
#include <ctype.h>
#include "memory_.h"
#include "string_.h"
/* Capture stdin/out/err before gs.h redefines them. */
#include <stdio.h>
static FILE *real_stdin, *real_stdout, *real_stderr;
static void
get_real()
{	real_stdin = stdin, real_stdout = stdout, real_stderr = stderr;
}
/* Define PROGRAM_NAME before we include std.h */
#ifndef PROGRAM_NAME
#  define PROGRAM_NAME "Ghostscript"
#endif
#include "ghost.h"
#include "gxdevice.h"
#include "gxdevmem.h"
#include "alloc.h"
#include "errors.h"
#include "estack.h"
#include "main.h"
#include "ostack.h"
#include "store.h"
#include "stream.h"
  
#ifndef GS_LIB
#  define GS_LIB "GS_LIB"
#endif

/* Library routines not declared in a standard header */
extern char *getenv(P1(const char *));
/* Note: sscanf incorrectly defines its first argument as char * */
/* rather than const char *.  This accounts for the ugly casts below. */

/* Device procedures imported from gsdevice.c. */
extern gx_device *gs_getdevice(P1(int));
extern char *gs_devicename(P1(gx_device *));

/* Other imported data */
extern int gs_alloc_debug;
extern int gs_log_errors;
extern gx_device *gx_device_list[];
extern const int gs_revision;
extern const char *gs_revisiondate;

/*
 * Help strings.  We have to break them up into parts, because
 * the Watcom compiler has a limit of 510 characters for a single token.
 * For PC displays, we want to limit the strings to 24 lines.
 */
private const char *gs_help1 = "\
Usage: gs [switches] [file1.ps file2.ps ...]\n\
  or : gs [switches] [file1.ps ...] -- filen.ps arg1 arg2 ...\n\
The latter passes arg1 ... to the program in filen.ps.\n\
Available devices:\n   ";
/* We have to break help2 up into parts, because  */
private const char *gs_help2a = "\n\
Switches: (you can use # in place of =)\n\
    @<file>              treat file like part of the command line\n\
                           (to get around DOS command line limit)\n\
    -d<name>[=<token>]   define name as token, or null if no token given\n\
    -f<file>             read this file even if its name begins with - or @\n";
private const char *gs_help2b = "\
    -g<width>x<height>   set width and height (`geometry'), in pixels\n\
    -I<prefix>           add prefix to search path\n\
    -q                   `quiet' mode, suppress most messages\n\
    -r<res>              set resolution, in pixels per inch\n";
private const char *gs_help2c = "\
    -s<name>=<string>    define name as string\n\
    -sDEVICE=<devname>   select initial device\n\
    -sOutputFile=<file>  select output file: embed %d for page #,\n\
                           - means stdout, use |command to pipe\n\
`-' alone as a file name means read from stdin non-interactively.\n\
For more information, please read the use.doc file.\n";

/* Forward references */
private int swproc(P1(const char *));
private void argproc(P1(const char *));
private void cmdproc(P1(const char *));
private void print_revision(P0());
private int esc_strlen(P1(const char *));
private void esc_strcat(P2(char *, const char *));
private void runarg(P4(const char **, const char *, const char *, int));
private void run_string(P1(const char *));

/* Parameters set by swproc */
private int quiet;
private int batch;

main(int argc, const char *argv[])
{	int argi;
	get_real();
	gs_init0(real_stdin, real_stdout, real_stderr, argc);
	   {	char *lib = getenv(GS_LIB);
		if ( lib != 0 ) 
		   {	int len = strlen(lib);
			gs_lib_env_path = gs_malloc(len + 1, 1, "GS_LIB");
			strcpy(gs_lib_env_path, lib);
		   }
	   }
	/* Execute files named in the command line, */
	/* processing options along the way. */
	/* Wait until the first file name (or the end */
	/* of the line) to finish initialization. */
	batch = 0;
	quiet = 0;
	/* If debugging is enabled, trace the device calls. */
#ifdef DEBUG
	   {	extern gx_device *gs_trace_device(P1(gx_device *));
		extern const gx_device_memory
			mem_mono_device, mem_mapped2_color_device,
			mem_mapped4_color_device, mem_mapped8_color_device,
			mem_true16_color_device,
			mem_true24_color_device, mem_true32_color_device;
		static const gx_device_memory *mdevs[8] =
		   {	&mem_mono_device, &mem_mapped2_color_device,
			&mem_mapped4_color_device, &mem_mapped8_color_device,
			&mem_true16_color_device,
			&mem_true24_color_device, &mem_true32_color_device,
			0
		   };
		gx_device **pdevs[3];
		gx_device ***ppdev;
		gx_device **pdev;
		pdevs[0] = gx_device_list;
		pdevs[1] = (gx_device **)mdevs;
		pdevs[2] = 0;
		for ( ppdev = pdevs; *ppdev != 0; ppdev++ )
		 for ( pdev = *ppdev; *pdev != 0; pdev++ )
		   {
/******
			gx_device *tdev = gs_trace_device(*pdev);
			if ( tdev == 0 )
			   {	lprintf("Can't allocate traced device!\n");
				gs_exit(1);
			   }
			*pdev = tdev;
 ******/
		   }
	   }
#endif
	for ( argi = 1; argi < argc; argi++ )
	   {	const char **argp = &argv[argi];
		const char *arg = *argp;
		switch ( *arg )
		{
		case '@':
			cmdproc(arg);
			break;
		case '-':
			if ( !strcmp(arg, "--") || !strcmp(arg, "-+") )
			{	/* run with command line args */
				int nstrs = argc - argi - 2;
				if ( nstrs < 0 )	/* no file to run! */
				{	puts("Usage: gs ... -- file.ps arg1 ... argn");
					gs_exit(1);
				}
				runarg(argp + 1, "{userdict /ARGUMENTS [", "] put (", nstrs);
				gs_exit(0);
			}
			else
			{	if ( swproc(arg) < 0 )
				  fprintf(stdout, "Unknown switch %s - ignoring\n", arg);
			}
			break;
		default:
			argproc(arg);
		}
	   }
	gs_init2();
	if ( !batch ) run_string("start");
	gs_exit(0);
}

/* Process switches */
private int
swproc(const char *arg)
{	char sw = arg[1];
	arg += 2;		/* skip - and letter */
	switch ( sw )
	   {
	default:
		return -1;
	case 0:				/* read stdin as a file */
		batch = 1;
		/* Set NOPAUSE so showpage won't try to read from stdin. */
		swproc("-dNOPAUSE");
		gs_init2();		/* Finish initialization */
		run_string("(%stdin) (r) file cvx execute0");
		break;
	case 'A':			/* trace allocator */
		gs_alloc_debug = 1; break;
	case 'e':			/* log errors */
		gs_log_errors = 1; break;
	case 'E':			/* log errors */
		gs_log_errors = 2; break;
	case 'f':			/* run file of arbitrary name */
		argproc(arg); break;
	case 'h':			/* print help */
	case '?':			/* ditto */
		print_revision();
		fputs(gs_help1, stdout);
		   {	int i;
			gx_device *pdev;
			for ( i = 0; (pdev = gs_getdevice(i)) != 0; i++ )
				fprintf(stdout, " %s", gs_devicename(pdev));
		   }
		fputs(gs_help2a, stdout);
		fputs(gs_help2b, stdout);
		fputs(gs_help2c, stdout);
		gs_exit(0);
	case 'I':			/* specify search path */
		gs_add_lib_path(arg);
		break;
	case 'q':			/* quiet startup */
	   {	ref vtrue;
		quiet = 1;
		gs_init1();
		make_true(&vtrue);
		initial_enter_name("QUIET", &vtrue);
	   }	break;
	case 'D':			/* define name */
	case 'd':
	case 'S':			/* define name as string */
	case 's':
	   {	char *eqp = strchr(arg, '=');
		int isd = (sw == 'D' || sw == 'd');
		ref value;
		if ( eqp == NULL ) eqp = strchr(arg, '#');
		/* Initialize the object memory, scanner, and */
		/* name table now if needed. */
		gs_init1();
		if ( eqp == arg )
		   {	puts("Usage: -dname, -dname=token, -sname=string");
			gs_exit(1);
		   }
		if ( eqp == NULL )
		   {	if ( isd ) make_null(&value);
			else make_tasv(&value, t_string, a_readonly,
				       0, bytes, (byte *)"");
		   }
		else
		   {	int code;
			*eqp++ = 0;	/* delimit name */
			if ( isd )
			   {	stream astream;
				sread_string(&astream,
					     (byte *)eqp, strlen(eqp));
				code = scan_token(&astream, 0, &value);
				if ( code )
				   {	puts("-dname= must be followed by a valid token");
					gs_exit(1);
				   }
			   }
			else
			   {	int len = strlen(eqp);
				char *str = gs_malloc((uint)len, 1, "-s");
				if ( str == 0 )
				   {	lprintf("Out of memory!\n");
					gs_exit(1);
				   }
				memcpy(str, eqp, len);
				make_tasv(&value, t_string, a_readonly,
					  len, bytes, (byte *)str);
			   }
		   }
		/* Enter the name in systemdict */
		initial_enter_name(arg, &value);
		break;
	   }
	case 'g':			/* define device geometry */
	   {	long width, height;
		ref value;
		gs_init1();
		if ( sscanf((char *)arg, "%ldx%ld", &width, &height) != 2 )
		   {	puts("-g must be followed by <width>x<height>");
			gs_exit(1);
		   }
		make_int(&value, width);
		initial_enter_name("DEVICEWIDTH", &value);
		make_int(&value, height);
		initial_enter_name("DEVICEHEIGHT", &value);
		break;
	   }
	case 'M':			/* set memory allocation increment */
	   {	unsigned msize = 0;
		sscanf((char *)arg, "%d", &msize);
		if ( msize <= 0 || msize >= 64 )
		   {	puts("-M must be between 1 and 63");
			gs_exit(1);
		   }
		gs_memory_chunk_size = msize << 10;
	   }
		break;
	case 'r':			/* define device resolution */
	   {	float xres, yres;
		ref value;
		gs_init1();
		switch ( sscanf((char *)arg, "%fx%f", &xres, &yres) )
		   {
		default:
			puts("-r must be followed by <res> or <xres>x<yres>");
			gs_exit(1);
		case 1:			/* -r<res> */
			yres = xres;
		case 2:			/* -r<xres>x<yres> */
			make_real(&value, xres);
			initial_enter_name("DEVICEXRESOLUTION", &value);
			make_real(&value, yres);
			initial_enter_name("DEVICEYRESOLUTION", &value);
		   }
		break;
	   }
	case 'v':			/* print revision */
		print_revision();
		gs_exit(0);
	case 'Z':
		if ( !*arg )
		   {	/* No options, set all flags */
			memset(gs_debug, 0xff, 128);
		   }
		else
		   {	while ( *arg )
				gs_debug[*arg++ & 127] = 0xff;
		   }
		break;
	   }
	return 0;
}

/* Define versions of strlen and strcat that insert \ escapes */
/* before \, (, and ). */
#define needs_esc(ch) ((ch) == '(' || (ch) == ')' || (ch) == '\\')
private int
esc_strlen(const char *str)
{	int n = strlen(str);
	const char *p;
	for ( p = str; *p; p++ ) if ( needs_esc(*p) ) n++;
	return n;
}
private void
esc_strcat(char *dest, const char *src)
{	char *d = dest + strlen(dest);
	const char *p;
	for ( p = src; *p; p++ )
	   {	if ( needs_esc(*p) ) *d++ = '\\';
		*d++ = *p;
	   }
	*d = 0;
}

/* Process file names */
private void
argproc(const char *arg)
{	runarg(&arg, "{", "(", 0);
}
private void
runarg(const char **argp, const char *pre, const char *post, int nstrs)
{	const char *arg = *argp;
	static const char *pex = ")run}execute";
	int len = strlen(pre) + esc_strlen(arg) + nstrs * 2 + strlen(post) + strlen(pex) + 1;
	char *line;
	int i;
	for ( i = 1; i <= nstrs; i++ )
		len += esc_strlen(argp[i]) + 2;
	gs_init2();	/* Finish initialization */
	line = gs_malloc(len, 1, "argproc");
	if ( line == 0 )
	{	lprintf("Out of memory!\n");
		gs_exit(1);
	}
	strcpy(line, pre);
	for ( i = 1; i <= nstrs; i++ )
	   {	strcat(line, "(");
		esc_strcat(line, argp[i]);
		strcat(line, ")");
	   }
	strcat(line, post);
	esc_strcat(line, arg);
	strcat(line, pex);
	run_string(line);
}
private void
run_string(const char *str)
{	int exit_code;
	ref error_object;
	int code = gs_run_string(str, gs_user_errors, &exit_code, &error_object);
	zflush((ref *)0);	/* flush stdout */
	zflushpage((ref *)0); /* force display update */
	switch ( code )
	{
	case 0:
		break;
	case e_Quit:
		gs_exit(0);
	case e_Fatal:
		eprintf1("Unrecoverable error, exit code %d\n", exit_code);
		gs_exit(exit_code);
	default:
		gs_debug_dump_stack(code, &error_object);
		gs_exit(255);
	}
}

/* Process command line indirection. */
private void
cmdproc(const char *arg)
{	FILE *f;
#define cstr_max 128
	char cstr[cstr_max + 1];
	arg++;		/* skip over @ */
	f = fopen(arg, "r");
	if ( f == NULL )
	{	fprintf(stdout, "Unable to open command line file %s\n", arg);
		gs_exit(1);
	}
	while ( 1 )
	{	register int c;
		register int i;
		while ( isspace(c = fgetc(f)) ) ;
		if ( c == EOF ) break;
		for ( i = 0; ; )
		{	if ( i == cstr_max )
			{	cstr[i] = 0;
				fprintf(stdout, "Command too long: %s\n", cstr);
				gs_exit(1);
			}
			cstr[i++] = c;
			c = fgetc(f);
			if ( c == EOF || isspace(c) )
				break;
		}
		cstr[i] = 0;
		switch ( cstr[0] )
		{
		case '@':
			cmdproc(cstr);
			break;
		case '-':
		{	/* swproc wants strings to be in the heap! */
			char *sstr = gs_malloc(i + 1, 1, "cmdproc");
			if ( sstr == 0 )
			{	lprintf("Out of memory!\n");
				gs_exit(1);
			}
			strcpy(sstr, cstr);
			if ( swproc(sstr) < 0 )
			  fprintf(stdout, "Unknown switch %s - ignoring\n", sstr);
		}
			break;
		default:
			argproc(cstr);
		}
	}
	fclose(f);
}

/* Print the revision and revision date. */
private void
print_revision()
{	int sub_rev = gs_revision % 10;
	fprintf(stdout, "%s %d.%d", PROGRAM_NAME, gs_revision / 100,
		(gs_revision / 10) % 10);
	if ( sub_rev != 0 ) fprintf(stdout, ".%d", sub_rev);
	fprintf(stdout, " (%s)\n", gs_revisiondate);
}
