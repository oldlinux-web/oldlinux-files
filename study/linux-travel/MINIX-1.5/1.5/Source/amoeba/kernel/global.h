/****************************************************************************/
/*									    */
/* (c) Copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands */
/*									    */
/*    This product is part of the  Amoeba  distributed operating system.    */
/*									    */
/*    Permission to use, sell, duplicate or disclose this software must be  */
/* obtained in writing.  Requests for such permissions may be sent to	    */
/*									    */
/*									    */
/*		Dr. Andrew S. Tanenbaum					    */
/*		Dept. of Mathematics and Computer Science		    */
/*		Vrije Universiteit					    */
/*		Postbus 7161						    */
/*		1007 MC Amsterdam					    */
/*		The Netherlands						    */
/*									    */
/****************************************************************************/

#define KERNEL		0
#define USER		1

typedef unshort address;
typedef int func;

#define bufptr		vir_bytes

#define NOWHERE		((address) 0)
#define SOMEWHERE	((address) -1)
#define NILVECTOR	((func (*)()) 0)

#ifdef lint
#define ABSPTR(t, c)	(use(c), (t) 0)
#else
#define ABSPTR(t, c)	((t) (c))
#endif

#define bit(b)		(1 << (b))	/* simulate type 'bit' */

#define lobyte(x)	((unshort) (x) & 0xFF)
#define hibyte(x)	((unshort) (x) >> 8)
#define concat(x, y)	((unshort) (x) << 8 | (unshort) (y) & 0xFF)

#define sizeoftable(t)	(sizeof(t) / sizeof((t)[0]))
