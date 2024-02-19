/*
 *      sr.c by David Giller
 *
 *      adapted from:
 *	sd.c Copyright (C) 1992 Drew Eckhardt 
 *	Linux scsi disk driver by
 *		Drew Eckhardt 
 *
 *	<drew@colorado.edu>
 */

#include <linux/config.h>

#ifdef CONFIG_BLK_DEV_SR

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/errno.h>

#include "scsi.h"
#include "sr.h"
#include "scsi_ioctl.h"   /* For the door lock/unlock commands */

#define MAJOR_NR 11

#include "../blk.h"

#define MAX_RETRIES 0
#define SR_TIMEOUT 200

int NR_SR=0;
Scsi_CD scsi_CDs[MAX_SR];
static int sr_sizes[MAX_SR << 4];
static unsigned long int this_count;

struct block_buffer
	{
	unsigned	block;
	unsigned	start;
	unsigned	use:1;
	unsigned char	buffer[2048];
	};

static struct block_buffer * bb;

static int sr_open(struct inode *, struct file *);

extern int sr_ioctl(struct inode *, struct file *, unsigned int, unsigned int);

static void sr_release(struct inode * inode, struct file * file)
{
	sync_dev(inode->i_rdev);
	if(! --scsi_CDs[MINOR(inode->i_rdev)].device->access_count)
	  sr_ioctl(inode, NULL, SCSI_IOCTL_DOORUNLOCK, 0);
}

static struct file_operations sr_fops = 
{
	NULL,			/* lseek - default */
	block_read,		/* read - general block-dev read */
	block_write,		/* write - general block-dev write */
	NULL,			/* readdir - bad */
	NULL,			/* select */
	sr_ioctl,		/* ioctl */
	NULL,			/* mmap */
	sr_open,       		/* no special open code */
	sr_release		/* release */
};

/*
 * This function checks to see if the media has been changed in the
 * CDROM drive.  It is possible that we have already sensed a change,
 * or the drive may have sensed one and not yet reported it.  We must
 * be ready for either case. This function always reports the current
 * value of the changed bit.  If flag is 0, then the changed bit is reset.
 * This function could be done as an ioctl, but we would need to have
 * an inode for that to work, and we do not always have one.
 */

int check_cdrom_media_change(int full_dev, int flag){
	int retval, target;
	struct inode inode;

	target =  MINOR(full_dev);

	if (target >= NR_SR) {
		printk("CD-ROM request error: invalid device.\n");
		return 0;
	};

	inode.i_rdev = full_dev;  /* This is all we really need here */
	retval = sr_ioctl(&inode, NULL, SCSI_IOCTL_TEST_UNIT_READY, 0);

	if(retval){ /* Unable to test, unit probably not ready.  This usually
		     means there is no disc in the drive.  Mark as changed,
		     and we will figure it out later once the drive is
		     available again.  */

	  scsi_CDs[target].device->changed = 1;
	  return 1; /* This will force a flush, if called from
		       check_disk_change */
	};

	retval = scsi_CDs[target].device->changed;
	if(!flag) scsi_CDs[target].device->changed = 0;
	return retval;
}

/*
 * The sense_buffer is where we put data for all mode sense commands performed.
 */
static unsigned char sense_buffer[255];

/*
 * rw_intr is the interrupt routine for the device driver.  It will be notified on the 
 * end of a SCSI read / write, and will take on of several actions based on success or failure.
 */

