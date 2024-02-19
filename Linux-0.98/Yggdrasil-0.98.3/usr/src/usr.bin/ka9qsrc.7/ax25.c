/* Low level AX.25 frame processing - address header */

#include <stdio.h>
#include <time.h>
#include "config.h"
#include "global.h"
#include "mbuf.h"
#include "iface.h"
#include "timer.h"
#include "arp.h"
#include "slip.h"
#include "ax25.h"
#include "lapb.h"
#include <ctype.h>
#include "heard.h"
#ifdef	UNIX
#include <memory.h>
#include <sys/types.h>
time_t time();
#endif

#ifdef MULPORT
extern int mport;
#endif

/* AX.25 broadcast address: "QST-0" in shifted ascii */
struct ax25_addr ax25_bdcst = {
     'Q'<<1, 'S'<<1, 'T'<<1, ' '<<1, ' '<<1, ' '<<1,
     ('0'<<1) | E,
};
#ifdef SID2
struct ax25_addr bbscall;
#endif
char axbdcst[AXALEN];    /* Same thing, network format */
struct ax25_addr mycall;
int digipeat = 1;   /* Controls digipeating */

/* Send IP datagrams across an AX.25 link */
/*ARGSUSED*/
int
ax_send(bp,interface,gateway,precedence,delay,throughput,reliability)
struct mbuf *bp;
struct interface *interface;
int32 gateway;
char precedence;
char delay;
char throughput;
char reliability;
{
     char *hw_addr,*res_arp();
     struct ax25_cb *axp,*find_ax25(),*open_ax25();
     struct ax25 addr;
     struct ax25_addr destaddr;
     struct mbuf *tbp;
     extern int16 axwindow;
     void ax_incom();
     int16 size,bsize,seq;

     if((hw_addr = res_arp(interface,ARP_AX25,gateway,bp)) == NULLCHAR)
          return 0; /* Wait for address resolution */

     if(delay || (!reliability && (interface->flags == DATAGRAM_MODE))){
          /* Use UI frame */
          return (*interface->output)(interface,hw_addr,
               interface->hwaddr,PID_FIRST|PID_LAST|PID_IP,bp);
     }
     /* Reliability is needed; use I-frames in AX.25 connection */
     memcpy(destaddr.call,hw_addr,ALEN);
     destaddr.ssid = hw_addr[ALEN];

     if((axp = find_ax25(&destaddr)) == NULLAX25 || (axp->state != CONNECTED && axp->state!=RECOVERY)){
          /*NOTE: ADDED W9NK's SABM FIX OF && AXP->STATE!=RECOVERY GRC*/
          /* Open a new connection or reinitialize the old one */
          atohax25(&addr,hw_addr,(struct ax25_addr *)interface->hwaddr);
          axp = open_ax25(&addr,axwindow,ax_incom,NULLVFP,NULLVFP,interface,(char *)0);
          if(axp == NULLAX25){
               free_p(bp);
               return -1;
          }
     }
     /* If datagram is too big for one frame, send all but the last with
      * the extension PID. Note: the copy to a new buf is necessary because
      * AX.25 may continue retransmitting the frame after a local TCB has
      * gone away, and using the buf directly would cause heap garbage to be
      * transmitted. Besides, nobody would ever use AX.25 anywhere
      * high performance is needed anyway...
      */
     bsize = len_mbuf(bp);
     seq = 0;
     while(bsize != 0){
          size = min(bsize,axp->paclen);
          /* Allocate buffer, allowing space for PID */
          if((tbp = alloc_mbuf(size + 1)) == NULLBUF)
               break;         /* out of memory! */
          *tbp->data = PID_IP;
          if(seq++ == 0)
               *tbp->data |= PID_FIRST;  /* First in sequence */
          if(size == bsize)
               *tbp->data |= PID_LAST;       /* That's all of it */
          /* else more to follow */

          tbp->cnt = 1;
          tbp->cnt += pullup(&bp,tbp->data + 1,size);
          send_ax25(axp,tbp);
          bsize -= size;
     }
     free_p(bp);    /* Shouldn't be necessary */
     return 0;
}
/* Add AX.25 link header and send packet.
 * Note that the calling order here must match ec_output
 * since ARP also uses it.
 */
int
ax_output(interface,dest,source,pid,data)
struct interface *interface;
char *dest;         /* Destination AX.25 address (7 bytes, shifted) */
               /* Also includes digipeater string */
char *source;       /* Source AX.25 address (7 bytes, shifted) */
char pid;      /* Protocol ID */
struct mbuf *data;  /* Data field (follows PID) */
{
     struct mbuf *abp,*cbp,*htonax25();
     struct ax25 addr;

     /* Allocate mbuf for control and PID fields, and fill in */
     if((cbp = pushdown(data,2)) == NULLBUF){
          free_p(data);
          return -1;
     }
     cbp->data[0] = UI;
     cbp->data[1] = pid;

