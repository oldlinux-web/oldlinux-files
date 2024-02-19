/* $Header: /sys/linux-0.97/include/linux/RCS/tpqic02.h,v 0.3 1992/08/09 23:26:24 root Exp root $
 *
 * Include file for QIC-02 driver for Linux.
 *
 * Copyright (c) 1992 by H. H. Bergman. All rights reserved.
 */

#ifndef _LINUX_TPQIC02_H
#define _LINUX_TPQIC02_H

#ifndef _CONFIG_H	/* shouldn't this be _LINUX_CONFIG_H ? */
#include <linux/config.h>
#endif

#ifdef CONFIG_TAPE_QIC02

/* NOTE: TP_HAVE_DENS should distinguish between available densities
 * NOTE: Drive select is not implemented -- I have only one tape streamer.
 */
#if defined(TAPE_QIC02_WANGTEK5150)
#define TAPE_QIC02_MT_TYPE	MT_ISWT5150
#define TP_HAVE_DENS
#define TP_HAVE_BSF	/* nope */
#define TP_HAVE_FSR	/* nope */
#define TP_HAVE_BSR	/* nope */
#define TP_HAVE_EOD	/* most of the time */
#define TP_HAVE_RAS1
#define TP_HAVE_RAS2


#elif defined(TAPE_QIC02_ALL_FEATURES)
#define TP_HAVE_DENS		/* can do set density */
#define TP_HAVE_BSF		/* can search filemark backwards */
#define TP_HAVE_FSR		/* can skip one block forwards */
#define TP_HAVE_BSR		/* can skip one block backwards */
#define TP_HAVE_EOD		/* can seek to end of recorded data */
#define TP_HAVE_RAS1		/* can run selftest 1 */
#define TP_HAVE_RAS2		/* can run selftest 2 */
/* These last two selftests shouldn't be used yet! */

#else
#error No QIC-02 tape drive type defined!
/* If your drive is not listed above, first try the 'ALL_FEATURES',
 * to see what commands are supported, then create your own entry in
 * the list above. You may want to mail it to me, so that I can include
 * it in the next release.
 */
#endif


/* streamer tape block size (fixed) */
#define TAPE_BLKSIZE	512
#define NR_BLK_BUF	20	/* max 128 blocks */
#define TPQBUF_SIZE	(TAPE_BLKSIZE*NR_BLK_BUF)

/* port locations */
#define QIC_STAT_PORT	TAPE_QIC02_PORT
#define QIC_CTL_PORT	TAPE_QIC02_PORT
#define QIC_CMD_PORT	(TAPE_QIC02_PORT+1)
#define QIC_DATA_PORT	(TAPE_QIC02_PORT+1)

/* status register bits (Active LOW!) */
#define WTS_READY	0x01
#define WTS_EXCEPTION	0x02
#define WTS_STAT	(WTS_READY|WTS_EXCEPTION)

#define WTS_RESETMASK	0x07
#define WTS_RESETVAL	(WTS_RESETMASK & ~WTS_EXCEPTION)

/* controller register (QIC_CTL_PORT) bits */
#define WTC_ONLINE	0x01
#define WTC_RESET	0x02
#define WTC_REQUEST	0x04
#define WTC_CMDOFF	0xC0 
#if TAPE_QIC02_DMA == 3	/* I don't get this -- can somebody explain this one to me? */
#define WTC_DMA		0x10	/* enable dma chan3 */
#elif TAPE_QIC02_DMA == 1
#define WTC_DMA		0x08	/* enable dma chan1 or chan2 */
#else
#error "unsupported or incorrect DMA configuration"
#endif

/* Standard QIC-02 commands -- rev F.  All QIC-02 drives must support these */
#define QCMD_SEL_1	0x01		/* select drive 1 */
#define QCMD_SEL_2	0x02		/* select drive 2 */
#define QCMD_SEL_3	0x04		/* select drive 3 */
#define QCMD_SEL_4	0x08		/* select drive 4 */
#define	QCMD_REWIND	0x21		/* rewind tape*/
#define QCMD_ERASE	0x22		/* erase tape */
#define QCMD_RETEN	0x24		/* retension tape */
#define	QCMD_WRT_DATA	0x40		/* write data */
#define	QCMD_WRT_FM	0x60		/* write file mark */
#define	QCMD_RD_DATA	0x80		/* read data */
#define	QCMD_RD_FM	0xA0		/* read file mark (forward direction) */
#define	QCMD_RD_STAT	0xC0		/* read status */

/* Other (optional/vendor unique) commands */
 /* Density commands are only valid when TP_BOM is set! */
#define QCMD_DENS_11	0x26		/* QIC-11 */
#define QCMD_DENS_24	0x27		/* QIC-24: 9 track 60MB */
#define QCMD_DENS_120	0x28		/* QIC-120: 15 track 120MB */
#define QCMD_DENS_150	0x29		/* QIC-150: 18 track 150MB */
#define QCMD_DENS_300	0x2A		/* QIC-300/QIC-2100 */
#define QCMD_DENS_600	0x2B		/* QIC-600/QIC-2200 */
/* don't know about QIC-1000 and QIC-1350 */

