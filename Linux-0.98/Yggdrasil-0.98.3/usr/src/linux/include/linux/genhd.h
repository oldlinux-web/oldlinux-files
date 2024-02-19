#ifndef _LINUX_GENHD_H
#define _LINUX_GENHD_H

/*
 * 	genhd.h Copyright (C) 1992 Drew Eckhardt
 *	Generic hard disk header file by  
 * 		Drew Eckhardt
 *
 *		<drew@colorado.edu>
 */
	
#define EXTENDED_PARTITION 5
	
struct partition {
	unsigned char boot_ind;		/* 0x80 - active */
	unsigned char head;		/* starting head */
	unsigned char sector;		/* starting sector */
	unsigned char cyl;		/* starting cylinder */
	unsigned char sys_ind;		/* What partition type */
	unsigned char end_head;		/* end head */
	unsigned char end_sector;	/* end sector */
	unsigned char end_cyl;		/* end cylinder */
	unsigned int start_sect;	/* starting sector counting from 0 */
	unsigned int nr_sects;		/* nr of sectors in partition */
};

struct hd_struct {
	long start_sect;
	long nr_sects;
};

struct gendisk {
	int major;			/* major number of driver */
	char *major_name;		/* name of major driver */
	int minor_shift;		/* number of times minor is shifted to
					   get real minor */
	int max_p;			/* maximum partitions per device */
	int max_nr;			/* maximum number of real devices */

	void (*init)(void);		/* Initialization called before we do our thing */
	struct hd_struct *part;		/* partition table */
	int *sizes;			/* block sizes */
	int nr_real;			/* number of real devices */

	void *real_devices;		/* internal use */
	struct gendisk *next;
};

extern int NR_GENDISKS;			/* total */
extern struct gendisk *gendisk_head;	/* linked list of disks */

#endif
