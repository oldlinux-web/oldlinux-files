/* OS- and machine-dependent stuff for 4.[23] BSD UNIX */

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
	
	If you want to use the select code, define SELECT in the makefile or
	in this file.
*/

#include <stdio.h>
#include <sys/types.h>
#include <sgtty.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/dir.h>
#include <sys/time.h>
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
struct sgttyb mysavetty, savecon;
int	IORser[ASY_MAX];


/* Called at startup time to set up console I/O, memory heap */
ioinit()
{
	struct sgttyb ttybuf;
	extern void ioexit();

	(void) signal(SIGHUP, ioexit);
	(void) signal(SIGINT, ioexit);
	(void) signal(SIGQUIT, ioexit);
	(void) signal(SIGTERM, ioexit);

	ioctl(0,TIOCGETP,&ttybuf);
	savecon = ttybuf;
	ttybuf.sg_flags &= ~ECHO;
	ttybuf.sg_flags |= CBREAK;
	ioctl(0,TIOCSETP,&ttybuf);
}
/* Called just before exiting to restore console state */
void
iostop()
{
	setbuf(stdout,NULLCHAR);

	while(ifaces != NULLIF){
		if(ifaces->stop != NULLFP)
			(*ifaces->stop)(ifaces->dev);
		ifaces = ifaces->next;
	}

	ioctl(0,TIOCSETP,&savecon);
}

void
ioexit()
{
	iostop();
	exit(0);
}

/* Initialize asynch port "dev" */
/*ARGSUSED*/
int slipisopen;
/*ARGSUSED*/
int
asy_init(dev,arg1,arg2,bufsize)
int16 dev;
char *arg1,*arg2;
unsigned bufsize;
{
	register struct asy *ap;
	extern struct interface *ifaces;
	struct sgttyb   sgttyb;

	ap = &asy[dev];

	if (ap == NULL || dev >= nasy)
		return(-1);

	ap->tty = malloc((unsigned)(strlen(arg2)+1));
	strcpy(ap->tty, arg2);

	printf("asy_init: tty name = %s\n", ap->tty);
	

	if ((IORser[dev] = open (ap->tty, (O_RDWR), 0)) < 0)
	{
		perror ("Could not open device IORser");
		return (-1);
	}
 /* 
  * get the stty structure and save it 
  */

	if (ioctl (IORser[dev], TIOCGETP, &mysavetty) < 0)
	{
		perror ("ioctl failed on device");
		return (-1);
	}
 /* 
  * copy over the structure 
  */

	sgttyb = mysavetty;
	sgttyb.sg_flags = (RAW | ANYP | CRMOD);
	sgttyb.sg_ispeed = sgttyb.sg_ospeed = B9600;

 /* 
  * now set the modes and flags 
  */
	if (ioctl (IORser[dev], TIOCSETP, &sgttyb) < 0)
	{
		perror ("ioctl could not set parameters for IORser");
		return (-1);
	}

	return 0;
}

/*ARGSUSED*/
int
asy_stop(iface)
struct interface *iface;
{
}

