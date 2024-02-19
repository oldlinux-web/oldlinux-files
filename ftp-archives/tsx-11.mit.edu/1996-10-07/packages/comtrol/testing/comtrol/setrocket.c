/* setrocket.c --- get/set Rocket flags */

#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MKDEV(a,b) ((int)((((a) & 0xff) << 8) | ((b) & 0xff)))

#include "rocket.h"

#define VERSION_STR "1.2"

char *progname;

int	verbosity = 1;		/* 1 = normal, 0=boot-time, 2=everything */
int	verbose_flag = 0;	/* print results after setting a port */
int	quiet_flag = 0;
int	old_device_name = 0;

#define CMD_FLAG	1
#define CMD_DELAY	2
#define CMD_WAIT	3

#define FLAG_CAN_INVERT	0x0001
#define FLAG_NEED_ARG	0x0002

struct flag_type_table {
	int	cmd;
	char	*name;
	int	bits;
	int	mask;
	int	level;
	int	flags;
} flag_type_tbl[] = {
	CMD_FLAG,	"spd_normal",	0,		ROCKET_SPD_MASK,	0, 0,
	CMD_FLAG,	"spd_hi",	ROCKET_SPD_HI, 	ROCKET_SPD_MASK, 0, 0,
	CMD_FLAG,	"spd_vhi",	ROCKET_SPD_VHI,	ROCKET_SPD_MASK,	0, 0,
	CMD_FLAG,	"spd_shi",	ROCKET_SPD_SHI,	ROCKET_SPD_MASK,	0, 0,
	CMD_FLAG,	"spd_warp",	ROCKET_SPD_WARP, ROCKET_SPD_MASK,	0, 0,
	
	CMD_FLAG, 	"SAK", 		ROCKET_SAK, 	ROCKET_SAK, 	0, FLAG_CAN_INVERT,
	CMD_FLAG,	"hup_notify",	ROCKET_HUP_NOTIFY, ROCKET_HUP_NOTIFY, 0, FLAG_CAN_INVERT,
	CMD_FLAG,	"split_termios", ROCKET_SPLIT_TERMIOS, ROCKET_SPLIT_TERMIOS, 2, FLAG_CAN_INVERT,
	CMD_FLAG,	"session_lockout", ROCKET_SESSION_LOCKOUT, ROCKET_SESSION_LOCKOUT, 2, FLAG_CAN_INVERT,
	CMD_FLAG,	"pgrp_lockout", ROCKET_PGRP_LOCKOUT, ROCKET_PGRP_LOCKOUT, 2, FLAG_CAN_INVERT,
	CMD_FLAG,	"callout_nohup", ROCKET_CALLOUT_NOHUP, ROCKET_CALLOUT_NOHUP, 2, FLAG_CAN_INVERT,

	CMD_DELAY,	"close_delay",	0,		0,		0, FLAG_NEED_ARG,
	CMD_WAIT,	"closing_wait",	0,		0,		0, FLAG_NEED_ARG,
	0,		0,		0,		0,		0, 0,
};
	
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

void print_flags(struct rocket_config *serinfo,
		 char *prefix, char *postfix)
{
	struct	flag_type_table	*p;
	int	flags;
	int	first = 1;

	flags = serinfo->flags;
	
	for (p = flag_type_tbl; p->name; p++) {
		if (p->cmd != CMD_FLAG)
			continue;
		if (verbosity < p->level)
			continue;
		if ((flags & p->mask) == p->bits) {
			if (first) {
				printf("%s", prefix);
				first = 0;
			} else
				printf(" ");
			printf("%s", p->name);
		}
	}
	
	if (!first)
		printf("%s", postfix);
}

void get_rocket(char *device)
{
	struct rocket_config config;
	int	fd;
	char	buf1[40];

	if ((fd = open(device, O_RDWR|O_NONBLOCK)) < 0) {
		perror(device);
		return;
	}
	if (ioctl(fd, RCKP_GET_CONFIG, &config) < 0) {
		perror("Cannot get rocket config structure");
		close(fd);
		return;
	}
	if (verbosity==2) {
		printf("%s, Line %d, Base port: 0x%.4x\n",
		       device, config.line, config.port);
		if (config.closing_wait == ROCKET_CLOSING_WAIT_INF)
			strcpy(buf1, "infinte");
		else if (config.closing_wait == ROCKET_CLOSING_WAIT_NONE)
			strcpy(buf1, "none");
		else
			sprintf(buf1, "%d", config.closing_wait);
		printf("\tclose_delay: %d, closing_wait: %s\n",
		       config.close_delay, buf1);
		print_flags(&config, "\tFlags: ", "");
		printf("\n\n");
	} else {
		printf("%s, ", device);
		print_flags(&config, "Flags: ", "");
		printf("\n");
	}
	close(fd);
}

