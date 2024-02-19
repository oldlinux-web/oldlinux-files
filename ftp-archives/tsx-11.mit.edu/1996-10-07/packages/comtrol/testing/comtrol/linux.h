/*
 * linux.h --- Linux specific definitions for the Rocketport driver
 */

struct r_port {
	int			magic;
	int			line;
	int			flags;
	int			count;
	int			blocked_open;
	struct tty_struct 	*tty;
	int			board:2;
	int			aiop:2;
	int			chan:3;
	CONTROLLER_t *ctlp;
	CHANNEL_t 		channel;
	int			closing_wait;
	int			close_delay;
	int			intmask;
	int			xmit_fifo_room;	/* room in xmit fifo */
	unsigned char 		*xmit_buf;
	int			xmit_head;
	int			xmit_tail;
	int			xmit_cnt;
	int			session;
	int			pgrp;
	int			cd_status;
	struct termios		normal_termios;
	struct termios		callout_termios;
	struct tq_struct	tqueue;
	struct wait_queue	*open_wait;
	struct wait_queue	*close_wait;
};
	
#define RPORT_MAGIC 0x525001

#define NUM_BOARDS 4
#define MAX_RP_PORTS (32*NUM_BOARDS)

/*
 * The size of the xmit buffer is 1 page, or 4096 bytes
 */
#define XMIT_BUF_SIZE 4096

/* number of characters left in xmit buffer before we ask for more */
#define WAKEUP_CHARS 256

/* Internal flags used only by the rocketport driver */
#define ROCKET_INITIALIZED	0x80000000 /* Port is active */
#define ROCKET_CLOSING		0x40000000 /* Serial port is closing */
#define ROCKET_NORMAL_ACTIVE	0x20000000 /* Normal port is active */
#define ROCKET_CALLOUT_ACTIVE	0x10000000 /* Callout port is active */

/*
 * tty subtypes
 *
 */
#define SERIAL_TYPE_NORMAL 1
#define SERIAL_TYPE_CALLOUT 2

/*
 * Assigned major numbers for the Comtrol Rocketport
 */
#define TTY_ROCKET_MAJOR	46
#define CUA_ROCKET_MAJOR	47

/*
 * Utility function.
 */
#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#ifdef PCI_VENDOR_ID_RP
#undef PCI_VENDOR_ID_RP
#undef PCI_DEVICE_ID_RP8OCTA
#undef PCI_DEVICE_ID_RP8INTF
#undef PCI_DEVICE_ID_RP16INTF
#undef PCI_DEVICE_ID_RP32INTF
#endif

#define PCI_VENDOR_ID_RP               0x11fe
#define PCI_DEVICE_ID_RP32INTF         0x0001
#define PCI_DEVICE_ID_RP8INTF          0x0002
#define PCI_DEVICE_ID_RP16INTF         0x0003
#define PCI_DEVICE_ID_RP8OCTA          0x0005

#ifndef RP4QUAD
#define PCI_DEVICE_ID_RP4QUAD	       0x0004
#endif
#ifndef RP8J
#define PCI_DEVICE_ID_RP8J	       0x0006
#endif
