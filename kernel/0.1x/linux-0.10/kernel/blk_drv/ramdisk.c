/*
 *  linux/kernel/blk_drv/ramdisk.c
 *
 *  Written by Theodore Ts'o
 */

#include <linux/config.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/hdreg.h>
#include <asm/system.h>
#include <asm/segment.h>
#include <asm/memory.h>

#define MAJOR_NR 1
#include "blk.h"

char	*ram_disk;			/* Start of ram disk */
int	ram_disk_size;			/* Size of ram disk */

void do_ram_request(void)
{
	int i,r;
	unsigned int block,dev;
	unsigned int sec,head,cyl;
	unsigned int nsect;

	INIT_REQUEST;
	if (MINOR(CURRENT->dev) != 0) {
		end_request(0);
		goto repeat;
	}
	block = CURRENT->sector;
	end_request(1);
}

void ram_init(void)
{

}
