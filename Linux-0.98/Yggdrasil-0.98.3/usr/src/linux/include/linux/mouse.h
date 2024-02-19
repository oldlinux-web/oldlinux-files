#ifndef _LINUX_MOUSE_H
#define _LINUX_MOUSE_H

/*
 * These are the minor numbers for the mice - undefine them
 * to get rid of the driver from the kernel binary..
 */

#define BUSMOUSE_MINOR 0
#define PSMOUSE_MINOR  1
#define MS_BUSMOUSE_MINOR 2
#define ATIXL_BUSMOUSE_MINOR 3

unsigned long mouse_init(unsigned long);

#endif
