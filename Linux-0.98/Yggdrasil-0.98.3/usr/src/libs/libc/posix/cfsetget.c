#include <termios.h>

speed_t cfgetospeed(struct termios *tp)
{
    return (tp->c_cflag & CBAUD);
}

speed_t cfgetispeed(struct termios *tp)
{
    return (tp->c_cflag & CBAUD);
}

int cfsetospeed(struct termios *tp, speed_t speed)
{
    if (speed < B0 || speed > B38400)
	return 0;
    tp->c_cflag &= ~CBAUD;
    tp->c_cflag |= (speed & CBAUD);

    return 0;
}

int cfsetispeed(struct termios *tp, speed_t speed)
{
    return cfsetospeed(tp, speed);
}
