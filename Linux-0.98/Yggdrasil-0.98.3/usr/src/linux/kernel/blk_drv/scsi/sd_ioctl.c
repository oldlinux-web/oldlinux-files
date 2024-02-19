#include <linux/config.h>
#ifdef CONFIG_BLK_DEV_SD
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include "scsi.h"
#include "sd.h"

extern int scsi_ioctl (Scsi_Device *dev, int cmd, void *arg);
extern int revalidate_scsidisk(int, int);

int sd_ioctl(struct inode * inode, struct file * file, unsigned long cmd, unsigned long arg)
{
	int dev = inode->i_rdev;

	switch (cmd) {
		case BLKRRPART: /* Re-read partition tables */
			return revalidate_scsidisk(dev, 1);
		default:
			return scsi_ioctl(rscsi_disks[MINOR(dev) >> 4].device , cmd, (void *) arg);
	}
}
#endif






