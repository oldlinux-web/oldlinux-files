#ifndef LINT
static char nixtimeid[]="@(#) nixtime.i 2.3 88/01/24 12:49:28";
#endif /* LINT */

/*
Time handling routines for UNIX systems.  These are included by the file
machine.c as needed.

The contents of this file are hereby released to the public domain.

                                    -- Rahul Dhesi  1986/12/31
*/

struct tm *localtime();

/*****************
Function gettime() gets the date and time of the file handle supplied.
Date and time is in MSDOS format.
*/
int gettime (file, date, time)
ZOOFILE file;
unsigned *date, *time;
{
   struct stat buf;           /* buffer to hold file information */
   struct tm *tm;             /* will hold year/month/day etc. */
	int handle;
	handle = fileno(file);
   if (fstat (handle, &buf) == -1) {
      prterror ('w', "Could not get file time\n");
      *date = *time = 0;
   } else {
      tm = localtime (&buf.st_mtime); /* get info about file mod time */
      *date = tm->tm_mday + ((tm->tm_mon + 1) << 5) +
         ((tm->tm_year - 80) << 9);
      *time = tm->tm_sec / 2 + (tm->tm_min << 5) +
         (tm->tm_hour << 11);
   }

}

/*****************
Function setutime() sets the date and time of the filename supplied.
Date and time is in MSDOS format.  It assumes the existence of a function
mstonix() that accepts MSDOS format time and returns **IX format time,
and a function gettz() that returns the difference (localtime - gmt)
in seconds, taking daylight savings time into account.
*/
int setutime(path,date,time)
char *path;
unsigned int date, time;
{
	long mstonix();
	long gettz();
	long utimbuf[2];
	utimbuf[0] = utimbuf[1] = gettz() + mstonix (date, time);
	return (utime (path, utimbuf));
}

/****************
Function mstonix() accepts an MSDOS format date and time and returns
a **IX format time.  No adjustment is done for timezone.
*/

long mstonix (date, time)
unsigned int date, time;
{
   int year, month, day, hour, min, sec, daycount;
   long longtime;
   /* no. of days to beginning of month for each month */
   static int dsboy[12] = { 0, 31, 59, 90, 120, 151, 181, 212,
                              243, 273, 304, 334};

   if (date == 0 && time == 0)			/* special case! */
      return (0L);

   /* part of following code is common to zoolist.c */
   year  =  (((unsigned int) date >> 9) & 0x7f) + 1980;
   month =  ((unsigned int) date >> 5) & 0x0f;
   day   =  date        & 0x1f;

   hour =  ((unsigned int) time >> 11)& 0x1f;
   min   =  ((unsigned int) time >> 5) & 0x3f;
   sec   =  ((unsigned int) time & 0x1f) * 2;

/*
DEBUG and leap year fixes thanks to Mark Alexander 
<uunet!amdahl!drivax!alexande>
*/
#ifdef DEBUG
   printf ("mstonix:  year=%d  month=%d  day=%d  hour=%d  min=%d  sec=%d\n",
           year, month, day, hour, min, sec);
#endif

   /* Calculate days since 1970/01/01 */
   daycount = 365 * (year - 1970) +    /* days due to whole years */
               (year - 1969) / 4 +     /* days due to leap years */
               dsboy[month-1] +        /* days since beginning of this year */
               day-1;                  /* days since beginning of month */

   if (year % 4 == 0 && 
       year % 400 != 0 && month >= 3)  /* if this is a leap year and month */
      daycount++;                      /* is March or later, add a day */

   /* Knowing the days, we can find seconds */
   longtime = daycount * 24L * 60L * 60L    +
          hour * 60L * 60L   +   min * 60   +    sec;
	return (longtime);
}
