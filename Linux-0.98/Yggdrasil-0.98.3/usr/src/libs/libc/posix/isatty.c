#include <unistd.h>
#include <termios.h>

/*
 * Simple isatty for Linux.
 */

int isatty(int fd)
{
	struct termios tmp;

	return ((ioctl(fd,TCGETS,&tmp)<0) ? 0 : 1);
}
