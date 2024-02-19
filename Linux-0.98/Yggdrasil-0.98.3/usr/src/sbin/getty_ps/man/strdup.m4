.\" +----------
.\" |	$Id: strdup.m4,v 2.0 90/09/19 20:14:57 paul Rel $
.\" |
.\" |	STRDUP man page.
.\" |
.\" |	Copyright 1989,1990 by Paul Sutcliffe Jr.
.\" |
.\" |	Permission is hereby granted to copy, reproduce, redistribute,
.\" |	or otherwise use this software as long as: there is no monetary
.\" |	profit gained specifically from the use or reproduction or this
.\" |	software, it is not sold, rented, traded or otherwise marketed,
.\" |	and this copyright notice is included prominently in any copy
.\" |	made.
.\" |
.\" |	The author make no claims as to the fitness or correctness of
.\" |	this software for any use whatsoever, and it is provided as is. 
.\" |	Any use of this software is at the user's own risk.
.\" |
.\"
.\" +----------
.\" |	$Log:	strdup.m4,v $
.\" |	Revision 2.0  90/09/19  20:14:57  paul
.\" |	Initial 2.0 release
.\" |	
.\" |	
.\" 
.\" +----------
.\" | M4 configuration
.\"
include(config.m4).\"
.\"
.\" +----------
.\" | Manpage source follows:
.\"
.TH STRDUP _library_section_
.SH NAME
strdup \- duplicate a string in memory
.SH SYNOPSIS
.B char *strdup(\fIstring\fB)\fR;
.br
.B char *\fIstring\fR;
.SH DESCRIPTION
.I Strdup
allocates storage space (with a call to
.IR malloc (_library_section_))
for a copy of
.I string
and returns a pointer to the storage space containing the copied
string.
.SH "RETURN VALUE"
.I Strdup
returns NULL if storage cannot be allocated.  Otherwise, a valid
pointer is returned.
.SH "SEE ALSO"
malloc(_library_section_),
string(_library_section_)
.SH AUTHOR
.nf
Paul Sutcliffe, Jr.  <paul@devon.lns.pa.us>
UUCP: ...!rutgers!devon!paul
