/* $Header: /sys/linux-0.97/include/sys/RCS/mtio.h,v 0.4 1992/08/10 21:17:14 root Exp root $
 *
 * sys/mtio.h header file for Linux. Written by H. Bergman
 */

#ifndef _SYS_MTIO_H
#define _SYS_MTIO_H


#ifndef _LINUX_TYPES_H
#include <linux/types.h>		/* need daddr_t */
#endif
#ifndef _SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

/*
 * Structures and definitions for mag tape io control commands
 */

/* structure for MTIOCTOP - mag tape op command */
struct	mtop {
	short	mt_op;		/* operations defined below */
	int	mt_count;	/* how many of them */
};

/* Magnetic Tape operations [Not all operations supported by all drivers]: */
#define MTRESET 0	/* +reset drive in case of problems */
#define MTFSF	1	/* forward space over FileMark,
			 * position at first record of next file 
			 */
#define MTBSF	2	/* backward space FileMark (position before FM) */
#define MTFSR	3	/* forward space record */
#define MTBSR	4	/* backward space record */
#define MTWEOF	5	/* write an end-of-file record (mark) */
#define MTREW	6	/* rewind */
#define MTOFFL	7	/* rewind and put the drive offline (eject?) */
#define MTNOP	8	/* no op, set status only (read with MTIOCGET) */
#define MTRETEN 9	/* retension tape */
#define MTBSFM	10	/* +backward space FileMark, position at FM */
#define MTFSFM  11	/* +forward space FileMark, position at FM */
#define MTEOM	12	/* +goto end of recorded media (for appending files).
			 * An EOM is 2 FMs. MTEOM positions at the last FM,
			 * ready for appending another file, overwriting the
			 * last FM.
			 */
#define MTERASE 13	/* erase tape -- be careful! */

#define MTRAS1  14	/* run self test 1 (nondestructive) */
#define MTRAS2	15	/* run self test 2 (destructive) */
#define MTRAS3  16	/* reserved for self test 3 */

/* may need to add
#define MTFORMAT
 * or something similar for QIC-40/80 type tapes.
 */

/* structure for MTIOCGET - mag tape get status command */

struct	mtget {
	long	mt_type;	/* type of magtape device */
	long	mt_resid;	/* residual count: (not sure)
				 *	number of bytes ignored, or
				 *	number of files not skipped, or
				 *	number of records not skipped.
				 */
	/* the following registers are device dependent */
	long	mt_dsreg;	/* status register */
	long	mt_gstat;	/* generic (device independent) status */
	long	mt_erreg;	/* error register */
	daddr_t	mt_fileno;	/* for block devices -- not used */
	daddr_t	mt_blkno;	/* for block devices -- not used */
};

/*
 * Constants for mt_type. Not all of these are supported.
 */
#define MT_ISUNKNOWN		0x01
#define MT_ISQIC02		0x02	/* Generic QIC-02 tape streamer */
#define MT_ISWT5150		0x02	/* Wangtek 5150EQ, QIC-150, QIC-02 */
#define MT_ISARCHIVE_5945L2	0x03	/* Archive 5945L-2, QIC-24, QIC-02? */
#define MT_ISCMSJ500		0x04	/* CMS Jumbo 500 (QIC-02?) */
#define MT_ISTDC3610		0x05	/* Tandberg 6310, QIC-24 */
#define MT_ISARCHIVE_VP60I	0x06	/* Archive VP60i, QIC-02 */
#define MT_ISWT5099EEN24	0x11	/* Wangtek 5099-een24, 60MB, QIC-24 */
#define MT_ISEVEREX_FT40A	0x22	/* Everex FT40A (QIC-40) */
#define MT_ISDDS1		0x31	/* DDS device without partitions */
#define MT_ISDDS2		0x32	/* DDS device with partitions */
#define MT_ISSCSI1		0x41	/* Generic ANSI SCSI-1 tape unit */

struct mt_tape_info {
	long t_type;		/* device type id (mt_type) */
	char *t_name;		/* descriptive name */
};
#define MT_TAPE_INFO	{ \
	{MT_ISUNKNOWN,		"Unknown type of tape device"}, \
	{MT_ISQIC02,		"Generic QIC-02 tape streamer"}, \
	{MT_ISWT5150,		"Wangtek 5150, QIC-150"}, \
	{MT_ISARCHIVE_5945L2,	"Archive 5945L-2"}, \
	{MT_ISCMSJ500,		"CMS Jumbo 500"}, \
	{MT_ISTDC3610,		"Tandberg TDC 3610, QIC-24"}, \
	{MT_ISARCHIVE_VP60I,	"Archive VP60i, QIC-02"}, \
	{MT_ISWT5099EEN24,	"Wangtek 5099-een24, 60MB"}, \
	{MT_ISEVEREX_FT40A,	"Everex FT40A, QIC-40"}, \
	{MT_ISSCSI1,		"Generic SCSI-1 tape"}, \
	{0, NULL} \
}


/* mag tape io control commands */
#define	MTIOCTOP	_IOW('m', 1, struct mtop)	/* do a mag tape op */
#define	MTIOCGET	_IOR('m', 2, struct mtget)	/* get tape status */

/* Generic Mag Tape (device independent) status macros for examining
 * mt_gstat -- HP-UX compatible.
 * There is room for more generic status bits here, but I don't
 * know which of them are reserved. At least three or so should
 * be added to make this really useful.
 */
#define GMT_EOF(x)              ((x) & 0x80000000)
#define GMT_BOT(x)              ((x) & 0x40000000)
#define GMT_EOT(x)              ((x) & 0x20000000)
#define GMT_SM(x)               ((x) & 0x10000000)  /* DDS setmark */
#define GMT_EOD(x)              ((x) & 0x08000000)  /* DDS EOD */
#define GMT_WR_PROT(x)          ((x) & 0x04000000)
/* #define GMT_ ? 		((x) & 0x02000000) */
#define GMT_ONLINE(x)           ((x) & 0x01000000)
#define GMT_D_6250(x)           ((x) & 0x00800000)
#define GMT_D_1600(x)           ((x) & 0x00400000)
#define GMT_D_800(x)            ((x) & 0x00200000)
/* #define GMT_ ? 		((x) & 0x00100000) */
/* #define GMT_ ? 		((x) & 0x00080000) */
#define GMT_DR_OPEN(x)          ((x) & 0x00040000)  /* door open (no tape) */
/* #define GMT_ ? 		((x) & 0x00020000) */
#define GMT_IM_REP_EN(x)        ((x) & 0x00010000)  /* immediate report mode */
/* 16 generic status bits unused */

/* DDS drives have 'setmarks', sort of like filemarks but used to group
 * files, rather than blocks. Not used. Not supported.
 * I think DDS drives are DAT drives.
 */


#ifndef DEFTAPE
#define	DEFTAPE	"/dev/tape"	/* default tape device */
#endif /* DEFTAPE */

#endif /* not _SYS_MTIO_H */
