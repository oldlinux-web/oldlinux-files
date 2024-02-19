/* rdev.c  -  query/set root device. */

/* usage: rdev [ boot-image ] [ root-device ] [ offset ] */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>


#define DEFAULT_OFFSET 508


static void die(char *msg)
{
    perror(msg);
    exit(1);
}


static char *find_dev(int number)
{
    DIR *dp;
    struct dirent *dir;
    static char name[PATH_MAX+1];
    struct stat s;

    if (!number) return "Boot device";
    if ((dp = opendir("/dev")) == NULL) die("opendir /dev");
    strcpy(name,"/dev/");
    while (dir = readdir(dp)) {
	strcpy(name+5,dir->d_name);
	if (stat(name,&s) < 0) die(name);
	if ((s.st_mode & S_IFMT) == S_IFBLK && s.st_rdev == number) return name;
    }
    sprintf(name,"0x%04x",number);
    return name;
}


main(int argc,char **argv)
{
    int image,offset,dev_nr;
    char *device;
    struct stat s;

    offset = DEFAULT_OFFSET;
    device = NULL;
    if (argc == 1 || argc > 4) {
	if (stat("/",&s) < 0) die("/");
	printf("%s /\n",find_dev(s.st_dev));
	exit(0);
    }
    if (argc == 4) {
	device = argv[2];
	offset = atoi(argv[3]);
    }
    else {
	if (argc == 3) {
	    if (isdigit(*argv[2])) offset = atoi(argv[2]);
	    else device = argv[2];
	}
    }
    if (device) {
	if (stat(device,&s) < 0) die(device);
	if ((image = open(argv[1],O_WRONLY)) < 0) die(argv[1]);
	if (lseek(image,offset,0) < 0) die("lseek");
	if (write(image,&s.st_rdev,2) != 2) die(argv[1]);
	if (close(image) < 0) die("close");
    }
    else {
	if ((image = open(argv[1],O_RDONLY)) < 0) die(argv[1]);
	if (lseek(image,offset,0) < 0) die("lseek");
	dev_nr = 0;
	if (read(image,&dev_nr,2) != 2) die(argv[1]);
	if (close(image) < 0) die("close");
	printf("Root device: %s\n",find_dev(dev_nr));
    }
    return 0;
}