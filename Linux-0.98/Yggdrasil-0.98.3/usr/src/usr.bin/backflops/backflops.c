#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

static int getdevminor(char *);
static void getvolume(int);
static void *getbuffer(int);
static void writevolrec(int, int, unsigned char, unsigned, unsigned);
static void readvolrec(int);
static int reportvolrec(int);
static void beep();
static int fillbuffer(int, void *, unsigned);
static char *tarexcludeflag(char *, char *);
static char *putcmdlinetoenv(int, char **, char *);

/*
 * Maps Linux device minor numbers to capacity.
 */
static int floppy_kb[] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1200, 1200,		/*  0-9  */
	  0, 0, 0, 0, 0, 0, 720, 720, 0, 0, 		/* 10-19 */
	  360, 360, 0, 0, 0, 0, 0, 0, 1440, 1440, 	/* 20-29 */
	};

/*
 * This is the record that goes on the front of each volume.
 */
static struct {
	unsigned vol_recsize; /* size of entire record */
	unsigned char vol_nvol; /* sequence number of this volume */
	unsigned char vol_last; /* boolean - is this the last volume? */
	unsigned char unused1, unused2; /* explicit padding */
	unsigned vol_cap; /* potential number of data bytes ... */
	unsigned vol_size; /* number of data bytes on this volume */
	char vol_creator[L_cuserid]; /* login name that made this backup */
	time_t vol_date; /* date this backup made */
	char vol_cmdline[128]; /* trying to keep size at 2^n */
	char unused3[87]; /* trying to keep size at 2^n */
	char vol_tag[8]; /* just a pattern to check for validity */
} volrec;
		
#ifndef DEFDEVICE
#define DEFDEVICE	"/dev/PS0"
#endif

#define WRITENAME	"writemulti"
#define READNAME	"readmulti"
#define BACKUPNAME	"backflops"
#define RESTORENAME	"restflops"

#define VOLTAG		"VOLREC"
#define VOLCMD		"VOL_CMDLINE"

    static void
usage(int uassert)
{
    char *dev = "[-d device]";
    if(uassert) return;
    fprintf(stderr, "Usage:\n");
    fprintf(stderr,
	"    %s  [-a aflags] %s [-l] [-r] [-u] [-v] [-x] [-z] dir ...\n",
	BACKUPNAME, dev);
    fprintf(stderr,
	"    %s  [-a aflags] %s [-r] [-t] [-u] [-v] [-x] [-z] file ...\n",
	RESTORENAME, dev);
    fprintf(stderr,
	"    %s %s [-r] [-u]\n",
	WRITENAME, dev);
    fprintf(stderr,
	"    %s  %s [-q] [-r] [-u]\n",
	READNAME, dev);
	
    fprintf(stderr,
	"\nFlags:\n");
    fprintf(stderr,
	"\
    -a aflags	Additional flags to pass to tar.\n\
    -d device	Special file to use (default: %s).\n\
    -l		Pass -l (stay within file system) flag to tar.\n\
    -q		Read and display volume record data, then exit.\n\
    -r		Ready to go (don't prompt for initial volume).\n\
    -t		Use -t with tar to list the contents of the archive.\n\
    -u		Print this usage message.\n\
    -v		Don't run tar verbosely.\n\
    %s\
    -z		Don't compress the archive.\n\
	", DEFDEVICE,
#ifdef GNU_TAR
	"-x		Don't exclude directories named .../tmp.\n"
#else
	"\n"
#endif
	);
	
    fprintf(stderr,
	"\nExamples:\n");
    fprintf(stderr,
	"    # %s /\n",
	BACKUPNAME);
    fprintf(stderr,
	"    # %s -t\n",
	RESTORENAME);
    fprintf(stderr,
	"    # cd /; %s\n",
	RESTORENAME);
    fprintf(stderr,
	"    # %s -dPS1 /etc /usr\n",
	BACKUPNAME);
    fprintf(stderr,
	"    # cd /; %s -dPS1 usr/bin/mtools\n",
	RESTORENAME);
    fprintf(stderr,
	"    # find . -print | cpio -ov | %s\n",
	WRITENAME);
    fprintf(stderr,
	"    # %s | cpio -idv\n",
	READNAME);
    exit(1);
}

    int
