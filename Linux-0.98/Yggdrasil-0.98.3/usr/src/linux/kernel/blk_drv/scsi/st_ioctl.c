#include <linux/config.h>
#ifdef CONFIG_BLK_DEV_ST
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include "st.h"

extern int scsi_ioctl(Scsi_Device *dev, int cmd, void *arg);

int st_ioctl(struct inode * inode, struct file * file, unsigned long cmd, unsigned long arg)
{
	int dev = inode->i_rdev;

	switch (cmd) {
		default:
			return scsi_ioctl(scsi_tapes[MINOR(dev)].device, cmd, (void *) arg);
	}
}
#endif
