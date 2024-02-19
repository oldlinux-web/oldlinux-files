.\" +----------
.\" |	$Id: issue.m4,v 2.0 90/09/19 20:13:21 paul Rel $
.\" |
.\" |	ISSUE man page.
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
.\" |	$Log:	issue.m4,v $
.\" |	Revision 2.0  90/09/19  20:13:21  paul
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
.TH ISSUE _file_section_
.SH NAME
issue \- issue identification file
.SH DESCRIPTION
The file
.B _issue_
contains the
.I issue
or project identification to be printed as a login prompt.  This
is an ASCII file which is read by the program
.IR getty (_mcmd_section_)
and then written to the terminal just prior to printing the
.I login:
prompt.
.PP
The line(s) may contain various
.BI @ char
and
\fB\\\fIchar\fR
parameters.  These are described in full in the
.IR getty (_mcmd_section_)
section PROMPT SUBSTITUTIONS.
.SH FILES
_issue_
.SH "SEE ALSO"
getty(_mcmd_section_)
