/* $Id: cproto.c 3.4 92/04/04 13:59:22 cthuang Exp $
 *
 * C function prototype generator and function definition converter
 */
#ifndef lint
static char rcsid[] = "$Id: cproto.c 3.4 92/04/04 13:59:22 cthuang Exp $";
#endif
#include <stdio.h>
#include <ctype.h>
#include "cproto.h"
#include "patchlev.h"

/* getopt declarations */
extern int getopt();
extern char *optarg;
extern int optind;

/* Name of the program */
char progname[] = "cproto";

/* Program options */

/* If TRUE, output "extern" before global declarations */
boolean extern_out = FALSE;

/* If TRUE, generate static declarations */
boolean static_out = FALSE;

/* If TRUE, generate variable declarations */
boolean variables_out = FALSE;

/* If TRUE, enable formal parameter promotion */
boolean promote_param = TRUE;

/* Style of function prototype to generate */
PrototypeStyle proto_style = PROTO_ANSI;

/* Function definition style converted to */
FuncDefStyle func_style = FUNC_NONE;

/* Name of macro to guard prototypes */
char *macro_name = "P_";

/* If TRUE, output prototype macro definition */
boolean define_macro = TRUE;

/* If TRUE, output comments in prototypes */
boolean proto_comments = TRUE;

/* Output formats for function declarators */
FuncFormat fmt[] = {
    /* miscellaneous function declarator */
    { "", " ", "", "", " ", "" },
    /* prototype */
    { "", " ", "", "", " ", "" },
    /* function definition */
    { "", "\n", " ", "", " ", "" },
    /* function definition with parameter comments */
    { "", "\n", " ", "\n    ", "\n    ", "\n" },
};

/* Include file directories */
#ifdef MSDOS
int num_inc_dir = 1;
char *inc_dir[MAX_INC_DIR] = { "" };
#else
int num_inc_dir = 2;
char *inc_dir[MAX_INC_DIR] = { "", "/usr/include" };
#endif

/* Run the C preprocessor */
#ifdef CPP
extern FILE *popen();
extern int pclose();
static char *cpp_cmd, *cmd;
#endif


/* Try to allocate some memory.
 * If unsuccessful, output an error message and exit.
 */
char *
xmalloc (n)
unsigned n;
{
    char *p;

    if ((p = malloc(n)) == NULL) {
	fprintf(stderr, "%s: out of memory\n", progname);
	exit(1);
    }
    return p;
}

/* Copy the string into allocated memory.
 * If unsuccessful, output an error message and exit.
 */
char *
xstrdup (src)
char *src;
{
    return strcpy(xmalloc(strlen(src)+1), src);
}

/* Output the current source file name and line number.
 */
void
put_error ()
{
    fprintf(stderr, "\"%s\", line %d: ", cur_file_name(), cur_line_num());
}

/* Scan for options from a string.
 */
static void
parse_options (src, maxargc, pargc, argv)
char *src;
int maxargc, *pargc;
char **argv;
{
    char *g, *p, c;
    int argc;

    argc = 0;
    g = xstrdup(src);
    c = *g;
    while (c != '\0' && argc < maxargc) {
	while (c == ' ' || c == '\t')
	    c = *++g;
	if (c == '\0')
	    break;
	argv[argc++] = g;

	p = g;
	while (1) {
	    if (c == ' ' || c == '\t' || c == '\0') {
		*p = '\0';
		break;
	    } else if (c == '"') {
		while (1) {
		    c = *++g;
		    if (c == '"') {
			c = *++g;
			break;
		    } else if (c == '\0') {
			break;
		    } else {
			*p++ = c;
		    }
		}
	    } else {
		*p++ = c;
		c = *++g;
	    }
	}
	if (c != '\0')
	    c = *++g;
    }

    *pargc = argc;
}

/* Replace any character escape sequences in a string with the actual
 * characters.	Return a pointer to malloc'ed memory containing the result.
 * This function knows only a few escape sequences.
 */
static char *
escape_string (src)
char *src;
{
    char *result, *get, *put;

    result = xstrdup(src);
    put = result;
    get = src;
    while (*get != '\0') {
	if (*get == '\\') {
	    switch (*(++get)) {
	    case 'n':
		*put++ = '\n';
		++get;
		break;
	    case 't':
		*put++ = '\t';
		++get;
		break;
	    default:
		if (*get != '\0')
		    *put++ = *get++;
	    }
	} else {
	    *put++ = *get++;
	}
    }
    *put = *get;
    return result;
}

