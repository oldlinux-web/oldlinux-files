/* timer.h */
/*
    Copyright (C) 1992  Ross Biro

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 

    The Author may be reached as bir7@leland.stanford.edu or
    C/O Department of Mathematics; Stanford University; Stanford, CA 94305
*/
#ifndef _TCP_TIMER_H
#define _TCP_TIMER_H

struct timer
{
  unsigned long len;
  volatile struct sock *sk;
  unsigned long when;
  struct timer *next;
};


void delete_timer (struct timer *);
void reset_timer (struct timer *);
void net_timer (void);

#define SEQ_TICK 3
#define timer_seq jiffies
extern unsigned long seq_offset;
#endif
