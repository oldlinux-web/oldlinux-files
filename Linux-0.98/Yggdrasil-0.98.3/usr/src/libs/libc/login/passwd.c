/* get password from user without echo */

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <termio.h>

static char passwd[10];

char *getpass(const char * prompt)
{
	struct termio	ti;
	char		ch;
	int		cnt = 8;
	char *		buf = passwd;
	int		fd = 0;
	
	fd = open("/dev/tty", O_RDWR);
	if(fd < 0) return (char *)0;
			
	(void)ioctl(fd, TCGETA, &ti);
	ti.c_lflag &= ~ECHO;
	
	write(fd, prompt, strlen(prompt));
	ioctl(fd, TCFLSH, 2);

	/* echo off */
	ioctl(fd, TCSETA, &ti);

	/* read the password from the tty */
	while(--cnt >= 0 && read(fd, &ch, 1) == 1 && ch != '\n')
		*buf++ = ch;

	*buf = '\0';	

	/* suck up extra chars, so they don't go to the shell */
	while(ch != '\n' && read(fd, &ch, 1) == 1) /* nothing */;
	
	ti.c_lflag |= ECHO;
	ioctl(fd, TCSETA, &ti);

	write(fd, "\n", 1);

	/* not really needed, but.... */
	(void) ioctl(fd, TCFLSH, 2);	/* flush input and output buffers */

	if(fd > 0) close(fd);
		
	return passwd;
}
