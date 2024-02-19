.\" +----------
.\" |	$Id: getutent.m4,v 2.0 90/09/19 20:12:55 paul Rel $
.\" |
.\" |	GETUTENT man page.
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
.\" |	$Log:	getutent.m4,v $
.\" |	Revision 2.0  90/09/19  20:12:55  paul
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
.TH GETUTENT _library_section_
.SH NAME
getutent, getutline, setutent, endutent,
utmpname \- access utmp file entry
.SH SYNOPSIS
.B \#include <utmp.h>

.B struct utmp *getutent();

.B struct utmp *getutline(\fIline\fB)\fR;
.br
.B struct utmp *\fIline\fR;

.B void setutent();

.B void endutent();

.B void utmpname(\fIfile\fB)\fR;
.br
.B char *\fIfile\fR;
.SH DESCRIPTION
.I Getutent
and
.I getutline
each return a pointer to a structure of the following type:
.nf

    struct utmp {
	    char	ut_line[8];        /* tty name */
	    char	ut_name[8];        /* user id */
	    long	ut_time;           /* time on */
    };

.fi
.I Getutent
reads in the next entry from a
.IR utmp \-like
file.  If the file is not already open, it opens it.  If it
reaches the end of file, it fails.
.PP
.I Getutline
searches forward from the current point in the
.I utmp
file until it finds an entry which has a
.I ut_line
string matching the
.I line\->ut_line
string.  If the end of file is reached without a match, it fails.
.PP
.I Setutent
resets the input stream to the beginning of the file.  This should be
done before each search for a new entry if it is desired that the
entire file be examined.
.PP
.I Endutent
closes the currently open file.
.PP
.I Utmpname
allows the user to change the name of the file examined, from
.B _utmp_
to any other file.  It is most often expected that this other file
will be
.BR _wtmp_ .
If the file does not exist, this will not be apparent until the first
attempt to reference the file is made.
.I Utmpname
does not open the file.  It just closes the old file if it is
currently open and saves the new file name.
.SH FILES
_utmp_
.br
_wtmp_
.SH BUGS
The most current entry is saved in a static structure.  Multiple
accesses require that it be copied before further accesses are made.
.PP
These routines use buffered standard I/O for input.
.SH "SEE ALSO"
utmp(_file_section_)
.SH AUTHOR
.nf
Paul Sutcliffe, Jr.  <paul@devon.lns.pa.us>
UUCP: ...!rutgers!devon!paul
