/*
 *	sd.c Copyright (C) 1992 Drew Eckhardt
 *	Linux scsi disk driver by
 *		Drew Eckhardt
 *
 *	<drew@colorado.edu>
 */

#include <linux/config.h>

#ifdef CONFIG_BLK_DEV_SD
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <asm/system.h>

#include "scsi.h"
#include "hosts.h"
#include "sd.h"
#include "scsi_ioctl.h"

#define MAJOR_NR 8

#include "../blk.h"
#include <linux/genhd.h>

/*
static const char RCSid[] = "$Header:";
*/

#define MAX_RETRIES 5

/*
 *	Time out in seconds
 */

#define SD_TIMEOUT 200

#define ISA_DMA_THRESHOLD (0x00ffffff)
struct hd_struct sd[MAX_SD << 4];

/* For a > 16 Mb system, we may need an intermediate buffer for data */

struct block_buffer
	{
	unsigned long int use;
	unsigned char	buffer[4096];
	};

static struct block_buffer * bb = NULL;

int NR_SD=0;
Scsi_Disk rscsi_disks[MAX_SD];
static int sd_sizes[MAX_SD << 4] = {0, };
static int this_count, total_count = 0;
static int the_result;
static int boot_init_done = 0;

static char sense_buffer[255];
int slow_scsi_io = -1;  /* This is set by aha1542.c, and others, if needed */

/* used to re-read partitions. */
extern void resetup_one_dev(struct gendisk *, unsigned int);

extern int sd_ioctl(struct inode *, struct file *, unsigned int, unsigned int);

static sd_init_onedisk(int);

static int sd_open(struct inode * inode, struct file * filp)
{
        int target;
	target =  DEVICE_NR(MINOR(inode->i_rdev));

	if(target >= NR_SD || !rscsi_disks[target].device)
	  return -EACCES;   /* No such device */

/* Make sure that only one process can do a check_change_disk at one time.
 This is also used to lock out further access when the partition table is being re-read. */

	while (rscsi_disks[target].device->busy);

	if(rscsi_disks[target].device->removable) {
	  check_disk_change(inode->i_rdev);

	  if(!rscsi_disks[target].device->access_count)
	    sd_ioctl(inode, NULL, SCSI_IOCTL_DOORLOCK, 0);
	};
	rscsi_disks[target].device->access_count++;
	return 0;
}

static void sd_release(struct inode * inode, struct file * file)
{
        int target;
	sync_dev(inode->i_rdev);

	target =  DEVICE_NR(MINOR(inode->i_rdev));

	rscsi_disks[target].device->access_count--;

	if(rscsi_disks[target].device->removable) {
	  if(!rscsi_disks[target].device->access_count)
	    sd_ioctl(inode, NULL, SCSI_IOCTL_DOORUNLOCK, 0);
	};
}

static struct gendisk sd_gendisk;

static void sd_geninit (void) {
	int i;
	for (i = 0; i < NR_SD; ++i)
	  sd[i << 4].nr_sects = rscsi_disks[i].capacity;
	sd_gendisk.nr_real = NR_SD;
}


static struct file_operations sd_fops = {
	NULL,			/* lseek - default */
	block_read,		/* read - general block-dev read */
	block_write,		/* write - general block-dev write */
	NULL,			/* readdir - bad */
	NULL,			/* select */
	sd_ioctl,		/* ioctl */
	NULL,			/* mmap */
	sd_open,		/* open code */
	sd_release		/* release */
};

static struct gendisk sd_gendisk = {
	MAJOR_NR,		/* Major number */
	"sd",		/* Major name */
	4,		/* Bits to shift to get real from partition */
	1 << 4,		/* Number of partitions per real */
	MAX_SD,		/* maximum number of real */
	sd_geninit,	/* init function */
	sd,		/* hd struct */
	sd_sizes,	/* block sizes */
	0,		/* number */
	(void *) rscsi_disks,	/* internal */
	NULL		/* next */
};

/*
	rw_intr is the interrupt routine for the device driver.  It will
	be notified on the end of a SCSI read / write, and
	will take on of several actions based on success or failure.
*/