     atohax25(&addr,dest,(struct ax25_addr *)source);
     if((abp = htonax25(&addr,cbp)) == NULLBUF){
          free_p(cbp);   /* Also frees data */
          return -1;
     }
     /* This shouldn't be necessary because redirection has already been
      * done at the IP router layer, but just to be safe...
      */
     if(interface->forw != NULLIF)
          return (*interface->forw->raw)(interface->forw,abp);
     else
          return (*interface->raw)(interface,abp);
}
/* Process incoming AX.25 packets.
 * After optional tracing, the address field is examined. If it is
 * directed to us as a digipeater, repeat it.  If it is addressed to
 * us or to QST-0, kick it upstairs depending on the protocol ID.
 */
int
ax_recv(interface,bp)
struct interface *interface;
struct mbuf *bp;
{
#ifdef MULPORT
     struct interface *repeater();
#endif
     void arp_input();
     int ip_route();
     struct ax25_addr *ap;
     struct mbuf *htonax25(),*hbp;
     char multicast;
     int nrnodes = 0;
     char control;
     struct ax25 hdr;
     struct ax25_cb *axp,*find_ax25(),*cr_ax25();
     struct ax25_addr ifcall;
     extern struct ax25_addr nr_nodebc ;
/* heard stuff */
     struct heard_stuff *hp;
     extern struct ax25_heard heard;
     int16 type, ftype();
     int prev, curr;
/* heard stuff */

     /* Use the address associated with this interface */
     memcpy(ifcall.call,interface->hwaddr,ALEN);
     ifcall.ssid = interface->hwaddr[ALEN];

     /* Pull header off packet and convert to host structure */
     if(ntohax25(&hdr,&bp) < 0){
          /* Something wrong with the header */
          free_p(bp);
          return;
     }
/* heard stuff */
	if (heard.enabled) {
		/* scan heard list.  if not there, add it */
		prev = -1;
		curr = heard.first;
		while (1) {
			if (curr != -1) {
				hp = &heard.list[curr];
				if (addreq(&hp->info.source, &hdr.source)) {
					time(&hp->htime);
					/* if not the first, make it so */
					if (prev != -1) {
						heard.list[prev].next = hp->next;
						hp->next = heard.first;
						heard.first = curr;
					}
					break;
				}
			}

			/* if the last one, add another if room, or overwrite it */
			if (curr == -1 || hp->next == -1) {
				/* not in heard list, add it.  if no room, bump oldest */
				if (heard.cnt < MAX_HEARD) {
					heard.list[heard.cnt].next = heard.first;
					heard.first = heard.cnt;
					hp = &heard.list[heard.cnt++];
				} else {
					heard.list[prev].next = -1;
                    heard.list[curr].next = heard.first;
					heard.first = curr;
					hp = &heard.list[curr];
				}
				hp->flags = 0;
				memcpy(&hp->info, &hdr, sizeof(struct ax25));
				time(&hp->htime);
				break;
			}
	
			prev = curr;
			curr = hp->next;
		}
		
		/* figure out what the other station is running */
		if (bp->cnt >= 2) {
			type = ftype(*bp->data);
			if(type == I || type == UI){	
				switch(*(bp->data + 1) & 0x3f){
				case PID_ARP:
					hp->flags |= HEARD_ARP;
					break;
				case PID_NETROM:
					hp->flags |= HEARD_NETROM;
					break;
				case PID_IP:
					hp->flags |= HEARD_IP;
					break;
				}
			}
		}
	} else {
		hp = NULL;
	}
/* heard stuff */
     /* Scan, looking for our call in the repeater fields, if any.
      * Repeat appropriate packets.
      */
     for(ap = &hdr.digis[0]; ap < &hdr.digis[hdr.ndigis]; ap++){
          if(ap->ssid & REPEATED)
               continue; /* Already repeated */
          /* Check if packet is directed to us as a digipeater */
          if(digipeat && addreq(ap,&ifcall)){
               /* Yes, kick it back out */
#ifdef MULPORT
/****************************************************************************
*                 Multiport repeater hack by KE4ZV                          *
****************************************************************************/
          if (mport){
              interface=repeater(ap,interface,&hdr);
             }
/****************************************************************************
*                  Multiport repeater hack ends                             *
****************************************************************************/
#endif
               ap->ssid |= REPEATED;
               if((hbp = htonax25(&hdr,bp)) != NULLBUF){
                    if(interface->forw != NULLIF)
                         (*interface->forw->raw)(interface->forw,hbp);
                    else
                         (*interface->raw)(interface,hbp);
                    bp = NULLBUF;
               }
          }
          free_p(bp);    /* Dispose if not forwarded */
          return;
     }
     /* Packet has passed all repeaters, now look at destination */
     if(addreq(&hdr.dest,&ax25_bdcst)){
          multicast = 1; /* Broadcast packet */
#ifdef SID2
     } else if(addreq(&hdr.dest,&ifcall) || addreq(&hdr.dest,&bbscall)){
#else
     } else if(addreq(&hdr.dest,&ifcall)){
#endif
          multicast = 0; /* Packet directed at us */
     } else if(addreq(&hdr.dest,&nr_nodebc)){
          nrnodes = 1 ;
     } else {
          /* Not for us */
          free_p(bp);
          return;
     }
     if(bp == NULLBUF){
          /* Nothing left */
          return;
     }
     /* Sneak a peek at the control field. This kludge is necessary because
      * AX.25 lacks a proper protocol ID field between the address and LAPB
      * sublayers; a control value of UI indicates that LAPB is to be
      * bypassed.
      */
     control = *bp->data & ~PF;
     if(uchar(control) == UI){
          char pid;

          (void) pullchar(&bp);
          if(pullup(&bp,&pid,1) != 1)
               return;        /* No PID */
          /* NET/ROM is very poorly layered. The meaning of the stuff
           * following the PID of CF depends on what's in the AX.25 dest
           * field.
           */
          if(nrnodes){
               if(uchar(pid) == (PID_NETROM | PID_FIRST | PID_LAST))
                    nr_nodercv(interface,&hdr.source,bp) ;
               else      /* regular UI packets to "nodes" aren't for us */
                    free_p(bp) ;
               return ;
          }
          /* Handle packets. Multi-frame messages are not allowed */
          switch(pid & (PID_FIRST | PID_LAST | PID_PID)){
          case (PID_IP | PID_FIRST | PID_LAST):
               ip_route(bp,multicast);
               break;
          case (PID_ARP | PID_FIRST | PID_LAST):
               arp_input(interface,bp);
               break;
          default:
               free_p(bp);
               break;
          }
          return;
     }
     /* Everything from here down is LAPB stuff, so drop anything
      * not directed to us:
      */