#define	QCMD_WRTNU_DATA	0x40		/* write data, no underruns, insert filler. */
#define QCMD_SPACE_FWD	0x81		/* skip next block */
#define QCMD_SPACE_BCK	0x89		/* move tape head one block back -- very useful! */
#define QCMD_RD_FM_BCK	0xA8		/* read filemark (backwards) */
#define QCMD_SEEK_EOD	0xA3		/* skip to EOD */
#define	QCMD_RD_STAT_X1	0xC1		/* read extended status 1 */
#define	QCMD_RD_STAT_X2	0xC4		/* read extended status 2 */
#define	QCMD_RD_STAT_X3	0xE0		/* read extended status 3 */
#define QCMD_SELF_TST1	0xC2		/* run self test 1 (nondestructive) */
#define QCMD_SELF_TST2	0xCA		/* run self test 2 (destructive) */

/* Minor device codes for tapes:
 * |7|6|5|4|3|2|1|0|
 *  | \ | / \ | / |_____ 1=rewind on close, 0=no rewind on close
 *  |  \|/    |_________ Density: 000=none, 001=QIC-11, 010=24, 011=120,
 *  |   |                100=QIC-150, 101..111 reserved.
 *  |   |_______________ Reserved for unit numbers.
 *  |___________________ Reserved for diagnostics during debugging.
 */

#define	TP_REWCLOSE(d)	((MINOR(d)&0x01) == 1)	   /* rewind bit */
		/* rewind is only done if data has been transfered */
#define	TP_DENS(dev)	((MINOR(dev) >> 1) & 0x07) /* tape density */
#define TP_UNIT(dev)	((MINOR(dev) >> 4) & 0x07) /* unit number */
#define TP_DIAGS(dev)	(MINOR(dev) & 0x80) /* print excessive diagnostics */


#ifndef __DEFONLY__
/* status codes returned by a WTS_RDSTAT call */
struct tpstatus {	/* sizeof(short)==2), LSB first */
	unsigned short	exs;	/* Drive exception flags */
	unsigned short	dec;	/* data error count: nr of blocks rewritten */
	unsigned short	urc;	/* underrun count: nr of times streaming was interrupted */
};
#define TPSTATSIZE	sizeof(struct tpstatus)
#endif /* __DEFONLY__ */


/* defines for tpstatus.exs -- taken from 386BSD wt driver */
#define	TP_POR		0x100	/* Power on/reset occurred */
#define	TP_RES1		0x200	/* Reserved for end of media */
#define	TP_RES2		0x400	/* Reserved for bus parity */
#define	TP_BOM		0x800	/* Beginning of media */
#define	TP_MBD		0x1000	/* Marginal block detected */
#define	TP_NDT		0x2000	/* No data detected */
#define	TP_ILL		0x4000	/* Illegal command */
#define	TP_ST1		0x8000	/* Status byte 1 bits */
#define	TP_FIL		0x01	/* File mark detected */
#define	TP_BNL		0x02	/* Bad block not located */
#define	TP_UDA		0x04	/* Unrecoverable data error */
#define	TP_EOM		0x08	/* End of media */
#define	TP_WRP		0x10	/* Write protected cartridge */
#define	TP_USL		0x20	/* Unselected drive */
#define	TP_CNI		0x40	/* Cartridge not in place */
#define	TP_ST0		0x80	/* Status byte 0 bits */

#define REPORT_ERR0	(TP_CNI|TP_USL|TP_WRP|TP_EOM|TP_UDA|TP_BNL|TP_FIL)
#define REPORT_ERR1	(TP_ILL|TP_NDT|TP_MBD)

#define TAPE_TIMEOUT	1000000

/* internal function return codes */
#define TE_OK	0	/* everything is fine */
#define TE_EX	1	/* exception detected */
#define TE_ERR	2	/* some error */
#define TE_NS	3	/* can't read status */
#define TE_TIM	4	/* timed out */
#define TE_DEAD	5	/* tape drive doesn't respond */

/* timeout timer values -- check these! */
#define TIM_S	(2*HZ)		/* 2 seconds (normal cmds) */
#define TIM_M	(5*HZ)		/* 5 seconds (write FM) */
#define TIM_R	(8*60*HZ)	/* 8 minutes (retensioning) */
#define TIM_F	(2*3600*HZ)	/* est. 1.2hr for full tape read/write+2 retens */

#define TIMERON(t)	timer_table[TAPE_QIC02_TIMER].expires = jiffies + (t); \
			timer_active |= (1<<TAPE_QIC02_TIMER)

#define TIMEROFF	timer_active &= ~(1<<TAPE_QIC02_TIMER)


typedef char flag;
#define YES	1	/* YES must be != 0 */
#define NO	0


extern long tape_qic02_init(long);	/* for kernel/mem.c */


#endif /* CONFIG_TAPE_QIC02 */

#endif /* _LINUX_TPQIC02_H */
