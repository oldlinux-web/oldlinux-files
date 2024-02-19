.\" set this non-zero to turn on debugging
.nr Z 0
.\" **************************************************************************
.\"
.\" partial -ms package for nroff. macros for simple paper/report formats
.\"
.\" usage: nroff -ms file [...]
.\"
.\" included here are:
.\"
.\"    TL, AU, AI, AB, AE, SH, NH, PP, LP, QP, XP, RS, RE, IP, I, B, R
.\"
.\" extensions include:
.\"
.\"    TI (temp indent), EX (exit NOW)
.\"
.\" includes somewhat kludgy support for XS, XA, XE, PX (table of contents)
.\"
.\" v1.10 7/22/90 rosenkra@convex.com (Bill Rosenkranz)
.\" freely distributable (no copyright, etc.)
.\" 
.\" **************************************************************************
.\"
.\" some perdefined strings (Quote, Unquote, dash, footer parts, etc):
.\"
.ds Q ""
.ds U ""
.ds - --
.ds CF "ImPoSsIbLe
.ds LF "\0
.ds RF "\0
.\"
.\" these are various predefined date and time strings
.\"
.\" DW day-of-week:
.if \n(dw=1  .ds DW "Sun
.if \n(dw=2  .ds DW "Mon
.if \n(dw=3  .ds DW "Tue
.if \n(dw=4  .ds DW "Wed
.if \n(dw=5  .ds DW "Thu
.if \n(dw=6  .ds DW "Fri
.if \n(dw=7  .ds DW "Sat
.\" MO month:
.if \n(mo=1  .ds MO "January
.if \n(mo=2  .ds MO "February
.if \n(mo=3  .ds MO "March
.if \n(mo=4  .ds MO "April
.if \n(mo=5  .ds MO "May
.if \n(mo=6  .ds MO "June
.if \n(mo=7  .ds MO "July
.if \n(mo=8  .ds MO "August
.if \n(mo=9  .ds MO "September
.if \n(mo=10 .ds MO "October
.if \n(mo=11 .ds MO "November
.if \n(mo=12 .ds MO "December
.\" make some composites:
.ds Dy "\*(MO \n(dy
.ds Da "\n(hh:\n(mm:\n(ss \n(mo/\n(dy/\n(yr
.ds Yr "19\n(yr
.ds dY "\*(Dy, \*(Yr
.ds DY "\n(dy \*(MO \*(Yr
.ds TM "\n(hh:\n(mm:\n(ss
.ds DA "\*(TM \*(DY
.ds CT "\*(DW \*(Dy \*(TM 19\n(yr
.\"
.\" they look like this:
.\"
.\"	DW	Sun
.\"	MO	March
.\" 	Dy	March 4
.\" 	dY	March 4, 1990
.\" 	DY	4 March 1990       <--- "normal" nroff form
.\"	Yr	1990
.\" 	TM	16:34:00
.\" 	DA	16:34:00 March 4, 1990
.\"	Da	16:34:00 2/4/90
.\" 	CT	Sun March 4 16:34:00 1990	almost like ctime(2)
.\"
.\" **************************************************************************
.\"
.\" startup stuff...
.\"
.pl 66
.ll 6.0i
.lt 6.0i
.m1 3
.m2 2
.m3 3
.m4 3
.\" no header line on first page! (set back in AB, SH, NH, LP, PP, QP)
.tl ||||
.\" this is the default footer (date, centered) unless string CF is defined
.fo ||\*(DY||
.\" these are for NH numbering (up to 5 levels, a la sun, X holds level)
.nr A 0 1
.af A 1
.nr B 0 1
.af B 1
.nr C 0 1
.af C 1
.nr D 0 1
.af D 1
.nr E 0 1
.af E 1
.nr X 1 1
.\"
.\" **************************************************************************
.\"
.\" MACROS...
.\"
.\" ---------------------------------------------------------------------- TL
.\" title for document (optional)
.\"
.de TL
.sp 4
.ce 1000
.\" reset footer. you MUST define CF, even to blank, to get the others!
.if !"\*(CF"ImPoSsIbLe" .fo |\*(LF|\*(CF|\*(RF|
..
.\" ---------------------------------------------------------------------- AU
.\" author(s) (optional, requires .TL)
.\"
.de AU
.sp 2
..
.\" ---------------------------------------------------------------------- AI
.\" author's institution (optional, requires .TL)
.\"
.de AI
.sp 1
..
.\" ---------------------------------------------------------------------- AB
.\" abstract (optional, requires .TL, .AE)
.\"
.de AB
.br
.sp 2
.\" check for arg to AB. can be "no" or something like "SUMMARY". if "no",
.\" no title above the abstract
.if !"$1"no" .if "$1"" ABSTRACT
.if !"$1"no" .if !"$1"" $1
.if !"$1"no" .sp 1
.\" set new line length...
.ce 0
.ll 5.5i
.in 0.5i
.tl ||- % -||
..
.\" ---------------------------------------------------------------------- AE
.\" abstract end (optional, requires .TL)
.\"
.de AE
.br
.sp 1
.\" reset...
.ce 0
.in 0.0i
.ll 6.0i
..
.\" ----------------------------------------------------------------------- SH
.\" section heading, no number (optional)
.\"
.de SH
.\" reset...
.ll 6.0i
.in 0.0i
.tl ||- % -||
.\" see note in TL
.if !"\*(CF"ImPoSsIbLe" .fo |\*(LF|\*(CF|\*(RF|
.br
.ce 0
.sp 2
.ne 4
.\" section title goes here, fill mode only so far...
..
.\" ---------------------------------------------------------------------- NH
.\" numbered section heading. arg (required) is the section level.
.\" this would be MUCH simpler if the .if command supported "{...}". there is
.\" a bug in nroff. i does not set the .$ number register correctly (number of
.\" args for the current macro). that is the reason why level 1 must be set.
.\"
.de NH
.\" do everything from SH...
.SH
.\" if ".NH 0", reset numbering
.if $1=0 .nr A 1 1
.if $1=0 .nr B 0 1
.if $1=0 .nr C 0 1
.if $1=0 .nr D 0 1
.if $1=0 .nr E 0 1
.if $1=0 .nr X 1 1
.\" level 1 (two types here: ".NH" and ".NH 1"):
.\" once .$ num reg is fixed, these 5 should be:  .if \n(.$=0 .nr A +1  etc.
.if "$1""  .nr A +1
.if "$1""  .nr B 0 1
.if "$1""  .nr C 0 1
.if "$1""  .nr D 0 1
.if "$1""  .nr E 0 1
.if "$1""  .nr X 1 1
.if "$1"1" .nr A +1
.if "$1"1" .nr B 0 1
.if "$1"1" .nr C 0 1
.if "$1"1" .nr D 0 1
.if "$1"1" .nr E 0 1
.if "$1"1" .nr X 1 1
.\" level 2 (increment B, reset lower levels):
.if "$1"2" .nr B +1
.if "$1"2" .nr C 0 1
.if "$1"2" .nr D 0 1
.if "$1"2" .nr E 0 1
.if "$1"2" .nr X 2 1
.\" level 3 (increment C, reset lower levels):
.if "$1"3" .nr C +1
.if "$1"3" .nr D 0 1
.if "$1"3" .nr E 0 1
.if "$1"3" .nr X 3 1
.\" level 4 (increment D, reset lower levels):
.if "$1"4" .nr D +1
.if "$1"4" .nr E 0 1
.if "$1"4" .nr X 4 1
.\" level 5 (increment E, no more lower levels!):
.if "$1"5" .nr E +1
.if "$1"5" .nr X 5 1
.\" print out the section number now, depending on current level...
.if \nX=1 \nA.
.if \nX=2 \nA.\nB.
.if \nX=3 \nA.\nB.\nC.
.if \nX=4 \nA.\nB.\nC.\nD.
.if \nX=5 \nA.\nB.\nC.\nD.\nE.
.\" section title goes here...
..
.\" ----------------------------------------------------------------------- LP
.\" start a new left block paragraph (either .LP or .PP required)
.\"
.de LP
.br
.\" reset...
.tl ||- % -||
.ce 0
.sp 1
.ll 6.0i
.in 0.0i
..
.\" ----------------------------------------------------------------------- PP
.\" start a new indented paragraph (either .LP or .PP required)
.\"
.de PP
.\" do everything for LP, then make a temp indent...
.LP
.ti +0.5i
..
.\" ----------------------------------------------------------------------- XP
.\" start a new extended paragraph (bibliography)
.\"
.de XP
.br
.\" reset...
.tl ||- % -||
.ce 0
.sp 1
.ll 6.0i
.in 0.5i
.ti -0.5i
..
.\" ----------------------------------------------------------------------- QP
.\" start a new quoted paragraph (indented and shorter)
.\"
.de QP
.br
.tl ||- % -||
.ce 0
.sp 1
.\" set new line length, indent. PP, LP, SH, and NH reset
.ll 6.0i
.in 0.0i
.ll -0.5i
.in +0.5i
..
.\" ----------------------------------------------------------------------- IP
.\" indented paragraph with tag (relative)
.\"
.de IP
.br
.tl ||- % -||
.ce 0
.sp 1
.if \n(.i>4 .in -0.5i
.in +0.5i
.}D "***DEBUG IP: indent before tag is: \n(.i"
.}D .br
.if !"$1"" .ti -0.5i
.if !"$1"" \&$1
.if !"$1"" .br
.}D "***DEBUG IP: indent after tag is: \n(.i"
..
.\" ----------------------------------------------------------------------- RS
.\" start relative indent (requires .RE)
.\"
.de RS
.ce 0
.br
.\" if there is arg, use that as indent, otherwise use +5
.if \n(.$>0 .in +$1
.if \n(.$=0 .in +0.5i
.}D "***DEBUG RS: indent after RS is: \n(.i"
.}D .br
..
.\" ----------------------------------------------------------------------- RE
.\" end relative indent
.\"
.de RE
.ce 0
.br
.\" if there is arg, use that as unindent, otherwise use -5
.if \n(.$>0 .in -$1
.if \n(.$=0 .in -0.5i
.if \n(.i<5 .in 0.0i
.}D "***DEBUG RE: indent after RE is: \n(.i"
.}D .br
..
.\" ----------------------------------------------------------------------- XS
.\" table of contents start
.\"
.de XS
.bp
.ce 1
Table of Contents
.ce 0
.ll 8.0i
.sp 1
.\" \&123456789012345678901234567890123456789012345678901234567890
\&NOTE: add/del "dots" until line ends here ---------------->| (del this line)
.br
.\" save the page number...
.ds Xx "$1
.\" first entry goes here...
..
.\" ----------------------------------------------------------------------- XE
.\" table of contents end
.\"
.de XE
.\" dump last page number...
\&\0........................... \*(Xx
.br
..
.\" ----------------------------------------------------------------------- XA
.\" subsequent table of contents entry
.\"
.de XA
.\" dump last page number...
\&\0........................... \*(Xx
.br
.\" save next page number...
.ds Xx "$1
.\" next entry goes here...
..
.\" ----------------------------------------------------------------------- PX
.\" print table of contents
.\"
.de PX
.\" this is a NOP without diversions...
.ll 6.0i
..
.\" ----------------------------------------------------------------------- TI
.\" temporary indent
.\"
.de TI
.ce 0
.ti +0.5i
..
.\" ----------------------------------------------------------------------- EX
.\" exit NOW! (no extra space at end of document)
.\"
.de EX
.sp
.ex
..
.\" ----------------------------------------------------------------------- I
.\" italic text
.\"
.de I
\&\fI
.if !"$1"" $1\fR$2
..
.\" ----------------------------------------------------------------------- B
.\" bold text
.\"
.de B
\&\fB
.if !"$1"" $1\fR$2
..
.\" ----------------------------------------------------------------------- R
.\" Roman (normal) text
.\"
.de R
\&\fR
..
.\" ----------------------------------------------------------------------- }D
.\" debug. use (e.g. print current indent):
.\"
.\"	.}D .br
.\"	.}D "** DEBUG ** before RS \n(.i"
.\"
.de }D
.if \nZ>0 \&$1
..
