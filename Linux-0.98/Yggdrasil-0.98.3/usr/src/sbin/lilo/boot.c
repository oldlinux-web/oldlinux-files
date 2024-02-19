/* boot.c  -  Boot image composition */

/* Written 1992 by Werner Almesberger */


#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <a.out.h>
#include <sys/stat.h>
#include <linux/config.h>

#include "common.h"
#include "geometry.h"
#include "map.h"
#include "boot.h"


static GEOMETRY geo;
static struct stat st;


void boot_image(char *spec,IMAGE_DESCR *descr)
{
    BOOT_SECTOR buff;
    int fd,sectors;

    if (verbose) printf("Boot image: %s\n",spec);
    fd = geo_open(&geo,spec,O_RDONLY);
    map_begin_section();
    if (fstat(fd,&st) < 0) die("fstat %s: 5s",spec,strerror(errno));
    if (read(fd,(char *) &buff,SECTOR_SIZE) != SECTOR_SIZE)
	die("read %s: %s",spec,strerror(errno));
    if (buff.par_l.root_dev) descr->root_dev = buff.par_l.root_dev;
    if (buff.par_l.swap_dev) descr->swap_dev = buff.par_l.swap_dev;
    map_add(&geo,1,(st.st_size+SECTOR_SIZE-1)/SECTOR_SIZE-1);
    sectors = map_end_section(&descr->start);
    geo_close(&geo);
    if (verbose > 1)
	printf("Mapped %d sector%s.\n",sectors,sectors == 1 ? "" : "s");
}


void boot_device(char *spec,char *range,IMAGE_DESCR *descr)
{
    char *here;
    int start,secs;
    int sectors;

    if (verbose) printf("Boot device: %s, range %s\n",spec,range);
    (void) geo_open(&geo,spec,O_NOACCESS);
    map_begin_section();
    if (here = strchr(range,'-')) {
	*here++ = 0;
	start = to_number(range);
	if ((secs = to_number(here)-start+1) < 0) die("Invalid range");
    }
    else {
	if (here = strchr(range,'+')) {
	    *here++ = 0;
	    start = to_number(range);
	    secs = to_number(here);
	}
	else {
	    start = to_number(range);
	    secs = 1;
	}
    }
    map_add(&geo,start,secs);
    sectors = map_end_section(&descr->start);
    geo_close(&geo);
    if (verbose > 1)
	printf("Mapped %d sector%s.\n",sectors,sectors == 1 ? "" : "s");
}


void boot_unstripped(char *setup,char *kernel,IMAGE_DESCR *descr)
{
    struct exec exec;
    int fd,sectors;

    if (verbose) printf("Unstripped: setup %s, kernel %s\n",setup,kernel);
    (void) geo_open(&geo,setup,O_RDONLY);
    map_begin_section();
    map_add(&geo,0,SETUPSECS);
    geo_close(&geo);
    fd = geo_open(&geo,kernel,O_RDONLY);
    if (read(fd,(char *) &exec,sizeof(struct exec)) != sizeof(struct exec))
	die("read %s: %s",kernel,strerror(errno));
    if (exec.a_entry || exec.a_trsize || exec.a_drsize || !exec.a_syms)
	die("Not an unstripped kernel: %s",kernel);
    map_add(&geo,2,(exec.a_text+exec.a_data+SECTOR_SIZE-1)/SECTOR_SIZE);
    if ((exec.a_text+exec.a_data) & 15)
	fprintf(stderr,"Warning: Unsupported BSS - no initialization.\n");
    else {
	if (verbose > 2)
	    printf("BSS: 0x%X+%d\n",exec.a_text+exec.a_data+DEF_SYSSEG,
	      exec.a_bss);
	descr->bss_seg = ((exec.a_text+exec.a_data) >> 4)+DEF_SYSSEG;
	descr->bss_segs = exec.a_bss >> 16;
	descr->bss_words = ((exec.a_bss & 0xffff)+1) >> 1;
    }
    geo_close(&geo);
    sectors = map_end_section(&descr->start);
    if (verbose > 1)
	printf("Mapped %d sector%s.\n",sectors,sectors == 1 ? "" : "s");
}


void boot_other(char *loader,char *boot,char *part,IMAGE_DESCR *descr)
{
    int l_fd,p_fd,walk,found;
    BOOT_SECTOR buff;

    if (verbose) printf("Boot other: %s, on %s, loader %s\n",boot,part,loader);
    (void) geo_open(&geo,boot,O_NOACCESS);
    if ((l_fd = open(loader,O_RDONLY)) < 0)
	die("open %s: %s",loader,strerror(errno));
    if (!*part) memset(&buff,0,SECTOR_SIZE);
    else {
	if ((p_fd = open(part,O_RDONLY)) < 0)
	    die("open %s: %s",part,strerror(errno));
	if (read(p_fd,(char *) &buff,SECTOR_SIZE) != SECTOR_SIZE)
	    die("read %s: %s",part,strerror(errno));
    }
    if (read(l_fd,(char *) &buff,PART_TABLE_OFFSET) < 0)
	die("read %s: %s",loader,strerror(errno));
    check_version(&buff,STAGE_CHAIN);
    if (*part) {
	found = 0;
	for (walk = 0; walk < PARTITION_ENTRIES; walk++)
	    if (!PART(buff,walk).sys_ind || PART(buff,walk).start_sect != geo.start) {
		if (PART(buff,walk).sys_ind != PART_DOS12 && PART(buff,walk).
		  sys_ind != PART_DOS16) PART(buff,walk).sys_ind = PART_INVALID;
	    }
	    else {
		if (found) die("Duplicate entry in partition table");
		buff.par_c.offset = walk*PARTITION_ENTRY;
		PART(buff,walk).boot_ind = 0x80;
		found = 1;
	    }
	if (!found) die("Partition entry not found.");
	(void) close(p_fd);
    }
    (void) close(l_fd);
    map_begin_section();
    map_add_sector(&buff);
    map_add(&geo,0,1);
    (void) map_end_section(&descr->start);
    geo_close(&geo);
    if (verbose > 1) printf("Mapped 2 (1+1) sectors.\n");
}
