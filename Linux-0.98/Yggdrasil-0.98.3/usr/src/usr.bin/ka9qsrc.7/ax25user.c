/* User subroutines for AX.25 */
#include "global.h"
#include "mbuf.h"
#include "timer.h"
#include "iface.h"
#include "ax25.h"
#include "lapb.h"
#include <ctype.h>

/* Open an AX.25 connection */
struct ax25_cb *
open_ax25(addr,window,r_upcall,t_upcall,s_upcall,iface,user)
struct ax25 *addr;		/* Addresses */
int16 window;			/* Window size in bytes */
void (*r_upcall)();		/* Receiver upcall handler */
void (*t_upcall)();		/* Transmitter upcall handler */
void (*s_upcall)();		/* State-change upcall handler */
struct interface *iface;	/* Hardware interface structure */
char *user;			/* User linkage area */
{
	struct ax25_cb *axp,*cr_ax25();
	void lapbstate();

	if((axp = cr_ax25(&addr->dest)) == NULLAX25)
		return NULLAX25;
	ASSIGN(axp->addr,*addr);
	if(addr->ndigis != 0){
		axp->t1.start *= (addr->ndigis + 1);
		axp->t2.start *= (addr->ndigis + 1);
		axp->t3.start *= (addr->ndigis + 1);
	}
	axp->window = window;
	axp->r_upcall = r_upcall;
	axp->t_upcall = t_upcall;
	axp->s_upcall = s_upcall;
	axp->interface = iface;
	axp->user = user;

	switch(axp->state){
	case DISCONNECTED:
		/* Don't send anything if the connection already exists */
		est_link(axp);
		lapbstate(axp,SETUP);
		break;
	case SETUP:
		free_q(&axp->txq);
		break;
	case DISCPENDING:	/* Ignore */
	case FRAMEREJECT:
		break;
	case RECOVERY:
	case CONNECTED:
		free_q(&axp->txq);
		est_link(axp);
		lapbstate(axp,SETUP);
		break;
	}
	return axp;
}

/* Send data on an AX.25 connection. Caller must provide PID */
int
send_ax25(axp,bp)
struct ax25_cb *axp;
struct mbuf *bp;
{
	if(axp == NULLAX25 || bp == NULLBUF)
		return -1;
	enqueue(&axp->txq,bp);
	return lapb_output(axp);
}

/* Receive incoming data on an AX.25 connection */
/*ARGSUSED*/
struct mbuf *
recv_ax25(axp,cnt)
register struct ax25_cb *axp;
int16 cnt;
{
	struct mbuf *bp;

	if(axp->rxq == NULLBUF)
		return NULLBUF;

	bp = axp->rxq;
	axp->rxq = NULLBUF;

	/* If this has un-busied us, send a RR to reopen the window */
	if(len_mbuf(bp) >= axp->window)
		sendctl(axp,RESPONSE,RR);
	return bp;
}

/* Close an AX.25 connection */
disc_ax25(axp)
struct ax25_cb *axp;
{
	void lapbstate();

	switch(axp->state){
	case DISCONNECTED:
		break;		/* Ignored */
	case DISCPENDING:
		lapbstate(axp,DISCONNECTED);
		del_ax25(axp);
		break;
	case SETUP:
	case CONNECTED:
	case RECOVERY:
	case FRAMEREJECT:
		free_q(&axp->txq);
		axp->retries = 0;
		sendctl(axp,COMMAND,DISC|PF);
		stop_timer(&axp->t3);
		start_timer(&axp->t1);
		lapbstate(axp,DISCPENDING);
		break;
	}
}

/* Verify that axp points to a valid ax25 control block */
ax25val(axp)
struct ax25_cb *axp;
{
	register struct ax25_cb *axp1;
	register int i;

	if(axp == NULLAX25)
		return 0;	/* Null pointer can't be valid */
	for(i=0; i < NHASH; i++)
		for(axp1 = ax25_cb[i];axp1 != NULLAX25; axp1 = axp1->next)
			if(axp1 == axp)
				return 1;
	return 0;
}

/* Force a retransmission */
kick_ax25(axp)
struct ax25_cb *axp;
{
	void recover();

	if(!ax25val(axp))
		return -1;
	recover((int *)axp);
	return 0;
}

/* Abruptly terminate an AX.25 connection */
reset_ax25(axp)
struct ax25_cb *axp;
{
	void lapbstate();

	lapbstate(axp,DISCONNECTED);
	del_ax25(axp);
}
