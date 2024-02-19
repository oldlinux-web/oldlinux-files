#include <stdio.h>
#include <grp.h>
#include <pwd.h>
#if defined(SYSV) || defined(LINUX)
#include <string.h>
#else /* not SYSV but BSD */
#include <strings.h>
#endif /* SYSV / BSD */


int atoi(), getopt();
char *arg0;

char *fmt = NULL;

#define GRGET	1
#define PWGET	2

int mode;			/* Mode of operation, either GRGET or PWGET. */

main(argc, argv)
int argc;
char **argv;
{
    int printgr(), printpw();
    int c;
    extern char *optarg;
    extern int optind;
    struct group *grp;
    struct passwd *pwd;
    int anyflag = 0,
	gflag = 0,
	nflag = 0,
	uflag = 0,
	xclude = 0;
    int gid, uid;
    char *name, *opts;

    mode = 0;

#if defined(SYSV) || defined(LINUX)
    if ((arg0 = strrchr(argv[0], '/')) == NULL)
#else /* not SYSV but BSD */
    if ((arg0 = rindex(argv[0], '/')) == NULL)
#endif /* SYSV / BSD */
	arg0 = argv[0];
    else
	arg0++;			/* Start after the '/' */

    if (strcmp(arg0, "grget") == 0)
	mode = GRGET;
    else if (strcmp(arg0, "pwget") == 0)
	mode = PWGET;
    else
	usage();

    switch(mode)
    {
case GRGET:
	setgrent();
	opts = "g:n:f:x";
	break;
case PWGET:
	setpwent();
	opts = "u:n:f:x";
	break;
    }

    while ((c = getopt(argc, argv, opts)) != EOF)
    {
	switch (c)
	{
    case 'x':
	    xclude = 1;
	    break;	    
    case 'f':
	    fmt =  optarg;
	    break;
    case 'g':
	    if (anyflag != 0)
		usage();

	    gflag++;
	    anyflag++;
	    gid = atoi(optarg);
	    break;
    case 'n':
	    if (anyflag != 0)
		usage();

	    nflag++;
	    anyflag++;
	    name = optarg;
	    break;
    case 'u':
	    if (anyflag != 0)
		usage();

	    uflag++;
	    anyflag++;
	    uid = atoi(optarg);
	    break;
    case '?':
	    usage();
	    break;
	}
    }

    if (argv[optind] != NULL)
	usage();

    if (gflag)
    {
	if (xclude) {
	    while ((grp = getgrent()) != NULL)
		if (grp->gr_gid != gid)
		    printgr(grp);
	}
	else if ((grp = getgrgid(gid)) != NULL)
	    printgr(grp);
	else
	    exit(1);
    }
    else if (nflag)
    {
	if (mode == GRGET)
	{
	    if (xclude) {
		while ((grp = getgrent()) != NULL)
		    if (strcmp(grp->gr_name,name))
			printgr(grp);
	    }
	    else if ((grp = getgrnam(name)) != NULL)
		printgr(grp);
	    else
		exit(1);
	}
	else if (mode == PWGET)
	{
	    if (xclude) {
		while ((pwd = getpwent()) != NULL)
		    if (strcmp(pwd->pw_name,name))
			printpw(pwd);
	    }
	    else if ((pwd = getpwnam(name)) != NULL)
		printpw(pwd);
	    else
		exit(1);
	}
    }
    else if (uflag)
    {
	if (xclude) {
	    while ((pwd = getpwent()) != NULL)
		if (pwd->pw_uid != uid)
		     printpw(pwd);
	}
	else if ((pwd = getpwuid(uid)) != NULL)
	    printpw(pwd);
	else
	    exit(1);
    }
    else
    {
	if (mode == GRGET)
	{
	    while ((grp = getgrent()) != NULL)
		printgr(grp);
	}
	else if (mode == PWGET)
	{
	    while ((pwd = getpwent()) != NULL)
		printpw(pwd);
	}
    }

    switch(mode)
    {
case GRGET:
	endgrent();
	break;
case PWGET:
	endpwent();
	break;
    }

    exit(0);
}


