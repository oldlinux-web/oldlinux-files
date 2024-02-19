/****************************************************************************
 *									    *
 * (c) Copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands *
 *									    *
 *    This product is part of the  Amoeba  distributed operating system.    *
 *    Permission is hereby granted to use it exclusively for educational    * 
 *    and research purposes.  It may also be freely duplicated and given    *
 *    to others for educational and research purposes only.  All other use  *
 *    requires written permission from the copyright owner.		    *
 *									    *
 *    Requests for such permissions may be sent to              	    *
 *									    *
 *									    *
 *		Dr. Andrew S. Tanenbaum					    *
 *		Dept. of Mathematics and Computer Science		    *
 *		Vrije Universiteit					    *
 *		De Boelelaan 1081					    *
 *		1081 HV Amsterdam					    *
 *		The Netherlands						    *
 *									    *
/****************************************************************************/

#define NDEBUG
#ifdef NDEBUG
#define assert(e)	/* NOTHING */
#define compare(a,t,b)	/* NOTHING */
#else
#ifdef lint
#define assert(e)	use(e)
#define compare(a,t,b)	use(a, b)
#else lint
#define assert(x)	if (!(x)) printf("assertion failed in %s at %d\n", __FILE__, __LINE__)
#define compare(a,t,b)	if (!((a) t (b))) \
			printf("comparison failed in %s at %d (%D)\n", \
						 __FILE__, __LINE__, a)
/*
#define assert(e)	do if (!(e)) badassertion(__FILE__,__LINE__); while (0)
#define compare(a,t,b)	do if (!((a) t (b))) \
				badcompare(__FILE__,__LINE__, (long) (a)); \
			while (0)
*/
#endif lint
#endif NDEBUG
