/* The <termios.h> header is used for controlling tty modes. */

#ifndef _TERMIOS_H
#define _TERMIOS_H

typedef unsigned short tcflag_t;
typedef unsigned char cc_t;
typedef unsigned int speed_t;

#define NCCS		11	/* size of cc_c array */

/* Primary terminal control structure. POSIX Table 7-1. */
struct termios {
  tcflag_t c_iflag;		/* input modes */
  tcflag_t c_oflag;		/* output modes */
  tcflag_t c_cflag;		/* control modes */
  tcflag_t c_lflag;		/* local modes */
  speed_t  c_ispeed;		/* input speed */
  speed_t  c_ospeed;		/* output speed */
  cc_t c_cc[NCCS];		/* control characters */
};

/* Values for termios c_iflag bit map.  POSIX Table 7-2. */
#define BRKINT        000001	/* signal interrupt on break */
#define ICRNL         000002	/* map CR to NL on input */
#define IGNBRK        000004	/* ignore break */
#define IGNCR         000010	/* ignore CR */
#define IGNPAR        000020	/* ignore characters with parity errors */
#define INLCR         000100	/* map NL to CR on input */
#define INPCK         000200	/* enable input parity check */
#define ISTRIP        000400	/* mask off 8th bit */
#define IXOFF         001000	/* enable start/stop input control */
#define IXON          002000	/* enable start/stop output control */
#define PARMRK        004000	/* mark parity errors in the input queue */

/* Values for termios c_oflag bit map.  POSIX Sec. 7.1.2.3. */
#define OPOST         000001	/* perform output processing */

/* Values for termios c_cflag bit map.  POSIX Table 7-3. */
#define CLOCAL        000001	/* ignore modem status lines */
#define CREAD         000002	/* enable receiver */
#define CSIZE         000014	/* number of bits per character */
#define CSTOPB        000020	/* send 2 stop bits if set, else 1 */
#define HUPCL         000040	/* hang up on last close */
#define PARENB        000100	/* enable parity on output */
#define PARODD        000200	/* use odd parity if set, else even */

#define CS5           000000	/* if CSIZE is CS5, characters are 5 bits */
#define CS6           000004	/* if CSIZE is CS6, characters are 6 bits */
#define CS7           000010	/* if CSIZE is CS7, characters are 7 bits */
#define CS8           000014	/* if CSIZE is CS8, characters are 8 bits */

/* Values for termios c_lflag bit map.  POSIX Table 7-4. */
#define ECHO          000001	/* enable echoing of input characters */
#define ECHOE         000002	/* echo ERASE as backspace */
#define ECHOK         000004	/* echo KILL */
#define ECHONL        000010	/* echo NL */
#define ICANON        000020	/* canonical input (erase and kill enabled) */
#define IEXTEN        000040	/* enable extended functions */
#define ISIG          000100	/* enable signals */
#define NOFLSH        000200	/* disable flush after interrupt or quit */
#define TOSTOP        000400	/* send SIGTTOU (job control, not implemented*/

/* Indices into c_cc array.  Default values in parentheses. POSIX Table 7-5. */
#define VEOF               0	/* cc_c[VEOF] = EOF char (CTRL-D) */
#define VEOL               1	/* cc_c[VEOL] = EOL char (??) */
#define VERASE             2	/* cc_c[VERASE] = ERASE char (CTRL-H) */
#define VINTR              3	/* cc_c[VINTR] = INTR char (DEL) */
#define VKILL              4	/* cc_c[VKILL] = KILL char (@) */
#define VMIN               5	/* cc_c[VMIN] = MIN value for timer */
#define VQUIT              6	/* cc_c[VQUIT] = QUIT char (CTRL-\) */
#define VTIME              7	/* cc_c[VTIME] = TIME value for timer */
#define VSUSP              8	/* cc_c[VSUSP] = SUSP (job control, not impl */
#define VSTART             9	/* cc_c[VSTART] = START char (always CTRL-S) */
#define VSTOP             10	/* cc_c[VSTOP] = STOP char (always CTRL-Q) */

/* Values for the baud rate settings.  POSIX Table 7-6. */
#define B0           0000000	/* hang up the line */
#define B50          0010000	/* 50 baud */
#define B75          0020000	/* 75 baud */
#define B110         0030000	/* 110 baud */
#define B134         0040000	/* 134.5 baud */
#define B150         0050000	/* 150 baud */
#define B200         0060000	/* 200 baud */
#define B300         0070000	/* 300 baud */
#define B600         0100000	/* 600 baud */
#define B1200        0110000	/* 1200 baud */
#define B1800        0120000	/* 1800 baud */
#define B2400        0130000	/* 2400 baud */
#define B4800        0140000	/* 4800 baud */
#define B9600        0150000	/* 9600 baud */
#define B19200       0160000	/* 19200 baud */
#define B38400       0170000	/* 38400 baud */

/* Optional actions for tcsetattr().  POSIX Sec. 7.2.1.2. */
#define TCSANOW            1	/* changes take effect immediately */
#define TCSADRAIN          2	/* changes take effect after output is done */
#define TCSAFLUSH          3	/* wait for output to finish and flush input */

/* Queue_selector values for tcflush().  POSIX Sec. 7.2.2.2. */
#define TCIFLUSH           1	/* flush accumulated input data */
#define TCOFLUSH           2	/* flush accumulated output data */
#define TCIOFLUSH          3	/* flush accumulated input and output data */

/* Action values for tcflow().  POSIX Sec. 7.2.2.2. */
#define TCOOFF             1	/* suspend output */
#define TCOON              2	/* restart suspended output */
#define TCIOFF             3	/* transmit a STOP character on the line */
#define TCION              4	/* transmit a START character on the line */


/* Function Prototypes. */
#ifndef _ANSI_H
#include <ansi.h>
#endif

_PROTOTYPE( int tcsendbreak, (int _fildes, int _duration)		     );
_PROTOTYPE( int tcdrain, (int _filedes)				     );
_PROTOTYPE( int tcflush, (int _filedes, int _queue_selector)		     );
_PROTOTYPE( int tcflow, (int _filedes, int _action)			     );
_PROTOTYPE( speed_t cfgetospeed, (struct termios *_termios_p) 		     );
_PROTOTYPE( speed_t cfsetospeed, \
		        (struct termios *_termios_p, speed_t _speed)       );
_PROTOTYPE( speed_t cfgetispeed, (struct termios *_termios_p) 		     );
_PROTOTYPE( speed_t cfsetispeed, \
			(struct termios *_termios_p, speed_t _speed)       );
_PROTOTYPE( int tcgetattr, (int _filedes, struct termios *_termios_p)      );
_PROTOTYPE( int tcsetattr, \
	(int _filedes, int _opt_actions, struct termios *_termios_p)      );

#endif /* _TERMIOS_H */
