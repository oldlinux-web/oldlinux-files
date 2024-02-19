/* bsect.c  -  Boot sector handling */

/* Written 1992 by Werner Almesberger */


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>

#include "config.h"
#include "common.h"
#include "device.h"
#include "map.h"
#include "boot.h"
#include "bsect.h"


static BOOT_SECTOR bsect,bsect_orig;
static DESCR_SECTOR descrs;
static DEVICE dev;
static char *boot_devnam,*map_name;
static int fd,boot_dev_nr,image = 0;
static char temp_map[PATH_MAX+1];


void bsect_open(char *boot_dev,char *map_file,char *install,int delay)
{
    GEOMETRY geo;
    struct stat st;
    int i_fd,sectors;

    if (stat(map_file,&st) >= 0 && !S_ISREG(st.st_mode))
	die("Map %s is not a regular file.",map_file);
    if (verbose)
	printf("Reading boot sector from %s\n",boot_dev ? boot_dev :
	  "current root.");
    if (boot_devnam = boot_dev) {
	if ((fd = open(boot_dev,O_RDWR)) < 0)
	    die("open %s: %s",boot_dev,strerror(errno));
	if (fstat(fd,&st) < 0) die("stat %s: %s",boot_dev,strerror(errno));
	if (!S_ISBLK(st.st_mode)) boot_dev_nr = 0;
	else boot_dev_nr = st.st_rdev;
    }
    else {
	if (stat("/",&st) < 0) pdie("stat /");
	if ((st.st_dev & PART_MASK) > PART_MAX)
	    die("Can't put the boot sector on logical partition 0x%X",
	      st.st_dev);
	fd = dev_open(&dev,boot_dev_nr = st.st_dev,O_RDWR);
    }
    if (read(fd,(char *) &bsect,SECTOR_SIZE) != SECTOR_SIZE)
	die("read %s: %s",boot_dev ? boot_dev : dev.name,strerror(errno));
    memcpy(&bsect_orig,&bsect,SECTOR_SIZE);
    if (install) {
	if (verbose) printf("Merging with %s\n",install);
	i_fd = geo_open(&geo,install,O_RDONLY);
	if (read(i_fd,(char *) &bsect,MAX_BOOT_SIZE) != MAX_BOOT_SIZE)
	    die("read %s: %s",boot_dev ? boot_dev : dev.name,strerror(errno));
	if (fstat(i_fd,&st) < 0)
	    die("stat %s: %s",boot_dev ? boot_dev : dev.name,strerror(errno));
	map_begin_section(); /* no access to the (not yet open) map file
		required, because this map is built in memory */
	map_add(&geo,1,(st.st_size+SECTOR_SIZE-1)/SECTOR_SIZE-1);
	sectors = map_write(bsect.par_1.secondary,MAX_SECONDARY);
	if (verbose > 1)
	    printf("Secondary loader: %d sector%s.\n",sectors,sectors == 1 ?
	      "" : "s");
	geo_close(&geo);
    }
    check_version(&bsect,STAGE_FIRST);
    strcat(strcpy(temp_map,map_name = map_file),MAP_TMP_APP);
    map_create(temp_map);
    *(unsigned short *) &bsect.sector[BOOT_SIG_OFFSET] = BOOT_SIGNATURE;
    bsect.par_1.delay = delay*100/55;
    memset(&descrs,0,SECTOR_SIZE);
}


static void bsect_dispatch(char *spec,IMAGE_DESCR *descr)
{
    char *here,*this;

    if (here = strchr(spec,'#')) {
	*here++ = 0;
	boot_device(spec,here,descr);
	return;
    }
    if (here = strchr(spec,'@')) {
	*here++ = 0;
	if (this = strchr(spec,'+')) {
	    *this++ = 0;
	    boot_other(spec,this,here,descr);
	    return;
	}
	boot_other(DFL_CHAIN,spec,here,descr);
	return;
    }
    if (here = strchr(spec,'+')) {
	*here++ = 0;
	boot_unstripped(spec,here,descr);
	return;
    }
    boot_image(spec,descr);
}


