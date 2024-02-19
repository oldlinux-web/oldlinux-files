/* Driver for FTP Software's packet driver interface */
#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "enet.h"
#include "iface.h"
#include "ec.h"
#include "timer.h"
#include "arp.h"
#include "trace.h"
#include "regs.h"
#include "pktdrvr.h"

struct pktdrvr pktdrvr[PK_MAX];
unsigned int npk;
int derr;

/* Send raw packet (caller provides header) */
int
pk_raw(interface,bp)
struct interface *interface;	/* Pointer to interface control block */
struct mbuf *bp;		/* Data field */
{
	register struct pktdrvr *pp;
	int16 size;
	struct mbuf *bp1;

	pp = &pktdrvr[interface->dev];
	size = len_mbuf(bp);

	switch(pp->class){
	case ETHERNET:
		dump(interface,IF_TRACE_OUT,TRACE_ETHER,bp);
		if(size < 60)
			size = 60;
		break;
	case SERIAL_LINE:
		dump(interface,IF_TRACE_OUT,TRACE_IP,bp);
		break;
	}
	if(bp->next != NULLBUF){
		/* Copy to contiguous buffer, since driver can't handle mbufs */
		bp1 = copy_p(bp,size);
		free_p(bp);
		bp = bp1;
		if(bp == NULLBUF)
			return -1;
	} else
		bp->cnt = size;	/* Make sure packet size is big enough */

	send_pkt(pp->intno,bp->data,bp->cnt);
	free_p(bp);
	return 0;
}

/* Packet driver receive routine. Called from an assembler hook that pushes
 * the caller's registers on the stack so we can access and modify them.
 * This is a rare example of call-by-location in C.
 */
int
pkint(di,si,bp,dx,cx,bx,ax,ds,es)
unsigned short di,si,bp,dx,cx,bx,ax,ds,es;
{
	register struct pktdrvr *pp;
	int i;

	for(i=0,pp = &pktdrvr[0];i < npk;i++,pp++)
		if(pp->handle1 == bx
		 || (pp->class == ETHERNET && pp->handle2 == bx))
			break;
	if(i == npk)
		return;	/* Unknown packet */

	switch(ax){
	case 0:	/* Space allocate call */
		if(pp->rcvcnt < pp->rcvmax && (pp->buffer = alloc_mbuf(cx)) != NULLBUF){
			es = FP_SEG(pp->buffer->data);
			di = FP_OFF(pp->buffer->data);
			pp->buffer->cnt = cx;
		} else {
			es = di = 0;
		}
		break;
	case 1:	/* Packet complete call */
		enqueue(&pp->rcvq,pp->buffer);
		pp->rcvcnt++;
		pp->buffer = NULLBUF;
		break;
	default:
		break;
	}
}
/* Process any incoming packets on the receive queue */
void
pk_recv(interface)
struct interface *interface;
{
	void arp_input();
	int ip_route();
	struct pktdrvr *pp;
	struct mbuf *bp;

	pp = &pktdrvr[interface->dev];
	if((bp = dequeue(&pp->rcvq)) == NULLBUF)
		return;

	pp->rcvcnt--;
	switch(pp->class){
	case ETHERNET:
		dump(interface,IF_TRACE_IN,TRACE_ETHER,bp);
		eproc(interface,bp);
		break;
	case SERIAL_LINE:
		dump(interface,IF_TRACE_IN,TRACE_IP,bp);
		ip_route(bp,0);
		break;
	}
}
/* Shut down the packet interface */
pk_stop(interface)
struct interface *interface;
{
	struct pktdrvr *pp;

	pp = &pktdrvr[interface->dev];
	/* Call driver's release_type() entry */
	if(release_type(pp->intno,pp->handle1) == -1)
		printf("%s: release_type error code %u\n",interface->name,derr);

	if(pp->class == ETHERNET)
		release_type(pp->intno,pp->handle2);
}
/* Attach a packet driver to the system
 * argv[0]: hardware type, must be "packet"
 * argv[1]: software interrupt vector, e.g., x7e
 * argv[2]: interface label, e.g., "trw0"
 * argv[3]: maximum number of packets allowed on receive queue, e.g., "5"
 * argv[4]: maximum transmission unit, bytes, e.g., "1500"
 */
