/* Process incoming TCP segments. Page number references are to ARPA RFC-793,
 * the TCP specification.
 */

#include "global.h"
#include "timer.h"
#include "mbuf.h"
#include "netuser.h"
#include "internet.h"
#include "tcp.h"
#include "icmp.h"
#include "iface.h"
#include "ip.h"

struct tcp_stat tcp_stat;

/* This function is called from IP with the IP header in machine byte order,
 * along with a mbuf chain pointing to the TCP header.
 */
void
tcp_input(bp,protocol,source,dest,tos,length,rxbroadcast)
struct mbuf *bp;	/* Data field, if any */
char protocol;		/* Should always be TCP_PTCL */
int32 source;		/* Remote IP address */
int32 dest;		/* Our IP address */
char tos;		/* Type of Service */
int16 length;		/* Length of data field */
char rxbroadcast;	/* Incoming broadcast - discard if true */
{
	void reset(),update();
	void proc_syn(),send_syn(),add_reseq(),get_reseq(),unlink_tcb();

	register struct tcb *tcb;	/* TCP Protocol control block */
	struct tcp seg;			/* Local copy of segment header */
	struct connection conn;		/* Local copy of addresses */
	struct pseudo_header ph;	/* Pseudo-header for checksumming */
	int hdrlen;			/* Length of TCP header */

	if(bp == NULLBUF)
		return;

	if(rxbroadcast){
		/* Any TCP packet arriving as a broadcast is
		 * to be completely IGNORED!!
		 */
		tcp_stat.bdcsts++;
		free_p(bp);
		return;
	}
	ph.source = source;
	ph.dest = dest;
	ph.protocol = protocol;
	ph.length = length;
	if(cksum(&ph,bp,length) != 0){
		/* Checksum failed, ignore segment completely */
		tcp_stat.checksum++;
		free_p(bp);
		return;
	}
	/* Form local copy of TCP header in host byte order */
	if((hdrlen = ntohtcp(&seg,&bp)) < 0){
		/* TCP header is too small */
		tcp_stat.runt++;
		free_p(bp);
		return;
	}
	/*
	 * internally, we keep the urgent pointer in sequence space,
	 */
	seg.up = seg.up + seg.seq;
	length -= hdrlen;

	/* Fill in connection structure and find TCB */
	conn.local.address = dest;
	conn.local.port = seg.dest;
	conn.remote.address = source;
	conn.remote.port = seg.source;
	
	if((tcb = lookup_tcb(&conn)) == NULLTCB){
		struct tcb *ntcb;
		void link_tcb();

		/* Check that this segment carries a SYN, and that
		 * there's a LISTEN on this socket with
		 * unspecified source address and port
		 */
		conn.remote.address = 0;
		conn.remote.port = 0;
		if(!(seg.flags & SYN) || (tcb = lookup_tcb(&conn)) == NULLTCB){
			/* No unspecified LISTEN either, so reject */
			free_p(bp);
			reset(source,dest,tos,length,&seg);
			return;
		}
		/* We've found an server listen socket, so clone the TCB */
		if(tcb->flags & CLONE){
			if((ntcb = (struct tcb *)malloc(sizeof (struct tcb))) == NULLTCB){
				free_p(bp);
				/* This may fail, but we should at least try */
				reset(source,dest,tos,length,&seg);
				return;
			}
			ASSIGN(*ntcb,*tcb);
			tcb = ntcb;
			tcb->timer.arg = (char *)tcb;
		} else
			unlink_tcb(tcb);	/* It'll be put back on later */

		/* Stuff the foreign socket into the TCB */
		tcb->conn.remote.address = source;
		tcb->conn.remote.port = seg.source;

		/* NOW put on right hash chain */
		link_tcb(tcb);
	}
	/* Do unsynchronized-state processing (p. 65-68) */
	switch(tcb->state){
	case CLOSED:
		free_p(bp);
		reset(source,dest,tos,length,&seg);
		return;
	case LISTEN:
		if(seg.flags & RST){
			free_p(bp);
			return;
		}
		if(seg.flags & ACK){
			free_p(bp);
			reset(source,dest,tos,length,&seg);
			return;
		}
		if(seg.flags & SYN){
			/* (Security check is bypassed) */
			/* page 66 */
			tcp_stat.conin++;
			proc_syn(tcb,tos,&seg);
			send_syn(tcb);
			setstate(tcb,SYN_RECEIVED);		
			if(length != 0 || (seg.flags & FIN)) {
				break;		/* Continue processing if there's more */
			}
			tcp_output(tcb);
		}
		free_p(bp);	/* Unlikely to get here directly */
		return;
	case SYN_SENT:
		if(seg.flags & ACK){
			if(!seq_within(seg.ack,tcb->iss+1,tcb->snd.nxt)){
				free_p(bp);
				reset(source,dest,tos,length,&seg);
				return;
			}
		}
		if(seg.flags & RST){	/* p 67 */
			if(seg.flags & ACK){
				/* The ack must be acceptable since we just checked it.
				 * This is how the remote side refuses connect requests.
				 */
				close_self(tcb,RESET);
			}
			free_p(bp);
			return;
		}
		/* (Security check skipped here) */
		/* Check incoming precedence; it must match if there's an ACK */
		if((seg.flags & ACK) && PREC(tos) != PREC(tcb->tos)){
			free_p(bp);
			reset(source,dest,tos,length,&seg);
			return;
		}
		if(seg.flags & SYN){
			proc_syn(tcb,tos,&seg);
			if(seg.flags & ACK){
				/* Our SYN has been acked, otherwise the ACK
				 * wouldn't have been valid.
				 */
				update(tcb,&seg,length);
				setstate(tcb,ESTABLISHED);
			} else {
				setstate(tcb,SYN_RECEIVED);
			}
			if(length != 0 || (seg.flags & FIN)) {
				break;		/* Continue processing if there's more */
			}
			tcp_output(tcb);
		} else {
			free_p(bp);	/* Ignore if neither SYN or RST is set */
		}
		return;
	}
	/* We reach this point directly in any synchronized state. Note that
	 * if we fell through from LISTEN or SYN_SENT processing because of a
	 * data-bearing SYN, window trimming and sequence testing "cannot fail".
	 */

	/* Trim segment to fit receive window. */
	if(trim(tcb,&seg,&bp,&length) == -1){
		/* Segment is unacceptable */
		if(!(seg.flags & RST)){
			tcb->flags |= FORCE;
			tcp_output(tcb);
		}
		return;
	}
	/* If segment isn't the next one expected, and there's data
	 * or flags associated with it, put it on the resequencing
	 * queue, ACK it and return.
	 *
	 * Processing the ACK in an out-of-sequence segment without
	 * flags or data should be safe, however.
	 */
	if(seg.seq != tcb->rcv.nxt
	 && (length != 0 || (seg.flags & (SYN|FIN)) )){
		add_reseq(tcb,tos,&seg,bp,length);
		tcb->flags |= FORCE;
		tcp_output(tcb);
		return;
	}
	/* This loop first processes the current segment, and then
	 * repeats if it can process the resequencing queue.
	 */
	for(;;){
		/* We reach this point with an acceptable segment; all data and flags
		 * are in the window, and the starting sequence number equals rcv.nxt
		 * (p. 70)
		 */	
		if(seg.flags & RST){
			if(tcb->state == SYN_RECEIVED
			 && !(tcb->flags & (CLONE|ACTIVE))){
				/* Go back to listen state only if this was
				 * not a cloned or active server TCB
				 */
				setstate(tcb,LISTEN);
			} else {
				close_self(tcb,RESET);
			}
			free_p(bp);
			return;
		}
		/* (Security check skipped here) p. 71 */
		/* Check for precedence mismatch or erroneous extra SYN */
		if(PREC(tos) != PREC(tcb->tos) || (seg.flags & SYN)){
			free_p(bp);
			reset(source,dest,tos,length,&seg);
			return;
		}
		/* Check ack field p. 72 */
		if(!(seg.flags & ACK)){
			free_p(bp);	/* All segments after synchronization must have ACK */
			return;
		}
		/* Process ACK */
		switch(tcb->state){
		case SYN_RECEIVED:
			if(seq_within(seg.ack,tcb->snd.una+1,tcb->snd.nxt)){
				update(tcb,&seg,length);
				setstate(tcb,ESTABLISHED);
			} else {
				free_p(bp);
				reset(source,dest,tos,length,&seg);
				return;
			}
			break;
		case ESTABLISHED:
		case CLOSE_WAIT:
			update(tcb,&seg,length);
			break;
		case FINWAIT1:	/* p. 73 */
			update(tcb,&seg,length);
			if(tcb->sndcnt == 0){
				/* Our FIN is acknowledged */
				setstate(tcb,FINWAIT2);
			}
			break;
		case FINWAIT2:
			update(tcb,&seg,length);
			break;
		case CLOSING:
			update(tcb,&seg,length);
			if(tcb->sndcnt == 0){
				/* Our FIN is acknowledged */
				setstate(tcb,TIME_WAIT);
				tcb->timer.start = MSL2 * (1000 / MSPTICK);
				start_timer(&tcb->timer);
			}
			break;
		case LAST_ACK:
			update(tcb,&seg,length);
			if(tcb->sndcnt == 0){
				/* Our FIN is acknowledged, close connection */
				close_self(tcb,NORMAL);
				return;
			}			
/* I think this is wrong, and can cause permanent ACK-ACK loops.  dmf.
		case TIME_WAIT:
			tcb->flags |= FORCE;
			start_timer(&tcb->timer);
*/
		}

		/* URGent bit processing */

		if(seg.flags & URG){
			if ((tcb->flags & URGCUR) == 0 ||
			    seq_gt(seg.up, tcb->rcv.up)) {
				tcb->flags |= URGCUR;
				tcb->rcv.up = seg.up;
			}
		}

		/* Process the segment text, if any, beginning at rcv.nxt (p. 74) */
		if(length != 0){
			switch(tcb->state){
			case SYN_RECEIVED:
			case ESTABLISHED:
			case FINWAIT1:
			case FINWAIT2:
				/* Place on receive queue */
				append(&tcb->rcvq,bp);
				tcb->rcvcnt += length;
				tcb->rcv.nxt += length;
				tcb->rcv.wnd -= length;
				tcb->flags |= FORCE;
				break;
			default:
				/* Ignore segment text */
				free_p(bp);
				break;
			}
		}
		/* If the user has set up a r_upcall function and there's
		 * data to be read, notify him.
		 *
		 * This is done before sending an acknowledgement,
		 * to give the user a chance to piggyback some reply data.
		 * It's also done before processing FIN so that the state
		 * change upcall will occur after the user has had a chance
		 * to read the last of the incoming data.
		 */
		if(tcb->r_upcall && tcb->rcvcnt != 0){
			(*tcb->r_upcall)(tcb,tcb->rcvcnt);
		}
		/* process FIN bit (p 75) */
		if(seg.flags & FIN){
			tcb->flags |= FORCE;	/* Always respond with an ACK */

			switch(tcb->state){
			case SYN_RECEIVED:
			case ESTABLISHED:
				tcb->rcv.nxt++;
				setstate(tcb,CLOSE_WAIT);
				break;
			case FINWAIT1:
				tcb->rcv.nxt++;
				if(tcb->sndcnt == 0){
					/* Our FIN has been acked; bypass CLOSING state */
					setstate(tcb,TIME_WAIT);
					tcb->timer.start = MSL2 * (1000/MSPTICK);
					start_timer(&tcb->timer);
				} else {
					setstate(tcb,CLOSING);
				}
				break;
			case FINWAIT2:
				tcb->rcv.nxt++;
				setstate(tcb,TIME_WAIT);
				tcb->timer.start = MSL2 * (1000/MSPTICK);
				start_timer(&tcb->timer);
				break;
			case CLOSE_WAIT:
			case CLOSING:
			case LAST_ACK:
				break;		/* Ignore */
			case TIME_WAIT:	/* p 76 */
				start_timer(&tcb->timer);
				break;
			}
		}
		/* Scan the resequencing queue, looking for a segment we can handle,
		 * and freeing all those that are now obsolete.
		 */
		while(tcb->reseq != NULLRESEQ && seq_ge(tcb->rcv.nxt,tcb->reseq->seg.seq)){
			get_reseq(tcb,&tos,&seg,&bp,&length);
			if(trim(tcb,&seg,&bp,&length) == 0)
				goto gotone;
			/* Segment is an old one; trim has freed it */
		}
		break;
gotone:	;
	}
	tcp_output(tcb);	/* Send any necessary ack */
}