main(int argc, char *argv[])
{
    unsigned char lastvol;
    char *rtarflg = "x", *verbtar="vv";
    char *prog;
    int c, n, vol_fd, nvol=0, queryvol=0, compress=1;
#ifdef GNU_TAR
    int exclude=1;
#else
    int exclude=0;
#endif
    unsigned capacity=0, datacapacity=0, bytesinbuffer;
    void *bbuffer;
    char sysbuf[256], aflags[32], volume[32] = DEFDEVICE;

    prog = (prog = strrchr(argv[0], '/')) ? prog+1 : argv[0];

    /* if we come by here twice, the first entry will be preserved */
    putcmdlinetoenv(argc, argv, VOLCMD);

    while((c = getopt(argc, argv, "a:c:d:lqrtuvxz")) != EOF) {
	switch(c) {
	    case 'a':
		strcat(aflags, optarg);
		break;
	    case 'c':
		capacity = atoi(optarg) * 1024;
		break;
	    case 'd':
		if(*optarg == '/') {
		    strcpy(volume, optarg);
		} else {
		    strcpy(volume+5, optarg); /* assuming the /dev/ */
		}
		break;
	    case 'l':
		strcat(aflags, "l");
		break;
	    case 'q':
		queryvol = 1;
		break;
	    case 'r':
		nvol = 1;
		break;
	    case 't':
		rtarflg = "t";
		break;
	    case '?':
	    default:
		fprintf(stderr, "Unrecognized argument: -%c\n", c);
	    case 'u':
		usage(0);
		break;
	    case 'v':
		verbtar = "";
		break;
	    case 'z':
		compress=0;
		break;
#ifdef GNU_TAR
	    case 'x':
		exclude=0;
		break;
#endif
	}
    }

    /*
     * Look up the device minor to see how much data it can hold.
     */
    if(!capacity) {
	int minor, kb;
	minor = getdevminor(volume);
	kb = floppy_kb[minor];
	if(kb == 0) {
	    fprintf(stderr,
		"I have no data on %s! Initialize 'floppy_kb[%d]'\n",
		volume, minor);
	    fprintf(stderr,
		"(%d is the device minor for %s) in the source\n",
		minor, volume);
	    fprintf(stderr,
		"to the number of 1024-byte blocks %s can hold\n", volume);
	    exit(1);
	} else {
	    fprintf(stderr, "Use preformatted %dKB floppies in drive %c:\n",
		kb, 'A' + (minor % 4));
	}
	capacity = kb * 1024;
    }
    datacapacity = capacity - sizeof(volrec);

    /*
     * Here we look at the name we were run under and do the appropriate
     * thing based on that.
     */
    if(!strncmp(prog, WRITENAME, strlen(WRITENAME))) {
	bbuffer = getbuffer(datacapacity);
	if(nvol == 0) {
	    getvolume(1);
	}
	for(nvol=1, lastvol=0; !lastvol; nvol++) {
	    bytesinbuffer = fillbuffer(0, bbuffer, datacapacity);
	    lastvol = (bytesinbuffer < datacapacity);
	    if(bytesinbuffer > 0) {
		if(nvol > 1) {
		    sleep(1); /* try to let trailing tar verbosity finish */
		    getvolume(nvol);
		}
		vol_fd = open(volume, O_WRONLY);
		if(vol_fd < 0) {
		    perror(volume);
		    exit(1);
		}
		writevolrec(vol_fd, nvol, lastvol, bytesinbuffer, datacapacity);
		if((n = write(vol_fd, bbuffer, bytesinbuffer)) <
			bytesinbuffer) {
		    fprintf(stderr, "wrote only %d bytes\n", n);
		    exit(1);
		}
		close(vol_fd);
	    }
	}
	exit(0);
    } else if(!strncmp(prog, READNAME, strlen(READNAME))) {
	bbuffer = getbuffer(datacapacity);
	if(nvol == 0) {
	    getvolume(1);
	}
	for(nvol=1, lastvol=0; !lastvol; ) {
	    vol_fd = open(volume, O_RDONLY);
	    if(vol_fd < 0) {
		perror(volume);
		exit(1);
	    }
	    if(queryvol) {
		exit(reportvolrec(vol_fd));
	    }
	    readvolrec(vol_fd);
	    if(nvol != volrec.vol_nvol) {
		fprintf(stderr,
		    "Bad sequence: need volume %d, got volume %d\n",
		    nvol, volrec.vol_nvol);
		close(vol_fd);
		getvolume(nvol);
		continue;
	    }
	    bytesinbuffer = fillbuffer(vol_fd, bbuffer, volrec.vol_size);
	    lastvol = volrec.vol_last;
	    if(bytesinbuffer > 0) {
		if((n = write(1, bbuffer, bytesinbuffer)) < bytesinbuffer) {
		    fprintf(stderr, "wrote only %d bytes\n", n);
		    exit(1);
		}
	    }
	    close(vol_fd);
	    if(!lastvol) {
		sleep(1); /* try to let trailing tar verbosity finish */
		getvolume(++nvol);
	    }
	}
	exit(0);
    } else if(!strncmp(prog, BACKUPNAME, strlen(BACKUPNAME))) {
	usage(argv[optind] != NULL);
	sprintf(sysbuf, "tar -c%s%sf -", verbtar, aflags);
	while(argv[optind]) {
	    strcat(sysbuf, " ");
	    strcat(sysbuf, argv[optind]);
	    if(exclude) {
		strcat(sysbuf, tarexcludeflag(argv[optind], "tmp"));
		strcat(sysbuf, tarexcludeflag(argv[optind], "usr/tmp"));
	    }
	    optind++;
	}
	if(compress) {
	    strcat(sysbuf, " | compress");
	}
	sprintf(strrchr(sysbuf, '\0'), " | %s -c %d -d %s -r",
		WRITENAME, capacity/1024, volume);
	if(nvol == 0) {
	    getvolume(1);
	}
	fprintf(stderr, "%s\n", sysbuf);
	exit(system(sysbuf));
    } else if(!strncmp(prog, RESTORENAME, strlen(RESTORENAME))) {
	if(nvol == 0) {
	    getvolume(1);
	}
	sprintf(sysbuf, "%s -c %d -d %s -r %s| tar -%s%s%sf -",
		READNAME, capacity/1024, volume,
		compress ? "| uncompress " : "",
		rtarflg, verbtar, aflags);
	while(argv[optind]) {
	    strcat(sysbuf, " ");
	    strcat(sysbuf, argv[optind++]);
	}
	fprintf(stderr, "%s\n", sysbuf);
	exit(system(sysbuf));
    } else {
	fprintf(stderr, "Unrecognized link: %s", prog);
	exit(1);
    }
    
    exit(0);
}

