/* device.c  -  Device access */

/* Written 1992 by Werner Almesberger */


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>

#include "config.h"
#include "common.h"
#include "device.h"


int dev_open(DEVICE *dev,int number,int flags)
{
    DIR *dp;
    struct dirent *dir;
    char name[PATH_MAX+1];
    int count;

    if ((dp = opendir("/dev")) == NULL) pdie("opendir /dev");
    strcpy(name,"/dev/");
    while (dir = readdir(dp)) {
	strcpy(name+5,dir->d_name);
	if (stat(name,&dev->st) < 0) die("stat %s: %s",name,strerror(errno));
	if ((dev->st.st_mode & S_IFMT) == S_IFBLK && dev->st.st_rdev == number)
	    break;
    }
    if (dev->delete = !dir) {
	for (count = 0; count <= MAX_TMP_DEV; count++) {
	    sprintf(name,TMP_DEV,count);
	    if (stat(name,&dev->st) < 0) break;
	}
	if (count > MAX_TMP_DEV) die("Failed to create a temporary device");
	if (mknod(name,0600 | S_IFBLK,number) < 0)
	    die("mknod %s: %s",name,strerror(errno));
	if (fstat(dev->fd,&dev->st) < 0)
	    die("fstat %s: %s",name,strerror(errno));
	if (verbose > 1)
	    printf("Created temporary device %s (0x%4X)\n",name,number);
    }
    if ((dev->fd = open(name,flags)) < 0)
	die("open %s: %s",name,strerror(errno));
    dev->name = stralloc(name);
    return dev->fd;
}


void dev_close(DEVICE *dev)
{
    if (close(dev->fd) < 0) die("close %s: %s",dev->name,strerror(errno));
    if (dev->delete) (void) remove(dev->name);
    free(dev->name);
}