/* Process an incoming ICMP response */
tcp_icmp(source,dest,type,code,bpp)
int32 source;			/* Original IP datagram source (i.e. us) */
int32 dest;			/* Original IP datagram dest (i.e., them) */
char type,code;			/* ICMP error codes */
struct mbuf **bpp;		/* First 8 bytes of TCP header */
{
	struct tcp seg;
	struct connection conn;
	register struct tcb *tcb;

	/* Extract the socket info from the returned TCP header fragment
	 * Note that since this is a datagram we sent, the source fields
	 * refer to the local side.
	 */
	ntohtcp(&seg,bpp);
	conn.local.port = seg.source;
	conn.remote.port = seg.dest;
	conn.local.address = source;
	conn.remote.address = dest;
	if((tcb = lookup_tcb(&conn)) == NULLTCB)
		return;	/* Unknown connection, ignore */

	/* Verify that the sequence number in the returned segment corresponds
	 * to something currently unacknowledged. If not, it can safely
	 * be ignored.
	 */
	if(!seq_within(seg.seq,tcb->snd.una,tcb->snd.nxt))
		return;

	/* The strategy here is that Destination Unreachable and Time Exceeded
	 * messages that occur after a connection has been established are likely
	 * to be transient events, and shouldn't kill our connection (at least
	 * until after we've tried a few more times). On the other hand, if
	 * they occur on our very first attempt to send a datagram on a new
	 * connection, they're probably "for real". In any event, the info
	 * is saved.
	 */
	switch(uchar(type)){
	case DEST_UNREACH:
	case TIME_EXCEED:
		tcb->type = type;
		tcb->code = code;
		if(tcb->state == SYN_SENT || tcb->state == SYN_RECEIVED){
			close_self(tcb,NETWORK);
		}
		break;
	case QUENCH:
		/* Source quench; cut the congestion window in half,
		 * but don't let it go below one packet
		 */
		tcb->cwind /= 2;
		tcb->cwind = max(tcb->mss,tcb->cwind);
		break;
	}
}
/* Send an acceptable reset (RST) response for this segment
 * The RST reply is composed in place on the input segment
 */
