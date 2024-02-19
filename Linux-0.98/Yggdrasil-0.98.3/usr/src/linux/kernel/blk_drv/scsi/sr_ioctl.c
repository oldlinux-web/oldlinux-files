#include <linux/config.h>
#ifdef CONFIG_BLK_DEV_SR
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <linux/errno.h>

#include "../blk.h"
#include "scsi.h"
#include "sr.h"
#include "scsi_ioctl.h"

#include <linux/cdrom.h>

#define IOCTL_RETRIES 3
/* The CDROM is fairly slow, so we need a little extra time */
#define IOCTL_TIMEOUT 200

static u_char 	sr_cmd[10];
static u_char 	data_buffer[255];
static u_char 	sense_buffer[255];
static int 	the_result;

static struct wait_queue *sr_cmd_wait = NULL;   /* For waiting until cmd done*/
static u_char 	sr_lock = 0;   /* To make sure that only one person is doing
				  an ioctl at one time */
static int 	target;

extern int scsi_ioctl (Scsi_Device *dev, int cmd, void *arg);

static void lock_sr_ioctl( void )
{
  /* We do not use wakeup here because there could conceivably be three
     processes trying to get at the drive simultaneously, and we would
     be screwed if that happened.
     */

	while (sr_lock);
	sr_lock = 1;
}

static void unlock_sr_ioctl( void )
{
	sr_lock = 0;
}

static void sr_ioctl_done( int host, int result )
{
	the_result = result;
	wake_up(&sr_cmd_wait);
}

/* We do our own retries because we want to know what the specific
   error code is.  Normally the UNIT_ATTENTION code will automatically
   clear after one error */

static int do_ioctl( void )
{
	int retries = IOCTL_RETRIES;
retry:     

	the_result = -1;

	scsi_do_cmd(scsi_CDs[target].device->host_no, scsi_CDs[target].device->id,
		    (void *) sr_cmd, (void *) data_buffer, 255, sr_ioctl_done, 
		    IOCTL_TIMEOUT, (void *) sense_buffer, 0);

	while (the_result < 0) sleep_on(&sr_cmd_wait);

	if(driver_byte(the_result) != 0 && 
	   (sense_buffer[2] & 0xf) == UNIT_ATTENTION) {
	  scsi_CDs[target].device->changed = 1;
	  printk("Disc change detected.\n");
	};

	if (the_result && retries)
	        {
		retries--;
		goto retry;
		}

/* Minimal error checking.  Ignore cases we know about, and report the rest. */
	if(driver_byte(the_result) != 0)
	  switch(sense_buffer[2] & 0xf) {
	  case UNIT_ATTENTION:
	    scsi_CDs[target].device->changed = 1;
	    printk("Disc change detected.\n");
	    break;
	  case NOT_READY: /* This happens if there is no disc in drive */
	    printk("CDROM not ready.  Make sure there is a disc in the drive.\n");
	    break;
	  case ILLEGAL_REQUEST:
	    printk("CDROM (ioctl) reports ILLEGAL REQUEST.\n");
	    break;
	  default:
	    printk("SCSI CD error: host %d id %d lun %d return code = %03x\n", 
		   scsi_CDs[target].device->host_no, 
		   scsi_CDs[target].device->id,
		   scsi_CDs[target].device->lun,
		   the_result);
	    printk("\tSense class %x, sense error %x, extended sense %x\n",
		   sense_class(sense_buffer[0]), 
		   sense_error(sense_buffer[0]),
		   sense_buffer[2] & 0xf);
	    
	};
      	return the_result;
}

