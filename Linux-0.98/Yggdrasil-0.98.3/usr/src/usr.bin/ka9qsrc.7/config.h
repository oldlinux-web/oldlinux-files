/* Software options */
#define	SERVERS		1	/* Include TCP servers */
#define	TRACE		1	/* Include packet tracing code */
#define	NSESSIONS	10	/* Number of interactive clients */
#undef	SCREEN		1	/* trace screen on the Atari-ST */
#define	TYPE		1	/* Include type command */
#define	FLOW		1	/* Enable local tty flow control */
#undef  TNC2		1       /* TNC2 MBBIOS emulator */
#define MODEM_CALL	1	/* Include modem dialing for SLIP */

/* Hardware configuration */
#undef	PC_EC		1	/* 3-Com 3C501 Ethernet controller */
#define	SLIP		1	/* Serial line IP */
#undef	SLFP		1	/* MIT Serial line framing protocol */
#define	KISS		1	/* KISS TNC code */
#undef	HAPN		1	/* Hamilton Area Packet Network driver code */
#undef	EAGLE		1	/* Eagle card driver */
#undef	PACKET		1	/* FTP Software's Packet Driver interface */
#undef	PC100		1	/* PAC-COM PC-100 driver code */
#undef	APPLETALK	1	/* Appletalk interface (Macintosh) */
#undef	PLUS		1	/* HP's Portable Plus is the platform */
#define LINUX_ETH	1	/* Linux generic Ethernet support */

#if	defined(NOMAD)
#undef	PLUS
#define	PLUS		1	/* NOMAD implies PLUS */
#endif

/* software options */
#define _FINGER		1	/* add finger command code */ 
#define	MULPORT		1	/* include GRAPES multiport digipeater code */
#define	NRS  		1	/* NET/ROM async interface */
#define	NETROM		1	/* NET/ROM network support */

#if	defined(PLUS)		/* these don't make sense on HP's laptop */
#undef	PC_EC		 	/* 3-Com 3C501 Ethernet controller */
#undef	HAPN		 	/* Hamilton Area Packet Network driver code */
#undef	EAGLE		 	/* Eagle card driver */
#undef	PACKET		 	/* FTP Software's Packet Driver interface */
#undef	PC100		 	/* PAC-COM PC-100 driver code */
#undef	APPLETALK	 	/* Appletalk interface (Macintosh) */
#undef	MULPORT		 	/* include GRAPES multiport digipeater code */
#endif

#if !defined(SLIP)
#undef MODEM_CALL	 	/* Don't Include modem dialing for SLIP */
#endif

#if	defined(NRS)
#undef	NETROM
#define	NETROM		1	/* NRS implies NETROM */
#endif

#if	(defined(NETROM) || defined(KISS) || defined(HAPN) || defined(EAGLE) || defined(PC100))
#define	AX25		1		/* AX.25 subnet code */
#endif

/* KISS TNC, SLIP, NRS or PACKET implies ASY */
#if (defined(KISS) || defined(PACKET) || defined(NRS) || defined(SLIP) || defined(SLFP))
#undef	ASY
#define	ASY		1	/* Asynch driver code */
#endif

#if (defined(PC_EC) || defined(PACKET) || defined(LINUX_ETH))
#define	ETHER	1		/* Generic Ethernet code */
#endif



