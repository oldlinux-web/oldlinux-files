/*
 *
 *	X server
 *
 *      C. Hedrick, July 1992
 *
 *	Based on Finger server routines.  Written by Michael T. Horne - KA7AXD.
 *	Copyright 1988 by Michael T. Horne, All Rights Reserved.
 *	Permission granted for non-commercial use and copying, provided
 *	that this notice is retained.
 *
 */

#include <stdio.h>
#include "global.h"
#include "config.h"
#include "mbuf.h"
#include "timer.h"
#include "internet.h"
#include "icmp.h"
#include "netuser.h"
#include "tcp.h"
#include "ftp.h"
#include "telnet.h"
#include "iface.h"
#include "ax25.h"
#include "lapb.h"
#include "session.h"
#include "nr4.h"
#ifdef	UNIX
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#endif

#if (defined(BSD) || defined(SUNOS4))
char *sprintf();
#endif

struct xstate {
	struct tcb	*tcb;		/* pointer to associated tcb */
	struct session	*session;	/* pointer to X session */
	int socket;			/* socket for X server */
};
#define	NULLX	(struct xstate *) 0

extern unsigned long xmask;
extern unsigned long selmask;

int xdebug = 1;

struct tcb *x_tcb = NULLTCB;
struct xstate * xstates[20];

void rcv_x();

xserv1(argc, argv)
int	argc;
char	*argv[];
{
	extern int32	ip_addr;
	struct socket	lsocket;
	void		x_state();

	if (x_tcb)
		return;
	/* start X server */
	lsocket.address = ip_addr;
	if(argc < 2)
		lsocket.port = 6000;
	else
		lsocket.port = atoi(argv[1]);
	x_tcb = open_tcp(&lsocket, NULLSOCK, TCP_SERVER, 0, rcv_x,
		NULLVFP, x_state, 0, (char *)NULL);
	return;
}
/*
 *	Handle incoming X connections and closures.
 * 
 */
/*ARGSUSED*/
void
x_state(tcb,old,new)
struct tcb	*tcb;
char		old,		/* old state */
		new;		/* new state */
{
	struct xstate *xpt;
	struct sockaddr_un unaddr;		/* UNIX socket data block */
	struct sockaddr *addr;		/* generic socket pointer */
	struct socket lsocket;
	int addrlen;			/* length of addr */
	int fd;				/* socket file descriptor */
	int retries = 5;

	switch(new){
	case ESTABLISHED:
		log(tcb,"open X");
		xpt = (struct xstate *) malloc(sizeof(struct xstate));

		tcb->user = (char *)xpt;	/* Upward pointer */
		xpt->tcb = tcb;			/* Downward pointer */
		if (xdebug) {
			printf("\007X connection from by %s!\r\n",
		 		psocket(&tcb->conn.remote));
			fflush(stdout);
		}

		unaddr.sun_family = AF_UNIX;
/*		sprintf (unaddr.sun_path, "%s%d", X_UNIX_PATH, idisplay); */
		sprintf (unaddr.sun_path, "%s%d", "/tmp/.X11-unix/X", 0);
		addr = (struct sockaddr *) &unaddr;
		addrlen = strlen(unaddr.sun_path) + sizeof(unaddr.sun_family);

		/*
		 * Open the network connection.
		 */

		do {
		  if ((fd = socket ((int) addr->sa_family, SOCK_STREAM, 0))
		      < 0) {
		    if (xdebug >= 2) {
		      printf("X socket failed\r\n");
		      fflush(stdout);
		    }
		    free(tcb->user);
		    del_tcp(tcb);
		    return;
		  }

		  if (connect (fd, addr, addrlen) < 0) {
		    int olderrno = errno;
		    (void) close (fd);
		    if (olderrno != ENOENT || retries <= 0) {
		      if (xdebug >= 2) {
			printf("X connect failed errno %d retries %d\r\n",
			       olderrno, retries);
			fflush(stdout);
		      }
		      free(tcb->user);
		      del_tcp(tcb);
		      errno = olderrno;
		      return;
		    }
		    sleep (1);
		  } else {
		    break;
		  }
		} while (retries-- > 0);

		fcntl(fd, F_SETFL, O_NDELAY);

		xpt->socket = fd;
		xstates[fd] = xpt;
		xmask |= 1 << fd;
		selmask |= 1 << fd;

		return;
	case CLOSED:
		xpt = (struct xstate *)tcb->user;
		close(xpt->socket);

		if (xpt) {
		  xmask &= ~ (1 << xpt->socket);
		  selmask &= ~ (1 << xpt->socket);
		  free(xpt);
		}
		del_tcp(tcb);
		break;
	}
}

/*
 *	Stop the X server.
 */

xserv0()
{
	if (x_tcb != NULLTCB) {
		close_tcp(x_tcb);
		x_tcb = NULLTCB;
	}
	return;
}

/*
 *	Send a short message on a tcb
 */

static
sndmsg(tcb, msg)
struct tcb	*tcb;		/* tcb to send on */
char		*msg;		/* message to send */
{
	struct mbuf *bp;

	bp = qdata(msg,(int16)strlen(msg));
	send_tcp(tcb,bp);
}

/*
 *	X receive upcall.  This is the guts of the finger server.
 *	The user to finger is read from the socket.  If only a newline
 *	is read, then send the remote host a list of all known 'users' on
 *	this system.
 */

/*ARGSUSED*/
void
rcv_x(tcb, ccnt)
register struct tcb	*tcb;
int16			ccnt;
{
	struct mbuf	*mbuf,
			*bp;
	char		*buf;
	int		cnt;
	int		size;
	char		buffer[512];

	if ((struct xstate *) tcb->user == NULLX)	/* uh oh! */
		return;
	if(recv_tcp(tcb,&bp,sizeof(buffer)) == 0)
		return;

	cnt = pullup(&bp, buffer, sizeof(buffer));	/* input data */
	free_p(bp);				/* all done with bp */

	if (xdebug >= 3) {
	  printf("from net %d\r\n", cnt);
	  fflush(stdout);
	}

	if (cnt > 0)
	  write(((struct xstate *) tcb->user)->socket, buffer, cnt);

	return;
}

send_x(mask)
     unsigned long mask;
{
  int fd;

  if (xdebug >= 4) {
    printf("sendx %o\r\n", mask);
    fflush(stdout);
  }

  for (fd = 0; mask; fd++, mask = mask >> 1)
    if (mask & 1) {
      send_x1 (xstates[fd]);
    }
}

send_x1 (xsp)
     struct xstate *xsp;
{
     char buffer[512];
     struct mbuf *bp;
     int cnt;

     if (xdebug >= 4) {
       printf("sendx1 %d\r\n", xsp->socket);
       fflush(stdout);
     }
     while ((cnt = read(xsp->socket, buffer, sizeof(buffer))) > 0) {
       if (xdebug >= 3) {
	 printf("from X %d\r\n", cnt);
	 fflush(stdout);
       }
       bp = qdata(buffer,cnt);
       send_tcp(xsp->tcb,bp);
     }

}
