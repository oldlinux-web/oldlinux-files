/*
 *  Code extracted from
 *  linux/kernel/hd.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 *  Thanks to Branko Lankester, lankeste@fwi.uva.nl, who found a bug
 *  in the early extended-partition checks and added DM partitions
 */

#include <linux/config.h>
#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/kernel.h>
struct gendisk *gendisk_head = NULL;

static int current_minor = 0;
extern int *blk_size[];
/*
 * Create devices for each logical partition in an extended partition.
 * The logical partitions form a linked list, with each entry being
 * a partition table with two entries.  The first entry
 * is the real data partition (with a start relative to the partition
 * table start).  The second is a pointer to the next logical partition
 * (with a start relative to the entire extended partition).
 * We do not create a Linux partition for the partition tables, but
 * only for the actual data partitions.
 */

static void extended_partition(struct gendisk *hd, int dev)
{
	struct buffer_head *bh;
	struct partition *p;
	unsigned long first_sector, this_sector;
	int mask = (1 << hd->minor_shift) - 1;

	first_sector = hd->part[MINOR(dev)].start_sect;
	this_sector = first_sector;

	while (1) {
		if ((current_minor & mask) >= (4 + hd->max_p))
			return;
		if (!(bh = bread(dev,0,1024))) {
			printk("Unable to read partition table of device %04x\n",dev);
			return;
		}
	  /*
	   * This block is from a device that we're about to stomp on.
	   * So make sure nobody thinks this block is usable.
	   */
		bh->b_dirt=0;
		bh->b_uptodate=0;
		if (*(unsigned short *) (bh->b_data+510) == 0xAA55) {
			p = 0x1BE + (void *)bh->b_data;
		/*
		 * Process the first entry, which should be the real
		 * data partition.
		 */
			if (p->sys_ind == EXTENDED_PARTITION ||
			    !(hd->part[current_minor].nr_sects = p->nr_sects))
				goto done;  /* shouldn't happen */
			hd->part[current_minor].start_sect = this_sector + p->start_sect;
			printk("  Logical part %d start %d size %d end %d\n\r", 
			       mask & current_minor, hd->part[current_minor].start_sect, 
			       hd->part[current_minor].nr_sects,
			       hd->part[current_minor].start_sect + 
			       hd->part[current_minor].nr_sects - 1);
			current_minor++;
			p++;
		/*
		 * Process the second entry, which should be a link
		 * to the next logical partition.  Create a minor
		 * for this just long enough to get the next partition
		 * table.  The minor will be reused for the real
		 * data partition.
		 */
			if (p->sys_ind != EXTENDED_PARTITION ||
			    !(hd->part[current_minor].nr_sects = p->nr_sects))
				goto done;  /* no more logicals in this partition */
			hd->part[current_minor].start_sect = first_sector + p->start_sect;
			this_sector = first_sector + p->start_sect;
			dev = ((hd->major) << 8) | current_minor;
			brelse(bh);
		} else
			goto done;
	}
done:
	brelse(bh);
}

static void check_partition(struct gendisk *hd, unsigned int dev)
{
	int i, minor = current_minor;
	struct buffer_head *bh;
	struct partition *p;
	unsigned long first_sector;
	int mask = (1 << hd->minor_shift) - 1;

	first_sector = hd->part[MINOR(dev)].start_sect;

	if (!(bh = bread(dev,0,1024))) {
		printk("Unable to read partition table of device %04x\n",dev);
		return;
	}
	printk("%s%c :\n\r", hd->major_name, 'a'+(minor >> hd->minor_shift));
	current_minor += 4;  /* first "extra" minor */
	if (*(unsigned short *) (bh->b_data+510) == 0xAA55) {
		p = 0x1BE + (void *)bh->b_data;
		for (i=1 ; i<=4 ; minor++,i++,p++) {
			if (!(hd->part[minor].nr_sects = p->nr_sects))
				continue;
			hd->part[minor].start_sect = first_sector + p->start_sect;
			printk(" part %d start %d size %d end %d \n\r", i, 
			       hd->part[minor].start_sect, hd->part[minor].nr_sects, 
			       hd->part[minor].start_sect + hd->part[minor].nr_sects - 1);
			if ((current_minor & 0x3f) >= 60)
				continue;
			if (p->sys_ind == EXTENDED_PARTITION) {
				extended_partition(hd, (hd->major << 8) | minor);
			}
		}
		/*
		 * check for Disk Manager partition table
		 */
		if (*(unsigned short *) (bh->b_data+0xfc) == 0x55AA) {
			p = 0x1BE + (void *)bh->b_data;
			for (i = 4 ; i < 16 ; i++, current_minor++) {
				p--;
				if ((current_minor & mask) >= mask-2)
					break;
				if (!(p->start_sect && p->nr_sects))
					continue;
				hd->part[current_minor].start_sect = p->start_sect;
				hd->part[current_minor].nr_sects = p->nr_sects;
				printk(" DM part %d start %d size %d end %d\n\r",
				       current_minor & mask,
				       hd->part[current_minor].start_sect, 
				       hd->part[current_minor].nr_sects,
				       hd->part[current_minor].start_sect + 
				       hd->part[current_minor].nr_sects - 1);
			}
		}
	} else
		printk("Bad partition table on dev %04x\n",dev);
	brelse(bh);
}

/* This function is used to re-read partition tables for removable disks.
   Much of the cleanup from the old partition tables should have already been
   done */

/* This function will re-read the partition tables for a given device,
and set things back up again.  There are some important caveats,
however.  You must ensure that no one is using the device, and no one
can start using the device while this function is being executed. */

void resetup_one_dev(struct gendisk *dev, int drive)
{
	int i;
	int start = drive<<dev->minor_shift;
	int j = start + dev->max_p;
	int major = dev->major << 8;

	current_minor = 1+(drive<<dev->minor_shift);
	check_partition(dev, major+(drive<<dev->minor_shift));

	for (i=start ; i < j ; i++)
		dev->sizes[i] = dev->part[i].nr_sects >> (BLOCK_SIZE_BITS - 9);
}

static void setup_dev(struct gendisk *dev)
{
	int i;
	int j = dev->max_nr * dev->max_p;
	int major = dev->major << 8;
	int drive;
	

	for (i = 0 ; i < j; i++)  {
		dev->part[i].start_sect = 0;
		dev->part[i].nr_sects = 0;
	}
	dev->init();	
	for (drive=0 ; drive<dev->nr_real ; drive++) {
		current_minor = 1+(drive<<dev->minor_shift);
		check_partition(dev, major+(drive<<dev->minor_shift));
	}
	for (i=0 ; i < j ; i++)
		dev->sizes[i] = dev->part[i].nr_sects >> (BLOCK_SIZE_BITS - 9);
	blk_size[dev->major] = dev->sizes;
}
	
/* This may be used only once, enforced by 'static int callable' */
int sys_setup(void * BIOS)
{
	static int callable = 1;
	struct gendisk *p;
	int nr=0;

	if (!callable)
		return -1;
	callable = 0;

	for (p = gendisk_head ; p ; p=p->next) {
		setup_dev(p);
		nr += p->nr_real;
	}
		
	if (nr)
		printk("Partition table%s ok.\n\r",(nr>1)?"s":"");

#ifdef RAMDISK
	rd_load();
#endif
	mount_root();
	return (0);
}
