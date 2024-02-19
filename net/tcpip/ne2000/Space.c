/* Space.c */
/* Holds initial configuration information for devices. */
#include "dev.h"
#include <linux/stddef.h>
#include <linux/config.h>

#ifdef ETHERLINK1
extern void etherlink_init(struct device *);

static struct device el_dev = {
    "if3c501",
    0, 0, 0, 0,   /* memory rx_end, rx_start, end, start are autoconfiged. */
    ETHERLINK1, 9,  0,0,0,0,0,		/* base i/o address, irq, and flags. */
    NULL, etherlink_init,  0,  {NULL},  NULL,  NULL,  NULL,  NULL,  NULL,
    NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  0,  0,  0,  {0,},  {0,},  0
    };
#endif

#ifdef EI8390
extern void ethif_init(struct device *);

static struct device el3c503_dev = {
    "eth_if",
    0, 0, 0, 0,   /* memory rx_end, rx_start, end, start are autoconfiged. */
    0x300, 5,  0,0,0,0,0,		/* base i/o address, irq, and flags. */
#ifdef ETHERLINK1
    &el_dev,
#else
    NULL,
#endif
    ethif_init, 0,  {NULL}, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, 0,  0,  0,  {0,},  {0,},  0
    };
#endif
extern void wd8003_init(struct device *);

static struct device wd8003_dev =
{
  "eth0",
  0xd2000,   /* recv memory end. */
  0xd0600,   /* recv memory start. */
  0xd2000,  /* memory end. */
  0xd0000,  /* memory start. */
  0x280,    /* base i/o address. */
  5,	    /* irq */
  0,0,0,0,0, /* flags */
#ifdef EI8390
  &el3c503_dev, /* next device */
#elif defined(ETHERLINK1)
  &el_dev, /* next device */
#else
  NULL, /* next device */
#endif
  wd8003_init,
  /* wd8003_init should set up the rest. */
  0,  /* trans start. */
  {NULL}, /* buffs */
  NULL, /* backlog */
  NULL, /* open */
  NULL, /* stop */
  NULL, /* hard_start_xmit */
  NULL, /* hard_header */
  NULL, /* add arp */
  NULL, /* queue xmit */
  NULL, /* rebuild header */
  NULL, /* type_trans */
  NULL, /* send_packet */
  NULL, /* private */
  0,    /* type. */
  0,    /* hard_header_len */
  0,    /* mtu */
  {0,}, /* broadcast address */
  {0,}, /* device address */
  0     /* addr len */
};

extern void loopback_init(struct device *dev);

static struct device loopback_dev =
{
  "loopback",
  -1,       /* recv memory end. */
  0x0,      /* recv memory start. */
  -1,       /* memory end. */
  0,        /* memory start. */
  0,        /* base i/o address. */
  0,	    /* irq */
  0,0,1,0,0, /* flags */
  &wd8003_dev, /* next device */
  loopback_init,
  /* loopback_init should set up the rest. */
  0,  /* trans start. */
  {NULL}, /* buffs */
  NULL, /* backlog */
  NULL, /* open */
  NULL, /* stop */
  NULL, /* hard_start_xmit */
  NULL, /* hard_header */
  NULL, /* add arp */
  NULL, /* queue xmit */
  NULL, /* rebuild header */
  NULL, /* type_trans */
  NULL, /* send_packet */
  NULL, /* private */
  0,    /* type. */
  0,    /* hard_header_len */
  0,    /* mtu */
  {0,}, /* broadcast address */
  {0,}, /* device address */
  0     /* addr len */
};

struct device *dev_base = &loopback_dev;
