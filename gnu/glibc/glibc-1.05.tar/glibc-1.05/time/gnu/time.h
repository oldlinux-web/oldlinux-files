/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	_GNU_TIME_H

#define	_GNU_TIME_H	1


/* A time value that is accurate to the nearest
   microsecond but also has a range of years.  */
struct __timeval
  {
    long int tv_sec;	/* Seconds.  */
    long int tv_usec;	/* Microseconds.  */
  };

/* Structure crudely representing a timezone.  */
struct __timezone
  {
    int tz_minuteswest;	/* Minutes west of GMT.  */
    int tz_dsttime;	/* Nonzero if DST is ever in effect.  */
  };


/* Values for the first argument to `getitimer' and `setitimer'.  */
enum __itimer_which
  {
    /* Timers run in real time.  */
    ITIMER_REAL = 0,
    /* Timers run only when the process is executing.  */
    ITIMER_VIRTUAL = 1,
    /* Timers run when the process is executing and when
       the system is executing on behalf of the process.  */
    ITIMER_PROF = 2,
  };

/* Type of the second argument to `getitimer' and
   the second and third arguments `setitimer'.  */
struct __itimerval
  {
    /* Value to put into `it_value' when the timer expires.  */
    struct __timeval it_interval;
    /* Time to the next timer expiration.  */
    struct __timeval it_value;
  };



#endif	/* gnu/time.h */