/*
 * Takes a device name and returns the minor. The use of dev_rdev
 * may be endian-dependent.
 */
    static int
getdevminor(char *dev)
{
    struct stat statbuf;
    if(stat(dev, &statbuf) != 0) {
	perror(dev);
	exit(1);
    }
    if(S_ISBLK(statbuf.st_mode)) {
	union {
	    dev_t rdev;
	    unsigned char minmaj[2];
	} devdata;
	devdata.rdev = statbuf.st_rdev;
	if(devdata.minmaj[1] == 2) {
	    return devdata.minmaj[0];
	} else {
	    fprintf(stderr, "%s is not a floppy device!\n", dev);
	}
    } else {
	fprintf(stderr, "%s is not a block device!\n", dev);
    }
    exit(1);
}

/*
 * Just a malloc with builtin error checking.
 */
    static void *
getbuffer(int nbytes)
{
    void *tb;
    if((tb = (void *) malloc(nbytes)) == NULL)  {
	fprintf(stderr, "Couldn't allocate buffer of %d bytes\n", nbytes);
	exit(1);
    }
    return tb;
}

/*
 * Prompt the user for a new volume.
 */
    static void
getvolume(int which)
{
    FILE *tty_strm = fopen("/dev/tty", "r");
    if(tty_strm == NULL) {
	perror("/dev/tty");
	exit(1);
    }
    beep();
    fflush(NULL);
    fprintf(stderr, "Insert volume %d and hit return: ", which);
    fflush(stderr);
    while(getc(tty_strm) != '\n');
    fclose(tty_strm);
}

/*
 * Write a volume record at the beginning of the disk.
 */
    static void
