/*
 * get option letter from argument vector
 */
extern int
	optind,			/* index into parent argv vector */
	optopt;			/* character checked for validity */
extern char
	*optarg;		/* argument associated with option */

#define BADCH	((int)'?')
#define EMSG	""
#define tell(s)	{fputs(*nargv,stderr);fputs((s),stderr); \
		fputc(optopt,stderr);fputc('\n',stderr);return(BADCH);}

extern int getopt(), getarg();