void set_rocket(char *device, char ** arg)
{
	struct rocket_config old_config, new_config;
	struct	flag_type_table	*p;
	int	fd;
	int	do_invert = 0;
	char	*word;
	

	if ((fd = open(device, O_RDWR|O_NONBLOCK)) < 0) {
		if (verbosity==0 && errno==ENOENT)
			exit(201);
		perror(device);
		exit(201);
	}
	if (ioctl(fd, RCKP_GET_CONFIG, &old_config) < 0) {
		perror("Cannot get rocket info");
		exit(1);
	}
	new_config = old_config;
	while (*arg) {
		do_invert = 0;
		word = *arg++;
		if (*word == '^') {
			do_invert++;
			word++;
		}
		for (p = flag_type_tbl; p->name; p++) {
			if (!strcasecmp(p->name, word))
				break;
		}
		if (!p->name) {
			fprintf(stderr, "Invalid flag: %s\n", word);
			exit(1);
		}
		if (do_invert && !(p->flags & FLAG_CAN_INVERT)) {
			fprintf(stderr, "This flag can not be inverted: %s\n", word);
			exit(1);
		}
		if ((p->flags & FLAG_NEED_ARG) && !*arg) {
			fprintf(stderr, "Missing argument for %s\n", word);
			exit(1);
		}
		switch (p->cmd) {
		case CMD_FLAG:
			new_config.flags &= ~p->mask;
			if (!do_invert)
				new_config.flags |= p->bits;
			break;
		case CMD_DELAY:
			new_config.close_delay = atonum(*arg++);
			break;
		case CMD_WAIT:
			if (!strcasecmp(*arg, "infinite"))
				new_config.closing_wait = ROCKET_CLOSING_WAIT_INF;
			else if (!strcasecmp(*arg, "none"))
				new_config.closing_wait = ROCKET_CLOSING_WAIT_NONE;
			else
				new_config.closing_wait = atonum(*arg);
			arg++;
			break;
		default:
			fprintf(stderr, "Internal error: unhandled cmd #%d\n", p->cmd);
			exit(1);
		}
	}
	if (ioctl(fd, RCKP_SET_CONFIG, &new_config) < 0) {
		perror("Cannot set rocket info");
		exit(1);
	}
	close(fd);
	if (verbose_flag)
		get_rocket(device);
}

void mkdev_rocket(char *device)
{
	struct rocket_ports ports;
	int	fd;
	gid_t	tty_group = 0;
	struct group *grp;
	int	board, port, line;
	char	pathname[64];
	dev_t	mode;

	grp = getgrnam("tty");
	if (grp)
		tty_group = grp->gr_gid;

	if ((fd = open(device, O_RDWR|O_NONBLOCK)) < 0) {
		perror(device);
		return;
	}
	if (ioctl(fd, RCKP_GET_PORTS, &ports) < 0) {
		perror("Cannot get rocket ports structure");
		close(fd);
		return;
	}
	close(fd);
	mode = S_IFCHR | 0666;
	if (verbose_flag)
		printf("Creating devices: ");
	fflush(stdout);
	for (line = 0; line < 128; line++) {
		board = line >> 5;
		port = line & 0x1f;
		if (ports.port_bitmap[board] & (1 << port)) {
			if (verbose_flag) {
				if (line && (line % 8) == 0)
					printf("\n\t");
				if (line < 8)
					printf("ttyR%d ", line);
				else
					printf("ttyR%-3d ", line);
				fflush(stdout);
			}
			sprintf(pathname, "/dev/ttyR%d", line);
			unlink(pathname);
			mknod(pathname, mode,
			      MKDEV(ports.tty_major, line));
			chmod(pathname, 0620);
			chown(pathname, 0, tty_group);
			if (old_device_name) {
				sprintf(pathname, "/dev/cur%d", line);
				unlink(pathname);
				sprintf(pathname, "/dev/cuR%d", line);
			} else {
				sprintf(pathname, "/dev/cuR%d", line);
				unlink(pathname);
				sprintf(pathname, "/dev/cur%d", line);
			}
			unlink(pathname);
			mknod(pathname, mode,
			      MKDEV(ports.callout_major, line));
			chmod(pathname, 0666);
			chown(pathname, 0, tty_group);
		}
	}
	if (verbose_flag)
		printf("\n");
}

