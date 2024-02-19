
/*
 *
 *	Finger support...
 *
 *	Finger server routines.  Written by Michael T. Horne - KA7AXD.
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
#include "finger.h"
#include "session.h"
#include "nr4.h"
#ifdef	UNIX
#include <string.h>
#endif

#if (defined(BSD) || defined(SUNOS4))
char *sprintf();
#endif

struct tcb *fing_tcb = NULLTCB;
int16 finger_notify = 1;

finger1(argc, argv)
int	argc;
char	*argv[];
{
	extern int32	ip_addr;
	struct socket	lsocket;
	void		fing_state();
	void		rcv_fing();

	if (fing_tcb)
		return;
	/* start finger daemon */
	lsocket.address = ip_addr;
	if(argc < 2)
		lsocket.port = FINGER_PORT;
	else
		lsocket.port = atoi(argv[1]);
	fing_tcb = open_tcp(&lsocket, NULLSOCK, TCP_SERVER, 0, rcv_fing,
		NULLVFP, fing_state, 0, (char *)NULL);
	return;
}
/*
 *	Handle incoming finger connections and closures.
 * 
 */
/*ARGSUSED*/
void
fing_state(tcb,old,new)
struct tcb	*tcb;
char		old,		/* old state */
		new;		/* new state */
{
	struct finger	*fing;
	void		snd_fing();

	switch(new){
	case ESTABLISHED:
		log(tcb,"open Finger");
		fing = (struct finger *) malloc(sizeof(struct finger));

		tcb->user = (char *)fing;	/* Upward pointer */
		fing->tcb = tcb;		/* Downward pointer */
		if (finger_notify)  {
			printf("\007You're being fingered by %s!\r\n",
		 		psocket(&tcb->conn.remote));
			fflush(stdout);
		}
		return;
	case CLOSED:
		if (tcb == fing_tcb)
			fing_tcb = NULLTCB;
		if (tcb->user != NULLCHAR)
			free(tcb->user);
		del_tcp(tcb);
		break;
	}
}

/*
 *	Stop the finger server.
 */

finger0()
{
	if (fing_tcb != NULLTCB) {
		close_tcp(fing_tcb);
		fing_tcb = NULLTCB;
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
 *	Finger receive upcall.  This is the guts of the finger server.
 *	The user to finger is read from the socket.  If only a newline
 *	is read, then send the remote host a list of all known 'users' on
 *	this system.
 */

/*ARGSUSED*/
void
rcv_fing(tcb, ccnt)
register struct tcb	*tcb;
int16			ccnt;
{
	FILE		*fuser;
	struct mbuf	*mbuf,
			*bp;
	char		*buf,
			*who,
			*finger_file,
			*path,
			ch,
			temp[80],
			user[80];
	int		cnt;
	int		size;


	if ((struct finger *) tcb->user == NULLFING)	/* uh oh! */
		return;
	if(recv_tcp(tcb,&bp,FINGNAMELEN) == 0)
		return;
	if ((who = malloc(FINGNAMELEN + 1)) == NULL) {
		free_p(bp);
		return;
	}

	cnt = pullup(&bp, who, FINGNAMELEN);	/* get 'user' name */
	who[cnt] = '\0';			/* NULL terminate it */
	free_p(bp);				/* all done with bp */

	if (*who == '\015' || *who == '\012') {	/* give him a user listing */
		int found = 0;

		path = (char *) malloc((unsigned)(strlen(fingerpath)
			+ strlen(fingersuf) + 2));
		/* create wildcard path to finger files */
		strcpy(path, fingerpath);
		strcat(path, "*");
		strcat(path, fingersuf);

		sndmsg(tcb, "Known users on this system:\015\012");
		for (filedir(path, 0, user); user[0] != '\0';
			filedir (path, 1, user))  {
			found++;
			*index(user, '.') = '\0';
			sprintf(temp, "        %s\015\012", user);
			sndmsg(tcb, temp);
		}
		if (!found)
			sndmsg(tcb, "None!\015\012");

		free(path);
	}
	else {
		buf = who;
		while (*buf != '\015' && *buf != '\012' && *buf != '\0')
			buf++;
		*buf = '\0';
		/*
		 *	Create path to user's finger file and see if the
		 *	the file exists.
		 */
		finger_file = malloc((unsigned)(strlen(fingerpath) + strlen(who)
				+ strlen(fingersuf) + 1));
		if (finger_file == NULLCHAR) {	/* uh oh... */
			free(who);		/* clean up */
			close_tcp(tcb);		/* close socket */
			return;
		}
		strcpy(finger_file, fingerpath);
		strcat(finger_file, who);
		strcat(finger_file, fingersuf);

		if ((fuser = fopen(finger_file, "r")) == (FILE *) NULL) {
			sprintf(temp, "User %s unknown on this system\015\012",
				who);
			sndmsg(tcb, temp);
		}
		else {				/* valid 'user' */
			char	nl = '\0';	/* newline flag */

			/*
			 * Here's a tricky routine to make sure we get
			 * everything in, including "\r\n".  It's needed since
			 * UNIX files have only a '\n' for EOL.  What is
			 * REALLY NEEDED is a standardized routine for filling
			 * mbufs from file input so that each server doesn't
			 * have to do it themselves!  Ditto for emptying
			 * mbufs!  The problem of "\r\n" doesn't rear its
			 * ugly head with MessyDOS, but with UNIX boxes...
			 */

			ch = fgetc(fuser);	/* first get must be outside */
			while (!feof(fuser)) {
				size = tcb->window;
				if ((mbuf = alloc_mbuf((int16)size)) == NULLBUF) {
					fclose(fuser);	/* barf */
					free(who);
					free(finger_file);
					return;
				}
				buf = mbuf->data;	/* pointer to buffer */
				while(!feof(fuser) && size--) {	/* loop */
					if (nl) {
						*buf++ = '\012';/* line feed */
						mbuf->cnt++;
						nl--;
					}
					else switch(ch) {
						case '\032':	/* NO ^Z's! */
							break;
#ifndef	UNIX
						case '\012':	/* ignore LF */
							break;
#else
						case '\012':	/* change LF */
							ch = '\015'; /* to CR */
								/* fall thru */
#endif
						case '\015':	/* EOL */
							nl++;	/* fall thru */
						default:
							*buf++ = ch;
							mbuf->cnt++;
							break;
					}
					if (!nl)
						ch = fgetc(fuser);
				}
				send_tcp(tcb, mbuf);	/* send info */
			}
			fclose(fuser);
		}
		free(finger_file);
	}
	free(who);
	close_tcp(tcb);			/* close socket */
	return;
}

