/* Space.c */
/* Holds initial configuration information for devices. */
#include "dev.h"
#include <linux/stddef.h>
#include <linux/config.h>

#define NEXT_DEV NULL

#ifdef ETHERLINK1
extern int etherlink_init(struct device *);

#ifndef ETHERLINK1_IRQ
#define ETHERLINK1_IRQ 9
#endif

static struct device el_dev = {
    "if3c501",
    0, 0, 0, 0,   /* memory rx_end, rx_start, end, start are autoconfiged. */
    ETHERLINK1, ETHERLINK1_IRQ,			/* base i/o address, irq. */
    0,0,0,0,0,
    NEXT_DEV,
    etherlink_init,  0,  {NULL},  NULL,  NULL,  NULL,  NULL,  NULL,
    NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  0,  0,  0,  {0,},  {0,},  0
    };
#undef NEXT_DEV
#define NEXT_DEV (&el_dev)

#endif  /* ETHERLINK1 */

#if defined(EI8390) || defined(EL2) || defined(NE2000) \
    || defined(WD80x3) || defined(HPLAN)
extern int ethif_init(struct device *);

#ifndef EI8390_IRQ
#define EI8390_IRQ 0
#endif

static struct device ei8390_dev = {
#ifdef EI_NAME
    EI_NAME,
#else
    "eth_if",
#endif
    0, 0, 0, 0,   /* memory rx_end, rx_start, end, start are autoconfiged. */
    EI8390, EI8390_IRQ,  0,0,0,0,0,	/* base i/o address, irq, and flags. */
    NEXT_DEV,
    ethif_init, 0,  {NULL}, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, 0,  0,  0,  {0,},  {0,},  0
    };
#undef NEXT_DEV
#define NEXT_DEV (&ei8390_dev)

#endif  /* The EI8390 drivers. */

#ifdef WD8003
extern int wd8003_init(struct device *);

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
  NEXT_DEV,
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
#undef NEXT_DEV
#define NEXT_DEV (&wd8003_dev)

#endif /* WD8003 */

extern int loopback_init(struct device *dev);

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
  NEXT_DEV,			/* next device */
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
#undef NEXT_DEV
#define NEXT_DEV (error no next device &loopback_dev)

struct device *dev_base = &loopback_dev;
