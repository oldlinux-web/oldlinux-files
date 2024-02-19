/* Link Access Procedures Balanced (LAPB) - with changes for rational
 * behavior over packet radio
 */
#include "global.h"
#include "mbuf.h"
#include "timer.h"
#include "ax25.h"
#include "lapb.h"
#include "iface.h"
#ifdef	UNIX
#include <memory.h>
#endif

/* Process incoming frames */
int
lapb_input(axp,cmdrsp,bp)
struct ax25_cb *axp;		/* Link control structure */
char cmdrsp;			/* Command/response flag */
struct mbuf *bp;		/* Rest of frame, starting with ctl */
{
	int16 ftype();
	void lapbstate();
	char control;
	char class;		/* General class (I/S/U) of frame */
	int16 type;		/* Specific type (I/RR/RNR/etc) of frame */
	char pf;		/* extracted poll/final bit */
	char poll = 0;
	char final = 0;
	int nr;			/* ACK number of incoming frame */
	int ns;			/* Seq number of incoming frame */
	char tmp;

	if(bp == NULLBUF || axp == NULLAX25){
		free_p(bp);
		return -1;
	}

	/* Extract the various parts of the control field for easy use */
	control = pullchar(&bp);
	type = ftype(control);
	class = type & 0x3;
	pf = control & PF;
	/* Check for polls and finals */
	if(pf){
		switch(cmdrsp){
		case COMMAND:
			poll = YES;
			break;
		case RESPONSE:
			final = YES;
			break;
		}
	}
	/* Extract sequence numbers, if present */
	switch(class){
	case I:
	case I+2:
		ns = (control >> 1) & MMASK;
	case S:	/* Note fall-thru */
		nr = (control >> 5) & MMASK;
		break;
	}
	/* This section follows the SDL diagrams by K3NA fairly closely */
	switch(axp->state){
	case DISCONNECTED:
		switch(type){
		case SABM:	/* Initialize or reset link */
			sendctl(axp,RESPONSE,UA|pf);	/* Always accept */
			clr_ex(axp);
			axp->unack = axp->vr = axp->vs = 0;
			lapbstate(axp,CONNECTED);/* Resets state counters */
			start_timer(&axp->t3);
			break;
		case DM:	/* Ignore to avoid infinite loops */
			break;
		default:	/* All others get DM */
			sendctl(axp,RESPONSE,DM|pf);
			break;
		}
		break;
	case SETUP:
		switch(type){
		case SABM:	/* Simultaneous open */
			sendctl(axp,RESPONSE,UA|pf);
			break;
		case DISC:
			sendctl(axp,RESPONSE,DM|pf);
			break;
		case UA:	/* Connection accepted */
			/* Note: xmit queue not cleared */
			stop_timer(&axp->t1);
			start_timer(&axp->t3);
			axp->unack = axp->vr = axp->vs = 0;
			lapbstate(axp,CONNECTED);
			break;			
		case DM:	/* Connection refused */
			free_q(&axp->txq);
			stop_timer(&axp->t1);
			lapbstate(axp,DISCONNECTED);
			break;
		default:	/* All other frames ignored */
			break;
		}
		break;
	case DISCPENDING:
		switch(type){
		case SABM:
			sendctl(axp,RESPONSE,DM|pf);
			break;
		case DISC:
			sendctl(axp,RESPONSE,UA|pf);
			break;
		case UA:
		case DM:
			stop_timer(&axp->t1);
			lapbstate(axp,DISCONNECTED);
			break;
		default:	/* Respond with DM only to command polls */
			if(poll)
				sendctl(axp,RESPONSE,DM|pf);
			break;
		}
		break;
	case CONNECTED:
		switch(type){
		case SABM:
			sendctl(axp,RESPONSE,UA|pf);
			clr_ex(axp);
			free_q(&axp->txq);
			stop_timer(&axp->t1);
			start_timer(&axp->t3);
			axp->unack = axp->vr = axp->vs = 0;
			lapbstate(axp,CONNECTED); /* Purge queues */
			break;
		case DISC:
			free_q(&axp->txq);
			sendctl(axp,RESPONSE,UA|pf);
			stop_timer(&axp->t1);
			stop_timer(&axp->t3);
			lapbstate(axp,DISCONNECTED);
			break;
/* This code is cribbed from the NOS version, in order to make a */
/* temporary fix to a pathological looping behavior during connect (dmf) */
		case DM:
			lapbstate(axp,DISCONNECTED);
			break;
		case UA:
			est_link(axp);
			lapbstate(axp,SETUP);	/* Re-establish */	
			break;
/* End of cribbed code (dmf) */			
		case FRMR:
			est_link(axp);
			lapbstate(axp,SETUP);	/* Re-establish link */
			break;
		case RR:
		case RNR:
			axp->remotebusy = (control == RNR) ? YES : NO;
			if(poll)
				enq_resp(axp);
			(void)ackours(axp,nr);
			break;
		case REJ:
			axp->remotebusy = NO;
			if(poll)
				enq_resp(axp);
			(void)ackours(axp,nr);
			stop_timer(&axp->t1);
			start_timer(&axp->t3);
			/* This may or may not actually invoke transmission,
			 * depending on whether this REJ was caused by
			 * our losing his prior ACK.
			 */
			inv_rex(axp);
			break;	
		case I:
			(void)ackours(axp,nr); /** == -1) */
			if(len_mbuf(axp->rxq) >= axp->window){
				/* Too bad he didn't listen to us; he'll
				 * have to resend the frame later. This
				 * drastic action is necessary to avoid
				 * deadlock.
				 */
				if(poll)
					sendctl(axp,RESPONSE,RNR|pf);
				free_p(bp);
				bp = NULLBUF;
				break;
			}
			/* Reject or ignore I-frames with receive sequence number errors */
			if(ns != axp->vr){
				if(axp->proto == V1 || !axp->rejsent){
					axp->rejsent = YES;
					sendctl(axp,RESPONSE,REJ | pf);
				}
				axp->response = 0;
				stop_timer(&axp->t2);
				break;
			}
			axp->rejsent = NO;
			axp->vr = (axp->vr+1) & MMASK;
			tmp = len_mbuf(axp->rxq) >= axp->window ? RNR : RR;
			if(poll){
				sendctl(axp,RESPONSE,tmp|PF);
			} else {
				axp->response = tmp;
				start_timer(&axp->t2);
			}
			procdata(axp,bp);
			bp = NULLBUF;
			break;
		default:	/* All others ignored */
			break;
		}
		break;
	case RECOVERY:
		switch(type){
		case SABM:
			sendctl(axp,RESPONSE,UA|pf);
			clr_ex(axp);
			stop_timer(&axp->t1);
			start_timer(&axp->t3);
			axp->unack = axp->vr = axp->vs = 0;
			lapbstate(axp,CONNECTED); /* Purge queues */
			break;
		case DISC:
			free_q(&axp->txq);
			sendctl(axp,RESPONSE,UA|pf);
			stop_timer(&axp->t1);
			stop_timer(&axp->t3);
			axp->response = UA;
			lapbstate(axp,DISCONNECTED);
			break;
/* This code is cribbed from the NOS version, in order to make a */
/* temporary fix to a pathological looping behavior during connect (dmf) */
		case DM:
			lapbstate(axp,DISCONNECTED);
			break;
		case UA:
			est_link(axp);
			lapbstate(axp,SETUP);	/* Re-establish */	
			break;
/* End of cribbed code (dmf) */			
		case FRMR:
			est_link(axp);
			lapbstate(axp,SETUP);	/* Re-establish link */
			break;
		case RR:
		case RNR:
			axp->remotebusy = (control == RNR) ? YES : NO;
			if(axp->proto == V1 || final){
				stop_timer(&axp->t1);
				(void)ackours(axp,nr);
				if(axp->unack != 0){
					inv_rex(axp);
				} else {
					start_timer(&axp->t3);
					lapbstate(axp,CONNECTED);
				}
			} else {
				if(poll)
					enq_resp(axp);
				(void)ackours(axp,nr);
				/* Keep timer running even if all frames
				 * were acked, since we must see a Final
				 */
				if(!run_timer(&axp->t1))
					start_timer(&axp->t1);
			}
			break;
		case REJ:
			axp->remotebusy = NO;
			/* Don't insist on a Final response from the old proto */
			if(axp->proto == V1 || final){
				stop_timer(&axp->t1);
				(void)ackours(axp,nr);
				if(axp->unack != 0){
					inv_rex(axp);
				} else {
					start_timer(&axp->t3);
					lapbstate(axp,CONNECTED);
				}
			} else {
				if(poll)
					enq_resp(axp);
				(void)ackours(axp,nr);
				if(axp->unack != 0){
					/* This is certain to trigger output */
					inv_rex(axp);
				}
				/* A REJ that acks everything but doesn't
				 * have the F bit set can cause a deadlock.
				 * So make sure the timer is running.
				 */
				if(!run_timer(&axp->t1))
					start_timer(&axp->t1);
			}
			break;
		case I:
			(void)ackours(axp,nr); /** == -1) */
			/* Make sure timer is running, since an I frame
			 * cannot satisfy a poll
			 */
			if(!run_timer(&axp->t1))
				start_timer(&axp->t1);
			if(len_mbuf(axp->rxq) >= axp->window){
				/* Too bad he didn't listen to us; he'll
				 * have to resend the frame later. This
				 * drastic action is necessary to avoid
				 * memory deadlock.
				 */
				sendctl(axp,RESPONSE,RNR | pf);
				free_p(bp);
				bp = NULLBUF;
				break;
			}
			/* Reject or ignore I-frames with receive sequence number errors */
			if(ns != axp->vr){
				if(axp->proto == V1 || !axp->rejsent){
					axp->rejsent = YES;
					sendctl(axp,RESPONSE,REJ | pf);
				}
				axp->response = 0;
				stop_timer(&axp->t2);
				break;
			}
			axp->rejsent = NO;
			axp->vr = (axp->vr+1) & MMASK;
			tmp = len_mbuf(axp->rxq) >= axp->window ? RNR : RR;
			if(poll){
				sendctl(axp,RESPONSE,tmp|PF);
			} else {
				axp->response = tmp;
				start_timer(&axp->t2);
			}
			procdata(axp,bp);
			bp = NULLBUF;
			break;
		default:
			break;		/* Ignored */
		}
		break;
	case FRAMEREJECT:
		switch(type){
		case SABM:
			sendctl(axp,RESPONSE,UA|pf);
			clr_ex(axp);
			axp->unack = axp->vr = axp->vs = 0;
			stop_timer(&axp->t1);
			start_timer(&axp->t3);
			lapbstate(axp,CONNECTED);
			break;
		case DISC:
			free_q(&axp->txq);
			sendctl(axp,RESPONSE,UA|pf);
			stop_timer(&axp->t1);
			lapbstate(axp,DISCONNECTED);
			break;
		case DM:
			stop_timer(&axp->t1);
			lapbstate(axp,DISCONNECTED);
			break;
		default:
			frmr(axp,0,0);
			break;
		}
		break;
	}
	free_p(bp);	/* In case anything's left */