writevolrec(int fd, int seq, unsigned char last, unsigned size, unsigned cap)
{
    char *cl = getenv(VOLCMD);
    (void) memset(&volrec, 0, sizeof(volrec));
    volrec.vol_recsize = sizeof(volrec);
    volrec.vol_nvol = seq;
    volrec.vol_last = last;
    volrec.vol_cap = cap;
    volrec.vol_size = size;
    cuserid(volrec.vol_creator);
    volrec.vol_date = time(0);
    strncpy(volrec.vol_cmdline, cl ? cl : "", sizeof(volrec.vol_cmdline));
    strcpy(volrec.vol_tag, VOLTAG);
    fprintf(stderr, "Writing volume %d%s using %d%% of %d available bytes.\n",
	seq, last ? " (last)" : "", (size * 100) / cap, cap);
    if(write(fd, (void *) &volrec, sizeof(volrec)) == sizeof(volrec)) {
	return;
    }
    fprintf(stderr, "Failed write of volume record to current volume!\n");
    exit(1);
}

/*
 * Read the volume record back off.
 */
    static void
readvolrec(int fd)
{
    if(read(fd, (void *) &volrec, sizeof(volrec)) == sizeof(volrec)) {
	if((volrec.vol_recsize == sizeof(volrec)) &&
		!strcmp(volrec.vol_tag, VOLTAG)) {
	    fprintf(stderr,
		"Reading volume %d%s containing %d%% of %d available bytes.\n",
		volrec.vol_nvol,
		volrec.vol_last ? " (last)" : "",
		(volrec.vol_size * 100) / volrec.vol_cap,
		volrec.vol_cap);
	    return;
	}
    }
    fprintf(stderr, "Bogus volume record on current volume!\n");
    exit(1);
}

/*
 * Read the volume record and report its contents.
 */
     static int
reportvolrec(int fd)
{
    read(fd, (void *) &volrec, sizeof(volrec));
    fprintf(stderr, "Record size:\t	%d\n", volrec.vol_recsize);
    fprintf(stderr, "Volume sequence:	%d\n", volrec.vol_nvol);
    fprintf(stderr, "Last?:\t		%s\n", volrec.vol_last ? "YES" : "NO");
    fprintf(stderr, "Volume capacity:	%d\n", volrec.vol_cap);
    fprintf(stderr, "Amount used:\t	%d\n", volrec.vol_size);
    fprintf(stderr, "Volume creator:\t	%s\n", volrec.vol_creator);
    fprintf(stderr, "Creation date:\t	%24.24s\n", ctime(&(volrec.vol_date)));
    fprintf(stderr, "Command line:\t	%s\n", volrec.vol_cmdline);
    fprintf(stderr, "Volume tag:\t	%s\n", volrec.vol_tag);
    return !((volrec.vol_recsize == sizeof(volrec)) &&
	    !strcmp(volrec.vol_tag, VOLTAG));
}
 
    static void
beep()
{
    putc('', stderr);
}

/*
 * Do repeated reads until the buffer is filled or EOF is reached.
 * Returns bytes read.
 */
    static int
fillbuffer(int mfd, void *mbp, unsigned mtotal)
{
    void *bp;
    int n;
    
    for(bp=mbp; bp < (mbp + mtotal); bp+=n) {
	if((n = read(mfd, bp, mtotal - (bp - mbp))) <= 0) {
	    break;
	}
    }
    return bp - mbp;
}

/*
 * Generate tar flags to exclude directories named 'tmp' if they exist.
 */
    static char *
tarexcludeflag(char *head, char *tail)
{
    static char tmp[MAXPATHLEN+16];
    char *end = strrchr(head, '\0') - 1;
    sprintf(tmp, " +exclude %s%s%s", head, *end=='/' ? "" : "/", tail);
    if(!access(strchr(strchr(tmp, '+'), ' ') + 1, X_OK)) {
	return tmp;
    } else {
	return "";
    }
}

/*
 * String the argv into a char * and place it in the environment
 * under the name ename.
 */
    char *
putcmdlinetoenv(int argc, char *argv[], char *ename)
{
    static char cmdlinebuf[sizeof(volrec.vol_cmdline) * 2];
    char *cmdenv;
    
    if(cmdenv = getenv(ename)) {
	return cmdenv;
    } else {
	sprintf(cmdlinebuf, "%s=", ename);
	for(;argv[0]; argv++) {
	    strcat(cmdlinebuf, argv[0]);
	    strcat(cmdlinebuf, " ");
	}
	cmdlinebuf[sizeof(volrec.vol_cmdline)] = '\0';
	putenv(cmdlinebuf);
	return getenv(ename);
    }
}