static void
reset(source,dest,tos,length,seg)
int32 source;			/* Remote IP address */
int32 dest;			/* Our IP address */
char tos;			/* Type of Service */
int16 length;			/* Length of data portion */
register struct tcp *seg;	/* Offending TCP header */
{
	struct mbuf *hbp;
	struct pseudo_header ph;
	int16 tmp;
	char rflags;

	if(seg->flags & RST)
		return;	/* Never send an RST in response to an RST */

	tcp_stat.resets++;

	/* Compose the RST IP pseudo-header, swapping addresses */
	ph.source = dest;
	ph.dest = source;
	ph.protocol = TCP_PTCL;
	ph.length = TCPLEN;

	/* Swap port numbers */
	tmp = seg->dest;
	seg->dest = seg->source;
	seg->source = tmp;

	rflags = RST;
	if(seg->flags & ACK){
		/* This reset is being sent to clear a half-open connection.
		 * Set the sequence number of the RST to the incoming ACK
		 * so it will be acceptable.
		 */
		seg->seq = seg->ack;
		seg->ack = 0;
	} else {
		/* We're rejecting a connect request (SYN) from LISTEN state
		 * so we have to "acknowledge" their SYN.
		 */
		rflags |= ACK;
		seg->ack = seg->seq;
		seg->seq = 0;
		if(seg->flags & SYN)
			seg->ack++;
		seg->ack += length;
		if(seg->flags & FIN)
			seg->ack++;
	}
	seg->flags = rflags;
	seg->wnd = 0;
	seg->up = 0;
	seg->mss = 0;
	if((hbp = htontcp(seg,NULLBUF,&ph)) == NULLBUF)
		return;
	/* Ship it out (note swap of addresses) */
	ip_send(dest,source,TCP_PTCL,tos,0,hbp,ph.length,0,0);
}

