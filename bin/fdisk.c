#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include <linux/hdreg.h>

#define DISK_STRING "/dev/hd"

static int current_minor;
static int indent;

char * disk_type(unsigned char type)
{
	switch (type) {
		case 1: return "12-bit DOS";
		case 4:	return "16-bit DOS (<32M)";
		case 5: return "extended partition (don't use)";
		case 6: return "16-bit DOS (>=32M)";
		case 0x81: return "minix";
	}
	return NULL;
}

char * dev_name(int minor)
{
	char * ctl;
	static char name[100];

	if (minor & 0x3f)
		ctl = "%s%c%d";
	else
		ctl = "%s%c";
	sprintf(name,ctl,DISK_STRING,'a'+(minor >> 6),minor & 0x3f);
	return name;
}

void fdisk(int minor)
{
	char * type, * name;
	char buffer[1024];
	struct partition * p;
	int fd;
	int i;
	int this_minor = current_minor;

	if ((fd=open(name = dev_name(minor),O_RDONLY)) < 0) {
		fprintf(stderr,"Unable to open %s\n",name);
		exit(1);
	}
	if (1024 != read(fd,buffer,1024))
		return;
	if (!(minor & 0x3f)) {
		printf("Disk %d:\n", minor >> 6);
		indent = 4;
	}
	p = (struct partition *) (buffer + 0x1be);
	for (i=0 ; i<4 ; p++,i++) {
		if (!p->nr_sects)
			continue;
		printf("%*c",indent,' ');
		printf("%s: %6d blocks",dev_name(this_minor+i),p->nr_sects>>1);
		if (p->boot_ind == 0x80)
			printf(" active");
		else if (p->boot_ind)
			printf(" active? (%02x)",p->boot_ind);
		if (type = disk_type(p->sys_ind))
			printf(" %s\n",type);
		else
			printf(" unknown partition type 0x%02X\n",p->sys_ind);
		if (p->sys_ind == 5 && (0x3f & current_minor) < 60) {
			indent += 4;
			current_minor += 4;
			fdisk(this_minor+i);
			indent -= 4;
		}
	}
/* check for disk-manager partitions */
	if (*(unsigned short *) (buffer + 0xfc) != 0x55AA)
		return;
	p = (struct partition *) (buffer + 0x1be);
	for (i=4; i<16; i++) {
		p--;
		if ((current_minor & 0x3f) >= 60)
			break;
		if (!p->nr_sects)
			continue;
		printf("%*c",indent,' ');
		printf("%s: %6d blocks disk-manager",dev_name(current_minor),p->nr_sects>>1);
		if (p->boot_ind == 0x80)
			printf(" active");
		else if (p->boot_ind)
			printf(" active? (%02x)",p->boot_ind);
		if (type = disk_type(p->sys_ind))
			printf(" %s\n",type);
		else
			printf(" unknown partition type 0x%02X\n",p->sys_ind);
		current_minor++;
	}
}
		

int main(int argc, char ** argv)
{
	current_minor = 1;
	fdisk(0);
	current_minor = 65;
	fdisk(64);
	return 0;
}