	/* See if we can send some data, perhaps piggybacking an ack.
	 * If successful, lapb_output will clear axp->response.
	 */
	lapb_output(axp);

	/* Empty the trash */
	if(axp->state == DISCONNECTED)
		del_ax25(axp);
	return 0;
}
/* Handle incoming acknowledgements for frames we've sent.
 * Free frames being acknowledged.
 * Return -1 to cause a frame reject if number is bad, 0 otherwise
 */
static int
ackours(axp,n)
struct ax25_cb *axp;
char n;
{	
	struct mbuf *bp;
	int acked = 0;	/* Count of frames acked by this ACK */
	int oldest;	/* Seq number of oldest unacked I-frame */

	/* Free up acknowledged frames by purging frames from the I-frame
	 * transmit queue. Start at the remote end's last reported V(r)
	 * and keep going until we reach the new sequence number.
	 * If we try to free a null pointer,
	 * then we have a frame reject condition.
	 */
	oldest = (axp->vs - axp->unack) & MMASK;
	while(axp->unack != 0 && oldest != n){
		if((bp = dequeue(&axp->txq)) == NULLBUF){
			/* Acking unsent frame */
			return -1;
		}
		free_p(bp);
		axp->unack--;
		acked++;
		axp->retries = 0;
		oldest = (oldest + 1) & MMASK;
	}
	if(axp->unack == 0){
		/* All frames acked, stop timeout */
		stop_timer(&axp->t1);
		start_timer(&axp->t3);
	} else if(acked != 0) { 
		/* Partial ACK; restart timer */
		start_timer(&axp->t1);
	}
	/* If user has set a transmit upcall, indicate how many frames
	 * may be queued
	 */
	if(acked != 0 && axp->t_upcall != NULLVFP)
		(*axp->t_upcall)(axp,axp->paclen * (axp->maxframe - axp->unack));

	return 0;
}

