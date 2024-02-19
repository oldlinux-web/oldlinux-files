/*
 *   bin86/bcc/bcc.c
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* bcc.c - driver for Bruce's C compiler and for CvW's C compiler */

#define _POSIX_SOURCE 1

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#define FALSE	0
#define FORWARD	static
#ifndef NULL
#define NULL	0
#endif
#define PRIVATE	static
#define PUBLIC
#define TRUE	1

#if __STDC__
#define P(x)	x
#else
#define P(x)	()
#endif

#define AS	"as"
#define BAS86
#define BCC86
#define CC1	"cc1"
#define CC1_MINUS_O_BROKEN	FALSE
#define CPP	"cc1"
#define CPPFLAGS	"-E"
#define CRTSO	"crtso.o"
#define GCC	"/usr/bin/gcc"
#define LD	"ld"
#define STANDARD_CRTSO0_PREFIX	"/usr/local/lib/i86/"
#define STANDARD_CRTSO3_PREFIX	"/usr/local/lib/i386/"
#define STANDARD_EXEC_PREFIX	"/usr/local/libexec/i386/bcc/"

#ifdef CCC
#undef BCC86
#undef CC1
#define CC1	"c386"
#undef CC1_MINUS_O_BROKEN
#define CC1_MINUS_O_BROKEN	TRUE
#undef CPP
#define CPP	"cpp"		/* could also use bcc-cc1 -E */
#undef CPPFLAGS
#define CPPFLAGS	"-traditional"
#undef GCC
#undef STANDARD_CRTSO0_PREFIX
#undef STANDARD_CRTSO3_PREFIX
#define STANDARD_CRTSO_PREFIX	"/usr/local/lib/i386/"
#define STANDARD_EXEC_PREFIX_2	"/usr/local/bin/"
#endif /* CCC */

#ifdef MC6809
#undef BAS86
#undef BCC86
#undef CRTSO
#undef GCC
#undef STANDARD_CRTSO0_PREFIX
#undef STANDARD_CRTSO3_PREFIX
#undef STANDARD_EXEC_PREFIX
#define STANDARD_EXEC_PREFIX	"/usr/local/libexec/m09/bcc/"
#endif /* MC6809 */

#define ALLOC_UNIT	16	/* allocation unit for arg arrays */
#define DIRCHAR	'/'
#define START_ARGS	4	/* number of reserved args */

typedef unsigned char bool_T;	/* boolean: TRUE if nonzero */

struct arg_s
{
    char *prog;
    bool_T minus_O_broken;
    int argc;
    char **argv;
    unsigned nr_allocated;
};

struct prefix_s
{
    char *name;
    struct prefix_s *next;
};

PRIVATE struct arg_s asargs = { AS, };
PRIVATE struct arg_s ccargs = { CC1, CC1_MINUS_O_BROKEN, };
PRIVATE struct arg_s cppargs = { CPP, };
#ifdef STANDARD_CRTSO_PREFIX
PRIVATE struct prefix_s crtso_prefix = { STANDARD_CRTSO_PREFIX, };
#endif
#ifdef STANDARD_CRTSO0_PREFIX
PRIVATE struct prefix_s crtso0_prefix = { STANDARD_CRTSO0_PREFIX, };
#endif
#ifdef STANDARD_CRTSO3_PREFIX
PRIVATE struct prefix_s crtso3_prefix = { STANDARD_CRTSO3_PREFIX, };
#endif
PRIVATE struct prefix_s exec_prefix;
PRIVATE struct arg_s ldargs = { LD, };
#ifdef BAS86
PRIVATE struct arg_s ldrargs = { LD, };
#endif
PRIVATE char *progname;
PRIVATE bool_T runerror;	/* = FALSE */
PRIVATE struct arg_s tmpargs;	/* = empty */
PRIVATE char *tmpdir;
PRIVATE bool_T verbose;		/* = FALSE */

/* Who can say if the standard headers declared these? */
int chmod P((const char *name, int mode));
int execv P((char *name, char **argv));
void exit P((int status));
pid_t fork P((void));
char *getenv P((const char *_name));
pid_t getpid P((void));
void *malloc P((unsigned size));
void *realloc P((void *ptr, unsigned size));
void (*signal P((int sig, void (*func) P((int sig))))) P((int sig));
char *strcpy P((char *target, const char *source));
size_t strlen P((const char *s));
char *strrchr P((const char *s, int c));
int unlink P((const char *name));
pid_t wait P((int *status));
int write P((int fd, char *buf, unsigned nbytes));

