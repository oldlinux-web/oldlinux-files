/* ccc.c - driver for CvW's C compiler */

/*{{{  includes*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/*}}}  */
/*{{{  defines*/
/*{{{  DEBUG __STDC__ defines*/
#define FALSE   0
#ifdef DEBUG
#       define FORWARD
#       define PRIVATE
#else
#       define FORWARD  static
#       define PRIVATE  static
#endif
#ifndef NULL
#       define NULL     0
#endif
#define PUBLIC
#define TRUE    1

#if __STDC__
#define P(x)    x
#else
#define P(x)    ()
#endif
/*}}}  */

typedef unsigned char bool_t;   /* boolean: TRUE if nonzero */

#define AS      "/usr/bin/as"
#define CC1     "/usr/local/bin/c386"
#define LD      "/usr/bin/ld"
#define CPP     "/usr/local/bin/unproto"

#define CRTSO   "/usr/lib/gcc-lib/i386-linux/2.2.2/crt0.o"
#define SHARED  "-L/usr/lib/gcc-lib/i386-linux/2.2.2/shared"
#define STATIC  "-L/usr/lib/gcc-lib/i386-linux/2.2.2"
#define INCLUDE "-I/usr/local/lib/cvw/include"
#define CVWLIB 	"/usr/local/lib/cvw/lib/libcvw.a"
#define GNULIB  "/usr/lib/gcc-lib/i386-linux/2.2.2/libgcc.a" /* Is needed to link static */

#define TMPNAME "/tmp/cccYYYYXXXX"

#define ALLOC_UNIT      16      /* allocation unit for arg arrays */
#define DIRCHAR '/'
/*}}}  */
/*{{{  declarations*/
struct arg_s {
        int             argc;
        char          **argv;
        unsigned        size;
};

PRIVATE struct arg_s asargs;    /* = NULL */
PRIVATE struct arg_s ccargs;    /* = NULL */
PRIVATE struct arg_s ldargs;    /* = NULL */
PRIVATE struct arg_s prargs;    /* = NULL */
PRIVATE char   *progname;
PRIVATE char   tmpname[255] = TMPNAME;
PRIVATE struct arg_s tmpargs;   /* = NULL */
PRIVATE bool_t  verbose;        /* = FALSE */

FORWARD void addarg P ((register struct arg_s * argp, char *arg));
FORWARD void fatal P ((char *message));
FORWARD void killtemps P ((void));
FORWARD void   *my_malloc P ((unsigned size, char *where));
FORWARD char   *my_mktemp P ((void));
FORWARD void my_unlink P ((char *name));
FORWARD void outofmemory P ((char *where));
FORWARD int run P ((char *prog, char *arg1, char *arg2, struct arg_s * argp));
FORWARD void set_trap P ((void));
FORWARD void show_who P ((char *message));
FORWARD void startarg P ((struct arg_s * argp));
FORWARD char   *stralloc P ((char *s));
FORWARD void trap P ((int signum));
FORWARD void unsupported P ((char *option, char *message));
FORWARD void writen P ((void));
FORWARD void writes P ((char *s));
FORWARD void writesn P ((char *s));
/*}}}  */