/* Trim any path name separator from the end of the string.
 * Return a pointer to the string.
 */
char *
trim_path_sep (s)
char *s;
{
    char ch;
    int n;

    n = strlen(s);
    if (n > 0) {
	ch = s[n-1];
	if (ch == '/' || ch == '\\')
	    s[n-1] = '\0';
    }
    return s;
}

/* Output usage message and exit.
 */
static void
usage ()
{
    fprintf(stderr, "usage: %s [ option ... ] [ file ... ]\n", progname);
    fputs("Options:\n", stderr);
    fputs("  -a       Convert function definitions to ANSI style\n", stderr);
    fputs("  -c       Omit comments in generated prototypes\n", stderr);
    fputs("  -e       Output \"extern\" keyword before global declarations\n",
	stderr);
    fputs("  -f n     Set function prototype style (0 to 4)\n", stderr);
    fputs("  -p       Disable formal parameter promotion\n", stderr);
    fputs("  -s       Output static declarations\n", stderr);
    fputs("  -t       Convert function definitions to traditional style\n",
	stderr);
    fputs("  -v       Output variable declarations\n", stderr);
    fputs("  -m name  Set name of prototype macro\n", stderr);
    fputs("  -d       Omit prototype macro definition\n", stderr);
    fputs("  -P fmt   Set prototype format template \"int main (a, b)\"\n",
	stderr);
    fputs("  -F fmt   Set function definition format template \"int main (a, b)\"\n",
	stderr);
    fputs("  -C fmt   Set format for function definition with parameter comments\n",
	stderr);
    fputs("  -V       Print version information\n", stderr);
    fputs("  -D name[=value]\n", stderr);
    fputs("  -U name\n", stderr);
    fputs("  -I directory\n", stderr);
    fputs("           Set C preprocessor options\n", stderr);
    exit(1);
}

#define MAX_OPTIONS 40

/* Process the command line options.
 */
