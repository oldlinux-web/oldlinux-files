#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "ax25.h"
#include "timer.h"
#include "lapb.h"
#include "trace.h"
#ifdef	UNIX
#include <memory.h>
#endif

extern FILE *trfp;

/* Dump an AX.25 packet header */
/*ARGSUSED*/
ax25_dump(bpp,check)
struct mbuf **bpp;
int check;	/* Not used */
{
	char *decode_type();
	char tmp[20];
	char control,pid;
	int16 type,ftype();
	struct ax25 hdr;
	struct ax25_addr *hp;

	fprintf(trfp,"AX25: ");
	/* Extract the address header */
	if(ntohax25(&hdr,bpp) < 0){
		/* Something wrong with the header */
		fprintf(trfp," bad header!\n");
		return;
	}
	pax25(tmp,&hdr.source);
	fprintf(trfp,"%s",tmp);
	pax25(tmp,&hdr.dest);
	fprintf(trfp,"->%s",tmp);
	if(hdr.ndigis > 0){
		fprintf(trfp," v");
		for(hp = &hdr.digis[0]; hp < &hdr.digis[hdr.ndigis]; hp++){
			/* Print digi string */
			pax25(tmp,hp);
			fprintf(trfp," %s%s",tmp,(hp->ssid & REPEATED) ? "*":"");
		}
	}
	if(pullup(bpp,&control,1) != 1)
		return;

	putchar(' ');
	type = ftype(control);
	fprintf(trfp,"%s",decode_type(type));
	/* Dump poll/final bit */
	if(control & PF){
		switch(hdr.cmdrsp){
		case COMMAND:
			fprintf(trfp,"(P)");
			break;
		case RESPONSE:
			fprintf(trfp,"(F)");
			break;
		default:
			fprintf(trfp,"(P/F)");
			break;
		}
	}
	/* Dump sequence numbers */
	if((type & 0x3) != U)	/* I or S frame? */
		fprintf(trfp," NR=%d",(control>>5)&7);
	if(type == I || type == UI){	
		if(type == I)
			fprintf(trfp," NS=%d",(control>>1)&7);
		/* Decode I field */
		if(pullup(bpp,&pid,1) == 1){	/* Get pid */
			switch(pid & (PID_FIRST | PID_LAST)){
			case PID_FIRST:
				fprintf(trfp," First frag");
				break;
			case PID_LAST:
				fprintf(trfp," Last frag");
				break;
			case PID_FIRST|PID_LAST:
				break;	/* Complete message, say nothing */
			case 0:
				fprintf(trfp," Middle frag");
				break;
			}
			fprintf(trfp," pid=");
			switch(pid & 0x3f){
			case PID_ARP:
				fprintf(trfp,"ARP\n");
				break;
			case PID_NETROM:
				fprintf(trfp,"NET/ROM\n");
				break;
			case PID_IP:
				fprintf(trfp,"IP\n");
				break;
			case PID_NO_L3:
				fprintf(trfp,"Text\n");
				break;
			default:
				fprintf(trfp,"0x%x\n",pid);
			}
			/* Only decode frames that are the first in a
			 * multi-frame sequence
			 */
			switch(pid & (PID_PID | PID_FIRST)){
			case PID_ARP | PID_FIRST:
				arp_dump(bpp);
				break;
			case PID_IP | PID_FIRST:
				/* Only checksum complete frames */
				ip_dump(bpp,pid & PID_LAST);
				break;
			case PID_NETROM | PID_FIRST:
				netrom_dump(bpp);
				break;
			}
		}
	} else if(type == FRMR && pullup(bpp,tmp,3) == 3){
		fprintf(trfp,": %s",decode_type(ftype(tmp[0])));
		fprintf(trfp," Vr = %d Vs = %d",(tmp[1] >> 5) & MMASK,
			(tmp[1] >> 1) & MMASK);
		if(tmp[2] & W)
			fprintf(trfp," Invalid control field");
		if(tmp[2] & X)
			fprintf(trfp," Illegal I-field");
		if(tmp[2] & Y)
			fprintf(trfp," Too-long I-field");
		if(tmp[2] & Z)
			fprintf(trfp," Invalid seq number");
		fprintf(trfp,"\n");
	} else
		fprintf(trfp,"\n");

	fflush(stdout);
}
/* Display NET/ROM network and transport headers */
static
netrom_dump(bpp)
struct mbuf **bpp;
{
	struct ax25_addr src,dest;
	char x;
	char tmp[16];
	char thdr[5];
	register i;