/*{{{  PUBLIC int main (argc, argv)*/
PUBLIC int main (argc, argv)
        int             argc;
        char          **argv;
{
        /*{{{  local variables*/
        register char  *arg;
        int             argcount = argc;
        bool_t         *argdone = my_malloc ((unsigned) argc * sizeof *argdone, "argdone");
        bool_t          as_only = FALSE;
        char           *basename;
        bool_t          cc_only = FALSE;
        char           *crtso;
        bool_t          debug = FALSE;
        bool_t          echo = FALSE;
        unsigned        errcount = 0;
        char            ext;
        char           *f_out = "a.out";
        bool_t          float_emulation = FALSE;
        bool_t		use_static = FALSE;
        int             length;
        unsigned        ncsfiles = 0;
        unsigned        nfilters;
        unsigned        nifiles = 0;
        unsigned        nofiles = 0;
        char           *o_out;
        bool_t          optimize = FALSE;
        bool_t          profile = FALSE;
        bool_t          prep_debug = FALSE;
        bool_t          prep_only = FALSE;
        char           *s_out;
        int             status = 0;
        char           *i_out;
        /*}}}  */

        progname = argv[0];
        
        /*{{{  cpp (unproto) args*/
        addarg (&prargs, "-DNO_FIX_MALLOC");
        addarg (&prargs, "-D__STDC__");
        addarg (&prargs, "-D__CVW__");
        addarg (&prargs, "-DNeedFunctionPrototypes=0"); /* Hack for X11 include */
        addarg (&prargs, "-Dvolatile=");
        addarg (&prargs, "-Dconst=");
        addarg (&prargs, "-Dinline=");
        addarg (&prargs, INCLUDE);
        /*}}}  */

        for (; --argc != 0;)
        /*{{{  gather compile flags*/
        {
                arg = *++argv;
                *++argdone = TRUE;
                if (arg[0] == '-' && arg[1] != 0 && arg[2] == 0)
                        switch (arg[1])
                        /*{{{  cases: E,P,O,S,V,c,f,g,o,p,v*/
                        {
                        case 'E':
                                prep_debug = TRUE;
                                break;
                        case 'P':
                                prep_only = TRUE;
                                break;
                        case 'O':
                                optimize = TRUE;
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
                        case 'f':
                                float_emulation = TRUE;
                                ++errcount;
                                unsupported (arg, "float emulation");
                                break;
                        case 'g':
                                debug = TRUE;   /* unsupported( arg, "debug"
                                                 * ); */
                                break;
                        case 'o':
                                if (--argc < 1) {
                                        ++errcount;
                                        show_who ("output file missing after -o\n");
                                } else {
                                        ++nofiles;
                                        f_out = *++argv;
                                        *++argdone = TRUE;
                                }
                                break;
                        case 'p':
                                profile = TRUE;
                                ++errcount;
                                unsupported (arg, "profile");
                                break;
                        case 'v':
                                writesn ("verbose");
                                verbose = TRUE;
                                break;
                        default:
                                *argdone = FALSE;
                                break;
                        }
                        /*}}}  */
                else if (arg[0] == '-')
                        switch (arg[1])
                        /*{{{  cases: A,B,C,D,I,Q,L,U,T,t*/
                        {
                        case 'A':
                                addarg (&asargs, arg + 2);
                                break;
                        case 'B':
                                if(!strcmp(arg + 2, "static"))
                                  use_static=TRUE;
                                break;
                        case 'C':
                                addarg (&ccargs, arg + 2);
                                break;
                        case 'D':
                        case 'I':
                                addarg (&prargs, arg);
                                break;
                        case 'Q':
                                addarg (&ccargs, arg);
                                break;
                        case 'L':
                                addarg (&ldargs, arg + 2);
                                break;
                        case 'U':
                                ++errcount;
                                unsupported (arg, "undef");
                                break;
                        case 'T':
                                strcpy (tmpname, arg + 2);
                                strcat (tmpname, "/cccXXXXYYYY");
                                break;
                        case 't':
                                ++errcount;
                                unsupported (arg, "pass number");
                                break;
                        default:
                                *argdone = FALSE;
                                break;
                        }
                        /*}}}  */
                else {
                        ++nifiles;
                        *argdone = FALSE;
                        length = strlen (arg);
                        if (length >= 2 && arg[length - 2] == '.' &&
                            ((ext = arg[length - 1]) == 'c' || ext == 's'))
                                ++ncsfiles;
                }
        }
        /*}}}  */

        /*{{{  check if given args are valid*/
        nfilters = prep_debug + prep_only + cc_only + as_only;
        if (nfilters != 0) {
                if (nfilters > 1) {
                        ++errcount;
                        show_who ("more than 1 option from -E -P -S -c\n");
                }
                if (nofiles != 0 && ncsfiles > 1) {
                        ++errcount;
                        show_who ("cannot have more than 1 input with non-linked output\n");
                }
        }
        if (nifiles == 0) {
                ++errcount;
                show_who ("no input files");
        }
        if (errcount != 0)
                exit (1);
        /*}}}  */

        addarg (&ldargs, CRTSO);

        if (ncsfiles < 2)
                echo = FALSE;
        set_trap ();

        /*{{{  compile and assemble .c .s files / gather arguments for loader*/
        for (argv -= (argc = argcount) - 1, argdone -= argcount - 1; --argc != 0;) {
                arg = *++argv;
                if (!*++argdone) {
                        length = strlen (arg);
                        if (length >= 2 && arg[length - 2] == '.' &&
                            ((ext = arg[length - 1]) == 'c' || ext == 'i' || ext == 's')) {
                                if (echo || verbose) {
                                        writes (arg);
                                        writesn (":");
                                }
                                if ((basename = strrchr (arg, DIRCHAR)) == NULL)
                                        basename = arg;
                                else
                                        ++basename;
        
                                i_out = s_out = o_out = arg;
        
                                switch (ext)
                                /*{{{  cases c,i,s*/
                                {
                                case 'c':
                                        if (prep_only) {
                                                if (nofiles != 0)
                                                        i_out = f_out;
                                                else {
                                                        i_out = stralloc (basename);
                                                        i_out[strlen (s_out) - 1] = 'i';
                                                }
                                        } else {
                                                i_out = my_mktemp ();
                                        }
                                        if (run (CPP, arg, i_out, &prargs) != 0) {
                                                writesn ("CPP error");
                                                exit (1);
                                        }
                                        if (prep_only) break;
                                case 'i':
                                        if (cc_only) {
                                                if (nofiles != 0)
                                                        s_out = f_out;
                                                else {
                                                        s_out = stralloc (basename);
                                                        s_out[strlen (s_out) - 1] = 's';
                                                }
                                        } else
                                                s_out = my_mktemp ();
                                        if (run (CC1, i_out, s_out, &ccargs) != 0) {
                                                writesn ("CC Error");
                                                exit (1);
                                        }
                                        if (cc_only) break;
                                case 's':
                                        if (as_only) {
                                                if (nofiles != 0)
                                                        o_out = f_out;
                                                else {
                                                        o_out = stralloc (basename);
                                                        o_out[strlen (o_out) - 1] = 'o';
                                                }
                                        } else
                                                o_out = my_mktemp ();
                                        /*addarg (&asargs, arg);*/
                                        addarg (&asargs, s_out);
                                        if (run (AS, "-o", o_out, &asargs) != 0) {
                                                writesn ("AS error\n");
                                                exit (1);
                                        }
                                        if (!as_only)
                                                addarg (&ldargs, o_out);
                                }
                                /*}}}  */
                        } else
                                addarg (&ldargs, arg);
                }
        }
        /*}}}  */
        /*{{{  link files with libc libcvw regarding -Bstatic*/
        if (!prep_only && !cc_only && !as_only) {
                if (!use_static)
                  addarg (&ldargs, SHARED);
                addarg (&ldargs, STATIC);
                addarg (&ldargs, CVWLIB);
                addarg (&ldargs, "-lc");
                if (use_static)
                  addarg (&ldargs, GNULIB);
                status = run (LD, "-o", f_out, &ldargs) != 0;
        }
        /*}}}  */

        killtemps ();
        return status;
}
/*}}}  */