/* Process an incoming acknowledgement and window indication.
 * From page 72.
 */
static void
update(tcb,seg,length)
register struct tcb *tcb;
register struct tcp *seg;
int16 length;
{
	int16 acked;
	int16 expand;

	acked = 0;
	if(seq_gt(seg->ack,tcb->snd.nxt)){
		tcb->flags |= FORCE;	/* Acks something not yet sent */
		return;
	}
	/* Decide if we need to do a window update.
	 * This is always checked whenever a legal ACK is received,
	 * even if it doesn't actually acknowledge anything,
	 * because it might be a spontaneous window reopening.
	 */
	if(seq_gt(seg->seq,tcb->snd.wl1) || ((seg->seq == tcb->snd.wl1) 
	 && seq_ge(seg->ack,tcb->snd.wl2))){
		/* If the window had been closed, crank back the
		 * send pointer so we'll immediately resume transmission.
		 * Otherwise we'd have to wait until the next probe.
		 */
		if(tcb->snd.wnd == 0 && seg->wnd != 0)
			tcb->snd.ptr = tcb->snd.una;
		tcb->snd.wnd = seg->wnd;
		tcb->snd.wl1 = seg->seq;
		tcb->snd.wl2 = seg->ack;
	}
	/* See if anything new is being acknowledged */
	if(!seq_gt(seg->ack,tcb->snd.una)) {
		if(seg->ack != tcb->snd.una)
			return;	/* Old ack, ignore */

		if(seg->flags & SYN)
		if(length != 0 || (seg->flags & SYN) || (seg->flags & FIN))
			return;	/* Nothing acked, but there is data */

		/* Van Jacobson "fast recovery" code */
		if(++tcb->dupacks == TCPDUPACKS){
			/* We've had a burst of do-nothing acks, so
			 * we almost certainly lost a packet.
			 * Resend it now to avoid a timeout. (This is
			 * Van Jacobson's 'quick recovery' algorithm.)
			 */
			int32 ptrsave;

			/* Knock the threshold down just as though
			 * this were a timeout, since we've had
			 * network congestion.
			 */
			tcb->ssthresh = tcb->cwind/2;
			tcb->ssthresh = max(tcb->ssthresh,tcb->mss);

			/* Manipulate the machinery in tcp_output() to
			 * retransmit just the missing packet
			 */
			ptrsave = tcb->snd.ptr;
 			tcb->snd.ptr = tcb->snd.una;
			tcb->cwind = tcb->mss;
			tcp_output(tcb);
			tcb->snd.ptr = ptrsave;

			/* "Inflate" the congestion window, pretending as
			 * though the duplicate acks were normally acking
			 * the packets beyond the one that was lost.
			 */
			tcb->cwind = tcb->ssthresh + TCPDUPACKS*tcb->mss;
		} else if(tcb->dupacks > TCPDUPACKS){
			/* Continue to inflate the congestion window
			 * until the acks finally get "unstuck".
			 */
			tcb->cwind += tcb->mss;
		}
		return;
	}
	if(tcb->dupacks >= TCPDUPACKS && tcb->cwind > tcb->ssthresh){
		/* The acks have finally gotten "unstuck". So now we
		 * can "deflate" the congestion window, i.e. take it
		 * back down to where it would be after slow start
		 * finishes.
		 */
		tcb->cwind = tcb->ssthresh;
	}
	tcb->dupacks = 0;

	/* We're here, so the ACK must have actually acked something */
	acked = seg->ack - tcb->snd.una;

	/* Expand congestion window if not already at limit */
	if(tcb->cwind < tcb->snd.wnd && !(tcb->flags & RETRAN)) {
		if(tcb->cwind < tcb->ssthresh){
			/* Still doing slow start/CUTE, expand by amount acked */
			expand = min(acked,tcb->mss);
		} else {
			/* Steady-state test of extra path capacity */
			expand = ((long)tcb->mss * tcb->mss) / tcb->cwind;
		}
		/* Guard against arithmetic overflow */
		if(tcb->cwind + expand < tcb->cwind)
			expand = MAXINT16 - tcb->cwind;

		/* Don't expand beyond the offered window */
		if(tcb->cwind + expand > tcb->snd.wnd)
			expand = tcb->snd.wnd - tcb->cwind;

		if(expand != 0){
#ifdef	notdef
			/* Kick up the mean deviation estimate to prevent
			 * unnecessary retransmission should we already be
			 * bandwidth limited
			 */
			tcb->mdev += ((long)tcb->srtt * expand) / tcb->cwind;
#endif
			tcb->cwind += expand;
		}
	}
	/* Round trip time estimation */
	if(run_timer(&tcb->rtt_timer) && seq_ge(seg->ack,tcb->rttseq)){
		/* A timed sequence number has been acked */
		stop_timer(&tcb->rtt_timer);
		if(!(tcb->flags & RETRAN)){
			int32 rtt;	/* measured round trip time */
			int32 abserr;	/* abs(rtt - srtt) */

			/* This packet was sent only once and now
			 * it's been acked, so process the round trip time
			 */
			rtt = tcb->rtt_timer.start - tcb->rtt_timer.count;
			rtt *= MSPTICK;		/* milliseconds */

			/* If this ACKs our SYN, this is the first ACK
			 * we've received; base our entire SRTT estimate
			 * on it. Otherwise average it in with the prior
			 * history, also computing mean deviation.
			 */
			if(rtt > tcb->srtt &&
			 (tcb->state == SYN_SENT || tcb->state == SYN_RECEIVED)){
				tcb->srtt = rtt;
			} else {
				abserr = (rtt > tcb->srtt) ? rtt - tcb->srtt : tcb->srtt - rtt;
				tcb->srtt = ((AGAIN-1)*tcb->srtt + rtt) / AGAIN;
				tcb->mdev = ((DGAIN-1)*tcb->mdev + abserr) / DGAIN;
			}
			/* Reset the backoff level */
			tcb->backoff = 0;
		}
	}
	/* If we're waiting for an ack of our SYN, note it and adjust count */
	if(!(tcb->flags & SYNACK)){
		tcb->flags |= SYNACK;
		acked--;
		tcb->sndcnt--;
	}
	/* Remove acknowledged bytes from the send queue and update the
	 * unacknowledged pointer. If a FIN is being acked,
	 * pullup won't be able to remove it from the queue.
	 */
	pullup(&tcb->sndq,NULLCHAR,acked);

	/* This will include the FIN if there is one */
	tcb->sndcnt -= acked;
	tcb->snd.una = seg->ack;

	/* Stop retransmission timer, but restart it if there is still
	 * unacknowledged data.
	 */	
	stop_timer(&tcb->timer);
	if(tcb->snd.una != tcb->snd.nxt)
		start_timer(&tcb->timer);

	/* If retransmissions have been occurring, make sure the
	 * send pointer doesn't repeat ancient history
	 */
	if(seq_lt(tcb->snd.ptr,tcb->snd.una))
		tcb->snd.ptr = tcb->snd.una;

	/* Clear the retransmission flag since the oldest
	 * unacknowledged segment (the only one that is ever retransmitted)
	 * has now been acked.
	 */
	tcb->flags &= ~RETRAN;

	/* If outgoing data was acked, notify the user so he can send more
	 * unless we've already sent a FIN.
	 */
	if(acked != 0 && tcb->t_upcall){
		switch(tcb->state){
	 	case ESTABLISHED:
		case CLOSE_WAIT:
			(*tcb->t_upcall)(tcb,tcb->window - tcb->sndcnt);
		}
	}
}

