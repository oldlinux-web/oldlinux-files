.\" +----------
.\" |	$Id: getty.m4,v 2.0 90/09/19 20:11:33 paul Rel $
.\" |
.\" |	GETTY/UUGETTY man page.
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
.\" |	$Log:	getty.m4,v $
.\" |	Revision 2.0  90/09/19  20:11:33  paul
.\" |	Initial 2.0 release
.\" |	
.\" |	
.\" 
.\" +----------
.\" | M4 configuration
.\"
include(config.m4).\"
.\"
.\" define(`_gtab_file_', _gtab_`_file')
.\"
.\" +----------
.\" | Manpage source follows:
.\"
.TH GETTY _mcmd_section_ "DATE" "Release RELEASE"
.SH NAME
getty \- sets terminal mode, speed, and line discipline
.SH SYNOPSIS
.B /etc/getty
.\" +----------
.\" | M4_start (trs16)
ifdef(`trs16',
.B speed
.I [defaults_file],
.\" | else (trs16)
[\-d
.I defaults_file]
[\-a] [\-h] [\-r
.I delay]
[\-t
.I timeout]
[\-w
.I waitfor]
.B line
.I [speed [type [lined]]])
.\" | M4_end (trs16)
.\" +----------
.br
.B /etc/getty \-c
.I _gtab_file_
.SH DESCRIPTION
.I Getty
is the second of the three programs
.IR (init (_mcmd_section_),
.IR getty (_mcmd_section_),
and
.IR login (_mcmd_section_)),
used by the
.\" +----------
.\" | M4_start
ifdef(`M_XENIX', XENIX, .\")
ifdef(`XENIX', XENIX, .\")
ifdef(`UNIX', UNIX, .\")
.\" | M4_end
.\" +----------
system to allow users to login.
.I Getty
is invoked by
.IR init (_mcmd_section_)
to:
.br
.TP 3
1.
Open tty lines and set their modes.
.TP
2.
Print the login prompt, and get the user's name.
.TP
3.
Initiate a login process for the user.
.P
The actual procedure that
.I getty
follows is described below:  Initially,
.I getty
parses its command line.  If no errors are found,
.I getty
scans the defaults file (normally
.BR _defaults_`/getty' )
to determine certain runtime values.  The values in the defaults file
(whose compiled\-in name can be altered with the optional
.\" +----------
.\" | M4_start (trs16)
ifdef(`trs16', .\", .B \-d)
.\" | M4_end (trs16)
.\" +----------
.I defaults_file
argument) take precedence to those on the command line.
.I Getty
then opens the
.I line
for reading and writing, and disables stdio buffering.
.\" +----------
.\" | M4_start (logutmp)
ifdef(`logutmp',
The
.IR utmp (_file_section_)
and
.IR wtmp (_file_section_)
(if it exists) files are updated to indicate a LOGIN_PROCESS.,
.\" | else (logutmp)
.\")
.\" | M4_end (logutmp)
.\" +----------
If an initialization was specified, it is performed (see LINE
INITIALIZATION).
.PP
Next,
.I getty
types the
.\" +----------
.\" | M4_start (issue)
ifdef(`_issue_',
issue `(or login banner,' usually from
.BR _issue_ `)'
and,
.\" | else (issue)
.\")
.\" | M4_end (issue)
.\" +----------
login prompt.  Finally,
.I getty
reads the user's login name and invokes
.IR login (_mcmd_section_)
with the user's name as an argument.  While reading the name,
.I getty
attempts to adapt the system to the speed of the terminal being used,
and also sets certain terminal parameters (see
.\" +----------
.\" | M4_start (trs16)
ifdef(`trs16',
.IR tty `('_misc_section_`))',
.\" | else (trs16)
.IR termio `('_misc_section_`))')
.\" | M4_end (trs16)
.\" +----------
to conform with the user's login procedure.
.PP
The tty device used by
.I getty
is determined by
.\" +----------
.\" | M4_start (trs16)
ifdef(`trs16',
a call to
.IR ttyname (S)`,'
which sets the
.I line
value.,
.\" | else (trs16)
the
.I line
argument.)
.\" | M4_end (trs16)
.\" +----------
.I Getty
uses the string
.BI /dev/ line
as the name of the device to attach itself to.  Unless
.I getty
is invoked with the
.B \-h
flag (or
.B HANGUP=NO
is specified in the defaults file), it will force a hangup on the line
by setting the speed to zero.  Giving
.B \-r
.I delay
on the command line (or using
.B WAITCHAR=YES
and
.BI DELAY= delay
in the defaults file) will cause
.I getty
to wait for a single character from the line, and then to wait
.I delay
seconds before continuing.  If no delay is desired, use
.BR \-r0 .
Giving
.B \-w
.I waitfor
on the command line (or using
.BI WAITFOR= waitfor
in the defaults file) will cause
.I getty
to wait for the specified string of characters from the line
before continuing.  Giving
.B \-t
.I timeout
on the command line (or using
.BR TIMEOUT= timeout
in the defaults file) will cause
.I getty
to exit if no user name is accepted within
.I timeout
seconds after the login prompt is typed.
.PP
The
.I speed
argument is a label to a entry in the
.B _gettytab_
file (see
.IR _gtab_ `('_file_section_)).
This entry defines to
.I getty
the initial speed (baud rate) and tty settings, the login prompt to be
used, the final speed and tty settings, and a pointer to another entry
to try should the user indicate that the speed is not correct.  This
is done by sending a
.I <break>
character (actually sequence).  Under certain conditions, a
carriage\-return will perform the same function.  This is usually the
case when getty is set to a higher speed than the modem or terminal.
.I Getty
scans the _gtab_ file sequentially looking for a matching entry.
.\" +----------
.\" | M4_start (trs16)
ifdef(`trs16',
If an entry for the given
.I speed,
.\" | else (trs16)
If no
.I speed
was given or the entry)
.\" | M4_end (trs16)
.\" +----------
cannot be found, the first entry in the
.B _gettytab_
file is used as a default.  In the event that the _gtab_ file cannot be
accessed`,' there is a compiled\-in default entry that is used.
.PP
.\" +----------
.\" | M4_start (trs16)
ifdef(`trs16',
The terminal type is determined by examining the
.I ttytype
file`,' using the value associated with
.I line.
.I Getty
uses this value to determine how to clear the video display.,
.\" | else (trs16)
The
.I type
argument is a string which names the type of terminal attached to the
line.  The
.I type
should be a valid terminal name listed in the
.\" +----------
.\" | M4_start (termcap)
ifdef(`termcap',
.BR termcap (_misc_section_),
.\" | else (termcap)
.BR terminfo (_misc_section_))
.\" | M4_end (termcap)
.\" +----------
database.
.\" +----------
.\" | M4_start (ttytype)
ifdef(`ttytype',
If no
.I type
is given on the command line`,' one is deduced from the
.B ttytype
file.,
.\" | else (ttytype)
.\")
.\" | M4_end (ttytype)
.\" +----------
.I Getty
uses this value to determine how to clear the video display.
.PP
The
.I lined
argument is a string describing the line discipline to use on the
line.  The default is
.BR LDISC0 .)
.\" | M4_end (trs16)
.\" +----------
.PP
As mentioned,
.I getty
types the login prompt and then reads the user's login name.  If a
null character is received, it is assumed to be the result of the user
pressing the
.I <break>
key or the carriage\-return key to indicate the speed is wrong.  This
causes
.I getty
to locate the next
.I speed
in the series (defined in _gettytab_).
.PP
The user's name is terminated by a new\-line or carriage\-return
character.  A carriage\-return results in the system being set to map
those to new\-lines (see
.IR ioctl (_system_section_)).
.PP
The user's name is scanned to see if it contains only upper\-case
characters.  If so,
.\" +----------
.\" | M4_start (warncase)
ifdef(`warncase',
a warning is issued to the user to retry his login`,' using lower\-case
if possible.  If the second attempt is still all upper\-case`,',
.\" | else (warncase)
.\")
.\" | M4_end (warncase)
.\" +----------
the system is set to map any future upper\-case characters into
lower\-case.
.PP
A check option is provided for testing the _gtab_ file.  When
.I getty
is invoked with the
.BI \-c _gtab_
option, it scans the named
.I _gtab_
file and prints out (to the standard output) the values it sees.  If
any parsing errors occur (due to errors in the syntax of the _gtab_
file), they are reported.
.SH "DEFAULTS FILE"
During its startup,
.I getty
looks for the file
.BI _defaults_`/getty' .line,
(or, if it cannot find that file, then
.BR _defaults_`/getty' ),
and if found, reads the contents for lines of the form

.in +.5i
NAME=\fIvalue\fR
.in -.5i

This allows getty to have certain features configurable at runtime,
without recompiling.  The recognized NAME strings, and their
corresponding values, follows:
.TP 6
SYSTEM=\fIname\fR
Sets the nodename value (displayed by
.B @S
\-\- see PROMPT SUBSTITUTIONS) to
.IR name .
The default is the
.I nodename
value returned by the
.IR uname (_library_section_)
call.  On XENIX systems, if the value of nodename is a zero\-length
string, the file
.B /etc/systemid
is examined to get the nodename.
Note that some sites may have elected to compile the nodename value
into
.IR getty .
.TP
VERSION=\fIstring\fR
Sets the value that is displayed by the
.B @V
parameter (see PROMPT SUBSTITUTIONS) to
.I string.
There is no default value.  If
.I string
begins with a '/' character, it is assumed to be the full pathname of a
file, and
.B @V
is set to be the contents of that file.
.TP
LOGIN=\fIname\fR
Sets the name of the login program to
.I name.
The default is
.B _login_
(see
.IR login (_mcmd_section_)).
If used,
.I name
must be the full pathname of the program that
.I getty
will execute instead of
.BR _login_ .
Note that this program is called, as is
.BR _login_ ,
the with the user's name as its only argument.
.TP
INIT=\fIstring\fR
If defined,
.I string
is an expect/send sequence that is used to initialize the line before
.I getty
attempts to use it.  This string is in a form resembling that used in
the
.I _systems_
file of
.IR uucp (_cmd_section_).
For more details, see LINE INITIALIZATION.  By default, no
initialization is done.
.\" +----------
.\" | M4_start (issue)
ifdef(`_issue_',
.TP
ISSUE=\fIstring\fR
During startup`,'
.I getty
defaults to displaying`,' as an issue or login banner`,' the contents of
the
.B _issue_
file.  If ISSUE is defined to a
.I string`,'
that string is typed instead.  If
.I string
begins with a '/' character`,' it is assumed to be the full pathname of
a file`,' and that file is used instead of
.BR _issue_ .,
.\" | else (issue)
.\")
.\" | M4_end (issue)
.\" +----------
.TP
CLEAR=\fIvalue\fR
If
.I value
is
.BR NO ,
then 
.I getty
will not attempt to clear the video screen before typing the
.\" +----------
.\" < M4_start (issue)
ifdef(`_issue_',
issue or login prompts.,
.\" | else (issue)
login prompt.)
.\" | M4_end (issue)
.\" +----------
The default is to clear the screen.
.TP
HANGUP=\fIvalue\fR
If
.I value
is
.BR NO ,
then
.I getty
will NOT hangup the line during its startup.  This is analogus to
giving the
.B \-h
argument on the command line.
.TP
WAITCHAR=\fIvalue\fR
If
.I value
is
.BR YES ,
then
.I getty
will wait for a single character from it's line before continuing.
This is useful for modem connections where the modem has CD forced
high at all times, to keep getty from endlessly chatting with the
modem.
.TP
DELAY=\fIseconds\fR
Used in conjunction with
.BR WAITCHAR ,
this adds a time delay of
.I seconds
after the character is accepted before allowing
.I getty
to continue.  Both
.B WAITCHAR
and
.B DELAY
have the same effect as specifying
.BI \-r delay
on the command line.
If
.B WAITCHAR
is given without a
.BR DELAY ,
the result is equal to having said
.B \-r0
on the command line.
The default is to not wait for a character.
.TP
TIMEOUT=\fInumber\fR
As with the
.B \-t
.I timeout
command line argument, tells
.I getty
to exit if no user name is accepted before the
.I number
of seconds elapse after the login prompt is typed.
The default is to wait indefinetly for the user name.
.TP
CONNECT=\fIstring\fR
If defined,
.I string
should be an expect/send sequence (like that for INIT) to direct
.I getty
in establishing the connection.
.I String
may be defined as
.BR DEFAULT ,
which will substitute the built\-in string:

.in +.5i
.nf
_connect_
.fi
.in -.5i

The \\A escape marks the place where the digits showing the speed
will be seen.  See CONNECTION AND AUTOBAUDING for more details.
The default is to not perform a connection chat sequence.
.TP
WAITFOR=\fIstring\fR
This parameter is similar to WAITCHAR, but defines a string of
characters to be waited for.
.I Getty
will wait until
.I string
is received before issuing the login prompt.
This parameter is best used when combined with CONNECT, as in
this example:

.in +.5i
.nf
WAITFOR=RING
CONNECT="" ATA\\r CONNECT\\s\\A
.fi
.in -.5i

This would cause
.I getty
to wait for the string
.BR RING ,
then expect nothing, send
.B ATA
followed by a carriage\-return, and then wait for a string such as
.BR "CONNECT 2400" ,
in which case,
.I getty
would set itself to 2400 baud.
The default is not to wait for any string of characters.
.TP
ALTLOCK=\fIline\fR
.I Uugetty
uses this parameter to lock an alternate device, in addition to the
one it is attached to.  This is for those systems that have two
different device names that refer to the same physical port; e.g.
/dev/tty1A vs. /dev/tty1a, where one uses modem control and the
other doesn't.  See the section on UUGETTY for more details.
The default is to have no alternate lockfile.
.P
The name of the defaults file can be changed by specifying
.\" +----------
.\" | M4_start (trs16)
ifdef(`trs16',
.\" Nothing,
.\" | else (trs16)
.B \-d)
.\" | M4_end (trs16)
.\" +----------
.I defaults_file
on the command line.  If
.I defaults_file
begins with a slash, it is assumed to be a complete pathname of the
defaults file to be used.  Otherwise, it is assumed to be a regular
filename, causing
.I getty
to use the pathname
.BI _defaults_ `/defaults_file.'
.SH "PROMPT SUBSTITUTIONS"
When
.I getty
is typing
.\" +----------
.\" | M4_start (issue)
ifdef(`_issue_',
the issue or login banner (ususally
.BR _issue_ )`,'
or,
.\" | else (issue)
.\")
.\" | M4_end (issue)
.\" +----------
the
.I login\-prompt,
it recognizes several escape (quoted) characters.  When one of these
quoted characters is found, its value is substituted in the output
produced by
.I getty.
Recognized escape characters are:
.br
.TP 6
\\\\\\\\\\\\\\\\
Backslash (\\).
.TP
\\\\\\\\b
Backspace (^H).
.TP
\\\\\\\\c
Placed at the end of a string, this prevents a new\-line from
being typed after the string.
.TP
\\\\\\\\f
Formfeed (^L).
.TP
\\\\\\\\n
New\-line (^J).
.TP
\\\\\\\\r
Carriage\-return (^M).
.TP
\\\\\\\\s
A single space (' ').
.TP
\\\\\\\\t
Horizontal tab (^I).
.TP
\\\\\\\\\fInnn\fR
Outputs the ASCII character whose decimal value is
.IR nnn .
If
.I nnn
begins with 0, the value is taken to be in octal.  If it begins
with 0x, the value is taken to be in hexidecimal.
.P
In addition, a single backslash at the end of a line causes the
immediately following new\-line to be ignored, allowing continuation
lines.
.PP
Also, certain
.BI "@" char
parameters are recognized.  Those parameters, and the value that is
substituted for them are:
.TP 6
@B
The current (evaluated at the time the
.B @B
is seen) baud rate.
.TP
@D
The current date, in MM/DD/YY format.
.TP
@L
The
.I line
to which
.I getty
is attached.
.TP
@S
The system node name.
.TP
@T
The current time, in HH:MM:SS (24-hour) format.
.TP
@U
The number of currently signed\-on users.  This is a count of the
number of entries in the
.I _utmp_
file
.\" +----------
.\" | M4_start (logutmp)
ifdef(`logutmp',
whose ut_type field is USER_PROCESS.,
.\" | else (logutmp)
that have a non\-null ut_name field.)
.\" | M4_end (logutmp)
.\" +----------
.TP
@V
The value of
.BR VERSION ,
as given in the defaults file.
.P
To display a single '@' character, use either '\\@' or '@@'.
.SH "LINE INITIALIZATION"
One of the greatest benefits (in the author's opinion, at least) is
the ability of
.I getty
to initialize its line before use.  This will most likely be done on
lines with modems, not terminals, although initializing terminals is
not out of the question.
.PP
Line initialization is performed just after the
.I line
is opened and prior to handling the WAITCHAR and/or WAITFOR options.
Initialization is accomplished by placing an

.in +.5i
INIT=\fIstring\fR
.in -.5i

line in the defaults file.
.I String
is a series of one or more fields in the form

.in +.5i
expect [ send [ expect [ send ] ] ... ]
.in -.5i

This format resembles the expect/send sequences used in the UUCP
.I _systems_
file, with the following exception:
A carriage return is NOT appended automatically to sequences that
are 'sent.'  If you want a carriage\-return sent, you must explicitly
show it, with '\\r'.
.PP
.I Getty
supports subfields in the expect field of the form

.in +.5i
expect[\-send\-expect]...
.in -.5i

as with UUCP.  All the escape characters (those beginning with a '\\'
character) listed in the PROMPT SUBSTITUTIONS section are valid in
the send and expect fields.
In addition, the following escape characters are recognized:
.br
.TP 6
\\\\\\\\p
Inserts a 1\-second delay.
.TP
\\\\\\\\d
Inserts a 2\-second delay.
.TP
\\\\\\\\K
Sends a .25\-second Break.
.TP
\\\\\\\\T\fInnn\fR
Modifies the default timeout (usually 30 seconds) to
the value indicated by
.IR nnn .
The value
.I nnn
may be decimal, octal, or hexidecimal; see the usage of
\fB\\\fInnn\fR in PROMPT SUBSTITUTIONS.
.P
Note that for these additional escape characters, no actual
character is sent.
.SH "CONNECTION AND AUTOBAUDING"
.I Getty
will perform a chat sequence establish a proper connection.
The best use of this feature is to look for the
.B CONNECT
message sent by a modem and set the line speed to the number given
in that message (e.g. CONNECT 2400).  
.PP
The format for the connect chat script is exactly the same as that
for the INIT script (see LINE INITIALIZATION), with the following
addition:
.br
.TP 6
\\\\\\\\A
Marks the spot where the baud rate will be seen.  This mark will
match any and all digits 0\-9 at that location in the script, and
set it's speed to that value, if possible.
.P
Autobauding, therefore, is enabled by placing the
.B \\\\A
mark in the chat script.  For example, the definition:

.in+.5i
CONNECT=CONNECT\\s\\A
.in-.5i

would match the string
.B "CONNECT 1200"
and cause
.I getty
to set it's baud rate to 1200, using the following steps:
.TP 3
1.
Having matched the value 1200,
.I getty
will attempt to find an entry with the label
.B 1200
in the
.B _gtab_
file.  If a matching _gtab_ entry is found, those values are
used.  If there is no match, then
.TP
2.
The _gtab_ values currently in use are modified to use the
matched speed (e.g. 1200).  However, if the matched speed
is invalid, then
.TP
3.
.I Getty
logs a warning message and resumes normal operation.  This
allows the practice of toggling through linked entries in the
_gtab_ file to behave as expected.
.P
.SH UUGETTY
.I Uugetty
has identical behavior to
.I getty,
except that
.I uugetty
is designed to create and use the lock files maintained by the UUCP
family
.IR (uucp (_cmd_section_),
.IR cu (_cmd_section_)
and others).  This prevents two or more processes from having conficting
use of a tty line.
.PP
When
.I uugetty
starts up, if it sees a lock file on the line it intends to use,
it will use the pid in the lock file to see if there is an active
process holding the lock.  If not,
.I uugetty
will remove the lock file and continue.  If a valid process is found,
.I uugetty
will sleep until that process releases the lock and then it will exit,
forcing
.IR init (_mcmd_section_)
to spawn a new
.I uugetty.
Once no conflicting process is found,
.I uugetty
grabs the
.I line
by creating the lock file itself before issuing the login prompt.
This prevents other processes from using the line.
.PP
.I Uugetty
will normally only lock the name of the line it is running on.  On
systems where there are two device names referring to the same port
(as is the case where one device uses modem control while the other
doesn't), place a line of the form

.in +.5i
ALTLOCK=\fIline\fR
.in -.5i

line in the defaults file.  For instance, if
.I uugetty
is on
.I /dev/tty1a,
and you want to have it lock
.I /dev/tty1A
also, use the line
.B ALTLOCK=tty1A
in the defaults file.
.SH FILES
.TP 16
_console_
The device to which errors are reported.
.\" +----------
.\" | M4_start (trymail)
ifdef(`trymail',
If the device is for some reason unavailable (cannot be written to)`,'
a mail message containing the error is sent to the user
.BR _notify_ .,
.\" | else (trymail)
.\")
.\" | M4_end (trymail)
.\" +----------
.TP
_defaults_`/getty[\fI.line\fR]'
Contains the runtime configuration.  Note that
.I uugetty
uses _defaults_`/uugetty[\fI.line\fR].'
.TP
_gettytab_
Contains speed and tty settings to be used by
.I getty.
.\" +----------
.\" | M4_start (issue)
ifdef(`_issue_',
.TP
_issue_
The default issue (or login banner).
.\" | else (issue)
.\")
.\" | M4_end (issue)
.\" +----------
.TP
_login_
The default login program called after the user's name is entered.
.\" +----------
.\" | M4_start (ttytype)
ifdef(`ttytype',
.TP
_ttytype_
Contains the terminal types for each line in the system.,
.\" | else (ttytype)
.\")
.\" | M4_end (ttytype)
.\" +----------
.P
.SH "SEE ALSO"
init(_mcmd_section_),
login(_mcmd_section_),
uucp(_cmd_section_),
ioctl(_system_section_),
uname(_library_section_),
.\" +----------
.\" | M4_start (issue)
ifdef(`issue',
issue`('_file_section_`)',
.\" | else (issue)
.\")
.\" | M4_end (issue)
.\" +----------
_gtab_`('_file_section_),
utmp(_file_section_),
.\" +----------
.\" | M4_start (trs16)
ifdef(`trs16',
tty`('_misc_section_`)',
.\" | else (trs16)
termio`('_misc_section_`)')
.\" | M4_end (trs16)
.\" +----------
.SH AUTHOR
.nf
Paul Sutcliffe, Jr.  <paul@devon.lns.pa.us>
UUCP: ...!rutgers!devon!paul
.br

Autobauding routines adapted from code submitted by
Mark Keating <...!utzoo!censor!markk>