static void
process_options (pargc, pargv)
int *pargc;
char ***pargv;
{
    int argc, eargc, nargc;
    char **argv, *eargv[MAX_OPTIONS], **nargv;
    int i, c;
    char *s;
#ifdef CPP
    unsigned n;
    char tmp[MAX_TEXT_SIZE];
#endif

    argc = *pargc;
    argv = *pargv;
    if ((s = getenv("CPROTO")) != NULL) {
	parse_options(s, MAX_OPTIONS, &eargc, eargv);
	nargv = (char **)xmalloc((eargc+argc+1)*sizeof(char *));
	nargv[0] = argv[0];
	nargc = 1;
	for (i = 0; i < eargc; ++i)
	    nargv[nargc++] = eargv[i];
	for (i = 1; i < argc; ++i)
	    nargv[nargc++] = argv[i];
	nargv[nargc] = NULL;
	argc = nargc;
	argv = nargv;
    }

#ifdef CPP
    /* Allocate buffer for C preprocessor command line. */
    n = strlen(CPP) + 1;
    for (i = 0; i < argc; ++i) {
	n += strlen(argv[i]) + 1;
    }
    cpp_cmd = xmalloc(n);
    strcpy(cpp_cmd, CPP);
    cmd = xmalloc(n);
#endif

    while ((c = getopt(argc, argv, "aC:cD:deF:f:I:m:P:pstU:Vv")) != EOF) {
	switch (c) {
	case 'I':
	    if (num_inc_dir < MAX_INC_DIR) {
		inc_dir[num_inc_dir++] = trim_path_sep(xstrdup(optarg));
	    } else {
		fprintf(stderr, "%s: too many include directories\n",
		    progname);
	    }
	case 'D':
	case 'U':
#ifdef CPP
	    sprintf(tmp, " -%c%s", c, optarg);
	    strcat(cpp_cmd, tmp);
#endif
	    break;
	case 'a':
	    func_style = FUNC_ANSI;
	    break;
	case 'c':
	    proto_comments = FALSE;
	    break;
	case 'd':
	    define_macro = FALSE;
	    break;
	case 'e':
	    extern_out = TRUE;
	    break;
	case 'C':
	case 'F':
	case 'P':
	    s = escape_string(optarg);
	    i = (c == 'C') ? FMT_FUNC_COMMENT :
		((c == 'F') ? FMT_FUNC : FMT_PROTO);

	    fmt[i].decl_spec_prefix = s;
	    while (*s != '\0' && isascii(*s) && !isalnum(*s)) ++s;
	    if (*s == '\0') usage();
	    *s++ = '\0';
	    while (*s != '\0' && isascii(*s) && isalnum(*s)) ++s;
	    if (*s == '\0') usage();

	    fmt[i].declarator_prefix = s;
	    while (*s != '\0' && isascii(*s) && !isalnum(*s)) ++s;
	    if (*s == '\0') usage();
	    *s++ = '\0';
	    while (*s != '\0' && isascii(*s) && isalnum(*s)) ++s;
	    if (*s == '\0') usage();

	    fmt[i].declarator_suffix = s;
	    while (*s != '\0' && *s != '(') ++s;
	    if (*s == '\0') usage();
	    *s++ = '\0';

	    fmt[i].first_param_prefix = s;
	    while (*s != '\0' && isascii(*s) && !isalnum(*s)) ++s;
	    if (*s == '\0') usage();
	    *s++ = '\0';
	    while (*s != '\0' && *s != ',') ++s;
	    if (*s == '\0') usage();

	    fmt[i].middle_param_prefix = ++s;
	    while (*s != '\0' && isascii(*s) && !isalnum(*s)) ++s;
	    if (*s == '\0') usage();
	    *s++ = '\0';
	    while (*s != '\0' && isascii(*s) && isalnum(*s)) ++s;
	    if (*s == '\0') usage();

	    fmt[i].last_param_suffix = s;
	    while (*s != '\0' && *s != ')') ++s;
	    *s = '\0';

	    break;
	case 'f':
	    proto_style = atoi(optarg);
	    if (proto_style < 0 || proto_style > PROTO_MACRO)
		usage();
	    break;
	case 'm':
	    macro_name = optarg;
	    break;
	case 'p':
	    promote_param = FALSE;
	    break;
	case 's':
	    static_out = TRUE;
	    break;
	case 't':
	    func_style = FUNC_TRADITIONAL;
	    break;
	case 'V':
	    fprintf(stderr, "%s patchlevel %d\n", rcsid, PATCHLEVEL);
	    break;
	case 'v':
	    variables_out = TRUE;
	    break;
	default:
	    usage();
	}
    }

    *pargc = argc;
    *pargv = argv;
}

int
main (argc, argv)
int argc;
char **argv;
{
    int i;
    FILE *inf;

    process_options(&argc, &argv);

    if (proto_style == PROTO_MACRO && define_macro) {
	printf("#if defined(__STDC__) || defined(__cplusplus)\n");
	printf("#define %s(s) s\n", macro_name);
	printf("#else\n");
	printf("#define %s(s) ()\n", macro_name);
	printf("#endif\n\n");
    }

    init_parser();
    if (optind == argc) {
	if (func_style != FUNC_NONE) {
	    proto_style = PROTO_NONE;
	    variables_out = FALSE;
	    proto_comments = FALSE;
	}
	process_file(stdin, "stdin");
	pop_file();
    } else {
	for (i = optind; i < argc; ++i) {
#ifdef CPP
	    if (func_style == FUNC_NONE) {
		sprintf(cmd, "%s %s", cpp_cmd, argv[i]);
		if ((inf = popen(cmd, "r")) == NULL) {
		    fprintf(stderr, "%s: error running cpp\n", progname);
		    continue;
		}
	    } else {
		if ((inf = fopen(argv[i], "r")) == NULL) {
		    fprintf(stderr, "%s: cannot read file %s\n", progname,
			argv[i]);
		    continue;
		}
	    }
#else
	    if ((inf = fopen(argv[i], "r")) == NULL) {
		fprintf(stderr, "%s: cannot read file %s\n", progname, argv[i]);
		continue;
	    }
#endif
	    process_file(inf, argv[i]);
#ifdef CPP
	    if (func_style == FUNC_NONE) {
		pclose(inf);
	    } else {
		pop_file();
	    }
#else
	    pop_file();
#endif
	}
    }

    if (proto_style == PROTO_MACRO && define_macro) {
	printf("\n#undef %s\n", macro_name);
    }

    return 0;
}
