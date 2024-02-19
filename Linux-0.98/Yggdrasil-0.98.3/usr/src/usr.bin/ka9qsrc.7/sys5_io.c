/* OS- and machine-dependent stuff for SYS5 */

/*
	FILE: UNIX.io.c
	
	Routines:
		ioinit()
		iostop()
		asy_init()
		asy_stop()
		asy_speed()
		asy_output()
		asy_recv()
		dir()
	Written or converted by Mikel Matthews, N9DVG
	SYS5 added by Jere Sandidge, K4FUM
	Directory pipe added by Ed DeHart, WA3YOA
	Numerous changes by Charles Hedrick and John Limpert, N3DMC
	Hell, *I* even hacked on it... :-)  Bdale, N3EUA
	
	If you want to use the select code, define SELECT in the makefile or
	in this file.
*/

#include <stdio.h>
#include <sys/types.h>
#include <termios.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/dir.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <memory.h>
#include <string.h>
#include "global.h"
#include "asy.h"
#include "mbuf.h"
#include "internet.h"
#include "iface.h"
#include "unix.h"
#include "cmdparse.h"

#ifndef	B19200
#define	B19200	EXTA
#endif

#ifndef	B38400
#define	B38400	EXTB
#endif

struct asy asy[ASY_MAX];
struct interface *ifaces;
struct termios mysavetty, savecon;
int savettyfl;
int	IORser[ASY_MAX];
extern unsigned long selmask;
extern unsigned long mainmask;
extern long waittime;

/* Called at startup time to set up console I/O, memory heap */
ioinit()
{
	struct termios ttybuf;
	extern void ioexit();

	(void) signal(SIGHUP, ioexit);
	(void) signal(SIGINT, ioexit);
	(void) signal(SIGQUIT, ioexit);
	(void) signal(SIGTERM, ioexit);

	ioctl(0, TCGETS, &ttybuf);
	savecon = ttybuf;

	ttybuf.c_iflag &= ~(ICRNL);
	ttybuf.c_lflag &= ~(ICANON|ISIG|ECHO);
	ttybuf.c_cc[VTIME] = '\01';
	ttybuf.c_cc[VMIN] = '\0';
	if ((savettyfl = fcntl(0, F_GETFL, 0)) == -1) {
		perror("Could not read console flags");
		return -1;
	}

/* #ifdef  undef */
	fcntl(0, F_SETFL, savettyfl | O_NDELAY);
/* #endif */

	ioctl(0, TCSETSW, &ttybuf);
	return 0;
}


/* Called just before exiting to restore console state */
void
iostop()
{
	setbuf(stdout,NULLCHAR);

	while (ifaces != NULLIF) {
		if (ifaces->stop != NULLFP)
			(*ifaces->stop)(ifaces);
		ifaces = ifaces->next;
	}

	ioctl(0, TCSETSW, &savecon);
	fcntl(0, F_SETFL, savettyfl);
}

void
ioexit()
{
	iostop();
	exit(0);
}

void
clearout()
{
	fflush(stdout);
	ioctl(1, TCFLSH, 1);
}

void
flowon()
{
	struct termios ttybuf;
	ioctl(0, TCGETS, &ttybuf);
	ttybuf.c_iflag |= IXON;
	ioctl(0, TCSETSW, &ttybuf);
}

void
flowoff()
{
	struct termios ttybuf;
	ioctl(0, TCGETS, &ttybuf);
	ttybuf.c_iflag &= ~IXON;
	ioctl(0, TCSETSW, &ttybuf);
}

void
flowdefault()
{
	struct termios ttybuf;
	ioctl(0, TCGETS, &ttybuf);
	if (savecon.c_iflag & IXON)
		ttybuf.c_iflag |= IXON;
	else
		ttybuf.c_iflag &= ~IXON;
	ioctl(0, TCSETSW, &ttybuf);
}

void
set_stdout(mode)
	int mode;
{
	struct termios ttybuf;
	ioctl(0, TCGETS, &ttybuf);
	if (mode == 2)  /* cooked */
		ttybuf.c_oflag |= ONLCR;
	else
		ttybuf.c_oflag &= ~ONLCR;
	ioctl(0, TCSETSW, &ttybuf);
}