void usage()
{
	fprintf(stderr, "setrocket Version %s\n\n", VERSION_STR);
	fprintf(stderr,
		"usage: %s rocket-device [cmd1 [arg]] ... \n\n", progname);
	fprintf(stderr, "Available commands: (* = Takes an argument)\n");
	fprintf(stderr, "\t\t(^ = can be preceded by a '^' to turn off the option)\n");
	fprintf(stderr, "\t* close_delay\tset the amount of time (in 1/100 of a\n");
	fprintf(stderr, "\t\t\t\tsecond) that DTR should be kept low\n");
	fprintf(stderr, "\t\t\t\twhile being closed\n");
	fprintf(stderr, "\t* closing_wait\tset the amount of time (in 1/100 of a\n");
	fprintf(stderr, "\t\t\t\tsecond) that the rocket port should wait for\n");
	fprintf(stderr, "\t\t\t\tdata to be drained while being closed\n");
	fprintf(stderr, "\n");
#if 0
	fprintf(stderr, "\t^ sak\t\tset the break key as the Secure Attention Key\n");
#endif
	fprintf(stderr, "\t^ session_lockout Lock out callout port across different sessions\n");
	fprintf(stderr, "\t^ pgrp_lockout\tLock out callout port across different process groups\n");
	fprintf(stderr, "\t^ callout_nohup\tDon't hangup the tty when carrier detect drops\n");
	fprintf(stderr, "\t\t\t\t on the callout device\n");
	fprintf(stderr, "\t^ split_termios Use separate termios for callout and dailin lines\n");
	fprintf(stderr, "\t^ hup_notify\tNotify a process blocked on opening a dial in line\n");
	fprintf(stderr, "\t\t\t\twhen a process has finished using a callout\n");
	fprintf(stderr, "\t\t\t\tline by returning EAGAIN to the open.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\t  spd_hi\tuse 56kb instead of 38.4kb\n");
	fprintf(stderr, "\t  spd_vhi\tuse 115kb instead of 38.4kb\n");
	fprintf(stderr, "\t  spd_shi\tuse 230kb instead of 38.4kb\n");
	fprintf(stderr, "\t  spd_warp\tuse 460kb instead of 38.4kb\n");
	fprintf(stderr, "\t  spd_normal\tuse 38.4kb when a buad rate of 38.4kb is selected\n");
	fprintf(stderr, "\n");
	exit(1);
}

main(int argc, char **argv)
{
	int	get_flag = 0;
	int	mkdev_flag = 0;
	int	c;
	extern int optind;
	extern char *optarg;
	
	progname = argv[0];
	if (argc == 1)
		usage();
	while ((c = getopt(argc, argv, "abgoqvMVW")) != EOF) {
		switch (c) {
		case 'a':
			verbosity = 2;
			break;
		case 'b':
			verbosity = 0;
			break;
		case 'q':
			quiet_flag++;
			break;
		case 'v':
			verbose_flag++;
			break;
		case 'g':
			get_flag++;
			mkdev_flag = 0;
			break;
		case 'o':
			old_device_name++;
			break;
		case 'M':
			mkdev_flag++;
			get_flag = 0;
			break;
		case 'V':
			fprintf(stderr, "setrocket version %s\n", VERSION_STR);
			exit(0);
		default:
			usage();
		}
	}
	if (get_flag) {
		argv += optind;
		while (*argv)
			get_rocket(*argv++);
		exit(0);
	}
	if (argc == optind)
		usage();
	if (mkdev_flag) {
		mkdev_rocket(argv[optind]);
		exit(0);
	}
	if (argc-optind == 1)
		get_rocket(argv[optind]);
	else
		set_rocket(argv[optind], argv+optind+1);
	exit(0);
}

