/*
	login.c
	by qpliu@phoenix.princeton.edu 1992
	a rudimentary way to avoid being root on linux 0.11
	no motd or other messages or .hushlogin
	no passwords
	no /etc/nologin
	no checking if term is secure
*/

#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <ttyent.h>

#define DEFAULT_SHELL	"/bin/bash"
#define TIMEOUT	60
#define STIMEOUT	"60"

extern char **environ;
static void do_login (char *);
static int check_passwd (struct passwd *);
static void timeout ();
static void setenviron ();
static char savenv = 0;
static struct passwd *entry;

void
main (int argc, char **argv)
{
	char login[L_cuserid];

	if (argc > 1)
		if (!(strcmp ("-p", *(argv + 1)))) {
			--argc; ++argv; savenv = 1;
		}

	if (argc > 1)
		do_login (*++argv);

	for (;;) {
		fputs ("login: ", stdout);
		fflush (stdout);
		signal (SIGALRM, timeout);
		alarm (TIMEOUT);
		fgets (login, L_cuserid, stdin);
		alarm (0);
		signal (SIGALRM, SIG_DFL);
		if (feof (stdin)) exit(0);
		if (*login == '\n') continue;

		if (login [strlen (login) - 1] == '\n')
			login [strlen (login) - 1] = '\0';
		else {	/* discard rest of line */
			char junk[80];
			do fgets (junk, 80, stdin);
			while (junk [strlen (junk) - 1] != '\n');
		}

		do_login (login);
	}
}

static void
do_login (char *login)
{
	entry = getpwnam (login);
	if (check_passwd (entry)) {
		puts ("Login incorrect");
		return;
	}

	if (setgid (entry->pw_gid)) {
		perror ("setgid");
		return;
	}

	if (setuid (entry->pw_uid)) {
		setgid (0);
		perror ("setuid");
		return;
	}

	if (chdir (entry->pw_dir))
		perror ("Couldn't chdir");

	{
		char *p, *d, *e;
		if (!(e = entry->pw_shell)) e = DEFAULT_SHELL;
		p = strrchr (e, '/');
		if (!p++) p = e;
		if (!p) p = DEFAULT_SHELL;
		d = (char *)malloc (strlen(p) + 1);
		*d = '-'; *(d+1) = '\0';
		strcat(d, p);
		setenviron ();
		execl(e, d, (char *)0);
		perror("No shell");
	}
}

static int
check_passwd (struct passwd *entry)
{
	/* room to get and check actual (yuck) passwords */
	return entry ? 0 : 1;
}

static void
timeout ()
{
	/* printf ("Login timed out after %d seconds\n", TIMEOUT); */
	/* 8k for 1 printf too big */
	puts ("Login timed out after " STIMEOUT " seconds");
	exit (0);
}

extern char *ttyname (int);

static void
setenviron ()
{
	/* too much bother, man pages say HOME, SHELL, TERM, USER set */
	char **c = environ, *d;

	while (*c) free (*(c++));
	free (environ);

	c = (char **)malloc (5 * sizeof (char *));
	environ = c;

	d = ttyname (0);
	if (d) {
		d += 5;	/* "/dev/" */
		d = getttynam (d)->ty_type;
	}
	*c = (char *)malloc ((5 + strlen (entry->pw_dir)) * sizeof (char));
	strcpy (*c, "HOME="); strcat (*c, entry->pw_dir);
	*++c = (char *)malloc ((6 + strlen (entry->pw_shell)) * sizeof (char));
	strcpy (*c, "SHELL="); strcat (*c, entry->pw_shell);
	*++c = (char *)malloc ((5 + strlen (d)) * sizeof (char));
	strcpy (*c, "TERM="); strcat (*c, d);
	*++c = (char *)malloc ((5 + strlen (entry->pw_name)) * sizeof (char));
	strcpy (*c, "NAME="); strcat (*c, entry->pw_name);
	*++c = (char *)0;
	endttyent ();
}