int sr_ioctl(struct inode * inode, struct file * file, unsigned long cmd, unsigned long arg)
{
	int dev = inode->i_rdev;
	int result;

	target = MINOR(dev);

	switch (cmd) 
		{
		/* Sun-compatible */
		case CDROMPAUSE:
			lock_sr_ioctl();

			sr_cmd[0] = SCMD_PAUSE_RESUME;
			sr_cmd[1] = scsi_CDs[target].device->lun << 5;
			sr_cmd[2] = sr_cmd[3] = sr_cmd[4] = 0;
			sr_cmd[5] = sr_cmd[6] = sr_cmd[7] = 0;
			sr_cmd[8] = 1;
			sr_cmd[9] = 0;

			result = do_ioctl();

			unlock_sr_ioctl();
			return result;

		case CDROMRESUME:
			lock_sr_ioctl();

			sr_cmd[0] = SCMD_PAUSE_RESUME;
			sr_cmd[1] = scsi_CDs[target].device->lun << 5;
			sr_cmd[2] = sr_cmd[3] = sr_cmd[4] = 0;
			sr_cmd[5] = sr_cmd[6] = sr_cmd[7] = 0;
			sr_cmd[8] = 0;
			sr_cmd[9] = 0;

			result = do_ioctl();

			unlock_sr_ioctl();
			return result;

		case CDROMPLAYMSF:
			{
			struct cdrom_msf msf;
			lock_sr_ioctl();

			memcpy_fromfs(&msf, (void *) arg, sizeof(msf));

			sr_cmd[0] = SCMD_PLAYAUDIO_MSF;
			sr_cmd[1] = scsi_CDs[target].device->lun << 5;
			sr_cmd[2] = 0;
			sr_cmd[3] = msf.cdmsf_min0;
			sr_cmd[4] = msf.cdmsf_sec0;
			sr_cmd[5] = msf.cdmsf_frame0;
			sr_cmd[6] = msf.cdmsf_min1;
			sr_cmd[7] = msf.cdmsf_sec1;
			sr_cmd[8] = msf.cdmsf_frame1;
			sr_cmd[9] = 0;

			result = do_ioctl();

			unlock_sr_ioctl();
			return result;
			}

		case CDROMPLAYTRKIND:
			{
			struct cdrom_ti ti;
			lock_sr_ioctl();

			memcpy_fromfs(&ti, (void *) arg, sizeof(ti));

			sr_cmd[0] = SCMD_PLAYAUDIO_TI;
			sr_cmd[1] = scsi_CDs[target].device->lun << 5;
			sr_cmd[2] = 0;
			sr_cmd[3] = 0;
			sr_cmd[4] = ti.cdti_trk0;
			sr_cmd[5] = ti.cdti_ind0;
			sr_cmd[6] = 0;
			sr_cmd[7] = ti.cdti_trk1;
			sr_cmd[8] = ti.cdti_ind1;
			sr_cmd[9] = 0;

			result = do_ioctl();

			unlock_sr_ioctl();
			return result;
			}

		case CDROMREADTOCHDR:
			return -EINVAL;
		case CDROMREADTOCENTRY:
			return -EINVAL;

		case CDROMSTOP:
		        lock_sr_ioctl();

		        sr_cmd[0] = START_STOP;
			sr_cmd[1] = ((scsi_CDs[target].device->lun) << 5) | 1;
			sr_cmd[2] = sr_cmd[3] = sr_cmd[5] = 0;
			sr_cmd[4] = 0;

			result = do_ioctl();

			unlock_sr_ioctl();
			return result;
			
		case CDROMSTART:
		        lock_sr_ioctl();

		        sr_cmd[0] = START_STOP;
			sr_cmd[1] = ((scsi_CDs[target].device->lun) << 5) | 1;
			sr_cmd[2] = sr_cmd[3] = sr_cmd[5] = 0;
			sr_cmd[4] = 1;

			result = do_ioctl();

			unlock_sr_ioctl();
			return result;

		case CDROMEJECT:
		        lock_sr_ioctl();

		        sr_cmd[0] = START_STOP;
			sr_cmd[1] = ((scsi_CDs[target].device->lun) << 5) | 1;
			sr_cmd[2] = sr_cmd[3] = sr_cmd[5] = 0;
			sr_cmd[4] = 0x02;

			result = do_ioctl();

			unlock_sr_ioctl();
			return result;

		case CDROMVOLCTRL:
			return -EINVAL;
		case CDROMSUBCHNL:
			return -EINVAL;
		case CDROMREADMODE2:
			return -EINVAL;
		case CDROMREADMODE1:
			return -EINVAL;

		RO_IOCTLS(dev,arg);
		default:
			return scsi_ioctl(scsi_CDs[target].device,cmd,(void *) arg);
		}
}

#endif