/* Establish data link */
est_link(axp)
struct ax25_cb *axp;
{
	clr_ex(axp);
	axp->retries = 0;
	sendctl(axp,COMMAND,SABM|PF);
	stop_timer(&axp->t3);
	start_timer(&axp->t1);
}
/* Clear exception conditions */
clr_ex(axp)
struct ax25_cb *axp;
{
	axp->remotebusy = NO;
	axp->rejsent = NO;
	axp->response = 0;
	stop_timer(&axp->t3);
}
/* Enquiry response */
enq_resp(axp)
struct ax25_cb *axp;
{
	char ctl;

	ctl = len_mbuf(axp->rxq) >= axp->window ? RNR|PF : RR|PF;	
	sendctl(axp,RESPONSE,ctl);
	axp->response = 0;
	stop_timer(&axp->t3);
}
/* Invoke retransmission */
inv_rex(axp)
struct ax25_cb *axp;
{
	axp->vs -= axp->unack;
	axp->vs &= MMASK;
	axp->unack = 0;
}
/* Generate Frame Reject (FRMR) response
 * If reason != 0, this is the initial error frame
 * If reason == 0, resend the last error frame
 */
int
frmr(axp,control,reason)
register struct ax25_cb *axp;
char control;
char reason;
{
	struct mbuf *frmrinfo;
	register char *cp;
	void lapbstate();

	if(reason != 0){
		cp = axp->frmrinfo;
		*cp++ = control;
		*cp++ =  axp->vr << 5 || axp->vs << 1;
		*cp = reason;
	}
	if((frmrinfo = alloc_mbuf(3)) == NULLBUF)
		return -1;	/* No memory */
	frmrinfo->cnt = 3;
	memcpy(frmrinfo->data,axp->frmrinfo,3);
	return sendframe(axp,RESPONSE,FRMR|(control&PF),frmrinfo);
}