static void rw_intr (int host, int result)
{
  	if (SR_HOST != host)		
		{
		panic ("sr.o : rw_intr() recieving interrupt for different host.");
		}

	if (!result)
		{ /* No error */
	        if (bb[DEVICE_NR(CURRENT->dev)].use)
		        {
			memcpy((char *)CURRENT->buffer, 
			       bb[DEVICE_NR(CURRENT->dev)].buffer + 
			       (bb[DEVICE_NR(CURRENT->dev)].start << 9), 
			       this_count << 9);
		        }

		CURRENT->nr_sectors -= this_count;

#ifdef DEBUG
		printk("(%x %x %x) ",CURRENT->bh, CURRENT->nr_sectors, 
		       this_count);
#endif
		if (CURRENT->nr_sectors)
			{	 
			CURRENT->sector += this_count;
			CURRENT->errors = 0;
			if (!CURRENT->bh)
			  {
			    (char *) CURRENT->buffer += this_count << 9;
			  } else {
			    end_request(1);
			    do_sr_request();
			  }
			}
		else 
			{
			end_request(1);  /* All done */
			do_sr_request();
			} 
		}

	/* We only come through here if we have an error of some kind */

	if (driver_byte(result) != 0) {
		bb[DEVICE_NR(CURRENT->dev)].block = -1;
		
		if ((sense_buffer[0] & 0x7f) == 0x70) {
			if ((sense_buffer[2] & 0xf) == UNIT_ATTENTION) {
				/* detected disc change.  set a bit and quietly refuse	*/
				/* further access.					*/
		    
				scsi_CDs[DEVICE_NR(CURRENT->dev)].device->changed = 1;
				end_request(0);
			        do_sr_request();
				return;
			}
		}
	    
		if (sense_buffer[2] == ILLEGAL_REQUEST) {
			printk("CD-ROM error: Drive reports ILLEGAL REQUEST.\n");
			if (scsi_CDs[DEVICE_NR(CURRENT->dev)].ten) {
				scsi_CDs[DEVICE_NR(CURRENT->dev)].ten = 0;
				do_sr_request();
				result = 0;
				return;
			} else {
			printk("CD-ROM error: Drive reports %d.\n", sense_buffer[2]);				end_request(0);
				do_sr_request(); /* Do next request */
				return;
			}

		}

		if (sense_buffer[2] == NOT_READY) {
			printk("CDROM not ready.  Make sure you have a disc in the drive.\n");
			end_request(0);
			do_sr_request(); /* Do next request */
			return;
		};
	      }
	
	/* We only get this far if we have an error we have not recognized */
	if(result) {
	  printk("SCSI CD error : host %d id %d lun %d return code = %03x\n", 
		 scsi_CDs[DEVICE_NR(CURRENT->dev)].device->host_no, 
		 scsi_CDs[DEVICE_NR(CURRENT->dev)].device->id,
		 scsi_CDs[DEVICE_NR(CURRENT->dev)].device->lun,
		 result);
	    
	if (status_byte(result) == CHECK_CONDITION)
	    printk("\tSense class %x, sense error %x, extended sense %x\n",
		 sense_class(sense_buffer[0]), 
		 sense_error(sense_buffer[0]),
		 sense_buffer[2] & 0xf);
	
	end_request(0);
	do_sr_request();
	}
}

static int sr_open(struct inode * inode, struct file * filp)
{
	if(MINOR(inode->i_rdev) >= NR_SR ||
	   !scsi_CDs[MINOR(inode->i_rdev)].device) return -EACCES;   /* No such device */

        check_disk_change(inode->i_rdev);

	if(!scsi_CDs[MINOR(inode->i_rdev)].device->access_count++)
	  sr_ioctl(inode, NULL, SCSI_IOCTL_DOORLOCK, 0);
	return 0;
}

/*
 * do_sr_request() is the request handler function for the sr driver.  Its function in life 
 * is to take block device requests, and translate them to SCSI commands.
 */
	