FORWARD void addarg P((struct arg_s *argp, char *arg));
FORWARD void addprefix P((struct prefix_s *prefix, char *name));
FORWARD void fatal P((char *message));
FORWARD char *fixpath P((char *path, struct prefix_s *prefix, int mode));
FORWARD void killtemps P((void));
FORWARD void *my_malloc P((unsigned size, char *where));
FORWARD char *my_mktemp P((void));
FORWARD void my_unlink P((char *name));
FORWARD void outofmemory P((char *where));
FORWARD int run P((char *in_name, char *out_name, struct arg_s *argp));
FORWARD void set_trap P((void));
FORWARD void show_who P((char *message));
FORWARD void startarg P((struct arg_s *argp));
FORWARD char *stralloc P((char *s));
FORWARD char *stralloc2 P((char *s1, char *s2));
FORWARD void trap P((int signum));
FORWARD void unsupported P((char *option, char *message));
FORWARD void writen P((void));
FORWARD void writes P((char *s));
FORWARD void writesn P((char *s));

PUBLIC int main(argc, argv)
int argc;
char **argv;
{
    char *arg;
    int argcount = argc;
    bool_T *argdone = my_malloc((unsigned) argc * sizeof *argdone, "argdone");
    bool_T as_only = FALSE;
    char *basename;
#ifdef BCC86
    bool_T bits32 = sizeof (char *) >= 4;
    char *bits_arg;
#endif
    bool_T cc_only = FALSE;
#ifdef CCC
    bool_T cpp_pass = TRUE;
#else
    bool_T cpp_pass = FALSE;
#endif
#ifdef BCC86
    char *crtso;
#endif
    bool_T debug = FALSE;
    bool_T echo = FALSE;
    unsigned errcount = 0;
    char ext;
    char *f_out = NULL;
    bool_T float_emulation = FALSE;
#ifdef BAS86
    bool_T gnu_objects = FALSE;
#endif
    char *in_name;
    int length;
    unsigned ncisfiles = 0;
    unsigned nifiles = 0;
    unsigned npass_specs;
    bool_T optimize = FALSE;
    char *out_name;
    bool_T profile = FALSE;
    bool_T prep_only = FALSE;
    bool_T prep_line_numbers = FALSE;
    int status;
    char *temp;

    progname = argv[0];
    addarg(&cppargs, CPPFLAGS);
#ifdef CCC
    addarg(&asargs, "-j");
#endif
    addarg(&asargs, "-u");
    addarg(&asargs, "-w");
#ifdef BCC86
    addarg(&ldargs, "-i");
#endif
#ifdef BAS86
    addarg(&ldrargs, "-r");
#endif

    /* pass 1 over argv to gather compile options */
    for (; --argc != 0;)
    {
	arg = *++argv;
	*++argdone = TRUE;
	if (arg[0] == '-' && arg[1] != 0 && arg[2] == 0)
	    switch (arg[1])
	    {
#ifdef BCC86
	    case '0':
		bits32 = FALSE;
		break;
	    case '3':
		bits32 = TRUE;
		break;
#endif
	    case 'E':
		prep_only = prep_line_numbers = cpp_pass = TRUE;
		break;
#ifdef BAS86
	    case 'G':
		gnu_objects = TRUE;
		break;
#endif
	    case 'P':
		prep_only = cpp_pass = TRUE;
		prep_line_numbers = FALSE;
		break;
	    case 'O':
		optimize = TRUE;	/* unsupported( arg, "optimize" ); */
		break;
	    case 'S':
		cc_only = TRUE;
		break;
	    case 'V':
		echo = TRUE;
		break;
	    case 'c':
		as_only = TRUE;
		break;
	    case 'e':
		cpp_pass = TRUE;
		break;
	    case 'f':
		float_emulation = TRUE;
		++errcount;
		unsupported(arg, "float emulation");
		break;
	    case 'g':
		debug = TRUE;	/* unsupported( arg, "debug" ); */
		break;
	    case 'o':
		if (--argc < 1)
		{
		    ++errcount;
		    show_who("output file missing after -o\n");
		}
		else
		{
		    if (f_out != NULL)
			show_who("more than one output file\n");
		    f_out = *++argv;
		    *++argdone = TRUE;
		}
		break;
	    case 'p':
		profile = TRUE;
		++errcount;
		unsupported(arg, "profile");
		break;
	    case 'v':
		verbose = TRUE;
		break;
	    default:
		*argdone = FALSE;
		break;
	    }
	else if (arg[0] == '-')
	    switch (arg[1])
	    {
	    case 'A':
		addarg(&asargs, arg + 2);
		break;
	    case 'B':
		addprefix(&exec_prefix, arg + 2);
		break;
	    case 'C':
		addarg(&ccargs, arg + 2);
		break;
	    case 'D':
	    case 'I':
	    case 'U':
#ifndef CCC
		addarg(&ccargs, arg);
#endif
		addarg(&cppargs, arg);
		break;
	    case 'L':
		addarg(&ldargs, arg + 2);
		break;
	    case 'P':
		addarg(&cppargs, arg + 2);
		break;
#ifdef CCC
	    case 'Q':
		addarg(&ccargs, arg);
		break;
#endif
	    case 'T':
		tmpdir = arg + 2;
		break;
	    case 't':
		++errcount;
		unsupported(arg, "pass number");
		break;
	    default:
		*argdone = FALSE;
		break;
	    }
	else
	{
	    ++nifiles;
	    *argdone = FALSE;
	    length = strlen(arg);
	    if (length >= 2 && arg[length - 2] == '.' &&
		((ext = arg[length - 1]) == 'c' || ext == 'i' || ext == 's'))
		++ncisfiles;
	}
    }
    npass_specs = prep_only + cc_only + as_only;
    if (npass_specs != 0)
    {
	if (npass_specs > 1)
	{
	    ++errcount;
	    show_who("more than 1 option from -E -P -S -c\n");
	}
	if (f_out != NULL && ncisfiles > 1)
	{
	    ++errcount;
	    show_who("cannot have more than 1 input with non-linked output\n");
	}
    }
    if (nifiles == 0)
    {
	++errcount;
	show_who("no input files\n");
    }
    if (errcount != 0)
	exit(1);

    if ((temp = getenv("BCC_EXEC_PREFIX")) != NULL)
	addprefix(&exec_prefix, temp);
    addprefix(&exec_prefix, STANDARD_EXEC_PREFIX);
#ifdef STANDARD_EXEC_PREFIX_2
    addprefix(&exec_prefix, STANDARD_EXEC_PREFIX_2);
#endif
    cppargs.prog = fixpath(cppargs.prog, &exec_prefix, X_OK);
    ccargs.prog = fixpath(ccargs.prog, &exec_prefix, X_OK);
    asargs.prog = fixpath(asargs.prog, &exec_prefix, X_OK);
    ldargs.prog = fixpath(ldargs.prog, &exec_prefix, X_OK);
    ldrargs.prog = fixpath(ldrargs.prog, &exec_prefix, X_OK);
    if (tmpdir == NULL && (tmpdir = getenv("TMPDIR")) == NULL)
	tmpdir = "/tmp";

    if (prep_only && !prep_line_numbers)
	addarg(&cppargs, "-P");
#ifdef BCC86
    if (bits32)
    {
	bits_arg = "-3";
	crtso = fixpath(CRTSO, &crtso3_prefix, R_OK);
    }
    else
    {
	bits_arg = "-0";
	crtso = fixpath(CRTSO, &crtso0_prefix, R_OK);
    }
    addarg(&ccargs, bits_arg);
    addarg(&cppargs, bits_arg);
    addarg(&asargs, bits_arg);
#ifdef BAS86
    if (!gnu_objects)
    {
	addarg(&ldargs, bits_arg);
	addarg(&ldrargs, bits_arg);
	addarg(&ldargs, crtso);
    }
#endif /* BAS86 */
#endif /* BCC86 */
#if defined(BAS86) && !defined(BCC86)
    if (!gnu_objects)
	addarg(&ldargs, fixpath(CRTSO, &crtso_prefix, R_OK));
#endif
    if (ncisfiles < 2)
	echo = FALSE;
    set_trap();

    /* pass 2 over argv to compile and assemble .c and .s files */
    /* and gather arguments for loader */
    for (argv -= (argc = argcount) - 1, argdone -= argcount - 1; --argc != 0;)
    {
	arg = *++argv;
	if (!*++argdone)
	{
	    length = strlen(arg);
	    if (length >= 2 && arg[length - 2] == '.' &&
		((ext = arg[length - 1]) == 'c' || ext == 'i' || ext == 's'))
	    {
		if (echo || verbose)
		{
		    writes(arg);
		    writesn(":");
		}
		if ((basename = strrchr(arg, DIRCHAR)) == NULL)
		    basename = arg;
		else
		    ++basename;
		in_name = arg;
		if (ext == 'c')
		{
		    if (cpp_pass)
		    {
			if (prep_only)
			    out_name = f_out;
			else
			    out_name = my_mktemp();
			if (run(in_name, out_name, &cppargs) != 0)
			    continue;
			in_name = out_name;
		    }
		    ext = 'i';
		}
		if (ext == 'i')
		{
		    if (prep_only)
			continue;
		    if (cc_only)
		    {
			if (f_out != NULL)
			    out_name = f_out;
			else
			{
			    out_name = stralloc(basename);
			    out_name[strlen(out_name) - 1] = 's';
			}
		    }
		    else
			out_name = my_mktemp();
		    if (run(in_name, out_name, &ccargs) != 0)
			continue;
		    in_name = out_name;
		    ext = 's';
		}
		if (ext == 's')
		{
		    if (prep_only || cc_only)
			continue;
		    out_name = stralloc(basename);
		    out_name[strlen(out_name) - 1] = 'o';
		    if (as_only)
		    {
			if (f_out != NULL)
			    out_name = f_out;
			else
			{
			    out_name = stralloc(basename);
			    out_name[strlen(out_name) - 1] = 'o';
			}
		    }
		    else
			out_name = my_mktemp();
		    addarg(&asargs, "-n");
		    arg[length - 1] = 's';
		    addarg(&asargs, arg);
#ifdef BCC86
		    if (gnu_objects)
		    {
			char *tmp_out_name;

			tmp_out_name = my_mktemp();
			status = run(in_name, tmp_out_name, &asargs);
			asargs.argc -= 2;
			if (status != 0)
			    continue;
			if (run(tmp_out_name, out_name, &ldrargs) != 0)
			    continue;
		    }
		    else
#endif
		    {
			status = run(in_name, out_name, &asargs);
			asargs.argc -= 2;
			if (status != 0)
			    continue;
		    }
		    ext = 'o';
		    in_name = out_name;
		}
		if (ext == 'o')
		{
		    if (prep_only || cc_only || as_only)
			continue;
		    addarg(&ldargs, in_name);
		}
	    }
	    else
		addarg(&ldargs, arg);
	}
    }

    if (!prep_only && !cc_only && !as_only && !runerror)
    {
	if (f_out == NULL)
	    f_out = "a.out";
#ifdef BCC86
	if (gnu_objects)
	{
	    ldargs.prog = GCC;
	    run((char *) NULL, f_out, &ldargs);
	}
	else
#endif
	{
	    addarg(&ldargs, "-lc");
	    run((char *) NULL, f_out, &ldargs);
	}
    }
    killtemps();
    return runerror ? 1 : 0;
}

