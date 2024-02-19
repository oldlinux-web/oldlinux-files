/* Format of packets on the Ethernet */

#define AMOEBAPROTO 0x8145		/* Official Ethernet protocol number */
#define ALTAMOEBAPROTO 2222		/* Old Amoeba protocol number */
#define ETHERBITS	0x80		/* These addresses on Ethernet */

typedef struct
{
    char e[6];
} Eth_addr;

typedef struct
{
	Eth_addr	 f_dstaddr;
	Eth_addr	 f_srcaddr;
	unshort		 f_proto;
	struct pktheader f_ah;
} Framehdr;
	
typedef struct
{
	Framehdr	ep_fr;
	char		ep_data[1490];
} Etherpacket;