static void rw_intr (int host, int result)
{
	if (HOST != host)
		panic ("sd.o : rw_intr() recieving interrupt for different host.");

#ifdef DEBUG
	printk("sd%d : rw_intr(%d, %x)\n", MINOR(CURRENT->dev), host, result);
#endif

/*
	First case : we assume that the command succeeded.  One of two things will
	happen here.  Either we will be finished, or there will be more
	sectors that we were unable to read last time.
*/

	if (!result) {
	  if (bb && bb[DEVICE_NR(CURRENT->dev)].use && CURRENT->cmd == READ)
	    {
	      memcpy((char *)CURRENT->buffer, 
		     bb[DEVICE_NR(CURRENT->dev)].buffer,
		     this_count << 9);
#ifdef DEBUG
	      printk("R");
#endif
	    };
	  if(bb) bb[DEVICE_NR(CURRENT->dev)].use = 0;

		CURRENT->nr_sectors -= this_count;
		if (slow_scsi_io == host) {
		  total_count -= this_count;
		  if(total_count){
		    CURRENT->sector += this_count;
		    CURRENT->buffer += (this_count << 9);
		    do_sd_request();
		    return;
		  };
		};

#ifdef DEBUG
		printk("sd%d : %d sectors remain.\n", MINOR(CURRENT->dev), CURRENT->nr_sectors);
#endif

/*
 * 	If multiple sectors are requested in one buffer, then
 *	they will have been finished off by the first command.  If
 *	not, then we have a multi-buffer command.
 */
		if (CURRENT->nr_sectors)
			{
			CURRENT->sector += this_count;
			CURRENT->errors = 0;

			if (!CURRENT->bh)
				{
#ifdef DEBUG
				printk("sd%d : handling page request, no buffer\n",
					MINOR(CURRENT->dev));
#endif

/*
	The CURRENT->nr_sectors field is always done in 512 byte sectors,
	even if this really isn't the case.
*/
				(char *) CURRENT->buffer += this_count << 9;
				}
			else
				{
#ifdef DEBUG
				printk("sd%d :  handling linked buffer request\n", MINOR(CURRENT->dev));
#endif
				end_request(1);
				}
			}
		else
			end_request(1);
		do_sd_request();
	}

/*
 *	Of course, the error handling code is a little Fubar down in scsi.c.
 *	Version 2 of the drivers will fix that, and we will *really* recover
 *	from errors.
 */

/*
	Now, if we were good little boys and girls, Santa left us a request
	sense buffer.  We can extract information from this, so we
	can choose a block to remap, etc.
*/

	else if (driver_byte(result) & DRIVER_SENSE) {
	  if (bb) bb[DEVICE_NR(CURRENT->dev)].use = 0;
		if (sugestion(result) == SUGGEST_REMAP) {
#ifdef REMAP
/*
	Not yet implemented.  A read will fail after being remapped,
	a write will call the strategy routine again.
*/
			if rscsi_disks[DEVICE_NR(CURRENT->dev)].remap
				{
				result = 0;
				}
			else

#endif
		}

/* A unit attention comes up if there is a media change on a removable
   disk drive */

		else if ((sense_buffer[0] & 0x7f) == 0x70) {
			if ((sense_buffer[2] & 0xf) == UNIT_ATTENTION) {
				/* detected disc change.  set a bit and quietly refuse	*/
				/* further access.					*/

				rscsi_disks[DEVICE_NR(CURRENT->dev)].device->changed = 1;
				end_request(0);
				do_sd_request();
				return;
			}
		      }

/*
	If we had an ILLEGAL REQUEST returned, then we may have performed
	an unsupported command.  The only thing this should be would be a  ten
	byte read where only a six byte read was supportted.  Also, on a
	system where READ CAPACITY failed, we mave have read past the end of the
	disk.
*/
		else if (sense_buffer[7] == ILLEGAL_REQUEST) {
			if (rscsi_disks[DEVICE_NR(CURRENT->dev)].ten) {
				rscsi_disks[DEVICE_NR(CURRENT->dev)].ten = 0;
				do_sd_request();
				result = 0;
			} else {
			}
		}
	}
	if (result) {
	        if (bb) bb[DEVICE_NR(CURRENT->dev)].use = 0;
		printk("SCSI disk error : host %d id %d lun %d return code = %x\n",
		       rscsi_disks[DEVICE_NR(CURRENT->dev)].device->host_no,
		       rscsi_disks[DEVICE_NR(CURRENT->dev)].device->id,
		       rscsi_disks[DEVICE_NR(CURRENT->dev)].device->lun, result);

		if (driver_byte(result) & DRIVER_SENSE)
			printk("\tSense class %x, sense error %x, extended sense %x\n",
				sense_class(sense_buffer[0]),
				sense_error(sense_buffer[0]),
				sense_buffer[2] & 0xf);

		end_request(0);
		do_sd_request();
	}
}

