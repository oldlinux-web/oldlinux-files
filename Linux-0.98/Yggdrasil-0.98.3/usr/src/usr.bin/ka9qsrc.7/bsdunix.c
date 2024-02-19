/*
	FILE: unix.c
	
	Routines: This file contains the following routines:
		eihalt()
		kbread()
		clksec()
		tmpfile()
		restore()
		stxrdy()
		disable()
		memstat()
		filedir()
		
	Written by Mikel Matthews, N9DVG
	SYS5 stuff added by Jere Sandidge, K4FUM
*/

#include <stdio.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <ctype.h>
#undef	tolower
#undef	toupper
#include <string.h>
#include "global.h"
#include "cmdparse.h"
#include "iface.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/wait.h>
#include "smtp.h"
#include "unix.h"

#define	MAXCMD	1024

int asy_attach();
unsigned nasy;
extern struct cmds attab[];

fileinit(argv0)
char *argv0;
{
	int el;
	char *ep, *cp, *malloc(), *getenv(), *getcwd();
	char tmp[MAXCMD];
	extern char *startup, *config, *userfile, *hosts, *mailspool;
	extern char *mailqdir, *mailqueue, *routeqdir, *alias, *netexe;
#ifdef  _FINGER
	extern char *fingerpath;
#endif

	/* Get the name of the currently executing program */
	if ((cp = malloc((unsigned)(strlen(argv0) + 1))) == NULL)
		perror("malloc");
	else {
		sprintf(cp, "%s", argv0);
		netexe = cp;
	}

	/* Try to get home directory name */
	if ((ep = getenv("NETHOME")) == NULLCHAR) {
		if ((ep = getenv("HOME")) == NULLCHAR) {
			ep = ".";
		}
	}
	el = strlen(ep);
	/* Replace each of the file name strings with the complete path */
	if (*startup != '/') {
		if ((cp = malloc((unsigned)(el + strlen(startup) + 2))) == NULL)
			perror("malloc");
		else {
			sprintf(cp, "%s/%s", ep, startup);
			startup = cp;
		}
	}

	if (*config != '/') {
		if ((cp = malloc((unsigned)(el + strlen(config) + 2))) == NULL)
			perror("malloc");
		else {
			sprintf(cp, "%s/%s", ep, config);
			config = cp;
		}
	}

	if (*userfile != '/') {
		if ((cp = malloc((unsigned)(el + strlen(userfile) + 2))) == NULL)
			perror("malloc");
		else {
			sprintf(cp, "%s/%s", ep, userfile);
			userfile = cp;
		}
	}

	if (*hosts != '/') {
		if ((cp = malloc((unsigned)(el + strlen(hosts) + 2))) == NULL)
			perror("malloc");
		else {
			sprintf(cp, "%s/%s", ep, hosts);
			hosts = cp;
		}
	}

	if (*alias != '/') {
		if ((cp = malloc((unsigned)(el + strlen(alias) + 2))) == NULL)
			perror("malloc");
		else {
			sprintf(cp, "%s/%s", ep, alias);
			alias = cp;
		}
	}

#ifdef	  _FINGER
	if (*fingerpath != '/') {
		if ((cp = malloc((unsigned)(el + strlen(fingerpath) + 2))) == NULL)
			perror("malloc");
		else {
			sprintf(cp, "%s/%s", ep, fingerpath);
			fingerpath = cp;
		}
	}
#endif

	/* Try to get home directory name */
	if ((ep = getenv("NETSPOOL")) == NULLCHAR)
		ep = "/usr/spool";
	el = strlen(ep);

	if (*mailspool != '/') {
		if ((cp = malloc((unsigned)(el + strlen(mailspool) + 2))) == NULL)
			perror("malloc");
		else {
			sprintf(cp, "%s/%s", ep, mailspool);
			mailspool = cp;
		}
	}

	if (*mailqdir != '/') {
		if ((cp = malloc((unsigned)(el + strlen(mailqdir) + 2))) == NULL)
			perror("malloc");
		else {
			sprintf(cp, "%s/%s", ep, mailqdir);
			mailqdir = cp;
		}
	}

	if (*mailqueue != '/') {
		if ((cp = malloc((unsigned)(el + strlen(mailqueue) + 2))) == NULL)
			perror("malloc");
		else {
			sprintf(cp, "%s/%s", ep, mailqueue);
			mailqueue = cp;
		}
	}

	if (*routeqdir != '/') {
		if ((cp = malloc((unsigned)(el + strlen(routeqdir) + 2))) == NULL)
			perror("malloc");
		else {
			sprintf(cp, "%s/%s", ep, routeqdir);
			routeqdir = cp;
		}
	}
}

/* action routine for remote reset */
sysreset()
{
	extern char *netexe;

	execlp(netexe,netexe,0);
	execlp("net","net",0);
	printf("reset failed: exiting\n");
	exit(1);
}

eihalt()
{
	void tnix_scan();

	tnix_scan();
}

kbread()
{
	int	mask;
	char	c = -1;
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 35;
	mask = 1<<0;
	select(1, &mask, (int *)NULL, (int *)NULL, &timeout);
	if ( mask &= 1<<0 )
	{
		read(fileno(stdin),&c, 1);
	}
	return(c);
}
clksec()
{
	struct timeval time;
	struct timezone zone;

	gettimeofday(&time, &zone);
	return(time.tv_sec);
}
FILE *
tmpfile()
{
	FILE *tmp;
	char *mktemp();
	char *ptr = "SMTPXXXXXX";
	char *name;
	name = mktemp(ptr);
	if ( ( tmp = fopen(name, "w") ) == NULL)
	{
		printf("tmpfile: counld not create temp file.\n");
		return(NULL);
	}
	(void) unlink(name);
	return ( tmp );
}

