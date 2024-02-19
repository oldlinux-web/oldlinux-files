#ifndef _LINUX_CONFIG_H
#define _LINUX_CONFIG_H

#define CONFIG_DISTRIBUTION

/*
 * Defines for what uname() should return 
 */
#ifndef UTS_SYSNAME
#define UTS_SYSNAME "Linux"
#endif
#ifndef UTS_NODENAME
#define UTS_NODENAME "(none)"	/* set by sethostname() */
#endif
#define UTS_MACHINE "i386"	/* hardware type */
/*
 * The definitions for UTS_RELEASE and UTS_VERSION are now defined
 * in linux/version.h, and should only be used by linux/version.c
 */

/* Don't touch these, unless you really know what your doing. */
#define DEF_INITSEG	0x9000
#define DEF_SYSSEG	0x1000
#define DEF_SETUPSEG	0x9020
#define DEF_SYSSIZE	0x7000

/* internal svga startup constants */
#define NORMAL_VGA	0xffff		/* 80x25 mode */
#define EXTENDED_VGA	0xfffe		/* 80x50 mode */
#define ASK_VGA		0xfffd		/* ask for it at bootup */

/*
 * The root-device is no longer hard-coded. You can change the default
 * root-device by changing the line ROOT_DEV = XXX in boot/bootsect.s
 */

/*
 * The keyboard is now defined in kernel/chr_dev/keyboard.S
 */

/*
 * Normally, Linux can get the drive parameters from the BIOS at
 * startup, but if this for some unfathomable reason fails, you'd
 * be left stranded. For this case, you can define HD_TYPE, which
 * contains all necessary info on your harddisk.
 *
 * The HD_TYPE macro should look like this:
 *
 * #define HD_TYPE { head, sect, cyl, wpcom, lzone, ctl}
 *
 * In case of two harddisks, the info should be sepatated by
 * commas:
 *
 * #define HD_TYPE { h,s,c,wpcom,lz,ctl },{ h,s,c,wpcom,lz,ctl }
 */
/*
 This is an example, two drives, first is type 2, second is type 3:

#define HD_TYPE { 4,17,615,300,615,8 }, { 6,17,615,300,615,0 }

 NOTE: ctl is 0 for all drives with heads<=8, and ctl=8 for drives
 with more than 8 heads.

 If you want the BIOS to tell what kind of drive you have, just
 leave HD_TYPE undefined. This is the normal thing to do.
*/

#undef HD_TYPE

#define CONFIG_BLK_DEV_HD
#undef CONFIG_BLK_DEV_SD
#undef CONFIG_BLK_DEV_ST
#undef CONFIG_BLK_DEV_SR


/*
	Choose supported SCSI adapters here.
*/

#undef CONFIG_SCSI_AHA1542
#undef CONFIG_SCSI_ALWAYS
#undef CONFIG_SCSI_CSC
#undef CONFIG_SCSI_DTC
#undef CONFIG_SCSI_FUTURE_DOMAIN
#undef CONFIG_SCSI_SEAGATE
#undef CONFIG_SCSI_ULTRASTOR
#undef CONFIG_SCSI_7000FASST

#if defined(CONFIG_BLK_DEV_SD) || defined(CONFIG_BLK_DEV_SR) || \
defined(CONFIG_CHR_DEV_ST)
#ifndef CONFIG_SCSI
	#define CONFIG_SCSI
#endif

#if !defined(CONFIG_SCSI_AHA1542) && !defined(CONFIG_SCSI_CSC) && !defined(CONFIG_SCSI_DTC) && \
		!defined(CONFIG_SCSI_FUTURE_DOMAIN) &&  !defined(CONFIG_SCSI_SEAGATE) && !defined(CONFIG_SCSI_ULTRASTOR) && \
		!defined(CONFIG_SCSI_7000FASST)
#error  Error : SCSI devices enabled, but no low level drivers have been enabled.
#endif
#endif

#ifdef CONFIG_DISTRIBUTION
#include <linux/config.dist.h>
#else
#include <linux/config.site.h>
#endif

/*
	File type specific stuff goes into this.
*/

#ifdef ASM_SRC
#endif

#ifdef C_SRC
#endif

#ifdef MAKE
#endif

#endif
