#ifndef _TERMIOS_H
#define _TERMIOS_H

#include <traditional.h>
#include <sys/types.h>
#include <linux/termios.h>

#ifdef __cplusplus
extern "C" {
#endif

extern speed_t cfgetispeed _ARGS ((struct termios *__termios_p));
extern speed_t cfgetospeed _ARGS ((struct termios *__termios_p));
extern int cfsetispeed _ARGS ((struct termios *__termios_p,
		speed_t __speed));
extern int cfsetospeed _ARGS ((struct termios *__termios_p,
		speed_t __speed));
extern int tcdrain _ARGS ((int __fildes));
extern int tcflow _ARGS ((int __fildes, int __action));
extern int tcflush _ARGS ((int __fildes, int __queue_selector));
extern pid_t tcgetpgrp _ARGS ((int __fildes));
extern int tcgetattr _ARGS ((int __fildes, struct termios *__termios_p));
extern int tcsendbreak _ARGS ((int __fildes, int __duration));
extern int tcsetattr _ARGS ((int __fildes, int __optional_actions,
	struct termios *__termios_p));
extern int tcsetpgrp _ARGS ((int __fildes, pid_t __pgrp_id));

#ifdef __cplusplus
}
#endif

#endif
