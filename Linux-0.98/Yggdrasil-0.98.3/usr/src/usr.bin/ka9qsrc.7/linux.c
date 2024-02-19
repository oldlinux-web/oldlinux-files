/* Driver for 3COM Ethernet card */

#define	TIMER	20000	/* Timeout on transmissions */

#include <stdio.h>
#include "eth.h"
#include "global.h"
#include "mbuf.h"
#include "enet.h"
#include "iface.h"
#include "timer.h"
#include "arp.h"
#include "trace.h"
#include <errno.h>
#include <fcntl.h>

extern unsigned long selmask;
extern unsigned long mainmask;
extern long waittime;

unsigned char linux_hwaddr[6]=
{
  0x0,0x0,0x0,0x0,0x0,0x0
};
static unsigned char buff[2048];
int
linux_raw(struct interface *i, struct mbuf *bp)
{
  int size;
  int count;
  int res;
  struct mbuf *b;
  size =0;
  b=bp;

  dump(i,IF_TRACE_OUT,TRACE_ETHER,bp);

  while (b != NULLBUF)
    {
      memcpy(buff+size, b->data, b->cnt);
      size+= b->cnt;
      b=b->next;
    }
  /* try to write it 10 times.  It shouldn't take long for the
     ethernet to finish sending a packet. */
  for (count=0; count < 10; count++)
    {
      res = write(i->dev,buff,size);
      if (res > 0) 
	{
	  free_mbuf(bp);
	  return (0);
	}
      printf("write failed %d\n", res);
      if (res != EAGAIN)
	break;
    }
  free_p(bp);
  return (-1);
  
}

int
linux_stop(int dev)
{
  close(dev);
  return (0);
}

void
linux_recv (struct interface *i)
{
  int len;
  struct mbuf *bp;

  while (1)
    {
      if (waittime && mainmask & (1 << i->dev) == 0) {
	return;
      }
      len = read(i->dev,buff,2048);
      if (len <= 0) return;
      bp= (void *)malloc (sizeof *bp);
      if (bp == NULL) return;
      bp->data = malloc (len);
      if (bp->data == NULL)
	{
	  free(bp);
	  return;
	}
      memcpy (bp->data,buff,len);
      bp->next = NULL;
      bp->anext = NULL;
      bp->size = len;
      bp->cnt = len;
      dump(i,IF_TRACE_IN,TRACE_ETHER,bp);
      eproc (i,bp);
    }
}

linux_attach(argc,argv)
int argc;
char *argv[];
{
	register struct interface *if_linux;
	extern struct interface *ifaces;
	int enet_send();
	int enet_output();
	int pether(),gaether();
	int i,dig[6];

	if (6 != sscanf(argv[4],"%x.%x.%x.%x.%x.%x", 
		       &dig[0], &dig[1], &dig[2], &dig[3], &dig[4], &dig[5])) {
	  printf("linux_attach: Ethernet address must 6 hex octets in format 1.2.3.4.5.6\n");
	  return -1;
	}
	for (i = 0; i < 6; i++)
	  linux_hwaddr[i] = dig[i];

	arp_init(ARP_ETHER,EADDR_LEN,IP_TYPE,ARP_TYPE,ether_bdcst,pether,gaether);
	if((if_linux = (struct interface *)calloc(1,sizeof(struct interface))) == NULLIF
	 ||(if_linux->name = malloc((unsigned)strlen(argv[2])+1)) == NULLCHAR){
		printf("linux_attach: no memory!\n");
		return -1;
	}
	if_linux->hwaddr=linux_hwaddr;
	strcpy(if_linux->name,argv[2]);
	if_linux->mtu = atoi(argv[3]);
	if_linux->send = enet_send;
	if_linux->output = enet_output;
	if_linux->raw = linux_raw;
	if_linux->recv = linux_recv;
	if_linux->stop = linux_stop;
	if_linux->dev = open ("/dev/eth",O_RDWR|O_NDELAY);
	if (if_linux->dev < 0)
	  {
	    perror ("/dev/eth");
	    return (-1);
	  }
	selmask |= (1 << if_linux->dev);

	if(strcmp(argv[1],"arpa") != 0){
		printf("Mode %s unknown for interface %s\n",
			argv[1],argv[2]);
		free(if_linux->name);
		free((char *)if_linux);
		return -1;
	}

	if_linux->next = ifaces;
	ifaces = if_linux;

	return 0;
}
