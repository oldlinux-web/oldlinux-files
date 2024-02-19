From owner-linux-activists@joker.cs.hut.fi Tue Jul 14 05:42:55 1992
Received: from funet.fi by lazy.qt.IPA.FhG.de with SMTP
	(5.61+/IDA-1.2.8/gandalf.2) id AA09550; Tue, 14 Jul 92 05:42:52 +0200
Received: from santra.hut.fi by funet.fi with SMTP (PP) id <24104-0@funet.fi>;
          Tue, 14 Jul 1992 06:41:04 +0300
Received: from joker.cs.hut.fi by santra.hut.fi (5.65c/8.0/TeKoLa) id AA24830;
          Tue, 14 Jul 1992 06:39:57 +0300
Received: by joker.cs.hut.fi (5.65b/6.8/S-TeKoLa) id AA08793;
          Tue, 14 Jul 92 06:39:34 +0300
Received: from relay2.UU.NET by joker.cs.hut.fi (5.65b/6.8/S-TeKoLa) id AA08245;
          Tue, 14 Jul 92 06:33:44 +0300
Received: from world.std.com by relay2.UU.NET 
          with SMTP (5.61/UUNET-internet-primary) id AA00942;
          Mon, 13 Jul 92 23:34:03 -0400
Received: by world.std.com (5.61+++/Spike-2.0) id AA12321;
          Mon, 13 Jul 92 23:33:37 -0400
Date: Mon, 13 Jul 92 23:33:37 -0400
From: jrs@world.std.com (Rick Sladkey)
Message-Id: <9207140333.AA12321@world.std.com>
Sender: owner-linux-activists@niksula.hut.fi
To: linux-activists@niksula.hut.fi
X-Note1: Remember to put 'X-Mn-Key: normal' to your mail body or header
Subject: setserial.c
X-Mn-Key: NORMAL

Here is a program that uses the recent serial ioctl calls to change
the port or IRQ of a serial device.  I use it to change /dev/ttys3 to
IRQ 5.  You must be root to run it or make it setuid to root.  Please
be careful when specifying port numbers!

If you get the message "Device busy" it means that you have another
serial port currently using that IRQ or that you have specified an IRQ
that is used by another hardware device such as the keyboard or a hard
disk controller.

IRQs 2, 3, 4, and 5 are typically available and can often be set by
DIP switches or jumpers on your serial card.  With 0.96c patchlevel 1
or higher you can use any IRQ that your card lets you configure and
that is unused by Linux.  If you are not sure, go ahead and try.
Linux will let you know.

Some examples:

# setserial
usage: setserial serial-device [ port irq ]
for example: setserial /dev/ttys3 0x03e8 5
Use a leading '0x' for hex numbers.
CAUTION: Using an invalid port can lock up your machine!
# setserial /dev/ttys3
/dev/ttys3, Type: 16450, Line: 2, Port: 0x03e8, IRQ: 4
# setserial /dev/ttys3 0 5
/dev/ttys3, Type: 16450, Line: 2, Port: 0x03e8 (was 0x03e8), IRQ: 5 (was 4)
#

There is a minor bug that prevents this from working with 0.96c
patchlevel 1.  Use unpatched 0.96c, wait for 0.96c patchlevel 2, or
apply this patch.

*** linux/kernel/chr_drv/serial.c.orig	Sun Jul 12 23:48:13 1992
--- linux/kernel/chr_drv/serial.c	Mon Jul 13 19:28:37 1992
***************
*** 416,421 ****
--- 416,422 ----
  		retval = request_irq(new_irq,handler);
  		if (retval)
  			return retval;
+ 		info->irq = new_irq;
  		free_irq(irq);
  	}
  	cli();

I have uploaded setserial.tar.Z to banjo and tsx-11.

Rick Sladkey
jrs@world.std.com
-----
/* setserial.c - get/set Linux serial port info - rick sladkey */

/* compile with something like:

	CC = gcc
	CFLAGS = -nostdinc -I/usr/src/linux/include -I/usr/include

	setserial: setserial.c
		$(CC) $(CFLAGS) setserial.c -o setserial

*/

#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

#include <linux/fs.h>
#include <linux/tty.h>

char *progname;

struct serial_type_struct {
	int id;
	char *name;
} serial_type_tbl[] = {
	PORT_UNKNOWN,	"unknown",
	PORT_8250,	"8250",
	PORT_16450,	"16450",
	PORT_16550,	"16550",
	PORT_16550A,	"16550A",
	-1,		NULL
};
	
char *serial_type(int id)
{
	int i;

	for (i = 0; serial_type_tbl[i].id != -1; i++)
		if (id == serial_type_tbl[i].id)
			return serial_type_tbl[i].name;
	return "undefined";
}

int atonum(char *s)
{
	int n;

	while (*s == ' ')
		s++;
	if (strncmp(s, "0x", 2) == 0 || strncmp(s, "0X", 2) == 0)
		sscanf(s + 2, "%x", &n);
	else if (s[0] == '0' && s[1])
		sscanf(s + 1, "%o", &n);
	else
		sscanf(s, "%d", &n);
	return n;
}

void getserial(char *device, int fd)
{
	struct serial_struct serinfo;

	if (ioctl(fd, TIOCGSERIAL, &serinfo) < 0) {
		perror("Cannot get serial info");
		exit(1);
	}
	printf("%s, Type: %s, Line: %d, Port: 0x%.4x, IRQ: %d\n",
		device, serial_type(serinfo.type),
		serinfo.line, serinfo.port, serinfo.irq);
}

void setserial(char *device, int fd, int port, int irq)
{
	struct serial_struct old_serinfo, new_serinfo;

	if (ioctl(fd, TIOCGSERIAL, &old_serinfo) < 0) {
		perror("Cannot get serial info");
		exit(1);
	}
	new_serinfo = old_serinfo;
	new_serinfo.port = port;
	new_serinfo.irq = irq;
	if (ioctl(fd, TIOCSSERIAL, &new_serinfo) < 0) {
		perror("Cannot set serial info");
		exit(1);
	}
	if (ioctl(fd, TIOCGSERIAL, &new_serinfo) < 0) {
		perror("Cannot get serial info");
		exit(1);
	}
	printf("%s, Type: %s, Line: %d, "
		"Port: 0x%.4x (was 0x%.4x), IRQ: %d (was %d)\n",
		device, serial_type(new_serinfo.type),
		new_serinfo.line, new_serinfo.port, old_serinfo.port,
		new_serinfo.irq, old_serinfo.irq);
}

main(int argc, char **argv)
{
	char *device;
	int fd;

	progname = argv[0];
	if (argc == 1) {
		fprintf(stderr, "usage: %s serial-device [ port irq ]\n",
			progname);
		fprintf(stderr, "for example: %s /dev/ttys3 0x03e8 5\n",
			progname);
		fprintf(stderr, "Use a leading '0x' for hex numbers.\n");
		fprintf(stderr, "CAUTION: "
			"Using an invalid port can lock up your machine!\n");
		exit(1);
	}
	device = argv[1];
	if ((fd = open(device, O_RDWR)) < 0) {
		perror("Cannot open serial device");
		exit(1);
	}
	if (argc == 2)
		getserial(device, fd);
	else if (argc == 4)
		setserial(device, fd, atonum(argv[2]), atonum(argv[3]));
	else {
		fprintf(stderr, "%s: wrong number of arguments\n", progname);
		exit(1);
	}
	exit(0);
}