PRIVATE void addarg(argp, arg)
register struct arg_s *argp;
char *arg;
{
    int new_argc;
    char **new_argv;

    if (argp->nr_allocated == 0)
	startarg(argp);
    new_argc = argp->argc + 1;
    if (new_argc >= argp->nr_allocated)
    {
	argp->nr_allocated += ALLOC_UNIT;
	new_argv = realloc(argp->argv, argp->nr_allocated * sizeof *argp->argv);
	if (new_argv == NULL)
	    outofmemory("addarg");
	argp->argv = new_argv;
    }
    argp->argv[argp->argc] = arg;
    argp->argv[argp->argc = new_argc] = NULL;
}

PRIVATE void addprefix(prefix, name)
struct prefix_s *prefix;
char *name;
{
    struct prefix_s *new_prefix;

    if (prefix->name == NULL)
	prefix->name = name;
    else
    {
	new_prefix = my_malloc(sizeof *new_prefix, "addprefix");
	new_prefix->name = name;
	new_prefix->next = NULL;
	prefix->next = new_prefix;
    }
}

PRIVATE void fatal(message)
char *message;
{
    writesn(message);
    killtemps();
    exit(1);
}

PRIVATE char *fixpath(path, prefix, mode)
char *path;
struct prefix_s *prefix;
int mode;
{
    char *ppath;

    for (; prefix != NULL; prefix = prefix->next)
    {
	ppath = stralloc2(prefix->name, path);
	if (access(ppath, mode) == 0)
	    return ppath;
	free(ppath);
    }
    return path;
}

