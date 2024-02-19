;	CAL1.CMD:	Interactive Calendar Macro
;			Asterics "**" replace the current date
;			in the current month.
;			Inspired by m.cs.uiuc.edu!reingold who
;			did a calendar for GNUmacs
;			(C)opyright 1989 by Ian Ornstein
;			Last Update Fri Feb 17 1989
;    Permission is granted to copy, distribute and use this software
;    wherever Daniel M Lawrence's MicroEMACS is used.
;
execute-file calutil.cmd
;
run save-ctx
run get-window
set %calcol 27
*today
	run get-date
	run month-number
	set %ynum  %curyear
	select-buffer "*Calendar*" 
	run disp1mon
	beginning-of-file
	search-forward %curday
	2 backward-character
	overwrite-string "**"
	beginning-of-file
	unmark-buffer
	!goto question
;
*displayit
	run disp1mon
	beginning-of-file
	unmark-buffer
	!goto question
;
*getmonth
	set %mnum @"Enter month number (1-12): "
	!if &and &gre %mnum 0 &les %mnum 13
		!goto getyear
	!else
     set %ans @"Month must be an number from 1 to 12   Press RETURN to Continue"
		!goto getmonth
	!endif
*getyear
	set %ynum @"Enter year number (yyyy): "
	!if &and &gre %ynum 1988 &les %ynum 2000
		!goto displayit
	!else
 set %ans @"Year must be an number from 1989 to 1999   Press RETURN to Continue"
		!goto getyear
	!endif
;
*help
set %helpmsg @"n-next, b-back, c-current, o-other, e-exit  Press RETURN to Continue"
	!goto question
;
*question
	update-screen
	set %calopt @"Enter Calendar Option (or ? for help): "
	!if &seq %calopt "e"
		!goto finish
	!else
		!if &seq %calopt "b"    			       ;backward
			set %mnum &sub %mnum 1
			!if &equ %mnum 0
				set %mnum 12
				set %ynum &sub %ynum 1
			!endif
			!goto displayit
		!else
			!if &seq %calopt "n"				;forward
				set %mnum &add %mnum 1
				!if &equ %mnum 13
					set %mnum 1
					set %ynum &add %ynum 1
				!endif
				!goto displayit
			!else
				!if &seq %calopt "c"			;current
					!goto today
				!else
					!if &seq %calopt "o"		;other
						!goto getmonth
					!else
						!if &seq %calopt "?"	;help
							!goto help
						!endif
					!endif
				!endif
			!endif
		!endif
	!endif
;
*finish
	next-window
	run restore-ctx
clear-message-line
;
 
