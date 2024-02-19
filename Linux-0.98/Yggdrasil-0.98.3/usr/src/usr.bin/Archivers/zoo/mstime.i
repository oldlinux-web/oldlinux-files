#ifndef LINT
static char mstimeid[]="@(#) mstime.i 2.2 88/01/24 12:47:58";
#endif /* LINT */

/*
(C) Copyright 1987 Rahul Dhesi -- All rights reserved
*/

#define BASEYEAR 1970

/****************
Function mstime() converts time in seconds since January 1 of BASEYEAR
to MS-DOS format date and time.
*/
mstime(longtime, date, time)
long longtime;       /* input:  seconds since Jan 1, BASEYEAR   */
int *date, *time;    /* output: MS-DOS format date and time */

{
   static int daysinmo[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
#define FEBRUARY 1
   int year, month, day, hour, min, sec;
   long secsinhour, secsinday, secsinyear, secsinleapyear;

   int leapyear;                             /* is this a leap year? */
   int done;                                 /* control variable */

   secsinhour = (long) (60 * 60);            /* seconds in an hour */
   secsinday  = 24 * secsinhour;             /* seconds in a day */
   secsinyear = 365 * secsinday;             /* seconds in a year */
   secsinleapyear = secsinyear + secsinday;  /* seconds in a leap year */

#ifdef DEBUG
printf("mstime:  input longtime = %ld\n", longtime);
#endif

   /* We can't handle dates before 1970 so force longtime positive */
   if (longtime < 0)
      longtime = 0;

   /* 
   Step through years from BASEYEAR onwards, subtracting number of
   seconds in each, stopping just before longtime would become negative.
   */
   year = BASEYEAR;
   done = 0;
   while (!done) {
      long yearlength;
      leapyear = (year % 4 == 0 && year % 100 != 0 || year % 400 == 0);
      if (leapyear)
         yearlength = secsinleapyear;
      else
         yearlength = secsinyear;

      if (longtime >= yearlength) {
         longtime -= yearlength;
         year++;
      } else
         done++;
   }

   /* Now `year' contains year and longtime contains remaining seconds */
   daysinmo[FEBRUARY] = leapyear ? 29 : 28;

   month = 0; /* range is 0:11 */
   while (longtime > daysinmo[month] * secsinday) {
      longtime = longtime - daysinmo[month] * secsinday;
      month++;
   }
   month++; /* range now 1:12 */

   day = longtime / secsinday;     /* day of month, range 0:30 */
   longtime = longtime % secsinday;
   day++;                         /* day of month, range 1:31 */

   hour = longtime / secsinhour;       /* hours, range 0:23 */
   longtime = longtime % secsinhour;

   min = longtime / 60L;               /* minutes, range 0:59 */
   longtime = longtime % 60L;

   sec = longtime;                     /* seconds, range 0:59 */

#ifdef DEBUG
printf("mstime:  date = %4d/%02d/%02d   time = %02d:%02d:%02d\n",
      year, month, day, hour, min, sec);
if (leapyear)
   printf("(leap year)\n");
#endif

   if (year < 1980)
      year = 1980;
   *date = day + (month << 5) + ((year - 1980) << 9);
   *time = (sec / 2) + (min << 5) + (hour << 11);
}
