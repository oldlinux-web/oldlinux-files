#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include "amoeba.h"
#include "sherver.h"

/* isn't minix wonderful */
#undef	BUFFERSIZE
#define	BUFFERSIZE	8192

#define COMPATIBLE			/* with the new rsh */

#define MAXWAIT	(4*60*60)		/* maximum waiting time in seconds */
#define Secs *10
#define LOCATETIMER	30 Secs

#ifdef DEBUG
char *
portstr(p) port *p; {
	static char strbuf[80];

	sprintf(strbuf,"%x:%x:%x:%x:%x:%x",
		p->_portbytes[0]&0xFF,
		p->_portbytes[1]&0xFF,
		p->_portbytes[2]&0xFF,
		p->_portbytes[3]&0xFF,
		p->_portbytes[4]&0xFF,
		p->_portbytes[5]&0xFF);
	return strbuf;
}
#endif

header hdr;
char *prog, buf[BUFFERSIZE], *envvec[MAXENV], *errstr;
int pidlist[NFD];
extern errno;
static char *sig[] = {
	"signal 0",
	"hangup",
	"interrupt",
	"quit",
	"illegal instruction",
	"trace/BPT trap",
	"IOT trap",
	"EMT trap",
	"floating exception",
	"kill",
	"bus error",
	"memory fault",
	"bad system call",
	"broken pipe",
	"alarm call",
	"terminate",
	"urgent socket",
	"stop (signal)",
	"stop",
	"continue",
	"child exited",
	"stop (tty input)",
	"stop (tty output)",
	"tty input interrupt",
	"cputime limit exceeded",
	"filesize limit exceeded",
	"virtual time alarm",
	"profiling timer alarm",
};

panic(s, arg)
char *s;
{
	register FILE *f;
	FILE *	fopen();

#ifdef DEBUG
#define f stderr
#else
	if (f = fopen("/dev/tty0", "w")) {
#endif DEBUG
		fprintf(f, "%s: ", prog);
		fprintf(f, s, arg);
		fprintf(f, "\n");
#ifdef DEBUG
#undef f
#else
		fclose(f);
	}
#endif DEBUG
	_exit(1);
}

alrm(){		/* set alarm again to avoid races */
	signal(SIGALRM, alrm);
	alarm(MAXWAIT);
}

killall(){	/* kill all the created processes */
	register n;

	for (n = 0; n < NFD; n++)
		if (pidlist[n] != 0)
			kill(pidlist[n], SIGKILL);
}

trap(){		/* got SIGAMOEBA */
	killall();
	_exit(0);
}

copyloop(pp, rfd, wfd)
port *pp;
{
	register unshort cnt;
	register n;

	do {
		alarm(MAXWAIT);
		n = read(rfd, buf, BUFFERSIZE);
		alarm(0); alarm(0);
		if (n < 0) {
			strcpy(buf, errno==EINTR ? "waited too long for input"
						 : "read error");
			killall();
			return(1);
		}
		hdr.h_port = *pp;
		hdr.h_command = WRITE_FD;
		hdr.h_size = n;
		hdr.h_extra = wfd;
		cnt = trans(&hdr, buf, (unshort) n, &hdr, NILBUF, 0);
		if (cnt != 0)
			panic("trans failed (%d iso 0)", (short) cnt);
	} while (n != 0);
	return(0);
}

