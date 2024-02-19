#include <stdio.h>
#include <termios.h>
#include <fcntl.h>

#define	TTY	"/dev/tty"

/* Issue prompt and read reply with echo turned off */
char *getpass(const char * prompt)
{
	struct termios ttyb,ttysav;
	char *cp;
	int c;
	FILE *tty;
	static char pbuf[128];
	void (*sig)();

	if ((tty = fdopen(open(TTY, O_RDWR), "r")) == NULL)
		tty = stdin;
	else
		setbuf(tty, (char *)NULL);

	ioctl(fileno(tty), TCGETS, &ttyb);
	ioctl(fileno(tty), TCGETS, &ttysav);

	ttyb.c_lflag &= ~(ECHO|ISIG);
	ioctl(fileno(tty), TCSETS, &ttyb);

	fprintf(stderr, "%s", prompt); fflush(stderr);

	cp = pbuf;
	for (;;) {
		c = getc(tty);
		if(c == '\r' || c == '\n' || c == EOF)
			break;
		if (cp < &pbuf[127])
			*cp++ = c;
	}
	*cp = '\0';

	fprintf(stderr,"\r\n"); fflush(stderr);

	ioctl(fileno(tty), TCSETS, &ttysav);
	if (tty != stdin)
		fclose(tty);

	return(pbuf);
}
