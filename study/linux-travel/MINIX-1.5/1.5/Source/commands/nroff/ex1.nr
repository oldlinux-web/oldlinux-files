.\" example 1.     execute with: nroff ex1.nr
.\"
.\" we first set up basic page parameters (left offset, line length,
.\" title length, and where titles and footers go. we define a macro
.\" to handle paragraphs. it just breaks the last line (.sp does this),
.\" spaces down 2 lines, and temprarily indents the next line (.ti).
.\" the .ti has a "+" so this means add to the current indent. then
.\" we center the title and redefine the page header after the first
.\" line of text to keep the page header off the first page. invoke
.\" the paragraph macro (.IP) at every paragraph.
.\"
.pl 66                           \" page length (lines)
.po 5                            \" page offset (left margin)
.ll 65                           \" line length
.lt 65                           \" title length
.m1 3                            \" lines from page top to header
.m2 3                            \" lines from header to text
.m3 3                            \" lines from text to footer
.m4 3                            \" lines from footer to page botton
.fo ||- % -||                    \" page footer (just page number)
.de IP                           \" define a macro
.sp 2                            \" 2 spaces
.ti +5                           \" indent next line 5 spaces
..                               \" end macro
.ce 6                            \" center next 6 lines
NROFF Report Formatting

by

Bill Rosenkranz
rosenkra@hall.cray.com
.\"
.\"
.tl ||NROFF Report Formatting||  \" page header (centered)
.IP                              \" invoke our macro
Nroff is a text processor and formatter based on the design
provided in "Software Tools" by Kernighan and Plauger.
It has been modified to closely resemble the Unix(tm) nroff command.
The text and commands found in the file(s) are processed to generate
formatted text.
Note that one (and only one) of the files can be "-"
which reads input from stdin at that point.
The output always goes to stdout which can be redirected by the shell.
The -o option lets you redirect error output to the specified file
rather than stderr.
Debugging information always goes to the file
"nroff.dbg" and is generally used only for program development.
.IP
Nroff recognizes the following environment variables from the shell.
.IP
An alternate directory to find the files tmac.* ("." for example).
The default is c:\\lib\\tmac under TOS and /usr/lib/tmac under
Minix or Unix(tm).
.IP
An alternate directory to place any temporary files.
The default is the current directory.
.IP
Commands typically are distinguished by a period in column
one of the input followed by a two character abbreviation
for the command funtion.
The abbreviation may then be
followed by an optional numeric or character argument.
The numeric argument may be an absolute value such as setting
the right margin to a particular column, or the argument may 
be preceded by a plus sign or a minus sign to indicate that
the parameter should be modified relative to a previous
setting.
The following commands are recognized (those
marked "extension" are requests added to the basic set
provided by Unix(tm) nroff):
.IP
Begin line adjustment.
If fill mode is not on, adjustment is defered until it is back on.
If a type indicator is present, the adjustment type is changed as follows: 
.ne 7           \" need 7 lines without a break
.nf             \" nofill for the table

Indicator      Type
   l           adjust left margin only
   r           adjust right margin only
   c           center
b or n         adjust both margins (default)
absent         unchanged
.fi            \" back to fill mode
.IP
Causes the following lines of text to appear in boldface.
The optional argument specifies the number of lines to be typed in boldface.
Boldface and underlining are mutually exclusive features.
The appearance of a boldface command will cause any underlining to cease.
.IP
Causes succeeding text to appear at the top of a new page.
The optional argument specifies the page number for the new page.
The initial value is one and the
default value is one more than the previous page number.
.\"
.\" the end...