/* Determine if the given sequence number is in our receiver window.
 * NB: must not be used when window is closed!
 */
static
int
in_window(tcb,seq)
struct tcb *tcb;
int32 seq;
{
	return seq_within(seq,tcb->rcv.nxt,(int32)(tcb->rcv.nxt+tcb->rcv.wnd-1));
}

/* Process an incoming SYN */
static void
proc_syn(tcb,tos,seg)
register struct tcb *tcb;
char tos;
struct tcp *seg;
{
	int16 mtu,ip_mtu();

	tcb->flags |= FORCE;	/* Always send a response */

	/* Note: It's not specified in RFC 793, but SND.WL1 and
	 * SND.WND are initialized here since it's possible for the
	 * window update routine in update() to fail depending on the
	 * IRS if they are left unitialized.
	 */
	/* Check incoming precedence and increase if higher */
	if(PREC(tos) > PREC(tcb->tos))
		tcb->tos = tos;
	tcb->rcv.nxt = seg->seq + 1;	/* p 68 */
	tcb->snd.wl1 = tcb->irs = seg->seq;
	tcb->snd.wnd = seg->wnd;
	if(seg->mss != 0)
		tcb->mss = min(seg->mss, tcp_mss);
	/* Check the MTU of the interface we'll use to reach this guy
	 * and lower the MSS so that unnecessary fragmentation won't occur
	 */
	if((mtu = ip_mtu(tcb->conn.remote.address)) != 0){
		/* Allow space for the TCP and IP headers */
		mtu -= TCPLEN + IPLEN;
		tcb->cwind = tcb->mss = min(mtu,tcb->mss);
	}
}

