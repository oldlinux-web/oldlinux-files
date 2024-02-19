.\" +----------
.\" |	$Id: putenv.m4,v 2.0 90/09/19 20:14:13 paul Rel $
.\" |
.\" |	PUTENV man page.
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
.\" |	$Log:	putenv.m4,v $
.\" |	Revision 2.0  90/09/19  20:14:13  paul
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
.TH PUTENV _library_section_
.SH NAME
putenv \- change or add value to environment
.SH SYNOPSIS
.B int putenv(\fIstring\fB)\fR;
.br
.B char *\fIstring\fR;
.SH DESCRIPTION
.I String
points to a string of the form
.I name=value.
.I Putenv
makes the value of the environment variable
.I name
equal to
.I value
by altering an existing variable or creating a new one.  In either
case, the string pointed to by
.I string
becomes part of the environment, so altering the string changes
the environment.  The space used by
.I string
is no longer used once a new string\-defining
.I name
is passed to
.I putenv.
.SH "RETURN VALUE"
.I Putenv
returns non\-zero if it was unable to obtain enough space via
.I malloc
for an expanded environment, otherwise zero.
.SH "SEE ALSO"
exec(_system_section_),
getenv(_library_section_),
malloc(_library_section_),
environ(_file_section_)
.SH WARNINGS
.I Putenv
manipulates the environment pointed to by
.I environ,
and can be used in conjunction with
.I getenv.
However,
.I envp
(the third argument to
.IR main )
is not changed.
.PP
This routine uses
.IR malloc (_library_section_)
to enlarge the environment.
.PP
After
.I putenv
is called, environmental variables are not in alphabetical order.
.PP
A potential error is to call
.I putenv
with an automatic variable as the argument, then exit the calling
function while
.I string
is still part of the environment.
.SH AUTHOR
.nf
Paul Sutcliffe, Jr.  <paul@devon.lns.pa.us>
UUCP: ...!rutgers!devon!paul
