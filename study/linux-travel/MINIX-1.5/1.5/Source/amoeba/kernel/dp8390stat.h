#ifdef DPSTAT
/* statistics from dp8390 */
struct dpstat {
	long	ds_read;	/* packets read */
	long	ds_written;	/* packets written */
        long    ds_fram;	/* Input framing errors */
        long    ds_crc;		/* Input CRC errors */
	long	ds_lost;	/* Packets lost */
	long	ds_btint;	/* Bogus transmit interrupts */
	long	ds_deferred;	/* Deferred packets */
	long	ds_collisions;	/* Packets collided at least once */
	long	ds_xcollisions;	/* Aborts due to excessive collisions */
	long	ds_carlost;	/* Carrier sense lost */
	long	ds_fifo;	/* Fifo underrun */
	long	ds_heartbeat;	/* Heart beat failure */
	long	ds_lcol;	/* Late collisions */
};
#define STINC(x) dpstat.x++
#define STADD(x,y) dpstat.x += y
#else
#define STINC(x)        /* nothing */
#define STADD(x,y)      /* nothing */
#endif DPSTAT
