#include <linux/config.h>
#ifdef CONFIG_SCSI 

#include <asm/io.h>
#include <asm/segment.h>
#include <asm/system.h>

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>

#include "scsi.h"
#include "hosts.h"
#include "scsi_ioctl.h"

#define MAX_RETRIES 5	
#define MAX_TIMEOUT 200
#define MAX_BUF 8192  	

#define max(a,b) (((a) > (b)) ? (a) : (b))

/*
 * If we are told to probe a host, we will return 0 if  the host is not
 * present, 1 if the host is present, and will return an identifying
 * string at *arg, if arg is non null, filling to the length stored at
 * (int *) arg
 */

static int ioctl_probe(int dev, void *buffer)
{
	int temp;
	int len;
	
	if ((temp = scsi_hosts[dev].present) && buffer) {
		len = get_fs_long ((int *) buffer);
		memcpy_tofs (buffer, scsi_hosts[dev].info(), len);
	}
	return temp;
}

/*
 * 
 * The SCSI_IOCTL_SEND_COMMAND ioctl sends a command out to the SCSI host.
 * The MAX_TIMEOUT and MAX_RETRIES  variables are used.  
 * 
 * dev is the SCSI device struct ptr, *(int *) arg is the length of the
 * input data, if any, not including the command string & counts, 
 * *((int *)arg + 1) is the output buffer size in bytes.
 * 
 * *(char *) ((int *) arg)[2] the actual command byte.   
 * 
 * Note that no more than MAX_BUF data bytes will be transfered.  Since
 * SCSI block device size is 512 bytes, I figured 1K was good.
 * but (WDE) changed it to 8192 to handle large bad track buffers.
 * 
 * This size *does not* include the initial lengths that were passed.
 * 
 * The SCSI command is read from the memory location immediately after the
 * length words, and the input data is right after the command.  The SCSI
 * routines know the command size based on the opcode decode.  
 * 
 * The output area is then filled in starting from the command byte. 
 */

static int the_result[MAX_SCSI_HOSTS] = {0,};

static void scsi_ioctl_done (int host, int result)
{
	the_result[host] = result;	
}	

/* This function will operate certain scsi functions which require no
   data transfer */
static int ioctl_internal_command(Scsi_Device *dev, char ** command)
{
	char * cmd;
	int temp, host;
	char sense_buffer[256];

	host = dev->host_no;
	cmd = command[0];

	do {
		cli();
		if (the_result[host]) {
			sti();
			while(the_result[host])
				/* nothing */;
		} else {
			the_result[host]=-1;
			sti();
			break;
		}
	} while (1);

	scsi_do_cmd(host,  dev->id,  cmd,  NULL,  0,
			scsi_ioctl_done,  MAX_TIMEOUT,
			sense_buffer,  MAX_RETRIES);

	while (the_result[host] == -1)
		/* nothing */;
	temp = the_result[host];

	if(driver_byte(the_result[host]) != 0)
	  switch(sense_buffer[2] & 0xf) {
	  case ILLEGAL_REQUEST:
	    printk("SCSI device (ioctl) reports ILLEGAL REQUEST.\n");
	    break;
	  case NOT_READY: /* This happens if there is no disc in drive */
	    if(dev->removable){
	      printk("Device not ready.  Make sure there is a disc in the drive.\n");
	      break;
	    };
	  case UNIT_ATTENTION:
	    if (dev->removable){
	      dev->changed = 1;
	      temp = 0; /* This is no longer considered an error */
	      printk("Disc change detected.\n");
	      break;
	    };
	  default: /* Fall through for non-removable media */
	    printk("SCSI CD error: host %d id %d lun %d return code = %x\n",
		   dev->host_no,
		   dev->id,
		   dev->lun,
		   the_result);
	    printk("\tSense class %x, sense error %x, extended sense %x\n",
		   sense_class(sense_buffer[0]),
		   sense_error(sense_buffer[0]),
		   sense_buffer[2] & 0xf);

	  };

	the_result[host] = 0;
	return temp;
}

