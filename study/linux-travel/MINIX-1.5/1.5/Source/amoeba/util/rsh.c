/*	rsh [-ei] machine command
 *
 *	-b:	run remote process in background
 *	-e:	separate error output
 *	-i:	send input
 */

#include <stdio.h>
#include <signal.h>
#include "amoeba.h"
#include "sherver.h"

char *prog, *mach, buf[BUFFERSIZE];

usage(){
	fprintf(stderr, "Usage: %s [-bei] machine command ...\n", prog);
	exit(1);
}

panic(s)
char *s;
{
	fprintf(stderr, "%s: %s\n", prog, s);
	exit(1);
}

char *collect(buf, vec, sep)
char *buf, **vec, sep;
{
	register char *p;

	while ((p = *vec++) != 0) {
		while (*p != 0) {
			*buf++ = *p++;
			if (buf == &buf[BUFFERSIZE])
				panic("environment too large\n");
		}
		*buf++ = *vec == 0 ? 0 : sep;
	}
	return(buf);
}

char *setupbuf(argv, flags)
char **argv;
int *flags;
{
	register char *p;
	extern char **environ, *getenv();

	if (*argv == 0) {
		if ((p = getenv("SHELL")) == 0)
			p = "/bin/sh";
		if (strlen(p) > BUFFERSIZE - 10) /* 10 > strlen("exec  -i") */
			panic("pathname of shell too large");
		sprintf(buf, "exec %s -i", p);
		p = &buf[strlen(buf) + 1];
		*flags |= IFLG;
	}
	else
		p = collect(buf, argv, ' ');
	return collect(p, environ, 0);
}

execute(hdr, p, flags)
header *hdr;
char *p;
{
	register unshort cnt;

	timeout(50);
	strncpy(&hdr->h_port, mach, PORTSIZE);
	hdr->h_command = EXEC_COMMAND;
	hdr->h_size = p - buf;
	hdr->h_extra = flags;
	cnt = trans(hdr, buf, p - buf, hdr, buf, BUFFERSIZE);
	if ((short) cnt < 0) {
		fprintf(stderr, "%s: %s not available\n", prog, mach);
		return(0);
	}
	if (hdr->h_status != COMM_DONE) {
		if (cnt == 0)
			fprintf(stderr, "can't execute command\n");
		else
			fprintf(stderr, "%.*s\n", cnt, buf);
		return(0);
	}
	return(1);
}

alrm(){
	fprintf(stderr, "rsh: getreq timed out\n");
	exit(1);
}

commandloop(hdr)
header *hdr;
{
	register unshort cnt;

	for (;;) {
		alarm(12*60*60);
		cnt = getreq(hdr, buf, BUFFERSIZE);
		alarm(0);
		if ((short) cnt < 0) {
			fprintf(stderr, "getreq failed\n");
			return(-1);
		}
		switch (hdr->h_command) {
		case READ_FD:
			if (hdr->h_extra < NFD)
				cnt = read(hdr->h_extra, buf, hdr->h_size);
			else
				fprintf(stderr, "read: bad fd\n");
			hdr->h_size = (unshort) cnt;
			putrep(hdr, buf, cnt < 0 ? 0 : cnt);
			if (cnt == 0)
				return(0);
			break;
		case WRITE_FD:
			if (hdr->h_extra < NFD)
				if (cnt == 0)
					close(hdr->h_extra);
				else
					write(hdr->h_extra, buf, cnt);
			else
				fprintf(stderr, "write: bad fd\n");
			putrep(hdr, NILBUF, 0);
			break;
		case EXIT_STATUS:
			if (cnt != 0) {
				write(2, "Remote message: ", 16);
				write(2, buf, cnt);
				write(2, "\n", 1);
			}
			putrep(hdr, NILBUF, 0);
			return(hdr->h_extra >> 8);
			break;
		default:
			fprintf(stderr, "unknown command\n");
			putrep(hdr, NILBUF, 0);
		}
	}
}

setupin(hdr)
header *hdr;
{
	register pid;

	uniqport(&hdr->h_priv.prv_random);
	if ((pid = fork()) < 0) {
		perror(prog);
		exit(-1);
	}
	if (pid == 0) {
		hdr->h_port = hdr->h_priv.prv_random;
		_exit(commandloop(hdr));   /* get read commands from sherver */
	}
	return(pid);
}

main(argc, argv)
char **argv;
{
	char *p;
	header hdr;
	register pid, status;
	int flags;

#ifdef sun
	put_user_name_in_env();
#endif
	prog = *argv++;
	if (argc == 1)
		usage();
	signal(SIGALRM, alrm);
	while (*argv != 0 && **argv == '-') {
		while (*++*argv)
			switch (**argv) {
			case 'b':	flags |= BFLG;		break;
			case 'e':	flags |= EFLG;		break;
			case 'i':	flags |= IFLG;		break;
			default:	usage();
			}
		argv++;
	}
	if ((mach = *argv++) == 0)
		usage();
	chkperm(mach);
	p = setupbuf(argv, &flags);	/* get command and environment */
	if (flags & IFLG)		/* set up input process */
		pid = setupin(&hdr);
	if (!execute(&hdr, p, flags)) {	/* send command to sherver */
		if (flags & IFLG)
			kill(pid, SIGKILL);
		exit(1);
	}
	if (!(flags & BFLG))		/* get commands from sherver */
		status = commandloop(&hdr);
	if (flags & IFLG) {		/* wait for input process */
		close(0); close(1); close(2);
		while (wait((int *) 0) != pid)
			;
	}
	exit(status);
}

chkperm(svport)
char *svport;
{
	static char chkfil[] = "/etc/XXXXXXXXXXXXXX";
	extern char *strchr();

	sprintf(strchr(chkfil, 'X'), "rsh:%.14s", svport);
	if (chkprot(chkfil, "/etc/rsh:default", geteuid(), getegid()) == 0)
		panic("permission denied");
}

char *strchr(s, c)
register char *s;
{
	while (*s) {
		if (*s == c)
			return s;
		s++;
	}
	return 0;
}

#ifdef sun
#include <pwd.h>

put_user_name_in_env()
{
	extern struct passwd *getpwuid();
	register struct passwd *pw;
	static char buf[512];
	char host[10];

	gethostname(host, sizeof host);
	pw = getpwuid(getuid());
	sprintf(buf, "UserName=%s@%s", pw ? pw->pw_name : "???", host);
	putenv(buf);
}
#endif sun