/*{{{  PRIVATE void addarg (argp, arg)*/
PRIVATE void addarg (argp, arg)
        register struct arg_s *argp;
        char           *arg;
{
        if (argp->size == 0)
                startarg (argp);
        if (++argp->argc >= argp->size &&
            (argp->argv = realloc (argp->argv, (argp->size += ALLOC_UNIT) *
                                   sizeof *argp->argv)) == NULL)
                outofmemory ("addarg");
        argp->argv[argp->argc - 1] = arg;
        argp->argv[argp->argc] = NULL;
}
/*}}}  */
/*{{{  PRIVATE void fatal (message)*/
PRIVATE void fatal (message)
        char           *message;
{
        killtemps ();
        exit (1);
}
/*}}}  */
/*{{{  PRIVATE void killtemps ()*/
PRIVATE void killtemps ()
{
        for (tmpargs.argc -= 2, tmpargs.argv += 2; --tmpargs.argc > 0;)
                my_unlink (*++tmpargs.argv);
}
/*}}}  */
/*{{{  PRIVATE void   *my_malloc (size, where)*/
PRIVATE void   *my_malloc (size, where)
        unsigned        size;
        char           *where;
{
        void           *block;

        if ((block = (void *) malloc (size)) == NULL)
                outofmemory (where);
        return block;
}
/*}}}  */
/*{{{  PRIVATE char   *my_mktemp ()*/
PRIVATE char   *my_mktemp ()
{
        register char  *p;
        unsigned        digit;
        unsigned        digits;
        char           *template;
        static unsigned tmpnum;

        p = template = stralloc (tmpname);
        p += strlen (p);
        digits = getpid ();
        while (*--p == 'X') {
                if ((digit = digits % 16) > 9)
                        digit += 'A' - ('9' + 1);
                *p = digit + '0';
                digits /= 16;
        }
        digits = tmpnum;
        while (*p == 'Y') {
                if ((digit = digits % 16) > 9)
                        digit += 'A' - ('9' + 1);
                *p-- = digit + '0';
                digits /= 16;
        }
        ++tmpnum;
        addarg (&tmpargs, template);
        return template;
}
/*}}}  */
/*{{{  PRIVATE void my_unlink (name)*/
PRIVATE void my_unlink (name)
        char           *name;
{
        if (verbose) {
                show_who ("unlinking ");
                writesn (name);
        }
        if (unlink (name) < 0 && verbose) {
                show_who ("error unlinking ");
                writesn (name);
        }
}
/*}}}  */
/*{{{  PRIVATE void outofmemory (where)*/
PRIVATE void outofmemory (where)
        char           *where;
{
        show_who ("out of memory in ");
        fatal (where);
}
/*}}}  */
/*{{{  PRIVATE int run (prog, arg1, arg2, argp)*/
PRIVATE int run (prog, arg1, arg2, argp)
        char           *prog;
        char           *arg1;
        char           *arg2;
        register struct arg_s *argp;
{
        int             i;
        int             status;

        if (argp->size == 0)
                startarg (argp);
        argp->argv[0] = prog;
        argp->argv[1] = arg1;
        argp->argv[2] = arg2;
        if (verbose) {
                for (i = 0; i < argp->argc; ++i) {
                        writes (argp->argv[i]);
                        writes (" ");
                }
                writen ();
        }
        switch (fork ()) {
        case -1:
                show_who ("fork failed");
                fatal ("");
        case 0:
                execv (prog, argp->argv);
                show_who ("exec of ");
                writes (prog);
                fatal (" failed");
        default:
                wait (&status);
                return status;
        }
}
/*}}}  */
/*{{{  PRIVATE void set_trap ()*/
PRIVATE void set_trap ()
{
        signal (SIGINT, trap);
	signal (SIGQUIT, trap);
	signal (SIGILL, trap);
}
/*}}}  */
/*{{{  PRIVATE void show_who (message)*/
PRIVATE void show_who (message)
        char           *message;
{
        writes (progname);
        writes (": ");
        writes (message);
}
/*}}}  */
/*{{{  PRIVATE void startarg (argp)*/
PRIVATE void startarg (argp)
        struct arg_s   *argp;
{
        argp->argv = my_malloc ((argp->size = ALLOC_UNIT) * sizeof *argp->argv,
                                "startarg");
        argp->argc = 3;
        argp->argv[3] = NULL;
}
/*}}}  */
/*{{{  PRIVATE char   *stralloc (s)*/
PRIVATE char   *stralloc (s)
        char           *s;
{
        return strcpy (my_malloc (strlen (s) + 1, "stralloc"), s);
}
/*}}}  */
/*{{{  PRIVATE void trap (signum)*/
PRIVATE void trap (signum)
        int             signum;
{
        signal (signum, SIG_IGN);
        if (verbose) {
                show_who ("caught signal");
		printf(" %d",signum);
	}
        fatal ("");
}
/*}}}  */
/*{{{  PRIVATE void unsupported (option, message)*/
PRIVATE void unsupported (option, message)
        char           *option;
        char           *message;
{
        show_who ("compiler option ");
        writes (option);
        writes (" (");
        writes (message);
        writesn (") not supported yet");
}
/*}}}  */
/*{{{  PRIVATE void writen ()*/
PRIVATE void writen ()
{
        writes ("\n");
}
/*}}}  */
/*{{{  PRIVATE void writes (s)*/
PRIVATE void writes (s)
        char           *s;
{
        write (2, s, strlen (s));
	fflush(stderr);
}
/*}}}  */
/*{{{  PRIVATE void writesn (s)*/
PRIVATE void writesn (s)
        char           *s;
{
        writes (s);
        writen ();
}
/*}}}  */
