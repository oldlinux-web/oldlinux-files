/* simpleinit.c - poe@daimi.aau.dk */

#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#define inline
#include <string.h>
#undef inline
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#include "pathnames.h"

#define CMDSIZ 150
#define NUMCMD 30
#define NUMTOK 20

#define RUN_RC

#define ever (;;)

struct initline {
	pid_t	pid;
	char	tty[10];
	char	termcap[30];
	char	*toks[NUMTOK];
	char	line[CMDSIZ];
};

struct initline inittab[NUMCMD];
int numcmd;
int stopped = 0;	/* are we stopped */

void do_rc(), spawn(), hup_handler(), read_inittab();
void tstp_handler(), int_handler();

void err(char *s)
{
	int fd;
	
	if((fd = open("/dev/console", O_WRONLY)) < 0) return;

	write(fd, "init: ", 6);	
	write(fd, s, strlen(s));
	close(fd);
}

main()
{
	int vec,i;
	pid_t	pid;
	
#ifdef RUN_RC
	do_rc();
#endif
	for(i = 0; i < NUMCMD; i++)
		inittab[i].pid = -1;

	read_inittab();

#ifdef DEBUGGING
	for(i = 0; i < numcmd; i++) {
		char **p;
		p = inittab[i].toks;
		printf("toks= %s %s %s %s\n",p[0], p[1], p[2], p[3]);
		printf("tty= %s\n", inittab[i].tty);
		printf("termcap= %s\n", inittab[i].termcap);
	}
	exit(0);
#endif
	signal(SIGHUP, hup_handler);
	signal(SIGTSTP, tstp_handler);
	signal(SIGINT, int_handler);
	
	for(i = 0; i < numcmd; i++)
		spawn(i);
	
	for ever {
		pid = wait(&vec);

		for(i = 0; i < numcmd; i++) {
			if(pid == inittab[i].pid || inittab[i].pid < 0) {
				if(stopped) inittab[i].pid = -1;
				else spawn(i);
				break;
			}
		}
	}
}	

void do_rc()
{
	pid_t pid;
	int stat;
	
	if((pid = fork()) == 0) {
		/* the child */
		char *argv[2];
		
		argv[0] = _PATH_BSHELL;
		argv[1] = (char *)0;

		close(0);
		if(open(_PATH_RC, O_RDONLY, 0) == 0) {
			execv(_PATH_BSHELL, argv);
			err("exec rc failed\n");
			_exit(2);
		}
		err("open of rc file failed\n");
		_exit(1);
	} else if(pid > 0) {
		/* parent, wait till rc process dies before spawning */
		while(wait(&stat) != pid) /* nothing */;
	}
}

void spawn(int i)
{
	pid_t pid;
	int j;
	
	if((pid = fork()) < 0) {
		inittab[i].pid = -1;
		err("fork failed\n");
		return;
	}
	if(pid) {
		/* this is the parent */
		inittab[i].pid = pid;
		return;
	} else {
		/* this is the child */
		char term[40];
		char *env[2];
		
		setsid();
		for(j = 0; j < getdtablesize(); j++)
			(void) close(j);

		(void) sprintf(term, "TERM=%s", inittab[i].termcap);
		env[0] = term;
		env[1] = (char *)0;

		execve(inittab[i].toks[0], inittab[i].toks, env);
		err("exec failed\n");
		sleep(5);
		_exit(1);
	}
}

void read_inittab()
{
	FILE *f;
	char buf[CMDSIZ];
	int i,j;
	char *ptr, *getty;
	char tty[50];
	struct stat stb;
	char *termenv, *getenv();
	
	termenv = getenv("TERM");	/* set by kernel */
	/* termenv = "vt100"; */
			
	if(!(f = fopen(_PATH_INITTAB, "r"))) {
		err("cannot open inittab\n");
		_exit(1);
	}

	i = 0;
	while(!feof(f) && i < NUMCMD - 2) {
		if(fgets(buf, CMDSIZ - 1, f) == 0) break;
		if(buf[0] == '#' || buf[0] == '\n') continue;

		(void) strcpy(inittab[i].line, buf);

		(void) strtok(inittab[i].line, ":");
		(void) strncpy(inittab[i].tty, inittab[i].line, 9);
		(void) strncpy(inittab[i].termcap,
				strtok((char *)0, ":"), 29);

		getty = strtok((char *)0, ":");
		(void) strtok(getty, " \t\n");
		inittab[i].toks[0] = getty;
		j = 1;
		while(ptr = strtok((char *)0, " \t\n"))
			inittab[i].toks[j++] = ptr;
		inittab[i].toks[j] = (char *)0;

		/* special-case termcap for the console ttys */
		(void) sprintf(tty, "/dev/%s", inittab[i].tty);
		if(!termenv || stat(tty, &stb) < 0) {
			err("no TERM or cannot stat tty\n");
		} else {
			/* is it a console tty? */
			if(major(stb.st_rdev) == 4 && minor(stb.st_rdev) < 64)
				strncpy(inittab[i].termcap, termenv, 29);
		}
		i++;
	}
	fclose(f);
	numcmd = i;
}

void hup_handler()
{
	int i,j;
	int oldnum;
	struct initline	savetab[NUMCMD];
	int had_already;

	(void) signal(SIGHUP, SIG_IGN);

	memcpy(savetab, inittab, NUMCMD * sizeof(struct initline));
	oldnum = numcmd;		
	read_inittab();
	
	for(i = 0; i < numcmd; i++) {
		had_already = 0;
		for(j = 0; j < oldnum; j++) {
			if(!strcmp(savetab[j].tty, inittab[i].tty)) {
				had_already = 1;
				if((inittab[i].pid = savetab[j].pid) < 0)
					spawn(i);
			}
		}
		if(!had_already) spawn(i);
	}
	
	(void) signal(SIGHUP, hup_handler);
}

void tstp_handler()
{
	stopped = ~stopped;
	if(!stopped) hup_handler();

	signal(SIGTSTP, tstp_handler);
}

void int_handler()
{
	/*
	 * After Linux 0.96b PL1, we get a SIGINT when
	 * the user presses Ctrl-Alt-Del...
	 */

	int pid;
	
	sync();
	sync();
	if((pid = fork()) == 0) {
		/* reboot properly... */
		execl(_PATH_REBOOT, _PATH_REBOOT, (char *)0);
		reboot(0xfee1dead, 672274793, 0x1234567);
	} else if(pid < 0)
		/* fork failed, try the hard way... */
		reboot(0xfee1dead, 672274793, 0x1234567);
}