PRIVATE void killtemps()
{
    while (tmpargs.argc > START_ARGS)
	my_unlink(tmpargs.argv[--tmpargs.argc]);
}

PRIVATE void *my_malloc(size, where)
unsigned size;
char *where;
{
    void *block;

    if ((block = malloc(size)) == NULL)
	outofmemory(where);
    return block;
}

PRIVATE char *my_mktemp()
{
    char *p;
    unsigned digit;
    unsigned digits;
    char *template;
    static unsigned tmpnum;

    p = template = stralloc2(tmpdir, "/bccYYYYXXXX");
    p += strlen(p);
    digits = getpid();
    while (*--p == 'X')
    {
	if ((digit = digits % 16) > 9)
	    digit += 'A' - ('9' + 1);
	*p = digit + '0';
	digits /= 16;
    }
    digits = tmpnum;
    while (*p == 'Y')
    {
	if ((digit = digits % 16) > 9)
	    digit += 'A' - ('9' + 1);
	*p-- = digit + '0';
	digits /= 16;
    }
    ++tmpnum;
    addarg(&tmpargs, template);
    return template;
}

PRIVATE void my_unlink(name)
char *name;
{
    if (verbose)
    {
	show_who("unlinking ");
	writesn(name);
    }
    if (unlink(name) < 0 && verbose)
    {
	show_who("error unlinking ");
	writesn(name);
    }
}

