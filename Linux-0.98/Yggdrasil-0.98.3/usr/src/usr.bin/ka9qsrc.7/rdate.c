/*
 * rdate
 */

#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include "global.h"
#include "mbuf.h"
#include "netuser.h"
#include "timer.h"
#include "udp.h"
#include "iface.h"
#include "ip.h"
#include "internet.h"
#include "cmdparse.h"

#define RDATE_TIMEOUT	30		/* Time limit for booting       */
#define RDATE_RETRANS	5		/* The inteval between sendings */
#define IPPORT_TIMESERVER 37

static int SilentStartup = 0;

int
dordate(argc,argv,p)
int argc;
char *argv[];
void *p;
{
	struct interface *ifp = NULLIF,*tifp;
	struct socket lsock, fsock;
	struct mbuf *bp;
	struct udp_cb *rdate_cb;
	register char *cp;
	time_t        now,		/* The current time (seconds)   */
		      starttime,	/* The start time of sending RDATE */
		      lastsendtime;	/* The last time of sending RDATE */
	int i;
	unsigned long newtime;
	long offset = 0;
	struct timeval timeval;
	int32 faddr = 0xffffffff;
	int32 taddr;

	ifp = ifaces; 		/* default to the first interface */
	if(argc > 1) {
		for(i = 1; i != argc; ++i){
			
			if((tifp = if_lookup(argv[i])) != NULLIF) 
				ifp = tifp;
			else if(strncmp(argv[i], "silent", strlen(argv[i])) == 0)
				SilentStartup = 1;
			else if(strncmp(argv[i], "noisy", strlen(argv[i])) == 0)
				SilentStartup = 0;
			else if (*argv[i] == '-' || isdigit(*argv[i])) {
			        if (*argv[i] == '-')
					offset = - atoi(argv[i] + 1);
				else
					offset = atoi(argv[i]);
			} else if (i == (argc-1) && (taddr = resolve(argv[i])))
				faddr = taddr;
			else {
				printf("rdate [net_name] [silent] [noisy] [offset in minutes] [host]\n");
				return 1;
			}
		}
	}

	if(ifp == NULLIF)
		return 0;
	lsock.address = ip_addr;
	lsock.port = 2000;  /* ?? a random number */

	open_udp(&lsock,NULLVFP);

	fsock.address = faddr;
	fsock.port = IPPORT_TIMESERVER;

  	/* Get starting time */
  	time(&starttime);
  	lastsendtime = 0;

  	/* Send the rdate request packet until a response is received or time
	   out */
  	for(;;){

		/* Get the current time */
		time(&now);

		/* Stop, if time out */
		if(now - starttime >= RDATE_TIMEOUT){
			printf("rdate: timed out, time not set\n");
			break;
		}

		/* Don't flood the network, send in intervals */
		if(now - lastsendtime > RDATE_RETRANS){
			if(!SilentStartup) printf("Requesting...\n");

			/* Allocate BOOTP packet and fill it in */
			if((bp = alloc_mbuf(sizeof(newtime))) == NULLBUF)
				break;

			/* Send out one BOOTP Request packet as a broadcast */
			send_udp(&lsock, &fsock,0,0,bp,sizeof(newtime),0,0);

			lastsendtime = now;
		}

		/* Give other tasks a chance to run. */
/*		pwait(NULL);*/
		keep_things_going();

		/* Test for and process any replies */
		if(recv_udp(&lsock, &fsock, &bp) > -1){
			timeval.tv_sec = pull32(&bp) - 2208988800 + 
			  (offset * 60);
			timeval.tv_usec = 0;
			if (settimeofday(&timeval,NULL) == 0) {
			  if(!SilentStartup) printf("Date set..\n");
			} else
			  printf("rdate: failed to set date\n");
		        break;
		} else if(net_error != WOULDBLK){
			printf("rdate: Net_error %d, no values set\n",
				 net_error);
			break;
		}
  	}

	del_udp(&lsock);
	return 0;
}
