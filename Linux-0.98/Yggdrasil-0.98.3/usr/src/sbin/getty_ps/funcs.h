/*
**	$Id: funcs.h,v 2.0 90/09/19 19:56:17 paul Rel $
**
**	Defines the miscellaneous functions.
*/

/*
**	Copyright 1989,1990 by Paul Sutcliffe Jr.
**
**	Permission is hereby granted to copy, reproduce, redistribute,
**	or otherwise use this software as long as: there is no monetary
**	profit gained specifically from the use or reproduction or this
**	software, it is not sold, rented, traded or otherwise marketed,
**	and this copyright notice is included prominently in any copy
**	made.
**
**	The author make no claims as to the fitness or correctness of
**	this software for any use whatsoever, and it is provided as is. 
**	Any use of this software is at the user's own risk.
*/

/*
**	$Log:	funcs.h,v $
**	Revision 2.0  90/09/19  19:56:17  paul
**	Initial 2.0 release
**	
*/


/*	States for settermio()
 */
#define	INITIAL	 0
#define	FINAL	 1

/*	Return values for getlogname()
 */
#define	BADSPEED 1
#define	BADCASE	 2
#define	NONAME	 3

int	Fputs(), chat(), getlogname();
char	*getuname();
void	settermio(), logerr();

#ifdef	DEBUG
void	debug(), dprint();
#endif	/* DEBUG */


/* end of funcs.h */