/* Generate an initial sequence number and put a SYN on the send queue */
void
send_syn(tcb)
register struct tcb *tcb;
{
	tcb->iss = iss();
	tcb->rttseq = tcb->snd.wl2 = tcb->snd.una = tcb->iss;
	tcb->snd.ptr = tcb->snd.nxt = tcb->rttseq;
	tcb->sndcnt++;
	tcb->flags |= FORCE;
}

/* Add an entry to the resequencing queue in the proper place */
static void
add_reseq(tcb,tos,seg,bp,length)
struct tcb *tcb;
char tos;
struct tcp *seg;
struct mbuf *bp;
int16 length;
{
	register struct reseq *rp,*rp1;

	/* Allocate reassembly descriptor */
	if((rp = (struct reseq *)malloc(sizeof (struct reseq))) == NULLRESEQ){
		/* No space, toss on floor */
		free_p(bp);
		return;
	}
	ASSIGN(rp->seg,*seg);
	rp->tos = tos;
	rp->bp = bp;
	rp->length = length;

	/* Place on reassembly list sorting by starting seq number */
	rp1 = tcb->reseq;
	if(rp1 == NULLRESEQ || seq_lt(seg->seq,rp1->seg.seq)){
		/* Either the list is empty, or we're less than all other
		 * entries; insert at beginning.
		 */
		rp->next = rp1;
		tcb->reseq = rp;
	} else {
		/* Find the last entry less than us */
		for(;;){
			if(rp1->next == NULLRESEQ || seq_lt(seg->seq,rp1->next->seg.seq)){
				/* We belong just after this one */
				rp->next = rp1->next;
				rp1->next = rp;
				break;
			}
			rp1 = rp1->next;
		}
	}
}

