/***********************************************************
	append.c -- append to archive
***********************************************************/
#include "lharc.h"
#include "intrface.h"

#ifdef NEED_INCREMENTAL_INDICATOR
#define MAX_INDICATOR_COUNT	64
long		indicator_count;
long		indicator_threshold;
#endif

extern int quiet;
extern int compress_method;
extern long int reading_size;
extern unsigned short dicbit;

struct interfacing interface;

int encode_lzhuf (infp, outfp, size, original_size_var, packed_size_var,
		  name,hdr_method)
	FILE *infp;
	FILE *outfp;
	long size;
	long *original_size_var;
	long *packed_size_var;
	char *name;
	char *hdr_method;
{
	static int method = -1;

	if (method < 0) {
		method = compress_method;
		if (method > 0) method = encode_alloc(method);
	}

	interface.method = method;

	if (interface.method > 0) {
		interface.infile = infp;
		interface.outfile = outfp;
		interface.original = size;
		start_indicator (name, size, "Freezing",1<<dicbit);
		encode(&interface);
		*packed_size_var = interface.packed;
		*original_size_var = interface.original;
	} else {
	  copyfile(infp, outfp, size, 1);
	  *packed_size_var = *original_size_var = size;
	}
	bcopy ("-lh -", hdr_method, 5);
	hdr_method[3] = interface.method + '0';

	finish_indicator2 (name, "Frozen",
			   (int)((*packed_size_var * 100L) / *original_size_var ));
	return crc;
}

void
start_indicator (name, size, msg, def_indicator_threshold)
char *name;
long size;
char *msg;
long def_indicator_threshold;
{
#ifdef NEED_INCREMENTAL_INDICATOR
	long	i;
	int	m;
#endif

	if (quiet)
		return;

#ifdef NEED_INCREMENTAL_INDICATOR
	m = MAX_INDICATOR_COUNT - strlen (name);
	if (m < 0)
		m = 3;		/* (^_^) */
	printf ("\r%s\t- %s :  ", name, msg);
	indicator_threshold =
		((size  + (m * def_indicator_threshold - 1)) /
		 (m * def_indicator_threshold) *
		 def_indicator_threshold);
	if ( indicator_threshold )
		i = ((size + (indicator_threshold - 1)) / indicator_threshold);
	else
		i = 0;
	while (i--)
		putchar ('.');
	indicator_count = 0;
	printf ("\r%s\t- %s :  ", name, msg);
#else
	printf ("%s\t- ", name);
#endif
	fflush (stdout);
	reading_size = 0L;
}

void
finish_indicator2 (name, msg, pcnt)
	char *name;
	char *msg;
	int pcnt;
{
	if (quiet)
		return;

	if (pcnt > 100) pcnt = 100;	/* (^_^) */
#ifdef NEED_INCREMENTAL_INDICATOR
	printf ("\r%s\t- %s(%d%%)\n", name, msg, pcnt);
#else
	printf ("%s\n", msg);
#endif
	fflush (stdout);
}

void
finish_indicator (name, msg)
	char *name;
	char *msg;
{
	if (quiet)
		return;

#ifdef NEED_INCREMENTAL_INDICATOR
	printf ("\r%s\t- %s\n", name, msg);
#else
	printf ("%s\n", msg);
#endif
	fflush (stdout);
}