	if(bpp == NULLBUFP || *bpp == NULLBUF)
		return;
	/* See if it is a routing broadcast */
	if(uchar(*(*bpp)->data) == 0xff) {
		pullup(bpp,tmp,1);		/* Signature */
		pullup(bpp,tmp,ALEN);
		tmp[ALEN] = '\0';
		fprintf(trfp,"NET/ROM Routing: %s\n",tmp);
		for(i = 0;i < 11;i++) {
			if (pullup(bpp,tmp,AXALEN) < AXALEN)
				break;
			memcpy(src.call,tmp,ALEN);
			src.ssid = tmp[ALEN];
			pax25(tmp,&src);
			fprintf(trfp,"        %12s",tmp);
			pullup(bpp,tmp,ALEN);
			tmp[ALEN] = '\0';
			fprintf(trfp,"%8s",tmp);
			pullup(bpp,tmp,AXALEN);
			memcpy(src.call,tmp, ALEN);
			src.ssid = tmp[ALEN];
			pax25(tmp,&src);
			fprintf(trfp,"    %12s", tmp);
			pullup(bpp,tmp,1);
			fprintf(trfp,"    %3u\n", (unsigned)uchar(tmp[0]));
		}
		return;
	}
	/* Decode network layer */
	pullup(bpp,tmp,AXALEN);
	memcpy(src.call,tmp,ALEN);
	src.ssid = tmp[ALEN];
	pax25(tmp,&src);
	fprintf(trfp,"NET/ROM: %s",tmp);

	pullup(bpp,tmp,AXALEN);
	memcpy(dest.call,tmp,ALEN);
	dest.ssid = tmp[ALEN];
	pax25(tmp,&dest);
	fprintf(trfp,"->%s",tmp);

	pullup(bpp,&x,1);
	fprintf(trfp," ttl %d\n",uchar(x));

	/* Read first five bytes of "transport" header */
	pullup(bpp,thdr,5);
	switch(thdr[4] & 0xf){
	case 0:	/* network PID extension */
		if (thdr[0] == PID_IP && thdr[1] == PID_IP)
			ip_dump(bpp,1) ;
		else
			fprintf(trfp,"         protocol family %x, proto %x",
					uchar(thdr[0]), uchar(thdr[1])) ;
		break ;
	case 1:	/* Connect request */
		fprintf(trfp,"         conn rqst: ckt %d/%d",uchar(thdr[0]),uchar(thdr[1]));
		pullup(bpp,&x,1);
		fprintf(trfp," wnd %d",x);
		pullup(bpp,(char *)&src,sizeof(struct ax25_addr));
		pax25(tmp,&src);
		fprintf(trfp," %s",tmp);
		pullup(bpp,(char *)&dest,sizeof(struct ax25_addr));
		pax25(tmp,&dest);
		fprintf(trfp,"->%s",tmp);
		break;
	case 2:	/* Connect acknowledgement */
		fprintf(trfp,"         conn ack: ur ckt %d/%d my ckt %d/%d",
			uchar(thdr[0]), uchar(thdr[1]), uchar(thdr[2]),
			uchar(thdr[3]));
		pullup(bpp,&x,1);
		fprintf(trfp," wnd %d",x);
		break;
	case 3:	/* Disconnect request */
		fprintf(trfp,"         disc: ckt %d/%d",uchar(thdr[0]),uchar(thdr[1]));
		break;
	case 4:	/* Disconnect acknowledgement */
		fprintf(trfp,"         disc ack: ckt %d/%d",uchar(thdr[0]),uchar(thdr[1]));
		break;
	case 5:	/* Information (data) */
		fprintf(trfp,"         info: ckt %d/%d",uchar(thdr[0]),uchar(thdr[1]));
		fprintf(trfp," txseq %d rxseq %d",uchar(thdr[2]), uchar(thdr[3]));
		break;
	case 6:	/* Information acknowledgement */
		fprintf(trfp,"         info ack: ckt %d/%d rxseq %d",
				uchar(thdr[0]),uchar(thdr[1]),uchar(thdr[3]));
		break;
	default:
		fprintf(trfp,"         unknown transport type %d", thdr[4] & 0x0f) ;
		break;
	}
	if(thdr[4] & 0x80)
		fprintf(trfp," CHOKE");
	if(thdr[4] & 0x40)
		fprintf(trfp," NAK");
	fprintf(trfp,"\n");
}
char *
decode_type(type)
int16 type;
{
	switch(uchar(type)){
	case I:
		return "I";
	case SABM:
		return "SABM";
	case DISC:
		return "DISC";
	case DM:
		return "DM";
	case UA:
		return "UA";
	case RR:
		return "RR";
	case RNR:
		return "RNR";
	case REJ:
		return "REJ";
	case FRMR:
		return "FRMR";
	case UI:
		return "UI";
	default:
		return "[invalid]";
	}
}