/*
	do_sd_request() is the request handler function for the sd driver.
	Its function in life is to take block device requests, and translate
	them to SCSI commands.
*/

static void do_sd_request (void)
{
	int dev, block;
	unsigned char cmd[10];
	char * buff;

repeat:
	INIT_REQUEST;
	dev =  MINOR(CURRENT->dev);
	block = CURRENT->sector;

#ifdef DEBUG
	printk("Doing sd request, dev = %d, block = %d\n", dev, block);
#endif

	if (dev >= (NR_SD << 4) || block + CURRENT->nr_sectors > sd[dev].nr_sects)
		{
		end_request(0);
		goto repeat;
		}

	block += sd[dev].start_sect;
	dev = DEVICE_NR(dev);

	if (rscsi_disks[dev].device->changed)
	        {
/*
 * quietly refuse to do anything to a changed disc until the changed bit has been reset
 */
		/* printk("SCSI disk has been changed.  Prohibiting further I/O.\n");	*/
		end_request(0);
		goto repeat;
		}

#ifdef DEBUG
	printk("sd%d : real dev = /dev/sd%d, block = %d\n", MINOR(CURRENT->dev), dev, block);
#endif


	if (!CURRENT->bh)
		this_count = CURRENT->nr_sectors;
	else
		this_count = (BLOCK_SIZE / 512);


/* This is a temporary hack for the AHA1742. */
	if(slow_scsi_io == HOST) {
	  if(total_count == 0)
	    total_count = this_count;
	  this_count = 1;  /* Take only 512 bytes at a time */
	};

#ifdef DEBUG
	printk("sd%d : %s %d/%d 512 byte blocks.\n", MINOR(CURRENT->dev),
		(CURRENT->cmd == WRITE) ? "writing" : "reading",
		this_count, CURRENT->nr_sectors);
#endif

	switch (CURRENT->cmd)
		{
		case WRITE :
			if (!rscsi_disks[dev].device->writeable)
				{
				end_request(0);
				goto repeat;
				}
			cmd[0] = WRITE_6;
			break;
		case READ :
			cmd[0] = READ_6;
			break;
		default :
			printk ("Unknown sd command %d\r\n", CURRENT->cmd);
			panic("");
		}

	cmd[1] = (LUN << 5) & 0xe0;

	buff = CURRENT->buffer;

/* Curses, curses. If this is a DMA transfer, we could be screwed. */
	if (((int) buff) + (this_count << 9) > ISA_DMA_THRESHOLD && 
	    (scsi_hosts[HOST].unchecked_isa_dma)) {
	  if (bb[DEVICE_NR(CURRENT->dev)].use) panic ("block buffer already in use");
	  bb[DEVICE_NR(CURRENT->dev)].use = 1;
	  if(this_count > 8) this_count = 8;
	  if (CURRENT->cmd == WRITE) {
	    memcpy(bb[DEVICE_NR(CURRENT->dev)].buffer,
		   (char *)CURRENT->buffer, this_count << 9);
#ifdef DEBUG
	    printk("W");
#endif
	  };
	  buff = bb[DEVICE_NR(CURRENT->dev)].buffer;
	};

	if (((this_count > 0xff) ||  (block > 0x1fffff)) && rscsi_disks[dev].ten)
		{
		if (this_count > 0xffff)
			this_count = 0xffff;

		cmd[0] += READ_10 - READ_6 ;
		cmd[2] = (unsigned char) (block >> 24) & 0xff;
		cmd[3] = (unsigned char) (block >> 16) & 0xff;
		cmd[4] = (unsigned char) (block >> 8) & 0xff;
		cmd[5] = (unsigned char) block & 0xff;
		cmd[6] = cmd[9] = 0;
		cmd[7] = (unsigned char) (this_count >> 8) & 0xff;
		cmd[8] = (unsigned char) this_count & 0xff;
		}
	else
		{
		if (this_count > 0xff)
			this_count = 0xff;

		cmd[1] |= (unsigned char) ((block >> 16) & 0x1f);
		cmd[2] = (unsigned char) ((block >> 8) & 0xff);
		cmd[3] = (unsigned char) block & 0xff;
		cmd[4] = (unsigned char) this_count;
		cmd[5] = 0;
		}

	scsi_do_cmd (HOST, ID, (void *) cmd, buff, this_count << 9,
		     rw_intr, SD_TIMEOUT, sense_buffer, MAX_RETRIES);
}

