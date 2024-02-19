/* Generic Ethernet constants and templates */

#define	EADDR_LEN	6
/* Format of an Ethernet header */
struct ether {
	char dest[EADDR_LEN];
	char source[EADDR_LEN];
	int16 type;
};
#define	ETHERLEN	14

/* Ethernet broadcast address */
extern char ether_bdcst[];

/* Ethernet type fields */
#define	IP_TYPE		0x800	/* Type field for IP */
#define	ARP_TYPE	0x806	/* Type field for ARP */

#define	RUNT		60	/* smallest legal size packet, no fcs */
#define	GIANT		1514	/* largest legal size packet, no fcs */

#define	MAXTRIES	16	/* Maximum number of transmission attempts */

struct mbuf *htonether();
void eproc();
