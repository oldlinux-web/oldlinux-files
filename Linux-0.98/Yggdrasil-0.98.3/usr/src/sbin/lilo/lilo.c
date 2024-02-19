/* lilo.c  -  LILO command-line parameter processing */

/* Written 1992 by Werner Almesberger */


#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "config.h"
#include "common.h"
#include "lilo.h"
#include "geometry.h"
#include "bsect.h"


#define S2(x) #x
#define S(x) S2(x)


static void show_images(char *map_file)
{
    DESCR_SECTOR descrs;
    char *name;
    int fd,image;

    if ((fd = open(map_file,O_RDONLY)) < 0)
	die("open %s: %s",map_file,strerror(errno));
    if (read(fd,(char *) &descrs,SECTOR_SIZE) != SECTOR_SIZE)
	die("read %s: %s",map_file,strerror(errno));
    (void) close(fd);
    for (image = 0; image < MAX_IMAGES; image++)
	if (*(name = descrs.descr[image].name)) {
	    printf("%-" S(MAX_IMAGE_NAME) "s%s",name,image ? "  " : " *");
	    if (verbose)
		printf(" <dev=0x%02x,hd=%d,cyl=%d,sct=%d>,root="
		  "0x%04x,swap=0x%04x",
		  descrs.descr[image].start.device,
		  descrs.descr[image].start.head,
		  descrs.descr[image].start.track,
		  descrs.descr[image].start.sector,
		  descrs.descr[image].root_dev,
		  descrs.descr[image].swap_dev);
	    printf("\n");
	}
    exit(0);
}


static void usage(char *name)
{
    char *here;

    if (here = strrchr(name,'/')) name = here+1;
    fprintf(stderr,"usage: %s [ -m map_file ] [ -v ]\n",name);
    fprintf(stderr,"%7s%s [ -b boot_device ] [ -c ] [ -i boot_sector "
      "[ :code ] ]\n","",name);
    fprintf(stderr,"%12s[ -f disk_tab ] [ -m map_file [ :code ] ] [ -v ] "
      "[ -t ]\n","");
    fprintf(stderr,"%12s[ [ -s | -S ] save_file ] [ -r root_dir ] "
      "[ boot_descr ... ]\n\n","");
    fprintf(stderr,"Descriptor format:\n");
    fprintf(stderr,"    [ label= ] image(s) [ ,root_dev [ ,swap_dev ] ]\n\n");
    fprintf(stderr,"Images:\n");
    fprintf(stderr,"  Boot image:  file_name [ :code ]\n");
    fprintf(stderr,"  Device:      device [ :code ] #start [ +num | -end ]\n");
    fprintf(stderr,"  Unstripped:  setup [ :code ] +kernel [ :code ]\n");
    fprintf(stderr,"  Other OS:    loader+boot_sect [ :code ] @ [ part_table "
      "]\n");
    exit(1);
}


int main(int argc,char **argv)
{
    char *name,*boot_device,*map_file,*install,*disktab_file,*backup_file;
    int image,test,delay,force_backup;
    BOOT_SECTOR dummy;

    boot_device = NULL;
    map_file = MAP_FILE;
    install = NULL;
    disktab_file = NULL;
    backup_file = NULL;
    delay = 0;
    test = 0;
    force_backup = 0;
    name = *argv++;
    argc--;
    while (argc && **argv == '-') {
	argc--;
	if (argv[0][2]) usage(name);
	switch ((*argv++)[1]) {
	    case 'b':
		if (!argc) usage(name);
		boot_device = *argv++;
		argc--;
		break;
	    case 'c':
		compact = 1;
		break;
	    case 'd':
		if (!argc) usage(name);
		delay = to_number(*argv++);
		argc--;
		break;
	    case 'f':
		if (!argc) usage(name);
		disktab_file = *argv++;
		argc--;
		break;
	    case 'm':
		if (!argc) usage(name);
		map_file = *argv++;
		argc--;
		break;
	    case 'i':
		if (!argc) usage(name);
		install = *argv++;
		argc--;
		break;
	    case 'X':
		printf("-DIMAGES=%d -DCODE_START_1=%d -DCODE_START_2=%d "
		  "-DDESCR_SIZE=%d -DDSC_OFF=%d\n",MAX_IMAGES,
		  sizeof(BOOT_PARAMS_1),sizeof(BOOT_PARAMS_2),
		  sizeof(IMAGE_DESCR),(void *) &dummy.par_1.descr-(void *)
		  &dummy);
		exit(0);
	    case 'S':
		force_backup = 1;
	    case 's':
		if (!argc) usage(name);
		backup_file = *argv++;
		argc--;
		break;
	    case 'r':
		if (!argc) usage(name);
		if (chroot(*argv) < 0)
		    die("chroot %s: %s",*argv,strerror(errno));
		argv++;
		argc--;
		break;
	    case 't':
		test = 1;
		break;
	    case 'v':
		verbose++;
		break;
	    default:
		usage(name);
	}
    }
    if (verbose)
	printf("LILO version 0.5\nWritten 1992 by Werner Almesberger\n\n");
    if ((install || test || boot_device || disktab_file || compact) && !argc)
	usage(name);
    if (!argc) show_images(map_file);
    if (argc > MAX_IMAGES) {
	fprintf(stderr,"Only %d images can be defined\n",MAX_IMAGES);
	exit(1);
    }
    geo_init(disktab_file);
    bsect_open(boot_device,map_file,install,delay);
    for (image = 0; image < argc; image++) bsect_image(*argv++);
    if (!test) bsect_update(backup_file,force_backup);
    else {
	bsect_cancel();
	printf("The boot sector and the map file have *NOT* been altered.\n");
    }
    return 0;
}