/* Send S or U frame to currently connected station */
int
sendctl(axp,cmdrsp,cmd)
struct ax25_cb *axp;
char cmdrsp,cmd;
{
	int16 ftype();

	if((ftype(cmd) & 0x3) == S)	/* Insert V(R) if S frame */
		cmd |= (axp->vr << 5);
	return sendframe(axp,cmdrsp,cmd,NULLBUF);
}
/* Start data transmission on link, if possible
 * Return number of frames sent
 */
int
lapb_output(axp)
register struct ax25_cb *axp;
{
	register struct mbuf *bp;
	struct mbuf *tbp;
	char control;
	int sent = 0;
	int i;

	if(axp == NULLAX25
	 || (axp->state != RECOVERY && axp->state != CONNECTED)
	 || axp->remotebusy)
		return 0;

	/* Dig into the send queue for the first unsent frame */
	bp = axp->txq;
	for(i = 0; i < axp->unack; i++){
		if(bp == NULLBUF)
			break;	/* Nothing to do */
		bp = bp->anext;
	}
	/* Start at first unsent I-frame, stop when either the
	 * number of unacknowledged frames reaches the maxframe limit,
	 * or when there are no more frames to send
	 */
	while(bp != NULLBUF && axp->unack < axp->maxframe){
		control = I | (axp->vs++ << 1) | (axp->vr << 5);
		axp->vs &= MMASK;
		dup_p(&tbp,bp,0,len_mbuf(bp));
		if(tbp == NULLBUF)
			return sent;	/* Probably out of memory */
		sendframe(axp,COMMAND,control,tbp);
		axp->unack++;
		/* We're implicitly acking any data he's sent, so stop any
		 * delayed ack
		 */
		axp->response = 0;
		stop_timer(&axp->t2);
		if(!run_timer(&axp->t1)){
			stop_timer(&axp->t3);
			start_timer(&axp->t1);
		}
		sent++;
		bp = bp->anext;
	}
	return sent;
}
/* Set new link state.
 * If the new state is disconnected, also free the link control block.
 */
void
lapbstate(axp,s)
struct ax25_cb *axp;
int s;
{
	int oldstate;

	oldstate = axp->state;
	axp->state = s;
	if(s == DISCONNECTED){
		stop_timer(&axp->t1);
		stop_timer(&axp->t2);
		stop_timer(&axp->t3);
		free_q(&axp->txq);
	}
	/* Don't bother the client unless the state is really changing */
	if(oldstate != s && axp->s_upcall != NULLVFP)
		(*axp->s_upcall)(axp,oldstate,s);
}
/* Process a valid incoming I frame */
static
procdata(axp,bp)
struct ax25_cb *axp;
struct mbuf *bp;
{
	char pid;
	int ip_route();

	/* Extract level 3 PID */
	if(pullup(&bp,&pid,1) != 1)
		return;	/* No PID */

	switch(pid & (PID_FIRST|PID_LAST)){
	case PID_FIRST:
		/* "Shouldn't happen", but flush any accumulated frags */
		free_p(axp->rxasm);
		axp->rxasm = NULLBUF;
	case 0:	/* Note fall-thru */
		/* Beginning or middle of message, just accumulate */
		append(&axp->rxasm,bp);
		return;
	case PID_LAST:
		/* Last frame of multi-frame message; extract it */
		append(&axp->rxasm,bp);
		bp = axp->rxasm;
		axp->rxasm = NULLBUF;
		break;
	case PID_FIRST|PID_LAST:
		/* Do nothing with reassembly queue, allowing single-frame
		 * messages to be interspersed with fragments of multi-frame
		 * messages
		 */
		break;
	}
	/* Last frame in sequence; kick entire message upstairs */
	switch(pid & PID_PID){
	case PID_IP:		/* DoD Internet Protocol */
		ip_route(bp,0);
		break;
	case PID_NO_L3:		/* Enqueue for application */
		append(&axp->rxq,bp);
		if(axp->r_upcall != NULLVFP)
			(*axp->r_upcall)(axp,len_mbuf(axp->rxq));
		break;	
	case PID_NETROM:
		nr_route(bp,axp);
		break;
	default:		/* Note: ARP is invalid here */	
		free_p(bp);
		break;			
	}
}

