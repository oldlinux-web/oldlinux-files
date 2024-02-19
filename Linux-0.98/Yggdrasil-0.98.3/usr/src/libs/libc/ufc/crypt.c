/*
 * UFC-crypt: ultra fast crypt(3) implementation
 *
 * Copyright (C) 1991, 1992, Michael Glad, email: glad@daimi.aau.dk
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * @(#)crypt.c	2.14 01/23/92
 *
 * Semiportable C version
 *
 */

#include "ufc-crypt.h"

extern ufc_long *ufc_dofinalperm();

#ifdef _UFC_32_

/*
 * 32 bit version
 */

extern long32 ufc_keytab[16][2];
extern long32 ufc_sb0[], ufc_sb1[], ufc_sb2[], ufc_sb3[];

#define SBA(sb, v) (*(long32*)((char*)(sb)+(v)))

ufc_long *ufc_doit(l1, l2, r1, r2, itr)
  ufc_long l1, l2, r1, r2, itr;
  { int i;
    long32 s, *k;

    while(itr--) {
      k = &ufc_keytab[0][0];
      for(i=8; i--; ) {
	s = *k++ ^ r1;
	l1 ^= SBA(ufc_sb1, s & 0xffff); l2 ^= SBA(ufc_sb1, (s & 0xffff) + 4);  
        l1 ^= SBA(ufc_sb0, s >>= 16);   l2 ^= SBA(ufc_sb0, (s)          + 4); 
        s = *k++ ^ r2; 
        l1 ^= SBA(ufc_sb3, s & 0xffff); l2 ^= SBA(ufc_sb3, (s & 0xffff) + 4);
        l1 ^= SBA(ufc_sb2, s >>= 16);   l2 ^= SBA(ufc_sb2, (s)          + 4);

        s = *k++ ^ l1; 
        r1 ^= SBA(ufc_sb1, s & 0xffff); r2 ^= SBA(ufc_sb1, (s & 0xffff) + 4);  
        r1 ^= SBA(ufc_sb0, s >>= 16);   r2 ^= SBA(ufc_sb0, (s)          + 4); 
        s = *k++ ^ l2; 
        r1 ^= SBA(ufc_sb3, s & 0xffff); r2 ^= SBA(ufc_sb3, (s & 0xffff) + 4);  
        r1 ^= SBA(ufc_sb2, s >>= 16);   r2 ^= SBA(ufc_sb2, (s)          + 4);
      } 
      s=l1; l1=r1; r1=s; s=l2; l2=r2; r2=s;
    }
    return ufc_dofinalperm(l1, l2, r1, r2);
  }

#endif

#ifdef _UFC_64_

/*
 * 64 bit version
 */

extern long64 ufc_keytab[16];
extern long64 ufc_sb0[], ufc_sb1[], ufc_sb2[], ufc_sb3[];

#ifdef cray
#define SBA(sb, v) (*(long64*)((long64)(sb)+(v)))
#else
#define SBA(sb, v) (*(long64*)((char*)(sb)+(v)))
#endif

ufc_long *ufc_doit(l1, l2, r1, r2, itr)
  ufc_long l1, l2, r1, r2, itr;
  { int i;
    long64 l, r, s, *k;

    l = (((long64)l1) << 32) | ((long64)l2);
    r = (((long64)r1) << 32) | ((long64)r2);

    while(itr--) {
      k = &ufc_keytab[0];
      for(i=8; i--; ) {
	s = *k++ ^ r;
	l ^= SBA(ufc_sb3, (s >>  0) & 0xffff);
        l ^= SBA(ufc_sb2, (s >> 16) & 0xffff);
        l ^= SBA(ufc_sb1, (s >> 32) & 0xffff);
        l ^= SBA(ufc_sb0, (s >> 48) & 0xffff);

	s = *k++ ^ l;
	r ^= SBA(ufc_sb3, (s >>  0) & 0xffff);
        r ^= SBA(ufc_sb2, (s >> 16) & 0xffff);
        r ^= SBA(ufc_sb1, (s >> 32) & 0xffff);
        r ^= SBA(ufc_sb0, (s >> 48) & 0xffff);
      } 
      s=l; l=r; r=s;
    }

    l1 = l >> 32; l2 = l & 0xffffffff;
    r1 = r >> 32; r2 = r & 0xffffffff;
    return ufc_dofinalperm(l1, l2, r1, r2);
  }

#endif