setupin(hdr)
header *hdr;
{
	int fd, p[2];
	register unshort cnt;
	port svport;

	if (pipe(p) < 0)
		errstr = "can't pipe";
	else if (p[0] != 0)
		errstr = "pipe returned wrong fd";
	else if ((pidlist[0] = fork()) < 0)
		errstr = "can't fork";
	else if (pidlist[0] == 0) {
		close(p[0]);
		signal(SIGPIPE, SIG_IGN);
#ifdef COMPATIBLE
		if (!NullPort(&hdr->h_signature))
			svport = hdr->h_signature;
		else
#endif
		svport = hdr->h_priv.prv_random;
		for (;;) {
			hdr->h_port = svport;
			hdr->h_command = READ_FD;
			hdr->h_size = BUFFERSIZE;
			hdr->h_extra = 0;
#ifdef DEBUG
			fprintf(stderr,"Using read port %s\n",portstr(&hdr->h_port));
#endif
			cnt = trans(hdr, NILBUF, 0, hdr, buf, BUFFERSIZE);
			if ((short) cnt < 0)
				panic("read trans failed (%d)", (short) cnt);
			if (cnt == 0)
				_exit(0);
			if (write(p[1], buf, cnt) != cnt) {
				timeout(300);
				hdr->h_port = svport;
				hdr->h_command = EXIT_STATUS;
				hdr->h_extra = 0x100;
				if ((cnt = trans(hdr, NILBUF, 0, hdr, NILBUF, 0)) != 0)
					panic("trans failed (%d iso 0)", (short) cnt);
				_exit(1);
			}
		}
	}
	else
		close(p[1]);
}

setuperr(){
	int errpipe[2];

	if (pipe(errpipe) < 0) {
		errstr = "can't pipe";
		return(-1);
	}
	if ((pidlist[2] = fork()) < 0) {
		close(errpipe[0]);
		close(errpipe[1]);
		errstr = "can't fork";
		return(-1);
	}
	if (pidlist[2] == 0) {
		close(errpipe[1]);
		timeout(LOCATETIMER);
		_exit(copyloop(&hdr.h_port, errpipe[0], 2));
	}
	close(errpipe[0]);
	return(errpipe[1]);
}

splitup(size){
	register char *p = buf, *top = &buf[size], **vec = envvec;

	while (p != top) {
		if (vec == &envvec[MAXENV]) {
			errstr = "too much environment";
			return(0);
		}
		*vec++ = p;
		while (*p++ != 0)
			if (p == top) {
				errstr = "bad format";
				return(0);
			}
	}
	*vec = 0;
	return(1);
}

runit(outpipe, errout)
int outpipe[2];
{
	register fd;
	register char *homedir;
	extern char **environ, *getenv();

	close(1); close(2);
	if (hdr.h_extra & BFLG) {
		if ((fd = open("/dev/null", 1)) != 1)
			panic("open returned wrong fd (%d iso 1)", fd);
		else if ((fd = dup(1)) != 2)
			panic("dup returned wrong fd (%d iso 2)", fd);
	}
	else {
		close(outpipe[0]);	/* outpipe[0] >= 1 */
		if ((fd = dup(outpipe[1])) != 1)
			panic("dup returned wrong fd (%d iso 1)", fd);
		if (hdr.h_extra & EFLG) {
			if ((fd = dup(errout)) != 2)
				panic("dup returned wrong fd (%d iso 2)", fd);
			else
				close(errout);
		}
		else if ((fd = dup(1)) != 2)
			panic("dup returned wrong fd (%d iso 2)", fd);
	}
	for (fd = 3; fd < 20; fd++)
		close(fd);
#ifdef LOG
	{
#include <fcntl.h>
		register int logfd;
		
		logfd = open("/usr/tmp/rsh.log", O_WRONLY|O_APPEND|O_CREAT, 0244);
		if (logfd >= 0) {
#include <time.h>
			register char *s;
			register struct tm *tm;
			long t;
			char buf[1024];		/* had better be large enough */
			static char *months[] = {
				"Jan", "Feb", "Mar", "Apr", "May", "Jun",
				"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
			};

			time(&t);
			tm = localtime(&t);
			environ = &envvec[1];
			s = getenv("UserName");
			if (s == 0)
				s = getenv("HOME");
			if (s == 0)
				s = getenv("LOGNAME");
			if (s == 0)
				s = getenv("USER");
			sprintf(buf, "%02d-%s-%02d %02d:%02d:%02d %s\t%s%s\n",
				tm->tm_mday, months[tm->tm_mon], tm->tm_year,
				tm->tm_hour, tm->tm_min, tm->tm_sec,
				s?s:"???", envvec[0],
				hdr.h_extra & IFLG ? " (interactive)" : "");
			lseek(logfd, 0L, 2);
			write(logfd, buf, strlen(buf));
			close(logfd);
		}
	}
#endif
	environ = &envvec[1];
	if ((homedir = getenv("HOME")) == 0 || chdir(homedir) < 0)
		chdir("/");
	execl("/bin/sh", "sh", "-c", envvec[0], (char *) 0);
	panic("can't execute shell", 0);
}

