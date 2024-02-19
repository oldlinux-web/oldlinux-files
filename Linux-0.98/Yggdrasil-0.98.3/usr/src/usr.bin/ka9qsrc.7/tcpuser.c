/* User calls to TCP */
#include "global.h"
#include "timer.h"
#include "mbuf.h"
#include "netuser.h"
#include "internet.h"
#include "tcp.h"
#ifdef	BSD
char *sprintf();
#endif

int16 tcp_window = DEF_WND;

struct tcb *
open_tcp(lsocket,fsocket,mode,window,r_upcall,t_upcall,s_upcall,tos,user)
struct socket *lsocket;	/* Local socket */
struct socket *fsocket;	/* Remote socket */
int mode;		/* Active/passive/server */
int16 window;		/* Receive window (and send buffer) sizes */
void (*r_upcall)();	/* Function to call when data arrives */
void (*t_upcall)();	/* Function to call when ok to send more data */
void (*s_upcall)();	/* Function to call when connection state changes */
char tos;
char *user;		/* User linkage area */
{
	struct connection conn;
	register struct tcb *tcb;
	void send_syn();

	if(lsocket == NULLSOCK){
		net_error = INVALID;
		return NULLTCB;
	}
	conn.local.address = lsocket->address;
	conn.local.port = lsocket->port;
	if(fsocket != NULLSOCK){
		conn.remote.address = fsocket->address;
		conn.remote.port = fsocket->port;
	} else {
		conn.remote.address = 0;
		conn.remote.port = 0;
	}
	if((tcb = lookup_tcb(&conn)) == NULLTCB){
		if((tcb = create_tcb(&conn)) == NULLTCB){
			net_error = NO_SPACE;
			return NULLTCB;
		}
	} else if(tcb->state != LISTEN){
		net_error = CON_EXISTS;
		return NULLTCB;
	}
	tcb->user = user;
	if(window != 0)
		tcb->window = tcb->rcv.wnd = window;
	else
		tcb->window = tcb->rcv.wnd = tcp_window;
	tcb->r_upcall = r_upcall;
	tcb->t_upcall = t_upcall;
	tcb->s_upcall = s_upcall;
	tcb->tos = tos;
	switch(mode){
	case TCP_SERVER:
		tcb->flags |= CLONE;
	case TCP_PASSIVE:	/* Note fall-thru */
		setstate(tcb,LISTEN);
		break;
	case TCP_ACTIVE:
		/* Send SYN, go into SYN_SENT state */
		tcb->flags |= ACTIVE;
		send_syn(tcb);
		setstate(tcb,SYN_SENT);
		tcp_output(tcb);
		tcp_stat.conout++;
		break;
	}
	return tcb;
}
/* User send routine */
int
send_tcp(tcb,bp)
register struct tcb *tcb;
struct mbuf *bp;
{
	int16 cnt;
	void send_syn();

	if(tcb == NULLTCB || bp == NULLBUF){
		free_p(bp);
		net_error = INVALID;
		return -1;
	}
	cnt = len_mbuf(bp);
#ifdef	TIGHT
	/* If this would overfill our send queue, reject it entirely */
	if(tcb->sndcnt + cnt > tcb->window){
		free_p(bp);
		net_error = WOULDBLK;
		return -1;
	}
#endif
	switch(tcb->state){
	case CLOSED:
		free_p(bp);
		net_error = NO_CONN;
		return -1;
	case LISTEN:	/* Change state from passive to active */
		tcb->flags |= ACTIVE;
		send_syn(tcb);
		setstate(tcb,SYN_SENT);	/* Note fall-thru */
	case SYN_SENT:
	case SYN_RECEIVED:
	case ESTABLISHED:
	case CLOSE_WAIT:
		append(&tcb->sndq,bp);
		tcb->sndcnt += cnt;
		tcp_output(tcb);
		break;
	case FINWAIT1:
	case FINWAIT2:
	case CLOSING:
	case LAST_ACK:
	case TIME_WAIT:
		free_p(bp);
		net_error = CON_CLOS;
		return -1;
	}
	return cnt;
}
/* User receive routine */
int
recv_tcp(tcb,bp,cnt)
register struct tcb *tcb;
struct mbuf **bp;
int16 cnt;
{
	if(tcb == NULLTCB || bp == (struct mbuf **)NULL){
		net_error = INVALID;
		return -1;
	}
	/* cnt == 0 means "I want it all" */
	if(cnt == 0)
		cnt = tcb->rcvcnt;
	/* If there's something on the queue, just return it regardless
	 * of the state we're in.
	 */
	if(tcb->rcvcnt != 0){
		/* See if the user can take all of it */
		if(tcb->rcvcnt <= cnt){
			cnt = tcb->rcvcnt;
			*bp = tcb->rcvq;
			tcb->rcvq = NULLBUF;
		} else {
			if((*bp = alloc_mbuf(cnt)) == NULLBUF){
				net_error = NO_SPACE;
				return -1;
			}
			pullup(&tcb->rcvq,(*bp)->data,cnt);
			(*bp)->cnt = cnt;
		}
		tcb->rcvcnt -= cnt;
		tcb->rcv.wnd += cnt;
		/* Do a window update if it was closed */
		if(cnt == tcb->rcv.wnd){
			tcb->flags |= FORCE;
			tcp_output(tcb);
		}
		return cnt;
	} else {
		/* If there's nothing on the queue, our action depends on what state
		 * we're in (i.e., whether or not we're expecting any more data).
		 * If no more data is expected, then simply return 0; this is
		 * interpreted as "end of file".
		 */
		switch(tcb->state){
		case LISTEN:
		case SYN_SENT:
		case SYN_RECEIVED:
		case ESTABLISHED:
		case FINWAIT1:
		case FINWAIT2:
			*bp = NULLBUF;
			net_error = WOULDBLK;
			return -1;
		case CLOSED:
		case CLOSE_WAIT:
		case CLOSING:
		case LAST_ACK:
		case TIME_WAIT:
			*bp = NULLBUF;
			return 0;
		}
	}
	return 0;	/* Not reached, but lint doesn't know that */
}
/* This really means "I have no more data to send". It only closes the
 * connection in one direction, and we can continue to receive data
 * indefinitely.
 */