int check_scsidisk_media_change(int full_dev, int flag){
        int retval;
	int target;
	struct inode inode;

	target =  DEVICE_NR(MINOR(full_dev));

	if (target >= NR_SD) {
		printk("SCSI disk request error: invalid device.\n");
		return 0;
	};

	if(!rscsi_disks[target].device->removable) return 0;

	inode.i_rdev = full_dev;  /* This is all we really need here */
	retval = sd_ioctl(&inode, NULL, SCSI_IOCTL_TEST_UNIT_READY, 0);

	if(retval){ /* Unable to test, unit probably not ready.  This usually
		     means there is no disc in the drive.  Mark as changed,
		     and we will figure it out later once the drive is
		     available again.  */

	  rscsi_disks[target].device->changed = 1;
	  return 1; /* This will force a flush, if called from
		       check_disk_change */
	};

	retval = rscsi_disks[target].device->changed;
	if(!flag) rscsi_disks[target].device->changed = 0;
	return retval;
}

static void sd_init_done (int host, int result)
{
	the_result = result;
}


static int sd_init_onedisk(int i)
{
  int j = 0;
  unsigned char cmd[10];
  unsigned char buffer[513];
  int try_again;

  try_again=2;
  cmd[0] = READ_CAPACITY;
  cmd[1] = (rscsi_disks[i].device->lun << 5) & 0xe0;
  memset ((void *) &cmd[2], 0, 8);

  /*
   *	Super Kludge - since the midlevel error handling code doesn't work
   *	Version 2 will - it's under development 8^)
   *
   *	We manually retry
   */

  do {
    the_result = -1;
#ifdef DEBUG
    printk("sd%d : READ CAPACITY\n ", i);
#endif
    scsi_do_cmd (rscsi_disks[i].device->host_no ,
		 rscsi_disks[i].device->id,
		 (void *) cmd, (void *) buffer,
		 512, sd_init_done,  SD_TIMEOUT, sense_buffer,
		 MAX_RETRIES);

    while(the_result < 0);
  } while (try_again  && the_result);
  /*
   *	The SCSI standard says "READ CAPACITY is necessary for self confuring software"
   *	While not mandatory, support of READ CAPACITY is strongly encouraged.
   *	We used to die if we couldn't successfully do a READ CAPACITY.
   *	But, now we go on about our way.  The side effects of this are
   *
   *	1.  We can't know block size with certainty.  I have said "512 bytes is it"
   *	   	as this is most common.
   *
   *	2.  Recovery from when some one attempts to read past the end of the raw device will
   *	    be slower.
   */

  if (the_result)
    {
      printk ("sd%d : READ CAPACITY failed.\n"
	      "sd%d : status = %x, message = %02x, host = %02x, driver = %02x \n",
	      i,i,
	      rscsi_disks[i].device->host_no, rscsi_disks[i].device->id,
	      rscsi_disks[i].device->lun,
	      status_byte(the_result),
	      msg_byte(the_result),
	      host_byte(the_result),
	      driver_byte(the_result)
	      );
      if (driver_byte(the_result)  & DRIVER_SENSE)
	printk("sd%d : extended sense code = %1x \n", i, sense_buffer[2] & 0xf);
      else
	printk("sd%d : sense not available. \n", i);

      printk("sd%d : block size assumed to be 512 bytes, disk size 1GB.  \n", i);
      rscsi_disks[i].capacity = 0x1fffff;
      rscsi_disks[i].sector_size = 512;
    }
  else
    {
      rscsi_disks[i].capacity = (buffer[0] << 24) |
	(buffer[1] << 16) |
	  (buffer[2] << 8) |
	    buffer[3];

      if ((rscsi_disks[i].sector_size = (buffer[4] << 24) |
	   (buffer[5] << 16) |
	   (buffer[6] << 8) |
	   buffer[7]) != 512)
	{
	  printk ("sd%d : unsupported sector size %d.\n",
		  i, rscsi_disks[i].sector_size);
	  if(rscsi_disks[j].device->removable){
	    rscsi_disks[j].capacity = 0;
	  } else {
	    printk ("scsi : deleting disk entry.\n");
	    for  (j=i;  j < NR_SD;)
	      rscsi_disks[j] = rscsi_disks[++j];
	    --i;
	    return i;
	  };
	}
    }

  rscsi_disks[i].ten = 1;
  rscsi_disks[i].remap = 1;
  return i;
}