PRIVATE void outofmemory(where)
char *where;
{
    show_who("out of memory in ");
    fatal(where);
}

PRIVATE int run(in_name, out_name, argp)
char *in_name;
char *out_name;
struct arg_s *argp;
{
    int arg0;
    int i;
    int status;

    arg0 = 0;
    if (in_name == NULL)
	++arg0;
    if (out_name == NULL)
	arg0 += 2;
    else if (argp->minus_O_broken)
	++arg0;
    if (argp->nr_allocated == 0)
	startarg(argp);
    argp->argv[arg0] = argp->prog;
    i = arg0 + 1;
    if (in_name != NULL)
	argp->argv[i++] = in_name;
    if (out_name != NULL)
    {
	if (!argp->minus_O_broken)
	    argp->argv[i++] = "-o";
	argp->argv[i++] = out_name;
    }
    if (verbose)
    {
	for (i = arg0; i < argp->argc; ++i)
	{
	    writes(argp->argv[i]);
	    writes(" ");
	}
	writen();
    }
    switch (fork())
    {
    case -1:
	show_who("fork failed");
	fatal("");
    case 0:
	execv(argp->prog, argp->argv + arg0);
	show_who("exec of ");
	writes(argp->prog);
	fatal(" failed");
    default:
	wait(&status);
	for (i = tmpargs.argc - 1; i >= START_ARGS; --i)
	    if (in_name == tmpargs.argv[i])
	    {
		my_unlink(in_name);
		--tmpargs.argc;
		memmove(tmpargs.argv + i, tmpargs.argv + i + 1,
			(tmpargs.argc - i) * sizeof tmpargs.argv[0]);
		break;
	    }
	if (status != 0)
	{
	    killtemps();
	    runerror = TRUE;
	}
	return status;
    }
}

PRIVATE void set_trap()
{
    int signum;

#ifndef _NSIG
#define NSIG	_NSIG
#endif
    for (signum = 0; signum <= _NSIG; ++signum)
#ifdef SIGCHLD
	if (signum != SIGCHLD)
#endif
	if (signal(signum, SIG_IGN) != SIG_IGN)
	    signal(signum, trap);
}

PRIVATE void show_who(message)
char *message;
{
    writes(progname);
    writes(": ");
    writes(message);
}

PRIVATE void startarg(argp)
struct arg_s *argp;
{
    argp->argv = my_malloc((argp->nr_allocated = ALLOC_UNIT)
			   * sizeof *argp->argv, "startarg");
    argp->argc = START_ARGS;
    argp->argv[START_ARGS] = NULL;
}

PRIVATE char *stralloc(s)
char *s;
{
    return strcpy(my_malloc(strlen(s) + 1, "stralloc"), s);
}

PRIVATE char *stralloc2(s1, s2)
char *s1;
char *s2;
{
    return strcat(strcpy(my_malloc(
	strlen(s1) + strlen(s2) + 1, "stralloc2"), s1), s2);
}

PRIVATE void trap(signum)
int signum;
{
    signal(signum, SIG_IGN);
    if (verbose)
	show_who("caught signal");
    fatal("");
}

PRIVATE void unsupported(option, message)
char *option;
char *message;
{
    show_who("compiler option ");
    writes(option);
    writes(" (");
    writes(message);
    writesn(") not supported yet");
}

PRIVATE void writen()
{
    writes("\n");
}

PRIVATE void writes(s)
char *s;
{
    write(2, s, strlen(s));
}

PRIVATE void writesn(s)
char *s;
{
    writes(s);
    writen();
}