static int dev_number(char *dev)
{
    struct stat st;

    if (stat(dev,&st) >= 0) return st.st_rdev;
    return to_number(dev);
}


void bsect_image(char *spec)
{
    IMAGE_DESCR *descr;
    struct stat st;
    char *here,*this,*label;
    int other;

    descr = &descrs.descr[image];
    if (stat("/",&st) < 0) pdie("stat /");
    descr->bss_seg = 0;
    descr->root_dev = st.st_dev;
    descr->swap_dev = 0;
    if (here = strrchr(spec,',')) *here++ = 0;
    if (this = strchr(label = spec,'=')) {
	*this++ = 0;
	spec = this;
    }
    bsect_dispatch(spec,descr);
    if (here) {
	descr->root_dev = dev_number(here);
	descr->swap_dev = 0;
	if (here = strrchr(spec,',')) {
	    *here++ = 0;
	    descr->swap_dev = descr->root_dev;
	    descr->root_dev = dev_number(here);
	}
    }
    if (here = strrchr(label,'/')) label = here+1;
    if (strlen(label) > MAX_IMAGE_NAME) die("Label \"%s\" is too long",label);
    for (other = 0; other < image; other++)
	if (!strcmp(descrs.descr[other].name,label))
	    die("Duplicate label \"%s\"",label);
    strcpy(descr->name,label);
    printf("Added %s\n",label);
    if (verbose > 1)
	printf("%4s<dev=0x%02x,hd=%d,cyl=%d,sct=%d>,root=0x%04x,swap=0x%04x\n",
	  "",descrs.descr[image].start.device,descrs.descr[image].start.head,
	  descrs.descr[image].start.track,descrs.descr[image].start.sector,
	  descrs.descr[image].root_dev,descrs.descr[image].swap_dev);
    image++;
}


static void unbootable(void)
{
    fprintf(stderr,"\nWARNING: The system is unbootable !\n");
    fprintf(stderr,"%9sRun LILO again to correct this.","");
    exit(1);
}


void bsect_update(char *backup_file,int force_backup)
{
    char temp_name[PATH_MAX+1];
    int bck_file;

    if (backup_file == NULL) {
	sprintf(temp_name,LILO_DIR "/boot.%04X",boot_dev_nr);
	backup_file = temp_name;
    }
    bck_file = open(backup_file,O_RDONLY);
    if (bck_file >= 0 && force_backup) {
	(void) close(bck_file);
	bck_file = -1;
    }
    if (bck_file >= 0) {
	if (verbose) printf("%s exists - no backup copy made.\n",backup_file);
    }
    else {
	if ((bck_file = creat(backup_file,0644)) < 0)
	    die("creat %s: %s",backup_file,strerror(errno));
	if (write(bck_file,(char *) &bsect_orig,SECTOR_SIZE) != SECTOR_SIZE)
	    die("write %s: %s",backup_file,strerror(errno));
	if (verbose) printf("Backup copy of boot sector in %s\n",backup_file);
    }
    if (close(bck_file) < 0) die("close %s: %s",backup_file,strerror(errno));
    map_done(&descrs,&bsect.par_1.descr);
    if (lseek(fd,0,0) < 0)
	die("lseek %s: %s",boot_devnam ? boot_devnam : dev.name,
	  strerror(errno));
    if (verbose) printf("Writing boot sector.\n");
    if (write(fd,(char *) &bsect,SECTOR_SIZE) != SECTOR_SIZE)
	die("write %s: %s",boot_devnam ? boot_devnam : dev.name,
	  strerror(errno));
    if (boot_devnam == NULL) dev_close(&dev);
    else if (close(fd) < 0) {
	    unbootable();
	    die("close %s: %s",boot_devnam,strerror(errno));
	}
    if (rename(temp_map,map_name) < 0) {
	unbootable();
	die("rename %s %s: %s",temp_map,map_name,strerror(errno));
    }
}


void bsect_cancel(void)
{
    map_done(&descrs,&bsect.par_1.descr);
    if (boot_devnam) (void) close(fd);
    else dev_close(&dev);
    (void) remove(temp_map);
}