/* Set asynch line speed */
int
asy_speed(dev,speed)
int16 dev;
int speed;
{
	struct sgttyb sgttyb;

	if(speed == 0 || dev >= nasy)
		return(-1);
	asy[dev].speed = speed;

	if (ioctl (IORser[dev], TIOCGETP, &sgttyb) < 0)
	{
		perror ("ioctl could not get parameters");
		return (-1);
	}
	switch(speed)
	{
		case 0:
			sgttyb.sg_ispeed = sgttyb.sg_ospeed = B0;
			break;
		case 50:
			sgttyb.sg_ispeed = sgttyb.sg_ospeed = B50;
			break;
		case 75:
			sgttyb.sg_ispeed = sgttyb.sg_ospeed = B75;
			break;
		case 110:
			sgttyb.sg_ispeed = sgttyb.sg_ospeed = B110;
			break;
		case 134:
			sgttyb.sg_ispeed = sgttyb.sg_ospeed = B134;
			break;
		case 150:
			sgttyb.sg_ispeed = sgttyb.sg_ospeed = B150;
			break;
		case 200:
			sgttyb.sg_ispeed = sgttyb.sg_ospeed = B200;
			break;
		case 300:
			sgttyb.sg_ispeed = sgttyb.sg_ospeed = B300;
			break;
		case 600:
			sgttyb.sg_ispeed = sgttyb.sg_ospeed = B600;
			break;
		case 1200:
			sgttyb.sg_ispeed = sgttyb.sg_ospeed = B1200;
			break;
		case 1800:
			sgttyb.sg_ispeed = sgttyb.sg_ospeed = B1800;
			break;
		case 2400:
			sgttyb.sg_ispeed = sgttyb.sg_ospeed = B2400;
			break;
		case 4800:
			sgttyb.sg_ispeed = sgttyb.sg_ospeed = B4800;
			break;
		case 9600:
			sgttyb.sg_ispeed = sgttyb.sg_ospeed = B9600;
			break;
		case 19200:
			sgttyb.sg_ispeed = sgttyb.sg_ospeed = B19200;
			break;
		case 38400:
			sgttyb.sg_ispeed = sgttyb.sg_ospeed = B38400;
			break;
		default:
			printf("asy_speed: Unknown speed (%d)\n", speed);
			break;
	}
	if (ioctl (IORser[dev], TIOCSETP, &sgttyb) < 0) {
		perror ("ioctl could not set parameters for IORser");
		return (-1);
	}
	return(0);
}
/* Send a buffer to serial transmitter */
asy_output(dev,buf,cnt)
unsigned dev;
char *buf;
unsigned short cnt;
{
	if(dev >= nasy)
		return(-1);
	if ( write(IORser[dev], buf, (int)cnt) < cnt)
	{
		perror("asy_output");
		printf("asy_output: error in writing to device %d\n", dev);
		return(-1);
	}
	return(0);
}
/* Receive characters from asynch line
 * Returns count of characters read
 */
unsigned
asy_recv(dev,buf,cnt)
int dev;
char *buf;
unsigned cnt;
{
#define IOBUFLEN	256
	unsigned tot;
	long amount;
	int r;
	static struct {
		char buf[IOBUFLEN];
		char *data;
		int cnt;
	} IOBUF[ASY_MAX];

#ifdef SELECT
	int	mask;
	int	writemask;
	int	ok;
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 35;
	mask = (1<<IORser[dev]);
	writemask = (1<<IORser[dev]);
	ok = 0;
	tot = 0;
	ok = select(mask, &mask, 0, 0, &timeout);
	if ( mask & (1<<IORser[dev]))
	{
		tot = read(IORser[dev], buf, cnt);
	}
	return (tot);
#else
	tot = 0;
	/* fill the read ahead buffer */
        if(IOBUF[dev].cnt == 0) {
                IOBUF[dev].data = IOBUF[dev].buf;
		amount = 0;
		if ((r = ioctl( IORser[dev], FIONREAD, &amount)) != -1) {
			if ( amount > 0)
				r = read(IORser[dev], buf, (int)cnt);
			else
				r = 0;
		}
                r = read(IORser[dev], IOBUF[dev].data, IOBUFLEN);
                /* check the read */
                if (r == -1) {
                        IOBUF[dev].cnt = 0;     /* bad read */
                        perror("asy_recv");
                        printf("asy_recv: error in reading from device %d\n", dev);
                        return(0);
                } else
                        IOBUF[dev].cnt = r;
        }
        r = 0;  /* return count */
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
#endif SELECT
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
        extern int      errno;
        int             pty;
        int             letcnt=0, numcnt=0;

        static char     *letters = "pqrs",
                        *numbers = "0123456789abcdef";
        static char master[] = "/dev/ptyXX";
        static int      letmax, nummax;

        letmax=strlen(letters)-1, nummax=strlen(numbers)-1;
        do {
                master[strlen("/dev/pty")] = letters[letcnt];
                master[strlen("/dev/ptyX")] = numbers[numcnt];
                if (letcnt > letmax) {
                        return -1;
                } else if (++numcnt > nummax) {
                        letcnt++;
                        numcnt = 0;
                }
        } while ((pty=open(master, O_RDWR)) < 0);

        return(pty);
}