usage()
{
    switch(mode)
    {
case GRGET:
	fprintf(stderr, "usage: %s [ -g gid | -n name ]\n", arg0);
	break;
case PWGET:
	fprintf(stderr, "usage: %s [ -n name | -u uid ]\n", arg0);
	break;
default:
	fprintf(stderr, "Call as either grget or pwget\n");
	break;
    }

    exit(2);
}


printgr(g)
struct group *g;
{
    char **chr;
    int comma;

    if (g != NULL)
    {
	if (fmt)
	    return grfmt(fmt,g);
	printf("%s:%s:%d:", g->gr_name, g->gr_passwd, g->gr_gid);

	/* prints "grp1,grp2,grp3, ... ,grpn" */
	for (comma = 0, chr = g->gr_mem; *chr != NULL; chr++)
	    printf("%s%s", ((comma==0)?comma++,"":","), *chr);

	printf("\n");
    }
}


printpw(p)
struct passwd *p;
{
    if (p != NULL)
    {
	if (fmt) {
	    return pwfmt(fmt,p);
	}
	printf("%s:%s", p->pw_name, p->pw_passwd);

#ifdef SYSV
	if (strcmp(p->pw_age, "") != 0)
	    printf(",%s", p->pw_age);
#endif /* SYSV */

	printf(":%d:%d:%s:%s:%s\n", p->pw_uid, p->pw_gid,
		p->pw_gecos, p->pw_dir, p->pw_shell);
    }
}

pwfmt(fmt,pwd)
	struct passwd *pwd;
	char *fmt;
{
     while (*fmt)
	if (*fmt == '\\') {
	    switch (fmt[1]) {
	    case '\\':	putchar('\\');	break;
	    case 'n':	putchar('\n');	break;
	    case '\000': return;
	    case 't':	putchar('\t');	break;
	    case 'b':	putchar('\b');	break;
	    case 'r':	putchar('\r');  break;
	    case 'f':	putchar('\f');	break;	
	    default:	putchar('\\'); putchar(fmt[1]);
	    }
	    fmt += 2;
	}
	else if (*fmt == '%') {
	    switch (fmt[1]) {
	    case '\000': return;
	    case '%':	putchar('%');	break;
	    case 'n':	fputs(pwd->pw_name,stdout); break;
	    case 'p':	fputs(pwd->pw_passwd,stdout); break;
#ifdef SYSV
	    case 'a':	fputs(pwd->pw_age,stdout); break;
#endif
	    case 'u':	printf("%d",pwd->pw_uid); break;
	    case 'g':	printf("%d",pwd->pw_gid); break;
	    case 'f':	fputs(pwd->pw_gecos,stdout); break;
	    case 'd':	fputs(pwd->pw_dir,stdout); break;
	    case 's':	fputs(pwd->pw_shell,stdout); break;
	    default:	putchar('%'); putchar(fmt[1]);
	    }
	    fmt += 2;
	}
	else 
	    putchar(*(fmt++));
    putchar('\n');
}

grfmt(fmt,grp)
	struct group *grp;
	char *fmt;
{
     while (*fmt)
	if (*fmt == '\\') {
	    switch (fmt[1]) {
	    case '\\':	putchar('\\');	break;
	    case 'n':	putchar('\n');	break;
	    case '\000': return;
	    case 't':	putchar('\t');	break;
	    case 'b':	putchar('\b');	break;
	    case 'r':	putchar('\r');  break;
	    case 'f':	putchar('\f');	break;	
	    default:	putchar('\\'); putchar(fmt[1]);
	    }
	    fmt += 2;
	}
	else if (*fmt == '%') {
	    switch (fmt[1]) {
	    case '\000': return;
	    case '%':	putchar('%');	break;
	    case 'n':	fputs(grp->gr_name,stdout); break;
	    case 'p':	fputs(grp->gr_passwd,stdout); break;
	    case 'g':	printf("%d",grp->gr_gid); break;
	    case 'm':
		{
		    int comma;
		    char **chr;
		    for (comma = 0, chr = grp->gr_mem; *chr != NULL; chr++)
			printf("%s%s", ((comma==0)?comma++,"":","), *chr);
		}
		break;
	    default:	putchar('%'); putchar(fmt[1]);
	    }
	    fmt += 2;
	}
	else 
	    putchar(*(fmt++));
    putchar('\n');
}
