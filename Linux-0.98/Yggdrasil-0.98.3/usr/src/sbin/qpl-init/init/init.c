/*
	init.c does some of the stuff /etc/init should do.
	by qpliu@phoenix.princeton.edu 1992
	run /etc/rc,
	then run getty for each entry in /etc/ttytab
	SIGHUP makes it reread /etc/ttytab
	SIGTSTP makes it stop forking
	SIGTERM is ignored

	ugly error messages might get stuck on console, syslog stuff?

	imported stuff from original init function
*/
/*
	my computer is a puny 2meg 386sx, and can't run gcc.
	so I made cc1 on my account on a sun 4/490
	and compile to .s on the sun.  however, cc1 dies with
	SIGIOT when trying to optimize some of the inlined string.h stuff
	when used in a big function.
*/
#define GCC_O_BUSTED

#include <unistd.h>
#include <time.h>
#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <asm/system.h>
#include <asm/io.h>
#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <linux/fs.h>
#include <ttyent.h>
#include <signal.h>
#include <string.h>

struct ttytablist {
	int pid;
	char *dev, *getty;
	struct ttytablist *next;
};

static struct ttyent *ent;
static struct ttytablist *tablist0, *tablist;
static char **getty_argv;

#ifdef GCC_O_BUSTED
static char *strtok1 (char *, const char *);
#else
#define strtok1 strtok
#endif GCC_O_BUSTED

static char * argv_rc[] = { "/bin/bash", NULL };
static char * envp_rc[] = { "HOME=/", NULL };
static char * argv[] = { "-bash", NULL };
static char * envp[] = { "HOME=/", NULL };

static char got_tstp = 0;

#define GETTY_RETRY_DLAY 30

static void rc ();
static void child ();
static void read_ttytab ();
static void free_ttytab (struct ttytablist *);
static void single_user ();
static void hup_handler ();
static void tstp_handler ();

void
main (void)
{
	rc ();
	read_ttytab ();	/* initially, tablist->pid = 0 if off, -1 if on */

	/* run getty */
	tablist = tablist0;
	while (tablist) {
		if (tablist->pid) {
			if (!(tablist->pid = fork())) 
				child ();
			if (tablist->pid == -1) {
				printf ("Fork failed in init for %s\n\r",
					tablist->dev);
				tablist->pid = 0;	/* waste memory */
			}
		}
		tablist = tablist->next;
	}

	signal (SIGHUP, hup_handler);
	signal (SIGTERM, single_user);
	signal (SIGTSTP, tstp_handler);

	/* reap children, restart getty unless got_tstp */
	for (;;) {
		int pid, i;

		pid = wait (&i);
		if (!got_tstp) {
			tablist = tablist0;
			while (tablist) {
				if (tablist->pid == pid) {
					if (!(tablist->pid = fork()))
						child ();
					if (tablist->pid == -1) {
						printf ("Fork failed in init for %s\n\r", tablist->dev);
						tablist->pid = 0; /* waste memory */
					}
				}
				tablist = tablist->next;
			}
		}
		sync();
	}
	_exit(0);	/* NOTE! _exit, not exit() */
}

static void
rc ()
{
	int i, pid;

	if (!(pid=fork())) {
		close(0);
		if (open("/etc/rc",O_RDONLY,0))
			_exit(1);
		execve("/bin/bash",argv_rc,envp_rc);
		_exit(2);
	}
	if (pid>0)
		while (pid != wait(&i))
			/* nothing */;
}

static void
free_ttytab (struct ttytablist *t)
{
	struct ttytablist *u;

	/* free all that malloced stuff */
	endttyent ();
	do {
		u = t->next;
		free (t->dev);
		free (t->getty);
		free (t);
	} while (t = u);
}

static void
read_ttytab ()
{
	tablist0 = (struct ttytablist *)malloc (sizeof (struct ttytablist));
	if (!tablist0) {
		printf ("Malloc failed in init\n\r");
		_exit (0);
	}
	tablist0->next = (struct ttytablist *)0;
	tablist = tablist0;
	ent = getttyent ();
	if (!ent) {
		printf ("Getttyent failed in init, check /etc/ttytab\n\r");
		/* should probably enter single-user mode */
		_exit (0);
	}

	/* assume first entry is on, if it's off, waste some memory */
	tablist->pid = 0;
	for (;;) {
		tablist->next = (struct ttytablist *)0;
		tablist->dev = (char *)malloc (5 + strlen (ent->ty_name));
		strcpy (tablist->dev, "/dev/");
		strcat (tablist->dev, ent->ty_name);
		tablist->getty = (char *)malloc (strlen (ent->ty_getty));
		strcpy (tablist->getty, ent->ty_getty);
		if (ent->ty_status & TTY_ON) tablist->pid = -1;
		else tablist->pid = 0;

		/* don't waste memory to store off entries */
		do {
			ent = getttyent ();
			if (!ent) {
				endttyent ();
				break;
			}
		} while (ent->ty_status & TTY_ON);
		if (!ent) break;

		tablist->next = (struct ttytablist *)malloc (sizeof (struct ttytablist));
		tablist = tablist->next;
	}
}

static void
child ()
{
	int i;
	char *c, *g = (char *)malloc (strlen (tablist->getty) * sizeof (char));

	close(0);close(1);close(2);
	setsid();
	(void) open(tablist->dev,O_RDWR,0);
	(void) dup(0);
	(void) dup(0);

	/* find argc for getty */
	strcpy (g, tablist->getty);
	i = 1; strtok1 (g, " \t\n");
	while (strtok1 ((char *)0, " \t\n")) ++i;
	getty_argv = (char **)malloc (i * sizeof (char **));
	strcpy (g, tablist->getty);
	i = 1; c = strtok1 (g, " \t\n"); *getty_argv = c;
	while (c = strtok1 ((char *)NULL, " \t\n"))
		*(getty_argv + i++) = c;

	free_ttytab (tablist0);

	execve (*getty_argv, getty_argv, envp);
	sleep (GETTY_RETRY_DLAY);	/* don't hog system */
	_exit (0);
}

static void
hup_handler ()
{
	struct ttytablist *t = tablist0, *u, *v;

	got_tstp = 0;
	signal (SIGHUP, SIG_IGN);
	read_ttytab ();

	/* copy running pids that still map onto enabled terminals */
	for (u = tablist0; u; u = u->next)
		if (u->pid) {
			for (v = t; v; v = v->next)	/* simple-minded */
				if (!strcmp (v->dev, u->dev)) {
					u->pid = v->pid;
					break;
				}

			/* check if new terminal enabled */
			if (!v) {
				if (!(u->pid = fork()))
					child ();
				if (u->pid == -1) {
					printf ("Fork failed in init for %s\n\r", tablist->dev);
					u->pid = 0; /* waste memory */
				}
			}
		}

	free_ttytab (t);
	signal (SIGHUP, hup_handler);
}

static void
tstp_handler ()
{
	got_tstp = 1;
}

static void
single_user ()
{
}

/* don't want this inlined with -finline-functions, so put it last */
#ifdef GCC_O_BUSTED
static char *strtok1 (char *a, const char *b)
{
	return strtok (a, b);
}
#endif GCC_O_BUSTED
