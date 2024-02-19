#ifndef	ATARI_ST
#define ASY_MAX		16		/* arbitrary number of async devices */
#define	ASYHANDLE_MAX	5		/* Five asynch int handlers on the PC */
#else
#define	ASY_MAX		2		/* Only two choices on stock ST */
#endif

extern unsigned nasy;

#define	SLIP_MODE	0
#define	AX25_MODE	1
#define	NRS_MODE	2
#define SLFP_MODE	3		/* MIT version of slip from */
