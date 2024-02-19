/* device.h  -  Device access */

/* Written 1992 by Werner Almesberger */


#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <sys/stat.h>


typedef struct {
    int fd;
    struct stat st;
    char *name;
    int delete;
} DEVICE;


int dev_open(DEVICE *dev,int number,int flags);

/* Searches /dev for a block device with the specified number. If no device
   can be found, a temporary device is created. The device is opened with
   the specified access mode and the file descriptor is returned. */

void dev_close(DEVICE *dev);

/* Closes a device that has previously been opened by dev_open. If the device
   had to be created, it is removed now. */

#endif
