#include <stdio.h>
#include "global.h"
#include "config.h"
#include "mbuf.h"
#include "ax25.h"
#include "timer.h"
#include "netuser.h"
#include "session.h"
#include "ftp.h"
#include "telnet.h"
#include "iface.h"
#include "finger.h"
#include "lapb.h"

/* Called whenever timer T1 expires */
void
recover(n)
int *n;
{
	register struct ax25_cb *axp;
	void lapbstate();

	axp = (struct ax25_cb *)n;

	switch(axp->state){
	case SETUP:
		if(axp->n2 != 0 && axp->retries == axp->n2){
			free_q(&axp->txq);
			lapbstate(axp,DISCONNECTED);
		} else {
			axp->retries++;
			sendctl(axp,COMMAND,SABM|PF);
			start_timer(&axp->t1);
		}
		break;
	case DISCPENDING:
		if(axp->n2 != 0 && axp->retries == axp->n2){
			lapbstate(axp,DISCONNECTED);
		} else {
			axp->retries++;
			sendctl(axp,COMMAND,DISC|PF);
			start_timer(&axp->t1);
		}
		break;
	case CONNECTED:
		axp->retries = 0;
	case RECOVERY:	/* note fall-thru */
		if(axp->n2 != 0 && axp->retries == axp->n2){
			/* Give up */
			sendctl(axp,RESPONSE,DM|PF);
			free_q(&axp->txq);
			lapbstate(axp,DISCONNECTED);
		} else {
			/* Transmit poll */
			tx_enq(axp);
			axp->retries++;
			lapbstate(axp,RECOVERY);
		}
		break;
	case FRAMEREJECT:
		if(axp->n2 != 0 && axp->retries == axp->n2){
			sendctl(axp,RESPONSE,DM|PF);
			free_q(&axp->txq);
			lapbstate(axp,DISCONNECTED);
		} else {
			frmr(axp,0,0);	/* Retransmit last FRMR */
			start_timer(&axp->t1);
			axp->retries++;
		}
		break;
	}
	/* Empty the trash */
	if(axp->state == DISCONNECTED)
		del_ax25(axp);
}

/* T2 has expired, we can't delay an acknowledgement any further */
void
send_ack(n)
int *n;
{
	char control;
	register struct ax25_cb *axp;

	axp = (struct ax25_cb *)n;
	switch(axp->state){
	case CONNECTED:
	case RECOVERY:
		control = len_mbuf(axp->rxq) > axp->window ? RNR : RR;
		sendctl(axp,RESPONSE,control);
		axp->response = 0;
		break;
	}
}

/* Send a poll (S-frame command with the poll bit set) */
void
pollthem(n)
int *n;
{
	register struct ax25_cb *axp;

	axp = (struct ax25_cb *)n;
	if(axp->proto == V1)
		return;	/* Not supported in the old protocol */
	switch(axp->state){
	case CONNECTED:
		axp->retries = 0;
		tx_enq(axp);
		lapbstate(axp,RECOVERY);
		break;
	}
}
/* Transmit query */
tx_enq(axp)
register struct ax25_cb *axp;
{
	char ctl;
	struct mbuf *bp;

	/* I believe that retransmitting the oldest unacked
	 * I-frame tends to give better performance than polling,
	 * as long as the frame isn't too "large", because
	 * chances are that the I frame got lost anyway.
	 * This is an option in LAPB, but not in the official AX.25.
	 */
	if(axp->txq != NULLBUF && len_mbuf(axp->txq) < axp->pthresh
	 && (axp->proto == V1 || !axp->remotebusy)){
		/* Retransmit oldest unacked I-frame */
		dup_p(&bp,axp->txq,0,len_mbuf(axp->txq));
		ctl = PF | I | ((axp->vs - axp->unack) & MMASK) << 1
		 | axp->vr << 5;
		sendframe(axp,COMMAND,ctl,bp);
	} else {
		ctl = len_mbuf(axp->rxq) >= axp->window ? RNR|PF : RR|PF;	
		sendctl(axp,COMMAND,ctl);
	}
	axp->response = 0;	
	stop_timer(&axp->t3);
	start_timer(&axp->t1);
}

