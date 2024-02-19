/* geometry.c  -  Device and file geometry computation */

/* Written 1992 by Werner Almesberger */


#ifdef NEW_INCLUDES
#include <linux/limits.h>
#define _LIMITS_H
#endif

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* resolve current types.h conflict */

#ifdef _SYS_TYPES_H
#define _LINUX_TYPES_H
#endif

/* resolve possible future conflicts when implicitly including sched.h */

#ifdef _SIGNAL_H
#define _LINUX_SIGNAL_H
#endif

#ifdef _SYS_TIME_H
#define _LINUX_TIME_H
#endif

#ifdef _SYS_RESOURCE_H
#define _LINUX_RESOURCE_H
#endif

#include <linux/fs.h>
#include <linux/fd.h>
#include <linux/hdreg.h>

#include "config.h"
#include "lilo.h"
#include "common.h"
#include "device.h"
#include "geometry.h"


typedef struct _dt_entry {
    int device,bios;
    int sectors,heads,cylinders,start;
    struct _dt_entry *next;
} DT_ENTRY;


static DT_ENTRY *disktab = NULL;


void geo_init(char *name)
{
    FILE *file;
    char line[MAX_LINE+1];
    char *here;
    DT_ENTRY *entry;

    if (name) {
	if ((file = fopen(name,"r")) == NULL)
	    die("open %s: %s",name,strerror(errno));
    }
    if ((file = fopen(DFL_DISKTAB,"r")) == NULL) return;
    while (fgets(line,MAX_LINE,file)) {
	if (here = strchr(line,'\n')) *here = 0;
	if (here = strchr(line,'#')) *here = 0;
	if (strspn(line," \t") != strlen(line)) {
	    entry = alloc(sizeof(DT_ENTRY));
	    if (sscanf(line,"0x%x 0x%x %d %d %d %d",&entry->device,&entry->bios,
	      &entry->sectors,&entry->heads,&entry->cylinders,&entry->start)
	      != 6)
		die("Invalid line in %s:\n\"%s\"",name ? name : DFL_DISKTAB,
		  line);
	    entry->next = disktab;
	    disktab = entry;
	}
    }
    (void) fclose(file);
}


static void geo_query_dev(GEOMETRY *geo,int device)
{
    DEVICE dev;
    int fd;
    struct floppy_struct fdprm;
    struct hd_geometry hdprm;

    fd = dev_open(&dev,device,O_NOACCESS);
    switch (MAJOR(device)) {
	case 2:
	    if (ioctl(fd,FDGETPRM,&fdprm) < 0)
		pdie("geo_query_dev FDGETPRM");
	    geo->device = device & 3;
	    geo->heads = fdprm.head;
	    geo->cylinders = fdprm.track;
	    geo->sectors = fdprm.sect;
	    geo->start = 0;
	    break;
	case 3:
	    if (ioctl(fd,HDIO_REQ,&hdprm) < 0)
		pdie("geo_query_dev HDIO_REQ");
	    geo->device = 0x80 | ((device & 255) >> 6);
	    geo->heads = hdprm.heads;
	    geo->cylinders = hdprm.cylinders;
	    geo->sectors = hdprm.sectors;
	    geo->start = hdprm.start;
	    break;
	default:
	    die("Sorry, don't know how to handle device 0x%04x",device);
    }
    dev_close(&dev);
}


static void geo_get(GEOMETRY *geo,int device,int user_device)
{
    DT_ENTRY *walk;

    for (walk = disktab; walk; walk = walk->next)
	if (walk->device == device) break;
    if (walk == NULL) geo_query_dev(geo,device);
    else {
	geo->device = walk->bios;
	geo->heads = walk->heads;
	geo->cylinders = walk->cylinders;
	geo->sectors = walk->sectors;
	geo->start = walk->start;
    }
    if (user_device != -1) geo->device = user_device;
    if (verbose > 2) {
	printf("Device 0x%04x: BIOS drive 0x%02x, %d heads, %d cylinders,\n",
	  device,geo->device,geo->heads,geo->cylinders);
	printf("%15s%d sectors. Partition offset: %d sectors.\n","",
	  geo->sectors,geo->start);
    }
}


int geo_open(GEOMETRY *geo,char *name,int flags)
{
    char *here;
    int user_dev,block_size;
    struct stat st;

    if ((here = strrchr(name,':')) == NULL) user_dev = -1;
    else {
	*here++ = 0;
	user_dev = to_number(here);
    }
    if ((geo->fd = open(name,flags)) < 0)
	die("open %s: %s",name,strerror(errno));
    if (fstat(geo->fd,&st) < 0) die("fstat %s: %s",name,strerror(errno));
    if (!S_ISREG(st.st_mode) && !S_ISBLK(st.st_mode))
	die("%s: neither a reg. file nor a block dev.",name);
    geo_get(geo,S_ISREG(st.st_mode) ? st.st_dev : st.st_rdev,user_dev);
    geo->file = S_ISREG(st.st_mode);
#ifndef FIGETBSZ
    geo->spb = 2;
#else
    if (!geo->file) geo->spb = 2;
    else {
	if (ioctl(geo->fd,FIGETBSZ,&block_size) < 0) {
	    fprintf(stderr,"Warning: FIGETBSZ %s: %s\n",name,strerror(errno));
	    geo->spb = 2;
	}
	else {
	    if (!block_size || (block_size & (SECTOR_SIZE-1)))
		die("Incompatible block size: %d\n",block_size);
	    geo->spb = block_size/SECTOR_SIZE;
	}
    }
#endif
    return geo->fd;
}


void geo_close(GEOMETRY *geo)
{
    (void) close(geo->fd);
}


#ifndef FIBMAP
#define FIBMAP BMAP_IOCTL
#endif


int geo_comp_addr(GEOMETRY *geo,int offset,SECTOR_ADDR *addr)
{
    int block,sector;

    block = offset/geo->spb/SECTOR_SIZE;
    if (geo->file) {
	if (ioctl(geo->fd,FIBMAP,&block) < 0) pdie("ioctl FIBMAP");
	if (!block) return 0;
    }
    sector = block*geo->spb+((offset/SECTOR_SIZE) % geo->spb);
    sector += geo->start;
    addr->device = geo->device;
    addr->sector = (sector % geo->sectors)+1;
    sector /= geo->sectors;
    addr->head = sector % geo->heads;
    sector /= geo->heads;
    if (sector > 1023)
	die("geo_comp_addr: Cylinder number is too big (%d > 1023)",sector);
    if (sector >= geo->cylinders)
	die("geo_comp_addr: Cylinder %d beyond end of media (%d)",sector,
	  geo->cylinders);
    if (verbose > 3)
	printf("fd %d: offset %d -> dev %d, head %d, track %d, sector %d\n",
	  geo->fd,offset,addr->device,addr->head,sector,addr->sector);
    addr->track = sector & 255;
    addr->sector |= (sector >> 8) << 6;
    addr->num_sect = 1;
    return 1;
}