/* Initialize asynch port "dev" */
/*ARGSUSED*/
int
asy_init(dev, arg1, arg2, bufsize)
int16 dev;
char *arg1, *arg2;
unsigned bufsize;
{
	register struct asy *ap;
	struct termios	sgttyb;

#ifdef	SYS5_DEBUG
	printf("asy_init: called\n");
#endif	/* SYS5_DEBUG */

	if (dev >= nasy)
		return -1;

	ap = &asy[dev];
	ap->tty = malloc((unsigned)(strlen(arg2)+1));
	strcpy(ap->tty, arg2);
	printf("asy_init: tty name = %s\n", ap->tty);

	if ((IORser[dev] = open(ap->tty, (O_RDWR|O_NDELAY), 0)) < 0) {
		perror("Could not open device IORser");
		return -1;
	}
	selmask |= (1 << IORser[dev]);

 /* 
  * get the stty structure and save it 
  */

	if (ioctl(IORser[dev], TCGETS, &mysavetty) < 0)	{
		perror("ioctl failed on device");
		return -1;
	}

 /* 
  * copy over the structure 
  */

	sgttyb = mysavetty;
	sgttyb.c_iflag = (IGNBRK | IGNPAR);
	sgttyb.c_oflag = 0;
	sgttyb.c_lflag = 0;
	sgttyb.c_cflag = (B9600 | CS8 | CREAD);
	sgttyb.c_line = 1;  /* special SLIP line discipline */
	sgttyb.c_cc[VEOL] = 0300;
	sgttyb.c_cc[VTIME] = 0;
	sgttyb.c_cc[VMIN] = 0;

	if (ioctl(IORser[dev], TCSETSF, &sgttyb) < 0) {
		perror("ioctl could not set parameters for IORser");
		return -1;
	}

	return 0;
}


/*ARGSUSED*/
/* 
 * The primary reason for this routine is to put the line discipline
 * back to 0.  Few programs think of initializing the line discipline
 */
int
asy_stop(iface)
struct interface *iface;
{
	struct termios	sgttyb;
	int dev = iface->dev;

	if (ioctl(IORser[dev], TCGETS, &sgttyb) < 0)	{
		perror("ioctl failed on device");
		return -1;
	}

	/* put back some reasonable defaults */

	sgttyb.c_line = 0;
	sgttyb.c_cc[VEOL] = 0;
	sgttyb.c_cc[VTIME] = 0;
	sgttyb.c_cc[VMIN] = 1;

	if (ioctl(IORser[dev], TCSETSF, &sgttyb) < 0) {
		perror("ioctl could not set parameters for IORser");
		return -1;
	}
}


/* Set asynch line speed */
int
asy_speed(dev, speed)
int16 dev;
int speed;
{
	struct termios sgttyb;

	if (speed == 0 || dev >= nasy)
		return -1;

#ifdef	SYS5_DEBUG
	printf("asy_speed: Setting speed for device %d to %d\n",dev, speed);
#endif

	asy[dev].speed = speed;

	if (ioctl(IORser[dev], TCGETS, &sgttyb) < 0) {
		perror("ioctl could not get parameters");
		return -1;
	}

	sgttyb.c_cflag &= ~CBAUD;

	switch ((unsigned)speed) {
	case 0:
		sgttyb.c_cflag |= B0;
		break;
	case 50:
		sgttyb.c_cflag |= B50;
		break;
	case 75:
		sgttyb.c_cflag |= B75;
		break;
	case 110:
		sgttyb.c_cflag |= B110;
		break;
	case 134:
		sgttyb.c_cflag |= B134;
		break;
	case 150:
		sgttyb.c_cflag |= B150;
		break;
	case 200:
		sgttyb.c_cflag |= B200;
		break;
	case 300:
		sgttyb.c_cflag |= B300;
		break;
	case 600:
		sgttyb.c_cflag |= B600;
		break;
	case 1200:
		sgttyb.c_cflag |= B1200;
		break;
	case 1800:
		sgttyb.c_cflag |= B1800;
		break;
	case 2400:
		sgttyb.c_cflag |= B2400;
		break;
	case 4800:
		sgttyb.c_cflag |= B4800;
		break;
	case 9600:
		sgttyb.c_cflag |= B9600;
		break;
	case 19200:
		sgttyb.c_cflag |= B19200;
		break;
	case 38400:
		sgttyb.c_cflag |= B38400;
		break;
	default:
		printf("asy_speed: Unknown speed (%d)\n", speed);
		break;
	}

#ifdef	SYS5_DEBUG
	printf("speed = %d\n", sgttyb.sg_ispeed);
#endif	/* SYS5_DEBUG */

	if (ioctl(IORser[dev], TCSETSW, &sgttyb) < 0) {
		perror("ioctl could not set parameters for IORser");
		return -1;
	}

	return 0;
}


/* Send a buffer to serial transmitter */
asy_output(dev, buf, cnt)
unsigned dev;
char *buf;
unsigned short cnt;
{
#ifdef	SYS5_DEBUG
	printf("asy_output called. dev = %d, cnt = %d\n", dev, cnt);
	printf("buf = %lx\n", (long) buf);
	fflush(stdout);
#endif
	
	if (dev >= nasy)
		return -1;

	if (write(IORser[dev], buf, (int)cnt) < cnt) {
		perror("asy_output");
		printf("asy_output: error in writing to device %d\n", dev);
		return -1;
	}

	return 0;
}

