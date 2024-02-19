/*
 *
 *	Finger support...
 *
 *	Finger client routines.  Written by Michael T. Horne - KA7AXD.
 *	Copyright 1988 by Michael T. Horne, All Rights Reserved.
 *	Permission granted for non-commercial use and copying, provided
 *	that this notice is retained.
 *
 */

#include <stdio.h>
#include "config.h"
#include "global.h"
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

extern char	badhost[],
		hostname[];

/*
 *
 *	Open up a socket to a remote (or the local) host on its finger port.
 *
 */

int
dofinger(argc,argv)
int	argc;
char	*argv[];
{
	void		f_state(),
			fingcli_rcv();
	char		*inet_ntoa();
	int32		resolve();
	struct session	*s;
	struct tcb	*tcb;
	struct socket	lsocket,
			fsocket;
	struct finger	*finger,
					*alloc_finger();
	char		*host;

	if (argc < 2) {
		printf("usage: %s [user | user@host | @host]\n", argv[0]);
		return(1);
	}

	lsocket.address = ip_addr;
	lsocket.port = lport++;

/*
 *	Extract user/host information.  It can be of the form:
 *	
 *	finger user,			# finger local user
 *	finger user@host,		# finger remote user
 *	finger @host			# finger host (give system status)
 *
 */
	if ((finger = alloc_finger()) == NULLFING)
		return(1);

	if ((host = index(argv[1], '@')) == NULL) {
		fsocket.address = ip_addr;	/* no host, use local */
		if ((finger->user = malloc((unsigned)(strlen(argv[1]) + 3))) == NULL) {
			free_finger(finger);
			return(1);
		}
		strcpy(finger->user, argv[1]);
		strcat(finger->user, "\015\012");
	}
	else {
		*host++ = '\0';		/* null terminate user name */
		if (*host == '\0') {	/* must specify host */
			printf("%s: no host specified\n", argv[0]);
			printf("usage: %s [user | user@host | @host]\n",
				argv[0]);
			free_finger(finger);
			return(1);
		}
		if ((fsocket.address = resolve(host)) == 0) {
			printf("%s: ", argv[0]);
			printf(badhost, host); 
			free_finger(finger);
			return(1);
		}
		if ((finger->user = malloc((unsigned)(strlen(argv[1])+3)))==NULL) {
			free_finger(finger);
			return 1;
		}
		strcpy(finger->user, argv[1]);
		strcat(finger->user, "\015\012");
	}
	
	fsocket.port = FINGER_PORT;		/* use finger wnp */

	/* Allocate a session descriptor */
	if ((s = newsession()) == NULLSESSION){
		printf("Too many sessions\n");
		free_finger(finger);
		return 1;
	}
	current = s;
	s->cb.finger = finger;
	finger->session = s;

	if (!host)				/* if no host specified */
		host = hostname;		/* use local host name */
	if ((s->name = malloc((unsigned)(strlen(host)+1))) != NULLCHAR)
		strcpy(s->name, host);

	s->type = FINGER;
	s->parse = (int (*)()) NULL;

	tcb = open_tcp(&lsocket, &fsocket, TCP_ACTIVE, 0,
	 fingcli_rcv, (void (*)()) 0, f_state, 0, (char *) finger);

	finger->tcb = tcb;
	tcb->user = (char *)finger;
	go();
	return 0;
}

/*
 *	Allocate a finger structure for the new session
 */
struct finger *
alloc_finger()
{
	struct finger *tmp;

	if ((tmp = (struct finger *) malloc(sizeof(struct finger))) == NULLFING)
		return(NULLFING);
	tmp->session = NULLSESSION;
	tmp->user = (char *) NULL;
	return(tmp);
}

/*
 *	Free a finger structure
 */
int
free_finger(finger)
struct finger *finger;
{
	if (finger != NULLFING) {
		if (finger->session != NULLSESSION)
			freesession(finger->session);
		if (finger->user != (char *) NULL)
			free(finger->user);
		free((char *)finger);
	}
	return 0;
}

/* Finger receiver upcall routine */
void
fingcli_rcv(tcb, cnt)
register struct tcb	*tcb;
int16			cnt;
{
	struct mbuf	*bp;
	char		*buf;

	/* Make sure it's a valid finger session */
	if ((struct finger *) tcb->user == NULLFING) {
		return;
	}

	/* Hold output if we're not the current session */
	if (mode != CONV_MODE || current == NULLSESSION
		|| current->type != FINGER)
		return;

	/*
	 *	We process the incoming data stream and make sure it
	 *	meets our requirments.  A check is made for control-Zs
	 *	since these characters lock up DoubleDos when sent to
	 *	the console (don't you just love it...).
	 */

	if (recv_tcp(tcb, &bp, cnt) > 0)
		while (bp != NULLBUF) {
			buf = bp->data;
			while(bp->cnt--) {
				switch(*buf) {
					case '\012':	/* ignore LF */
					case '\032':	/* NO ^Z's! */
						break;
					case '\015':
						fputc('\015', stdout);
						fputc('\012', stdout);
						break;
					default:
						fputc(*buf, stdout);
						break;
				}
				buf++;
			}
			bp = free_mbuf(bp);
		}
	fflush(stdout);
}

/* State change upcall routine */
/*ARGSUSED*/
void
f_state(tcb,old,new)
register struct tcb	*tcb;
char			old,		/* old state */
			new;		/* new state */
{
	struct finger	*finger;
	char		notify = 0;
	extern char	*tcpstates[];
	extern char	*reasons[];
	extern char	*unreach[];
	extern char	*exceed[];
	struct mbuf	*bp;

	finger = (struct finger *)tcb->user;

	if(current != NULLSESSION && current->type == FINGER)
		notify = 1;

	switch(new){

	case CLOSE_WAIT:
		if(notify)
			printf("%s\n",tcpstates[new]);
		close_tcp(tcb);
		break;

	case CLOSED:	/* finish up */
		if(notify) {
			printf("%s (%s", tcpstates[new], reasons[tcb->reason]);
			if (tcb->reason == NETWORK){
				switch(tcb->type){
				case DEST_UNREACH:
					printf(": %s unreachable",unreach[tcb->code]);
					break;
				case TIME_EXCEED:
					printf(": %s time exceeded",exceed[tcb->code]);
					break;
				}
			}
			printf(")\n");
			cmdmode();
		}
		if(finger != NULLFING)
			free_finger(finger);
		del_tcp(tcb);
		break;
	case ESTABLISHED:
		if (notify) {
			printf("%s\n",tcpstates[new]);
		}
		printf("[%s]\n", current->name);
		bp = qdata(finger->user, (int16) strlen(finger->user));
		send_tcp(tcb, bp);
		break;
		
	default:
		if(notify)
			printf("%s\n",tcpstates[new]);
		break;
	}
	fflush(stdout);
}

