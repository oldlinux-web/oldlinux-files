#define ETHERMTU 2048
#define ETHER_MIN_LEN 64
#define ETHER_HEADER 14
#define ETHER_ADDR_LEN 6
/* this must be a power of 2 */
#define ETH_BUFF_SIZE 0x2000

/* some ioctls. */
#define ETH_START 0
#define ETH_STOP 1

typedef void * (*callback)(void *);