static int ioctl_command(Scsi_Device *dev, void *buffer)
{
	char buf[MAX_BUF];
	char cmd[10];
	char * cmd_in;
	unsigned char opcode;
	int inlen, outlen, cmdlen, temp, host;

	if (!buffer)
		return -EINVAL;
	
	inlen = get_fs_long((int *) buffer);
	outlen = get_fs_long( ((int *) buffer) + 1);

	cmd_in = (char *) ( ((int *)buffer) + 2);
	opcode = get_fs_byte(cmd_in); 

	memcpy_fromfs ((void *) cmd,  cmd_in,  cmdlen = COMMAND_SIZE (opcode));
	memcpy_fromfs ((void *) buf,  (void *) (cmd_in + cmdlen),  inlen);
	host = dev->host_no;

#ifndef DEBUG_NO_CMD
	do {
		cli();
		if (the_result[host]) {
			sti();
			while(the_result[host])
				/* nothing */;
		} else {
			the_result[host]=-1;
			sti();
			break;
		}
	} while (1);
	
	scsi_do_cmd(host,  dev->id,  cmd,  buf,  ((outlen > MAX_BUF) ? 
			MAX_BUF : outlen),  scsi_ioctl_done,  MAX_TIMEOUT, 
			buf,  MAX_RETRIES);

	while (the_result[host] == -1)
		/* nothing */;
	temp = the_result[host];
	the_result[host] = 0;
	memcpy_tofs ((void *) cmd_in,  buf,  (outlen > MAX_BUF) ? MAX_BUF  : outlen);
	return temp;
#else
	{
	int i;
	printk("scsi_ioctl : device %d.  command = ", dev->id);
	for (i = 0; i < 10; ++i)
		printk("%02x ", cmd[i]);
	printk("\r\nbuffer =");
	for (i = 0; i < 20; ++i)
		printk("%02x ", buf[i]);
	printk("\r\n");
	printk("inlen = %d, outlen = %d, cmdlen = %d\n",
		inlen, outlen, cmdlen);
	printk("buffer = %d, cmd_in = %d\n", buffer, cmd_in);
	}
	return 0;
#endif
}

	
/*
	the scsi_ioctl() function differs from most ioctls in that it does
	not take a major/minor number as the dev filed.  Rather, it takes
	a pointer to a scsi_devices[] element, a structure. 
*/
int scsi_ioctl (Scsi_Device *dev, int cmd, void *arg)
{
        char scsi_cmd[10];
	char * command[2];

	if ((cmd != 0 && dev->id > NR_SCSI_DEVICES))
		return -ENODEV;
	if ((cmd == 0 && dev->host_no > MAX_SCSI_HOSTS))
		return -ENODEV;
	
	switch (cmd) {
		case SCSI_IOCTL_PROBE_HOST:
			return ioctl_probe(dev->host_no, arg);
		case SCSI_IOCTL_SEND_COMMAND:
			return ioctl_command((Scsi_Device *) dev, arg);
		case SCSI_IOCTL_DOORLOCK:
			if (!dev->removable) return 0;
		        scsi_cmd[0] = ALLOW_MEDIUM_REMOVAL;
			scsi_cmd[1] = dev->lun << 5;
			scsi_cmd[2] = scsi_cmd[3] = scsi_cmd[5] = 0;
			scsi_cmd[4] = SCSI_REMOVAL_PREVENT;
			command[0] = scsi_cmd;
			return ioctl_internal_command((Scsi_Device *) dev, command);
			break;
		case SCSI_IOCTL_DOORUNLOCK:
			if (!dev->removable) return 0;
		        scsi_cmd[0] = ALLOW_MEDIUM_REMOVAL;
			scsi_cmd[1] = dev->lun << 5;
			scsi_cmd[2] = scsi_cmd[3] = scsi_cmd[5] = 0;
			scsi_cmd[4] = SCSI_REMOVAL_ALLOW;
			command[0] = scsi_cmd;
			return ioctl_internal_command((Scsi_Device *) dev, command);
		case SCSI_IOCTL_TEST_UNIT_READY:
		        scsi_cmd[0] = TEST_UNIT_READY;
			scsi_cmd[1] = dev->lun << 5;
			scsi_cmd[2] = scsi_cmd[3] = scsi_cmd[5] = 0;
			scsi_cmd[4] = 0;
			command[0] = scsi_cmd;
			return ioctl_internal_command((Scsi_Device *) dev, command);
			break;
		default :			
			return -EINVAL;
	}
}
#endif
