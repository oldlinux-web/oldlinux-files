.\" +----------
.\" |	$Id: gettytab.m4,v 2.0 90/09/19 20:12:26 paul Rel $
.\" |
.\" |	GETTYTAB man page.
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
.\" |	$Log:	gettytab.m4,v $
.\" |	Revision 2.0  90/09/19  20:12:26  paul
.\" |	Initial 2.0 release
.\" |	
.\" |	
.\" 
.\" +----------
.\" | M4 configuration
.\"
include(config.m4).\"
.\"
.\" define(`_temp_', maketemp(m4bXXXXX))
.\" syscmd(echo _gtab_ | tr "[a-z]" "[A-Z]" | tr -d "\012" > _temp_)
.\" define(`_GTAB_', include(_temp_))
.\" syscmd(rm -f _temp_)
.\"
.\" +----------
.\" | Manpage source follows:
.\"
.TH _GTAB_ _file_section_
.SH NAME
_gtab_ \- speed and tty settings used by getty
.SH DESCRIPTION
The file
.B _gettytab_
contains information used by
.IR getty (_mcmd_section_)
to set up the speed and tty settings for a line.  It supplies
information on what the
.I login-prompt
should look like.  It also supplies the speed to try next if
the user indicates the current speed is not correct by typing a
.I <break>
character.
.PP
Each entry in
.B _gettytab_
has the following format:

.in +.2i
.ll 7.5i
label# initial-flags # final-flags # login-prompt #next-label
.ll
.in -.2i

Each entry is followed by a blank line.  Lines that begin with
.B \#
are ignored and may be used to comment the file.  The various
fields can contain quoted characters of the form
\fB\\b\fR, \fB\\n\fR, \fB\\c\fR, etc., as well as \fB\\\fInnn\fR,
where
.I nnn
is the octal value of the desired character.  The various fields are:
.TP 16
.I label
This is the string against which
.I getty
tries to match its second argument. It is often the speed, such as
.BR 1200 ,
at which the terminal is supposed to run, but it needn't be (see below).
.TP
.I initial-flags
These flags are the initial
.IR ioctl (_system_section_)
settings to which the terminal is to be set if a terminal type is
not specified to
.IR getty .
.I Getty
understands the symbolic names specified in
.B /usr/`include'/termio.h
(see
.\" +----------
.\" | M4_start (trs16)
ifdef(`trs16',
.IR tty `('_misc_section_`)).',
.\" | else (trs16)
.IR termio `('_misc_section_`)).')
.\" | M4_end (trs16)
.\" +----------
Normally only the speed flag is required in the
.I initial-flags
field.
.I Getty
automatically sets the terminal to raw input mode and takes care of
most of the other flags.  The
.I initial-flag
settings remain in effect until
.I getty
executes
.IR login (_mcmd_section_).
.TP
.I final-flags
These flags take the same values as the
.I initial-flags
and are set just prior to
.I getty
executes
.BR _login_ .
The speed flag is again required.  The composite flag
.B SANE
takes care of most of the other flags that need to be set so that
the processor and terminal are communicating in a rational fashion.
The other two commonly specified
.I final-flags
are
.BR TAB3 ,
so that tabs are sent to the terminal as spaces, and
.BR HUPCL ,
so that the line is hung up on the final close.
.TP
.I login-prompt
This entire field is printed as the
.IR login-prompt .
Unlike the above fields where white space is ignored (a space,
tab or new-line), they are included in the
.I login-prompt
field.

The
.I login-prompt
may contain various
.BI @ char
and
\fB\\\fIchar\fR
parameters.  These are described in full in the
.IR getty (_mcmd_section_)
section PROMPT SUBSTITUTIONS.
.TP
.I next-label
This indicates the next
.I label
of the entry in the table that
.I getty
should use if the user types a
.I <break>
or the input cannot be read.  Usually, a series of speeds are linked
together in this fashion, into a closed set.  For instance,
.B 2400
linked to
.BR 1200 ,
which in turn is linked to
.BR 300 ,
which finally is linked back to
.BR 2400 .
.P
If
.I getty
is called without a
.I speed
argument, then the first entry of
.B _gettytab_
is used, thus making the first entry of
.B _gettytab_
the default entry. It is also used if
.I getty
can't find the specified
.I label.
If
.B _gettytab_
itself is missing, there is one entry built into
.I getty
which will bring up a terminal at 9600 baud.
.P
It is strongly recommended that after making or modifying
.BR _gettytab_ `,'
it be run through
.I getty
with the check (\fB\-c\fR) option to be sure there are no errors.
.SH FILES
_gettytab_
.SH "SEE ALSO"
login(_mcmd_section_),
getty(_mcmd_section_),
ioctl(_system_section_),
.\" +----------
.\" | M4_start (trs16)
ifdef(`trs16',
tty`('_misc_section_`)',
.\" | else (trs16)
termio`('_misc_section_`)')
.\" | M4_end (trs16)
.\" +----------
