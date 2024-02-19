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
 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * @(#)ufc-crypt.h	1.11 01/20/92
 *
 * Definitions of datatypes 
 * 
 */

/* 
 * Requirements for datatypes:
 * 
 * A datatype 'ufc_long' of at least 32 bit
 * *and*
 *   A type 'long32' of exactly 32 bits (_UFC_32_)
 *   *or*
 *   A type 'long64' of exactly 64 bits (_UFC_64_)
 *
 * 'int' is assumed to be at least 8 bit
 */

/*
 * #ifdef's for various architectures
 */

#ifdef cray
/* Cray: all integer flavoured
   datatypes are 64 bit wide.

   Not tested -- please notify me
   if you are able to test it.
*/
typedef unsigned long ufc_long;
typedef unsigned long long64;
#define _UFC_64_
#endif

#ifdef convex
/* thanks to pcl@convex.oxford.ac.uk (Paul Leyland) for testing */
typedef unsigned long ufc_long;
typedef long long     long64;
#define _UFC_64_
#endif

/*
 * For debugging 64 bit code etc with 'gcc'
 */

#ifdef GCC3232
typedef unsigned long ufc_long;
typedef unsigned long long32;
#define _UFC_32_
#endif

#ifdef GCC3264
typedef unsigned long ufc_long;
typedef long long     long64;
#define _UFC_64_
#endif

#ifdef GCC6432
typedef long long ufc_long;
typedef unsigned long long32;
#define _UFC_32_
#endif

#ifdef GCC6464
typedef long long     ufc_long;
typedef long long     long64;
#define _UFC_64_
#endif

/*
 * Catch all for 99.95% of all UNIX machines
 */

#ifndef _UFC_64_
#ifndef _UFC_32_
#define _UFC_32_
typedef unsigned      ufc_long;
typedef unsigned long long32;
#endif
#endif