/* Fetch the first entry off the resequencing queue */
static void
get_reseq(tcb,tos,seg,bp,length)
register struct tcb *tcb;
char *tos;
struct tcp *seg;
struct mbuf **bp;
int16 *length;
{
	register struct reseq *rp;

	if((rp = tcb->reseq) == NULLRESEQ)
		return;

	tcb->reseq = rp->next;

	*tos = rp->tos;
	ASSIGN(*seg,rp->seg);
	*bp = rp->bp;
	*length = rp->length;
	free((char *)rp);
}

/* Trim segment to fit window. Return 0 if OK, -1 if segment is
 * unacceptable.
 */
static int
trim(tcb,seg,bp,length)
register struct tcb *tcb;
register struct tcp *seg;
struct mbuf **bp;
int16 *length;
{
	struct mbuf *nbp;
	long dupcnt,excess;
	int16 len;		/* Segment length including flags */
	char accept;

	accept = 0;
	len = *length;
	if(seg->flags & SYN)
		len++;
	if(seg->flags & FIN)
		len++;

	/* Acceptability tests */
	if(tcb->rcv.wnd == 0){
		/* Only in-order, zero-length segments are acceptable when our window
		 * is closed.
		 */
		if(seg->seq == tcb->rcv.nxt && len == 0){
			return 0;	/* Acceptable, no trimming needed */
		}
	} else {
		/* Some part of the segment must be in the window */
		if(in_window(tcb,seg->seq)){
			accept++;	/* Beginning is */
		} else if(len != 0){
			if(in_window(tcb,(int32)(seg->seq+len-1)) || /* End is */
			 seq_within(tcb->rcv.nxt,seg->seq,(int32)(seg->seq+len-1))){ /* Straddles */
				accept++;
			}
		}
	}
	if(!accept){
		free_p(*bp);
		return -1;
	}
	dupcnt = tcb->rcv.nxt - seg->seq;
	if(dupcnt > 0){
		tcb->rerecv += dupcnt;
		/* Trim off SYN if present */
		if(seg->flags & SYN){
			/* SYN is before first data byte */
			seg->flags &= ~SYN;
			seg->seq++;
			dupcnt--;
		}
		if(dupcnt > 0){
			pullup(bp,NULLCHAR,(int16)dupcnt);
			seg->seq += dupcnt;
			*length -= dupcnt;
		}
	}
	excess = seg->seq + *length - (tcb->rcv.nxt + tcb->rcv.wnd);
	if(excess > 0){
		tcb->rerecv += excess;
		/* Trim right edge */
		*length -= excess;
		nbp = copy_p(*bp,*length);
		free_p(*bp);
		*bp = nbp;
		seg->flags &= ~FIN;	/* FIN follows last data byte */
	}
	return 0;
}
