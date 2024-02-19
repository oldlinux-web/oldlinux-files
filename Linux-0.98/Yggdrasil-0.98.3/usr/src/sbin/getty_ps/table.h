/*
**	$Id: table.h,v 2.0 90/09/19 20:19:21 paul Rel $
**
**	Defines the structures and functions used to parse the
**	gettytab file.
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
**	$Log:	table.h,v $
**	Revision 2.0  90/09/19  20:19:21  paul
**	Initial 2.0 release
**	
*/


#ifndef	LDISC0
#define	LDISC0	0			/* default line discipline */
#endif	/* LDISC0 */

#ifndef	GETTYTAB
#define	GETTYTAB  "/etc/gettydefs"	/* default name of the gettytab file */
#endif	/* GETTYTAB */

/* 	Modes for gtabvalue()
 */
#define	G_CHECK	0			/* nothing */
#define	G_FIND	1			/* find requested entry only */
#define	G_FORCE	2			/* make alternate choices */

typedef	struct Gettytab {
	char	*cur_id;		/* current label */
	TERMIO	itermio;		/* initial termio flags */
	TERMIO	ftermio;		/* final termio flags */
	char	*login;			/* login prompt */
	char	*next_id;		/* next label */
} GTAB;

typedef	struct SymTab {
	char	*symbol;		/* symbolic name */
	ushort	value;			/* actual value */
} SYMTAB;

GTAB	*gtabvalue();


/* end of table.h */