     if (multicast || nrnodes) {
          free_p(bp) ;
          return ;
     }

     /* Find the source address in hash table */
     if((axp = find_ax25(&hdr.source)) == NULLAX25){
          /* Create a new ax25 entry for this guy,
           * insert into hash table keyed on his address,
           * and initialize table entries
           */
          if((axp = cr_ax25(&hdr.source)) == NULLAX25){
               free_p(bp);
               return;
          }
          axp->interface = interface;
          /* Swap source and destination, reverse digi string */
          ASSIGN(axp->addr.dest,hdr.source);
          ASSIGN(axp->addr.source,hdr.dest);
          if(hdr.ndigis > 0){
               int i,j;

               /* Construct reverse digipeater path */
               for(i=hdr.ndigis-1,j=0;i >= 0;i--,j++){
                    ASSIGN(axp->addr.digis[j],hdr.digis[i]);
                    axp->addr.digis[j].ssid &= ~(E|REPEATED);
               }
               /* Scale timers to account for extra delay */
               axp->t1.start *= hdr.ndigis+1;
               axp->t2.start *= hdr.ndigis+1;
               axp->t3.start *= hdr.ndigis+1;
          }
          axp->addr.ndigis = hdr.ndigis;
     }
     if(hdr.cmdrsp == UNKNOWN)
          axp->proto = V1;    /* Old protocol in use */
     else
          axp->proto = V2;

     lapb_input(axp,hdr.cmdrsp,bp);
}
/* Initialize AX.25 entry in arp device table */
/* General purpose AX.25 frame output */
int
sendframe(axp,cmdrsp,ctl,data)
struct ax25_cb *axp;
char cmdrsp;
char ctl;
struct mbuf *data;
{
     struct mbuf *hbp,*cbp,*htonax25();
     int i;

     if(axp == NULLAX25 || axp->interface == NULLIF)
          return -1;

     /* Add control field */
     if((cbp = pushdown(data,1)) == NULLBUF){
          free_p(data);
          return -1;
     }
     cbp->data[0] = ctl;

     axp->addr.cmdrsp = cmdrsp;
     /* Create address header */
     if((hbp = htonax25(&axp->addr,cbp)) == NULLBUF){
          free_p(cbp);
          return -1;
     }
     /* The packet is all ready, now send it */
     if(axp->interface->forw != NULLIF)
          i = (*axp->interface->forw->raw)(axp->interface->forw,hbp);
     else
          i = (*axp->interface->raw)(axp->interface,hbp);

     return i;
}
axarp()
{
     int psax25(),setpath();

     memcpy(axbdcst,ax25_bdcst.call,ALEN);
     axbdcst[ALEN] = ax25_bdcst.ssid;

     arp_init(ARP_AX25,AXALEN,PID_FIRST|PID_LAST|PID_IP,
      PID_FIRST|PID_LAST|PID_ARP,axbdcst,psax25,setpath);
}
