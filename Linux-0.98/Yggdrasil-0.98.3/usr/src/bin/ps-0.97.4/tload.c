/*
 * tload.c	- terminal version of xload
 *
 * Options:
 *	-s initial scaling exponent (default = 6)
 *	-d delay
 *	-b 	background mode: don't update if window in background
 *		Uses less cpu time and the screen saver still works if
 *		the tload console is not the current console.
 *
 *
 * Copyright (c) 1992 Branko Lankester
 */

#include <stdio.h>
#include <termios.h>
#include <sys/stat.h>

#define	NR_CONSOLE	8

char *screen;

int nrows = 25;
int ncols = 80;
int scr_size;

extern int optind;
extern char *optarg;

main(argc, argv)
char **argv;
{
    int row, col=0, i, opt, bgnd_mode = 0;
    int fd=1;
    int lines, min_scale = 6, scale_shift;
    int delay = 5;
    unsigned long av[3], _aver, _fg_console;
    int tload_console;
    struct winsize win;


    while ((opt = getopt(argc, argv, "bs:d:")) != -1)
	switch (opt) {
	    case 's': min_scale = atoi(optarg); break;
	    case 'd': delay = atoi(optarg); break;
	    case 'b': bgnd_mode = 1; break;
	    default:
		printf("usage: tload [-b] [-d delay] [-s scale] [tty]\n");
		exit(1);
	}

    if (argc > optind) {
	if ((fd = open(argv[optind], 1)) == -1) {
	    perror(argv[optind]);
	    exit(1);
	}
    }

    open_psdb();
    _aver = k_addr("_avenrun");
    _fg_console = k_addr("_fg_console");

    if (ioctl(fd, TIOCGWINSZ, &win) != -1) {
	if (win.ws_col > 0)
	    ncols = win.ws_col;
	if (win.ws_row > 0)
	    nrows = win.ws_row;
    }
    scr_size = nrows * ncols;
    if ((screen = (char *) malloc(scr_size)) == NULL) {
	perror("");
	exit(1);
    }
    memset(screen, ' ', scr_size-1);
    *(screen + scr_size - 2) = '\0';

    if (bgnd_mode) {
	struct stat st;
	fstat(fd, &st);
	tload_console = st.st_rdev & 0xff;
	if (tload_console <= 0 || tload_console > NR_CONSOLE)
	    bgnd_mode = 0;
	--tload_console;
    }
    scale_shift = min_scale;

    while (1) {
	int dly = delay;

 	if ( scale_shift > min_scale )
	    scale_shift--; /* help it drift back down.. */

	kmemread(av, _aver, sizeof av);
    repeat:
	lines = av[0] >> scale_shift;
	row = nrows-1;

	while (--lines >= 0) {
	    *(screen + row * ncols + col) = '*';
	    if (--row < 0) {
		++scale_shift;
		goto repeat;
	    }
	}
	while (row >= 0)
	    *(screen + row-- * ncols + col) = ' ';

	for (i = 1; ; ++i) {
	    char *p;
	    row = nrows - (i*2048 >> scale_shift);
	    if (row < 0)
		break;
	    if (*(p = screen + row * ncols + col) == ' ')
		*p = '-';
	    else
		*p = '+';
	}

	if (++col == ncols) {
	    --col;
	    memmove(screen, screen + 1, scr_size-1);

	    for(row = nrows-2; row >= 0; --row)
		*(screen + row * ncols + col) = ' ';
	}
	if (bgnd_mode) {
	    while (get_kword(_fg_console) != tload_console && --dly >= 0)
		sleep(1);
	    if (dly < 0)
		continue;
	}
	for (i=0; i<3; ++i) {
	    av[i] *= 100;
	    av[i] >>= 11;
	}
	i = sprintf(screen, " %d.%02d, %d.%02d, %d.%02d",
		av[0] / 100, av[0] % 100,
		av[1] / 100, av[1] % 100,
		av[2] / 100, av[2] % 100);
	if (i>0)
	    screen[i] = ' ';

	write(fd, "\033[H", 3);
	write(fd, screen, scr_size - 1);
	if (dly)
	    sleep(dly);
    }
}
