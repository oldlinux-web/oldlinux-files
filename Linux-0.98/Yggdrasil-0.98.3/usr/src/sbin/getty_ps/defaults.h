/*
**	$Id: defaults.h,v 2.0 90/09/19 19:45:49 paul Rel $
**
**	Defines the structures and functions used to read runtime
**	defaults.
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
**	$Log:	defaults.h,v $
**	Revision 2.0  90/09/19  19:45:49  paul
**	Initial 2.0 release
**	
*/


/* lines in defaults file are in the form "NAME=value"
 */

typedef struct Default {
	char	*name;		/* name of the default */
	char	*value;		/* value of the default */
} DEF;


DEF	**defbuild();		/* user-level routines */
char	*defvalue();

FILE	*defopen();		/* low-level routines */
DEF	*defread();
int	defclose();


/* end of defaults.h */