/*ARGSUSED*/
restore(state)
char state;
{
}

/*ARGSUSED*/
stxrdy(dev)
int16 dev;
{
	return 1;
}

disable()
{}

memstat()
{
	return(0);
}

/* wildcard filename lookup */
filedir (name, times, ret_str)
char *name;
int times;
char *ret_str;
{
	static char     dname[MAXCMD], fname[MAXCMD];
	static DIR *dirp = NULL;
	struct direct *dp;
	struct stat sbuf;
	char    *cp, temp[MAXCMD];

	/*
	 * Make sure that the NULL is there in case we don't find anything
	 */
	ret_str[0] = '\0';

	if (times == 0) {
		/* default a null name to *.* */
		if (name == NULL || *name == '\0')
			name = "*.*";
		/* split path into directory and filename */
		if ((cp = rindex(name, '/')) == NULL) {
			strcpy(dname, ".");
			strcpy(fname, name);
		} else {
			strcpy(dname, name);
			dname[cp - name] = '\0';
			strcpy(fname, cp + 1);
			/* root directory */
			if (dname[0] == '\0')
				strcpy(dname, "/");
			/* trailing '/' */
			if (fname[0] == '\0')
				strcpy(fname, "*.*");
		}
		/* close directory left over from another call */
		if (dirp != NULL)
			closedir(dirp);
		/* open directory */
		if ((dirp = opendir(dname)) == NULL) {
			printf("Could not open DIR (%s)\n", dname);
			return;
		}
	} else {
		/* for people who don't check return values */
		if (dirp == NULL)
			return;
	}

	/* scan directory */
	while ((dp = readdir(dirp)) != NULL) {
		/* test for name match */
		if (wildmat(dp->d_name, fname)) {
			/* test for regular file */
			sprintf(temp, "%s/%s", dname, dp->d_name);
			if (stat(temp, &sbuf) < 0)
				continue;
			if ((sbuf.st_mode & S_IFMT) != S_IFREG)
				continue;
			strcpy(ret_str, dp->d_name);
			break;
		}
	}

	/* close directory if we hit the end */
	if (dp == NULL) {
		closedir(dirp);
		dirp = NULL;
	}
}

/* checks the time then ticks and updates ISS */
static int clkval = 0;
void
check_time()
{
	int32 iss();

	if(clkval != clksec()){
		clkval = clksec();
		tick();
		(void)iss();
	}
}

getds()
{
	return (0);
}

audit()
{
}

doshell(argc, argv)
char **argv;
{
	register int pid, pid1, i, (*savi)();
	union wait rc;
	char *cp, str[MAXCMD], *getenv();
	struct sgttyb tt_config;
	extern struct sgttyb savecon;
	
	str[0] = '\0';
	for (i = 1; i < argc; i++) {
		strcat(str, argv[i]);
		strcat(str, " ");
	}

	ioctl(0, TIOCGETP, &tt_config);
	ioctl(0, TIOCSETP, &savecon);

	if ((cp = getenv("SHELL")) == NULL || *cp != '\0')
		cp = "/bin/sh";

	if ((pid = fork()) == 0) {
		if (argc > 1)
			(void)execl("/bin/sh", "sh", "-c", str, 0);
		else
			(void)execl(cp, cp, (char *)0, (char *)0, 0);
		perror("execl");
		exit(1);
	} else if (pid == -1) {
		perror("fork");
		rc.w_status = -1;
	} else {
		savi = signal(SIGINT, SIG_IGN);
		while ((pid1 = wait(&rc)) != pid && pid1 != -1)
			;	
		signal(SIGINT, savi);
	}

	ioctl(0, TIOCSETP, &tt_config);
	return (rc.w_status);
}


dodir(argc, argv)
char **argv;
{
	register int pid, pid1, i, (*savi)();
	union wait rc;
	char str[MAXCMD];
	struct sgttyb tt_config;
	extern struct sgttyb savecon;

	strcpy(str, "ls -l ");
	for (i = 1; i < argc; i++) {
		strcat(str, argv[i]);
		strcat(str, " ");
	}

	ioctl(0, TIOCGETP, &tt_config);
	ioctl(0, TIOCSETP, &savecon);

	if ((pid = fork()) == 0)
		system(str);

	savi = signal(SIGINT, SIG_IGN);
	while ((pid1 = wait(&rc)) != pid && pid1 != -1)
		;	
	signal(SIGINT, savi);

	ioctl(0, TIOCSETP, &tt_config);
	return (rc.w_status);
}

int
docd(argc, argv)
int argc;
char **argv;
{
	char tmp[MAXCMD];
	char *getwd();

	if (argc > 1) {
		if (chdir(argv[1]) == -1) {
			printf("Can't change directory\n");
			return 1;
		}
	}
	if (getwd(tmp) != NULL)
		printf("%s\n", tmp);

	return 0;
}

/*
 * These are here to prevent chars that are already lower or upper
 * case from being turned into gibberish.
 */
char
tolower(c)
char c;
{
	if (isupper(c))
		return(c - 'A' + 'a');
	else
		return(c);
}

char
toupper(c)
char c;
{
	if (islower(c))
		return(c - 'a' + 'A');
	else
		return(c);
}
