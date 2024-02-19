#include "global.h"
#include "mbuf.h"
#include "iface.h"
#include "kiss.h"
#include "trace.h"

/* Send raw data packet on KISS TNC */
kiss_raw(interface,data)
struct interface *interface;
struct mbuf *data;
{
	register struct mbuf *bp;

	dump(interface,IF_TRACE_OUT,TRACE_AX25,data);

	/* Put type field for KISS TNC on front */
	if((bp = pushdown(data,1)) == NULLBUF){
		free_p(data);
		return;
	}
	bp->data[0] = KISS_DATA;

	slip_raw(interface,bp);
}

/* Process incoming KISS TNC frame */
void
kiss_recv(interface,bp)
struct interface *interface;
struct mbuf *bp;
{
	char kisstype;

	kisstype = pullchar(&bp);
	switch(kisstype & 0xf){
	case KISS_DATA:
		dump(interface,IF_TRACE_IN,TRACE_AX25,bp);
		ax_recv(interface,bp);
		break;
	}
}
/* Perform device control on KISS TNC by sending control messages */
kiss_ioctl(interface,argc,argv)
struct interface *interface;
int argc;
char *argv[];
{
	struct mbuf *hbp;
	int i;
	char *cp;

	if(argc < 1){
		printf("Data field missing\r\n");
		return 1;
	}
	/* Allocate space for arg bytes */
	if((hbp = alloc_mbuf((int16)argc)) == NULLBUF){
		free_p(hbp);
		return 0;
	}
	hbp->cnt = argc;
	hbp->next = NULLBUF;
	for(i=0,cp = hbp->data;i < argc;)
		*cp++ = atoi(argv[i++]);

	slip_raw(interface,hbp);	/* Even more "raw" than kiss_raw */
	return 0;
}