int
close_tcp(tcb)
register struct tcb *tcb;
{
	if(tcb == NULLTCB){
		net_error = INVALID;
		return -1;
	}
	switch(tcb->state){
	case LISTEN:
	case SYN_SENT:
		close_self(tcb,NORMAL);
		return 0;
	case SYN_RECEIVED:
	case ESTABLISHED:
		tcb->sndcnt++;
		tcb->snd.nxt++;
		setstate(tcb,FINWAIT1);
		tcp_output(tcb);
		return 0;
	case CLOSE_WAIT:
		tcb->sndcnt++;
		tcb->snd.nxt++;
		setstate(tcb,LAST_ACK);
		tcp_output(tcb);
		return 0;
	case FINWAIT1:
	case FINWAIT2:
	case CLOSING:
	case LAST_ACK:
	case TIME_WAIT:
		net_error = CON_CLOS;
		return -1;
	}
	return -1;	/* "Can't happen" */
}
/* Delete TCB, free resources. The user is not notified, even if the TCB is
 * not in the CLOSED state. This function should normally be called by the
 * user only in response to a state change upcall to CLOSED state.
 */
int
del_tcp(tcb)
register struct tcb *tcb;
{
	void unlink_tcb();
	struct reseq *rp,*rp1;

	if(tcb == NULLTCB){
		net_error = INVALID;
		return -1;
	}
	unlink_tcb(tcb);
	stop_timer(&tcb->timer);
	stop_timer(&tcb->rtt_timer);
	for(rp = tcb->reseq;rp != NULLRESEQ;rp = rp1){
		rp1 = rp->next;
		free_p(rp->bp);
		free((char *)rp);
	}
	tcb->reseq = NULLRESEQ;
	free_p(tcb->rcvq);
	free_p(tcb->sndq);
	free((char *)tcb);
	return 0;
}
/* Do printf on a tcp connection */
/*VARARGS*/
tprintf(tcb,message,arg1,arg2,arg3)
struct tcb *tcb;
char *message,*arg1,*arg2,*arg3;
{
	struct mbuf *bp;
	int len;

	if(tcb == NULLTCB)
		return 0;

	bp = alloc_mbuf(256);
#ifdef	BSD
	sprintf(bp->data,message,arg1,arg2,arg3);
	len = bp->cnt = strlen(bp->data);
#else
	len = sprintf(bp->data,message,arg1,arg2,arg3);
	bp->cnt = strlen(bp->data);
#endif
	send_tcp(tcb,bp);
	return len;
}
/* Return 1 if arg is a valid TCB, 0 otherwise */
int
tcpval(tcb)
struct tcb *tcb;
{
	register int i;
	register struct tcb *tcb1;

	if(tcb == NULLTCB)
		return 0;	/* Null pointer can't be valid */
	for(i=0;i<NTCB;i++){
		for(tcb1=tcbs[i];tcb1 != NULLTCB;tcb1 = tcb1->next){
			if(tcb1 == tcb)
				return 1;
		}
	}
	return 0;
}
kick_tcp(tcb)
register struct tcb *tcb;
{
	void tcp_timeout();

	if(!tcpval(tcb))
		return -1;
	tcp_timeout((char *)tcb);
	return 0;
}
reset_tcp(tcb)
register struct tcb *tcb;
{
	close_self(tcb,RESET);
}
/* Return character string corresponding to a TCP well-known port, or
 * the decimal number if unknown.
 */
char *
tcp_port(n)
int16 n;
{
	static char buf[32];

	switch(n){
	case ECHO_PORT:
		return "echo";
	case DISCARD_PORT:
		return "discard";
	case FTPD_PORT:
		return "ftp_data";
	case FTP_PORT:
		return "ftp";	
	case TELNET_PORT:
		return "telnet";
	case SMTP_PORT:
		return "smtp";
        case FINGER_PORT:
		return "finger";
	default:
		sprintf(buf,"%u",n);
		return buf;
	}
}
