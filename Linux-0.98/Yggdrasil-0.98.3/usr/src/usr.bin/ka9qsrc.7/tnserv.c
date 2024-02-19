#include <stdio.h>
#include "global.h"
#include "config.h"
#include "mbuf.h"
#include "timer.h"
#include "icmp.h"
#include "netuser.h"
#include "tcp.h"
#include "telnet.h"
#include "session.h"
#include "ftp.h"
#include "iface.h"
#include "ax25.h"
#include "lapb.h"
#include "finger.h"
#include "nr4.h"
#ifdef	UNIX
#include <string.h>
#endif

struct tcb *tnet_tcb;
tn1(argc,argv)
char *argv[];
{
	struct socket lsocket;
	extern int32 ip_addr;
	void tnet_state();
	void t_state(),rcv_char();

	/* Incoming Telnet */
	lsocket.address = ip_addr;
	if(argc < 2)
		lsocket.port = TELNET_PORT;
	else
		lsocket.port = atoi(argv[1]);
	tnet_tcb = open_tcp(&lsocket,NULLSOCK,TCP_SERVER,0,rcv_char,NULLVFP,tnet_state,0,(char *)NULL);
}
/* Handle incoming Telnet connect requests by creating a Telnet session,
 * then change upcall vector so it behaves like an ordinary Telnet session.
 * 
 */
/*ARGSUSED*/
static void
tnet_state(tcb,old,new)
struct tcb *tcb;
char old,new;
{
	struct telnet *tn;
	struct session *s,*newsession();
	void t_state();
	char *a;

	switch(new){
	case ESTABLISHED:
		log(tcb,"open Telnet");
		/* Allocate a session descriptor */
		if((s = newsession()) == NULLSESSION){
			printf("\007Incoming Telnet call from %s refused; too many sessions\r\n",
			 psocket(&tcb->conn.remote));
			fflush(stdout);
			sndmsg(tcb,"Call rejected; too many sessions on remote system\n");
			close_tcp(tcb);
			return;
		}
		a = inet_ntoa(tcb->conn.remote.address);
		if((s->name = malloc((unsigned)strlen(a)+1)) != NULLCHAR)
			strcpy(s->name,a);
		s->type = TELNET;
		s->parse = send_tel;
		/* Create and initialize a Telnet protocol descriptor */
		if((tn = (struct telnet *)calloc(1,sizeof(struct telnet))) == NULLTN){
			printf("\007Incoming Telnet call refused; no space\r\n");
			fflush(stdout);
			sndmsg(tcb,"Call rejected; no space on remote system\n");
			close_tcp(tcb);
			s->type = FREE;
			return;
		}
		tn->session = s;	/* Upward pointer */
		tn->state = TS_DATA;
		s->cb.telnet = tn;	/* Downward pointer */

		tcb->user = (char *)tn;	/* Upward pointer */
		tn->tcb = tcb;		/* Downward pointer */
		printf("\007Incoming Telnet session %lu from %s\r\n",
		 (long)(s - sessions),psocket(&tcb->conn.remote));
		fflush(stdout);
		tcb->s_upcall = t_state;
		return;
	case CLOSED:
		/* This will only happen if the connection closed before
		 * the session was set up, e.g., if we refused it because
		 * there were too many sessions, or if the server is being
		 * shut down.
		 */
		if(tcb == tnet_tcb)
			tnet_tcb = NULLTCB;
		del_tcp(tcb);
		break;
	}
}
/* Shut down Telnet server */
tn0()
{
	if(tnet_tcb != NULLTCB)
		close_tcp(tnet_tcb);
}
sndmsg(tcb,msg)
struct tcb *tcb;
char *msg;
{
	struct mbuf *bp;

	bp = qdata(msg,(int16)strlen(msg));
	send_tcp(tcb,bp);
}
