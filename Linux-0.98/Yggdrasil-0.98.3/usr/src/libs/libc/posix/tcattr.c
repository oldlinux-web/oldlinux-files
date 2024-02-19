#include <termios.h>
#include <errno.h>
#include <unistd.h>

int tcgetattr(int fildes, struct termios *termios_p)
{
	return ioctl(fildes, TCGETS, termios_p);
}

int tcsetattr(int fildes, int optional_actions, struct termios *termios_p)
{
	switch(optional_actions) {
		case TCSANOW:
			return ioctl(fildes, TCSETS, termios_p);
		case TCSADRAIN:
			return ioctl(fildes, TCSETSW, termios_p);
		case TCSAFLUSH:
			return ioctl(fildes, TCSETSF, termios_p);
		default:
			errno = EINVAL;
			return -1;
	}
}