void do_sr_request (void)
{
	unsigned int dev, block, realcount;
	unsigned char cmd[10], *buffer, tries;

	tries = 2;

      repeat:
	INIT_REQUEST;
	dev =  MINOR(CURRENT->dev);
	block = CURRENT->sector;	

	bb[dev].start = block % 4;
	block = block / 4;

	if (dev >= NR_SR)
		{
		/* printk("CD-ROM request error: invalid device.\n");			*/
		end_request(0);
		tries = 2;
		goto repeat;
		}

	if (!scsi_CDs[dev].use)
		{
		/* printk("CD-ROM request error: device marked not in use.\n");		*/
		end_request(0);
		tries = 2;
		goto repeat;
		}

	if (scsi_CDs[dev].device->changed)
	        {
/* 
 * quietly refuse to do anything to a changed disc until the changed bit has been reset
 */
		/* printk("CD-ROM has been changed.  Prohibiting further I/O.\n");	*/
		end_request(0);
		tries = 2;
		goto repeat;
		}
	
	if (!CURRENT->bh)  	
		this_count = CURRENT->nr_sectors;
	else
		this_count = (CURRENT->bh->b_size / 512);

	if (bb[dev].start)
	        {				  
		bb[dev].use = 1;

	        this_count = ((this_count > 4 - bb[dev].start) ? 
			      (4 - bb[dev].start) : (this_count));

		if (bb[dev].block == block)
			{
			  rw_intr(SR_HOST, 0);
			  return;
			}

		buffer = bb[dev].buffer;
		bb[dev].block = block;
	        } 
	else if (this_count < 4)
	        {
		bb[dev].use = 1;

		if (bb[dev].block == block)
			{
			  rw_intr(SR_HOST, 0);
			  return;
			}

		buffer = bb[dev].buffer;
		bb[dev].block = block;
	        }
	else
	        {
		this_count -= this_count % 4;
		buffer = CURRENT->buffer;
		bb[dev].use = 0;
	        }

	realcount = (this_count + 3) / 4;

	switch (CURRENT->cmd)
		{
		case WRITE: 		
			end_request(0);
			goto repeat;
			break;
		case READ : 
		        cmd[0] = READ_6;
			break;
		default : 
			printk ("Unknown sr command %d\r\n", CURRENT->cmd);
			panic("");
		}
	
	cmd[1] = (SR_LUN << 5) & 0xe0;

	if (((realcount > 0xff) || (block > 0x1fffff)) && scsi_CDs[dev].ten) 
		{
		if (realcount > 0xffff)
		        {
			realcount = 0xffff;
			this_count = realcount * 4;
			}

		cmd[0] += READ_10 - READ_6 ;
		cmd[2] = (unsigned char) (block >> 24) & 0xff;
		cmd[3] = (unsigned char) (block >> 16) & 0xff;
		cmd[4] = (unsigned char) (block >> 8) & 0xff;
		cmd[5] = (unsigned char) block & 0xff;
		cmd[6] = cmd[9] = 0;
		cmd[7] = (unsigned char) (realcount >> 8) & 0xff;
		cmd[8] = (unsigned char) realcount & 0xff;
		}
	else
		{
		if (realcount > 0xff)
		        {
			realcount = 0xff;
			this_count = realcount * 4;
		        }
	
		cmd[1] |= (unsigned char) ((block >> 16) & 0x1f);
		cmd[2] = (unsigned char) ((block >> 8) & 0xff);
		cmd[3] = (unsigned char) block & 0xff;
		cmd[4] = (unsigned char) realcount;
		cmd[5] = 0;
		}   

	scsi_do_cmd (SR_HOST, SR_ID, (void *) cmd, buffer, realcount << 11, 
		     rw_intr, SR_TIMEOUT, sense_buffer, MAX_RETRIES);
}

unsigned long sr_init(unsigned long memory_start, unsigned long memory_end)
{
	int i;

	bb = (struct block_buffer *) memory_start;
	memory_start += NR_SR * sizeof(struct block_buffer);

	for (i = 0; i < NR_SR; ++i)
		{
		scsi_CDs[i].capacity = 0x1fffff;
		scsi_CDs[i].sector_size = 2048;
		scsi_CDs[i].use = 1;
		scsi_CDs[i].ten = 1;
		scsi_CDs[i].remap = 1;
		sr_sizes[i] = scsi_CDs[i].capacity;

		bb[i].block = -1;
		}

	blk_dev[MAJOR_NR].request_fn = DEVICE_REQUEST;
	blk_size[MAJOR_NR] = sr_sizes;	
	blkdev_fops[MAJOR_NR] = &sr_fops; 
	return memory_start;
}	
#endif





