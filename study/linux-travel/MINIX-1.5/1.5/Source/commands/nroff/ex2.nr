.\" example 2.     execute with: nroff ex2.nr
.\" 
.\" this is set up similar to ex1 except we show how to use number
.\" registers for section numbers. sections will be: I.00, I.01,...,
.\" II.00, II.01,... etc. the advantage is that you can add/remove
.\" sections without keeping track of the numbers. try it...
.\"
.pl 66                           \" page length (lines)
.po 5                            \" page offset (left margin)
.ll 60                           \" line length
.lt 60                           \" title length
.m1 3                            \" lines from page top to header
.m2 3                            \" lines from header to text
.m3 3                            \" lines from text to footer
.m4 3                            \" lines from footer to page botton
.tl |Using Number Registers For Sections|||
.fo |||Page %|                   \" page footer (just page number)
.in 5                            \" basic text indent (not header or major)
.nr A 0 1                        \" define reg A for major number
.af A I                          \" it will be upcase roman numerals
.nr B 0 1                        \" define reg B for minor number
.af B 01                         \" it will be 2 dig number (leading 0)
.\"
.\" start a major section. increase major number, reset minor number.
.\" first (and only) arg is the section name (text). we force a negative
.\" indent (temporary) back to the po for this title. make sure to quote
.\" multi-word titles...
.\"
.de MA
.nr A +1
.nr B 0 1
.sp 2
.ne 4
.ti -5
.\" print section title
\nA. $1
.sp 1
..
.\"
.\" start minor section. similar to MA except just increase minor number.
.\"
.de MI
.nr B +1
.sp 2
.ne 4
.\" print section title
\nA.\nB $1
.sp 1
..
.\"
.\"
.\" start text...
.\"
.MA "Major Section"
Blah blah blah blah blah blah blah blah blah blah.
Blah blah blah blah blah blah blah blah blah blah.
Blah blah blah blah blah blah blah blah blah blah.
.MI "Minor Section"
Blah blah blah blah blah blah blah blah blah blah.
Blah blah blah blah blah blah blah blah blah blah.
Blah blah blah blah blah blah blah blah blah blah.
Blah blah blah blah blah blah blah blah blah blah.
.MI "Minor Section"
Blah blah blah blah blah blah blah blah blah blah.
Blah blah blah blah blah blah blah blah blah blah.
.MI "Minor Section"
Blah blah blah blah blah blah blah blah blah blah.
Blah blah blah blah blah blah blah blah blah blah.
Blah blah blah blah blah blah blah blah blah blah.
Blah blah blah blah blah blah blah blah blah blah.
Blah blah blah blah blah blah blah blah blah blah.
.MA "Major Section"

.MI "Minor Section"
Blah blah blah blah blah blah blah blah blah blah.
Blah blah blah blah blah blah blah blah blah blah.
.MI "Minor Section"
Blah blah blah blah blah blah blah blah blah blah.
Blah blah blah blah blah blah blah blah blah blah.
Blah blah blah blah blah blah blah blah blah blah.
.MA "Major Section"
Blah blah blah blah blah blah blah blah blah blah.
Blah blah blah blah blah blah blah blah blah blah.
