#define ETHERMTU 2048
#define ETHER_MIN_LEN 64
#define ETHER_ADDR_LEN 6
/* this must be a power of 2 */
#define ETH_BUFF_SIZE 0x2000

/* some ioctls. */
#define ETH_START 0
#define ETH_STOP 1

typedef void * (*callback)(void *);

#define ETHERTYPE_ARP 0x806
#define ETHERTYPE_IP  0x800

#define NET16(x) (((x)<<8)|(x)&0xff)

struct enet_header
{
  unsigned char saddr[ETHER_ADDR_LEN];
  unsigned char daddr[ETHER_ADDR_LEN];
  unsigned short type;
};

#define ETHER_HEADER sizeof(enet_header)
