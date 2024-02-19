/*
 * Device tables.  See the Configure file for a complete description.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "msdos.h"

#ifdef DELL
struct device devices[] = {
	{'A', "/dev/rdsk/f0d9dt", 0L, 12, 0, (int (*) ()) 0, 40, 2, 9},
	{'A', "/dev/rdsk/f0q15dt", 0L, 12, 0, (int (*) ()) 0, 80, 2, 15},
	{'A', "/dev/rdsk/f0d8dt", 0L, 12, 0, (int (*) ()) 0, 40, 2, 8},
	{'B', "/dev/rdsk/f13ht", 0L, 12, 0, (int (*) ()) 0, 80, 2, 18},
	{'B', "/dev/rdsk/f13dt", 0L, 12, 0, (int (*) ()) 0, 80, 2, 9},
	{'C', "/dev/rdsk/dos", 0L, 16, 0, (int (*) ()) 0, 0, 0, 0},
	{'\0', (char *) NULL, 0L, 0, 0, (int (*) ()) 0, 0, 0, 0}
};
#endif /* DELL */

#ifdef ISC
struct device devices[] = {
	{'A', "/dev/rdsk/f0d9dt", 0L, 12, 0, (int (*) ()) 0, 40, 2, 9},
	{'A', "/dev/rdsk/f0q15dt", 0L, 12, 0, (int (*) ()) 0, 80, 2, 15},
	{'A', "/dev/rdsk/f0d8dt", 0L, 12, 0, (int (*) ()) 0, 40, 2, 8},
	{'B', "/dev/rdsk/f13ht", 0L, 12, 0, (int (*) ()) 0, 80, 2, 18},
	{'B', "/dev/rdsk/f13dt", 0L, 12, 0, (int (*) ()) 0, 80, 2, 9},
	{'C', "/dev/rdsk/0p1", 0L, 16, 0, (int (*) ()) 0, 0, 0, 0},
	{'D', "/usr/vpix/defaults/C:", 8704L, 12, 0, (int (*) ()) 0, 0, 0, 0},
	{'E', "$HOME/vpix/C:", 8704L, 12, 0, (int (*) ()) 0, 0, 0, 0},
	{'\0', (char *) NULL, 0L, 0, 0, (int (*) ()) 0, 0, 0, 0}
};
#endif /* ISC */

#ifdef MASSCOMP
struct device devices[] = {
	{'A', "/dev/rflp", 0L, 12, 0, (int (*) ()) 0, 80, 2, 8},
	{'\0', (char *) NULL, 0L, 0, 0, (int (*) ()) 0, 0, 0, 0}
};
#endif /* MASSCOMP */

#ifdef SPARC
struct device devices[] = {
	{'A', "/dev/rfd0c", 0L, 12, 0, (int (*) ()) 0, 80, 2, 18},
	{'A', "/dev/rfd0c", 0L, 12, 0, (int (*) ()) 0, 80, 2, 9},
	{'\0', (char *) NULL, 0L, 0, 0, (int (*) ()) 0, 0, 0, 0}
};
#endif /* SPARC */

#ifdef UNIXPC
#include <sys/gdioctl.h>
#include <fcntl.h>

int init_unixpc();

struct device devices[] = {
	{'A', "/dev/rfp020", 0L, 12, O_NDELAY, init_unixpc, 40, 2, 9},
	{'C', "/usr/bin/DOS/dvd000", 0L, 12, 0, (int (*) ()) 0, 0, 0, 0},
	{'\0', (char *) NULL, 0L, 0, 0, (int (*) ()) 0, 0, 0, 0}
};

int
init_unixpc(fd, ntracks, nheads, nsect)
int fd, ntracks, nheads, nsect;
{
	struct gdctl gdbuf;

	if (ioctl(fd, GDGETA, &gdbuf) == -1) {
		ioctl(fd, GDDISMNT, &gdbuf);
		return(1);
	}

	gdbuf.params.cyls = ntracks * nheads;
	gdbuf.params.heads = nheads;
	gdbuf.params.psectrk = nsect;

	gdbuf.params.pseccyl = gdbuf.params.psectrk * gdbuf.params.heads;
	gdbuf.params.flags = 1;		/* disk type flag */
	gdbuf.params.step = 0;		/* step rate for controller */
	gdbuf.params.sectorsz = 512;	/* sector size */

	if (ioctl(fd, GDSETA, &gdbuf) < 0) {
		ioctl(fd, GDDISMNT, &gdbuf);
		return(1);
	}
	return(0);
}
#endif /* UNIXPC */

#ifdef MERGED

#define MAX_LINE    255+40
#define MAX_DEVICES 10

#define SET(f,t,h,s) devices[dev].fat_bits = f; devices[dev].tracks = t; \
  devices[dev].heads = h; devices[dev].sectors = s;


struct device devices[MAX_DEVICES+1];


void load_devices()
{
    FILE *cfg;
    char buffer[MAX_LINE+1],name[255+1];
    char *here,*start;
    int items,dev,fat_bits;

    if ((cfg = fopen(CFG_FILE,"r")) == NULL) {
	perror(CFG_FILE);
	exit(1);
    }
    dev = 0;
    while (fgets(buffer,MAX_LINE,cfg)) {
	if (here = strchr(buffer,'#')) *here = 0;
	else if (here = strchr(buffer,'\n')) *here = 0;
	for (start = buffer; *start == ' ' || *start == '\t'; start++);
	if (!*start) continue;
	items = sscanf(start,"%c %255s %d %d %d %d %d",
	  &devices[dev].drive,name,&fat_bits,&devices[dev].tracks,
	  &devices[dev].heads,&devices[dev].sectors,&devices[dev].offset);
	if (dev >= MAX_DEVICES) {
	    fprintf(stderr,"Too many devices\n");
	    exit(1);
	}
	if (items == 6 || items == 7) devices[dev].fat_bits = fat_bits;
	else {
	    if (items == 2 || items == 3) {
#ifdef LINUX
#ifdef oldLINUX
		     if (!strncmp("/dev/at",name,7)) { SET(12,80,2,15) }
		else if (!strncmp("/dev/xt",name,7)) { SET(12,40,2,9) }
		else if (!strncmp("/dev/PS",name,7)) { SET(12,80,2,18) }
		else if (!strncmp("/dev/ps",name,7)) { SET(12,80,2,9) }
		else
#endif
		     if (!strncmp("/dev/hd",name,7)) { SET(16,0,0,0) }
		else if (!strncmp("/dev/sd",name,7)) { SET(16,0,0,0) }
		else
#endif
		{
		    fprintf(stderr,"Unknown device %s, please specify all \
parameters\n",name);
		    exit(1);
		}
		if (items == 3) devices[dev].fat_bits = fat_bits;
	    }
	    else {
		fprintf(stderr,"Config file syntax: drive device [ fat [ \
tracks heads sectors [ offset ] ] ]\n");
		exit(1);
	    }
	}
	if (islower(devices[dev].drive))
	    devices[dev].drive = toupper(devices[dev].drive);
	if (items < 7) devices[dev].offset = 0L;
	devices[dev].mode = 0;
	devices[dev].gioctl = NULL;
	if ((devices[dev].name = (char *) malloc(strlen(name)+1)) == NULL) {
	    fprintf(stderr,"Out of memory\n");
	    exit(1);
	}
	strcpy(devices[dev++].name,name);
    }
    memset(&devices[dev],0,sizeof(struct device));
    fclose(cfg);
}

#endif