/* Receive characters from asynch line
 * Returns count of characters read
 */
int16
asy_recv(dev,buf,cnt)
int16 dev;
char *buf;
unsigned cnt;
{
#define	IOBUFLEN	256
	extern int	errno;
	unsigned tot;
	int r;
	static struct	{
		char	buf[IOBUFLEN];
		char	*data;
		int	cnt;
	}	IOBUF[ASY_MAX];

#ifdef SELECT
	int	mask;
	int	writemask;
	int	ok;
	struct timeval timeout;

	if(IOBUF[dev].cnt == 0) {
	timeout.tv_sec = 0;
	timeout.tv_usec = 35;
	mask = (1<<IORser[dev]) | 1;
	writemask = (1<<IORser[dev]);
	ok = 0;
	tot = 0;
	ok = select(20, &mask, 0, 0, 0);
	}
#ifdef undef
	if ( mask & (1<<IORser[dev]))
	{
		tot = read(IORser[dev], buf, cnt);
	}
	return (tot);
#endif
#endif
	if(IORser[dev] < 0) {
		printf("asy_recv: bad file descriptor passed for device %d\n",
			dev);
		return(0);
	}
	tot = 0;
	/* fill the read ahead buffer */
#ifdef notdef
	if(IOBUF[dev].cnt == 0 && (rdchk(IORser[dev]) > 0)) {
#else
	if(IOBUF[dev].cnt == 0) {
#endif
		IOBUF[dev].data = IOBUF[dev].buf;
		if (waittime && ((mainmask & (1 << IORser[dev])) == 0)) {
			IOBUF[dev].cnt = 0;	/* bad read */
			return(0);
	        }
		r = read(IORser[dev], IOBUF[dev].data, IOBUFLEN);
/*		fprintf(stderr, "X%dX\r\n", r); */
		/* check the read */
		if (r == -1) {
			IOBUF[dev].cnt = 0;	/* bad read */
			if (errno != 11) {  /* ewouldblock */
				perror("asy_recv");
				printf("asy_recv: error in reading from device %d\n", dev);
			}
			return(0);
		} else
			IOBUF[dev].cnt = r;
		
	} 
	r = 0;	/* return count */
	/* fetch what you need with no system call overhead */
	if(IOBUF[dev].cnt > 0) {
		if(cnt == 1) { /* single byte copy, do it here */
			*buf = *IOBUF[dev].data++;
			IOBUF[dev].cnt--;
			r = 1;
		} else { /* multi-byte copy, left memcpy do the work */
			unsigned n = min(cnt, IOBUF[dev].cnt);
			memcpy(buf, IOBUF[dev].data, n);
			IOBUF[dev].cnt -= n;
			IOBUF[dev].data += n;
			r = n;
		}
	}
	tot = (unsigned int) r;
	return (tot);
}

/* Generate a directory listing by opening a pipe to /bin/ls.
 * If full == 1, give a full listing; else return just a list of names.
 */
FILE *
dir(path,full)
char *path;
int full;
{
	FILE *fp;
	char cmd[1024];

	if (path == NULLCHAR || path[0] == '\0')
		path = ".";

#ifdef	SYS5_DEBUG
	printf("DIR: path = %s\n", path);
#endif	/* SYS5_DEBUG */

	if (full)
		sprintf(cmd,"ls -l %s", path);
	else
		sprintf(cmd, "ls %s", path);

	if ((fp = popen(cmd,"r")) == NULLFILE) {
		perror("popen");
		return NULLFILE;
	}

	return fp;
}


asy_ioctl(interface, argc, argv)
struct interface *interface;
int	argc;
char	*argv[];
{
	if (argc < 1) {
		printf("%d\r\n", asy[interface->dev].speed);
		return 0;
	}

	return asy_speed(interface->dev, atoi(argv[0]));
}

int
OpenPty()
{
	extern int	errno;
	int		pty;
	int		letcnt=0, numcnt=0;

	static char	*letters = "pqrs",
			*numbers = "0123456789abcdef";
#ifdef hpux
	static char master[] = "/dev/ptym/ptyXX";
#else
	static char master[] = "/dev/ptyXX";
#endif
	static int	letmax, nummax;

	letmax=strlen(letters)-1, nummax=strlen(numbers)-1;
	do {
#ifdef hpux
		master[strlen("/dev/ptym/pty")] = letters[letcnt];
		master[strlen("/dev/ptym/ptyX")] = numbers[numcnt];
#else
		master[strlen("/dev/pty")] = letters[letcnt];
		master[strlen("/dev/ptyX")] = numbers[numcnt];
#endif
		if (letcnt > letmax) {
			return -1;
		} else if (++numcnt > nummax) {
			letcnt++;
			numcnt = 0;
		} 
	} while ((pty=open(master, O_RDWR | O_NDELAY)) < 0);

	return(pty);
}
