/* lilo.h  -  LILO constants */

/* Written 1992 by Werner Almesberger */


/* This file is also included by the boot loader assembler code. Put
   everything that isn't legal C syntax or isn't swallowed by the
   preprocessor into #ifdef LILO_ASM ... #endif */


#ifndef _LILO_H_
#define _LILO_H_

#define VERSION		4 /* boot sector and map file format revision */

#define MAX_IMAGES      (SECTOR_SIZE/sizeof(IMAGE_DESCR))
			  /* maximum number of images */
#define MAX_IMAGE_NAME	15 /* maximum name length (w/o terminating NUL) */
#define MAX_SECONDARY	8 /* maximum number of sectors in the 2nd stage
			  loader */
#define SECTOR_SIZE	512 /* disk sector size */
#define BLOCK_SIZE	1024 /* disk block size */

#define ADDR_OFFS MAX_IMAGE_NAME+1 /* map address offset */

#define PARTITION_ENTRY	   16	/* size of one partition entry */
#define PARTITION_ENTRIES  4    /* number of partition entries */
#define PART_TABLE_SIZE    (PARTITION_ENTRY*PARTITION_ENTRIES)
#define PART_TABLE_OFFSET  0x1be /* offset in the master boot sector */

#define PART_MASK  15 /* largest common partition bits mask */
#define PART_MAX    4 /* biggest primary partition number */

#define MAX_BOOT_SIZE	PART_TABLE_OFFSET /* scream if the boot sector gets
					     any bigger */

#define BOOT_SIGNATURE	0xaa55	/* boot signature */
#define BOOT_SIG_OFFSET	510	/* boot signature offset */

#define PART_DOS12	1	/* DOS 12 bit FAT partition type */
#define PART_DOS16	4	/* DOS 16 bit FAT partition type */
#define PART_INVALID	98	/* invalid partition type */

#define STAGE_FIRST	1	/* first stage loader code */
#define STAGE_SECOND	2	/* second stage loader code */
#define	STAGE_CHAIN	0x10	/* chain loader code */

#define SETUPSECS	4	/* nr of setup sectors */

#ifdef LILO_ASM

BOOTSEG   = 0x07C0			! original address of boot-sector
PART_TABLE= 0x0600			! partition table
INITSEG   = DEF_INITSEG			! we move boot here - out of the way
SETUPSEG  = DEF_SETUPSEG		! setup starts here
SYSSEG    = DEF_SYSSEG			! system loaded at 0x10000 (65536).
STACK	  = 0x2000			! top of stack
DESCR	  = 0x0e00			! descriptor load area offset
MAP	  = 0x1000			! map load area offset
SECOND	  = 0x2000			! second stage loader load address
SECONDSEG = 0x9200			! second stage loader segment

! 0x00600-0x0063F    64 B    partition table
! 0x07C00-0x07DFF   0.5 kB   boot load address
! 0x10000-0x5FFFF   320 kB   kernel
! 0x90000-0x901FF   0.5 kB   boot sector (first stage loader)
! 0x90200-0x909FF     2 kB   setup
! 0x91000-0x91FFF     4 kB   stack
! 0x92000-0x92DFF   3.5 kB   second stage loader
! 0x92E00-0x92FFF   0.5 kB   descriptor table load area
! 0x93000-0x931FF   0.5 kB   file map load area
! 0x93200-0x9FFFF  51.5 kB   scratch space

#endif

#endif