pk_attach(argc,argv)
int argc;
char *argv[];
{
	register struct interface *if_pk;
	extern struct interface *ifaces;
	int class;
	unsigned int intno;
	void pkvec();
	static char iptype[] = {IP_TYPE >> 8,IP_TYPE};
	static char arptype[] = {ARP_TYPE >> 8,ARP_TYPE};
	int pether(),gaether(),enet_send(),enet_output(),slip_send();
	void (*getvect())();
	long drvvec;
	char sig[8];	/* Copy of driver signature "PKT DRVR" */
	register struct pktdrvr *pp;

	if(npk >= PK_MAX){
		printf("Too many packet drivers\n");
		return -1;
	}

	intno = htoi(argv[1]);
	/* Verify that there's really a packet driver there, so we don't
	 * go off into the ozone (if there's any left)
	 */
	drvvec = (long)getvect(intno);
	movblock((unsigned short)drvvec+3, (unsigned short)(drvvec >> 16),
		sig,getds(),strlen(PKT_SIG));
	if(strncmp(sig,PKT_SIG,strlen(PKT_SIG)) != 0){
		printf("No packet driver loaded at int 0x%x\n",intno);
		return -1;
	}
	if((if_pk = (struct interface *)calloc(1,sizeof(struct interface))) == NULLIF
	 ||(if_pk->name = malloc((unsigned)strlen(argv[2])+1)) == NULLCHAR){
		if(if_pk != NULLIF)
			free((char *)if_pk);
		printf("pk_attach: no memory!\n");
		return -1;
	}
	pp = &pktdrvr[npk];
	strcpy(if_pk->name,argv[2]);
	pp->rcvmax = atoi(argv[3]);
	if_pk->mtu = atoi(argv[4]);
	if_pk->dev = npk++;
	if_pk->raw = pk_raw;
	if_pk->recv = pk_recv;
	if_pk->stop = pk_stop;
	pp->intno = intno;

	/* Find out by exhaustive search what class this driver is (ugh) */
	for(class=0;class<9;class++){
		pp->handle1 = access_type(intno,class,ANYTYPE,0,NULLCHAR,0,pkvec);
		if(pp->handle1 != -1)
			break;
	}
	/* Now that we know, release it and do it all over again with the
	 * right type fields
	 */
	release_type(intno,pp->handle1);
	switch(class){
	case ETHERNET:
		pp->handle1 = access_type(intno,class,ANYTYPE,0,iptype,2,pkvec);
		pp->handle2 = access_type(intno,class,ANYTYPE,0,arptype,2,pkvec);			
		if_pk->send = enet_send;
		if_pk->output = enet_output;
		/* Get hardware Ethernet address from driver */
		if_pk->hwaddr = malloc(EADDR_LEN);
		get_address(intno,pp->handle1,if_pk->hwaddr,EADDR_LEN);
		arp_init(ARP_ETHER,EADDR_LEN,IP_TYPE,ARP_TYPE,ether_bdcst,pether,gaether);
		break;
	case SERIAL_LINE:
		pp->handle1 = access_type(intno,class,ANYTYPE,0,NULLCHAR,0,pkvec);
		if_pk->send = slip_send;
		break;
	default:
		printf("Packet driver has unsupported class %u\n",class);
		free(if_pk->name);
		free((char *)if_pk);
		npk--;
		return -1;
	}
	pp->class = class;
	if_pk->next = ifaces;
	ifaces = if_pk;

	return 0;
}
static int
access_type(intno,if_class,if_type,if_number,type,typelen,receiver)
int intno;
int if_class;
int if_type;
int if_number;
char *type;
unsigned typelen;
void (*receiver)();
{
	struct regs regs;

	_AH = ACCESS_TYPE;	/* Access_type() function */
	_AL = if_class;		/* Class */
	_BX = if_type;			/* Type */
	_DL = if_number;		/* Number */
	/* Packet type template */
	_DS = FP_SEG(type);
	_SI = FP_OFF(type);
	_CX = typelen;			/* Length of type */
	/* Address of receive handler */
	_ES = FP_SEG(receiver);
	_DI = FP_OFF(receiver);
	if(sysint(intno,&regs,&regs) & FLAG_CARRY){
		derr = _DH;
		return -1;
	} else
		return _AX;
}
static int
release_type(intno,handle)
int intno;
int handle;
{
	struct regs regs;

	_AH = RELEASE_TYPE;
	_BX = handle;
	if(sysint(intno,&regs,&regs) & FLAG_CARRY){
		derr = _DH;
		return -1;
	} else
		return 0;
}
static int
send_pkt(intno,buffer,length)
int intno;
char *buffer;
unsigned length;
{
	struct regs regs;

	_AH = SEND_PKT;
	_DS = FP_SEG(buffer);
	_SI = FP_OFF(buffer);
	_CX = length;
	if(sysint(intno,&regs,&regs) & FLAG_CARRY){
		derr = _DH;
		return -1;
	} else
		return 0;
}
static int
driver_info(intno,handle,version,class,type,number,basic)
int intno;
int handle;
int *version,*class,*type,*number,*basic;
{
	struct regs regs;

	_AH = DRIVER_INFO;
	_AL = 0xff;
	_BX = handle;
	if(sysint(intno,&regs,&regs) & FLAG_CARRY){
		derr = _DH;
		return -1;
	}
	if(version != NULL)
		*version = _BX;
	if(class != NULL)
		*class = _CH;
	if(type != NULL)
		*type = _DX;
	if(number != NULL)
		*number = _CL;
	if(basic != NULL)
		*basic = _AL;
	return 0;
}
static int
get_address(intno,handle,buf,len)
int intno;
int handle;
char *buf;
int len;
{
	struct regs regs;

	_AH = GET_ADDRESS;
	_BX = handle;
	_ES = FP_SEG(buf);
	_DI = FP_OFF(buf);
	_CX = len;
	if(sysint(intno,&regs,&regs) & FLAG_CARRY){
		derr = _DH;
		return -1;
	}
	return 0;
}
