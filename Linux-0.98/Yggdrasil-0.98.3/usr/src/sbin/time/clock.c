#include <stdio.h>
#include <time.h>

/*
 * CMOS clock manipulation - Charles Hedrick, hedrick@cs.rutgers.edu, Apr 1992
 * clock [-u] -r  - read cmos clock
 * clock [-u] -w  - write cmos clock from system time
 * clock [-u] -s  - set system time from cmos clock
 *  -u indicates cmos clock is kept in universal time
 *
 * The program is designed to run setuid, since we need to be able to
 * write the CMOS port.
 *
 * I don't know what the CMOS clock will do in 2000, so this program
 * probably won't work past the century boundary.
 */

int readit;
int writeit;
int setit;
int universal;

usage() 
{
  printf("clock [-u] -r|w|s\n  r - read and print CMOS clock\n  w - write CMOS clock from system time\n  s - set system time from CMOS clock\n  u - CMOS clock is in universal time\n");
  exit(1);
}


int cmos_read(int fd, int addr)
{
  unsigned char buf[2];

  lseek(fd, 0x70, 0);
  buf[0] = 0x80 | addr;
  write(fd, buf, 1);
  lseek(fd, 0x71, 0);
  read(fd, buf, 1);
  return (*buf & 15) + (*buf>>4)*10;
}

int cmos_write(int fd, int addr, int value)
{
  unsigned char buf[2];

  value = ((value/10) << 4) + value % 10;
  lseek(fd, 0x70, 0);
  buf[0] = 0x80 | addr;
  write(fd, buf, 1);
  lseek(fd, 0x71, 0);
  buf[0] = value;
  write(fd, buf, 1);
}

main (int argc, char **argv, char **envp) {
  int fd;
  struct tm tm;
  struct tm *tmp;
  time_t systime;
  char *zone;
  char zonebuf[256];
  int century;

  while (argc > 1) {
    if (argv[1][0] != '-')
      usage();
    switch (argv[1][1]) {
    case 'r': 
      if (readit || writeit || setit)  /* only allow one of these */
	usage();
      readit = 1;
      break;
    case 'w':
      if (readit || writeit || setit)
	usage();
      writeit = 1;
      break;
    case 's':
      if (readit || writeit || setit)
	usage();
      setit = 1;
      break;
    case 'u':
      universal = 1;
      break;
    default:
      usage();
    }
    argc--;
    argv++;
  }

  if (! (readit | writeit | setit))  /* default to read */
    readit = 1;

  fd = open("/dev/port", 2);
  if (fd < 0) {
    printf("unable to open /dev/port read/write\n");
    exit(1);
  }

  if (readit || setit) {
    /* 
     * If we're unlucky, the seconds could overflow and everything
     * else change, while we're in the process of reading.  This
     * loop keeps trying until we get something that's consistent
     */
    do {
      tm.tm_sec = cmos_read(fd, 0);
      tm.tm_min = cmos_read(fd, 2);
      tm.tm_hour = cmos_read(fd, 4);
      tm.tm_wday = cmos_read(fd, 6);
      tm.tm_mday = cmos_read(fd, 7);
      tm.tm_mon = cmos_read(fd, 8);
      tm.tm_year = cmos_read(fd, 9);
    } while (tm.tm_min != cmos_read(fd, 2));
    tm.tm_mon--;
  }

  if (readit) {
    /*
     * Mktime assumes we're giving it local time.  If the CMOS clock
     * is in GMT, we have to set up TZ to mktime knows it.  Tzset gets
     * called implicitly by the time code, but only the first time.  When
     * changing the enviorment variable, better call tzset explicitly.
     */
    if (universal) {
      zone = (char *)getenv("TZ");  /* save original time zone */
      (void)putenv("TZ=");
      tzset();
      systime = mktime(&tm);
      /* now put back the original zone */
      if (zone) {
	if ((strlen(zone) + 4) > sizeof(zonebuf)) {
	  printf("Size of TZ variable is too long\n");
	  exit(1);
	}
	strcpy(zonebuf, "TZ=");
	strcat(zonebuf, zone);
	putenv(zonebuf);
      } else /* wasn't one, so clear it */
	putenv("TZ");
      tzset();
      printf("%s", ctime(&systime));
    } else
      printf("%s", asctime(&tm));
  } else if (setit) {
    if (getuid() != 0) {  /* program is designed to run setuid */
      printf("Sorry, must be root to set time\n");
      exit(1);
    }
    if (universal)
      (void)putenv("TZ=");    
    systime = mktime(&tm);
    if (stime(&systime) != 0) {
      printf("Unable to set time -- probably you are not root\n");
      exit(1);
    }
  } else /* writeit */ {
    systime = time(NULL);
    if (universal)
      tmp = gmtime(&systime);
    else
      tmp = localtime(&systime);
    /*
     * set the seconds to 0 first, to prevent any wraparounds while
     * we're setting everything else.  Then set the real seconds last.
     */
    cmos_write(fd, 0, 0);
    cmos_write(fd, 2, tmp->tm_min);
    cmos_write(fd, 4, tmp->tm_hour);
    cmos_write(fd, 6, tmp->tm_wday);
    cmos_write(fd, 7, tmp->tm_mday);
    cmos_write(fd, 8, tmp->tm_mon+1);
    cmos_write(fd, 9, tmp->tm_year);
    cmos_write(fd, 0, tmp->tm_sec);
  }
  exit(0);
}