execomm(size){
	int outpipe[2], errout;

	if (!splitup(size))
		return(-1);
	if (!(hdr.h_extra & BFLG)) {
		if ((hdr.h_extra & EFLG) && (errout = setuperr()) < 0)
			return(-1);
		if (pipe(outpipe) < 0) {
			if (hdr.h_extra & EFLG)
				close(errout);
			errstr = "can't pipe";
			return(-1);
		}
	}
	if ((pidlist[1] = fork()) < 0) {
		if (!(hdr.h_extra & BFLG)) {
			if (hdr.h_extra & EFLG)
				close(errout);
			close(outpipe[0]);
			close(outpipe[1]);
		}
		errstr = "can't fork";
		return(-1);
	}
	if (pidlist[1] == 0) 
		runit(outpipe, errout);
	if (hdr.h_extra & BFLG)
		return(0);
	if (hdr.h_extra & EFLG)
		close(errout);
	close(outpipe[1]);
	return(outpipe[0]);
}

awaitchild(hdr, status)
header *hdr;
{
	register pid, n;
	int wstat;

	if (status == 0) {
		alarm(MAXWAIT);
		while ((pid = wait(&wstat)) > 0) {
			if (pid == pidlist[1]) {
				pidlist[1] = 0;
				hdr->h_extra = status;
			}
			status = wstat;
		}
		alarm(0); alarm(0);
		if (pidlist[1] != 0) {
			strcpy(buf, errno == EINTR ? "waited too long"
						   : "wait error");
			killall();
			hdr->h_extra = 0x100;
		}
		else if (status & 0xFF)
			if ((status & 0x7F) < sizeof(sig) / sizeof(sig[0]))
				strcpy(buf, sig[status & 0x7F]);
			else
				sprintf(buf, "signal %d", status & 0x7F);
		else
			buf[0] = 0;
		if (status & 0x80)
			strcat(buf, " - core dumped");
	}
	else
		hdr->h_extra = status << 8;
	timeout(300);
	hdr->h_command = EXIT_STATUS;
	if ((n = trans(hdr, buf, strlen(buf), hdr, NILBUF, 0)) != 0)
		panic("trans failed (%d iso 0)", (short) n);
}

main(argc, argv)
char **argv;
{
	register unshort cnt;
	register fd;
	unshort getreq(), putrep();

	prog = argv[0];
	if (argc != 2)
		panic("usage: sherver machine", 0);
	signal(SIGALRM, alrm);
	signal(SIGAMOEBA, trap);
	strncpy(&hdr.h_port, argv[1], PORTSIZE);
/*cbo*/printf("before first getreq\n");
	cnt = getreq(&hdr, buf, BUFFERSIZE);
/*cbo*/printf("after first getreq\n");
	if ((short) cnt < 0)
		panic("getreq failed (%d)", (short) cnt);
	if (hdr.h_size == cnt) {
		close(0);
		if (hdr.h_extra & IFLG)
			setupin(&hdr);
		else if (open("/dev/null", 0) != 0)
			errstr = "can't open /dev/null";
		if (errstr == 0) {
			uniqport(&hdr.h_port);
			fd = execomm(cnt);
		}
	}
	else
		errstr = hdr.h_size > BUFFERSIZE ? "request too large" :
								"bad format";
	if (errstr == 0) {
		hdr.h_status = COMM_DONE;
		putrep(&hdr, NILBUF, 0);
		if (!(hdr.h_extra & BFLG)) {
			timeout(LOCATETIMER);
			awaitchild(&hdr, copyloop(&hdr.h_port, fd, 1));
		}
	}
	else {
		hdr.h_status = COMM_ERROR;
		putrep(&hdr, errstr, strlen(errstr));
	}
}