/*
	The sd_init() function looks at all SCSI drives present, determines
	their size, and reads partition	table entries for them.
*/

unsigned long sd_init(unsigned long memory_start, unsigned long memory_end)
{
	int i;

	for (i = 0; i < NR_SD; ++i)
	  i = sd_init_onedisk(i);

	blk_dev[MAJOR_NR].request_fn = DEVICE_REQUEST;
	blkdev_fops[MAJOR_NR] = &sd_fops;
	sd_gendisk.next = gendisk_head;
	gendisk_head = &sd_gendisk;
	boot_init_done++;
/* Allocate DMA block buffer */
	if(memory_end > ISA_DMA_THRESHOLD) {
	  bb = (struct block_buffer *) memory_start;
	  memory_start += NR_SD * sizeof(struct block_buffer);
	  for (i=0; i < NR_SD; ++i) bb[i].use = 0;
	};
	return memory_start;
}

#define DEVICE_BUSY rscsi_disks[target].device->busy
#define USAGE rscsi_disks[target].device->access_count
#define CAPACITY rscsi_disks[target].capacity
#define MAYBE_REINIT  sd_init_onedisk(target)
#define GENDISK_STRUCT sd_gendisk

/* This routine is called to flush all partitions and partition tables
   for a changed scsi disk, and then re-read the new partition table.
   If we are revalidating a disk because of a media change, then we
   enter with usage == 0.  If we are using an ioctl, we automatically have
   usage == 1 (we need an open channel to use an ioctl :-), so this
   is our limit.
 */
int revalidate_scsidisk(int dev, int maxusage){
	  int target, major;
	  struct gendisk * gdev;
	  int max_p;
	  int start;
	  int i;

	  target =  DEVICE_NR(MINOR(dev));
	  gdev = &GENDISK_STRUCT;

	  sti();
	  if (DEVICE_BUSY || USAGE > maxusage) {
	    cli();
	    printk("Device busy for revalidation (usage=%d)\n", USAGE);
	    return -EBUSY;
	  };
	  DEVICE_BUSY = 1;
	  cli();

	  max_p = gdev->max_p;
	  start = target << gdev->minor_shift;
	  major = MAJOR_NR << 8;

	  for (i=max_p - 1; i >=0 ; i--) {
	    sync_dev(major | start | i);
	    invalidate_inodes(major | start | i);
	    invalidate_buffers(major | start | i);
	    gdev->part[i].start_sect = 0;
	    gdev->part[i].nr_sects = 0;
	  };

#ifdef MAYBE_REINIT
	  MAYBE_REINIT;
#endif

	  gdev->part[start].nr_sects = CAPACITY;
	  resetup_one_dev(gdev, target);

	  DEVICE_BUSY = 0;
	  return 0;
}
#endif



