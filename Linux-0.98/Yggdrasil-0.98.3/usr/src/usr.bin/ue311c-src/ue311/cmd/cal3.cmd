;	CAL3.CMD:	Calendar Macro to display three months
;			Previous	Current		Next
;			Month		Month		Month
;			Asterics "**" replace the current date
;			in the current month
;			Inspired by m.cs.uiuc.edu!reingold who
;			did a calendar for GNUmacs
;			(C)opyright 1989 by Ian Ornstein
;			Last Update Fri Feb 17 1989
;    Permission is granted to copy, distribute and use this software
;    wherever Daniel M Lawrence's MicroEMACS is used.
;
;   Steps to solution
;
;	0.- Save context [save-buffer - restore-buffer wouldn't do it]
;	1.- Select buffer calwork
;	    insert $time
;	    parse Month day and year
;       2.- Create window to display calendar
;	3.- Do Current Month
;	4.- Mark Current Date
;	5.- Do Preceding Month
;	6.- Do Succeeding Month
;	7.- Restore context
;
;
;		Prolog - Setup for all months
;
execute-file calutil.cmd
run save-ctx
run get-date
;		Do current month first so current day can be
;		easily marked
run month-number
run get-window
set %ynum %curyear
set %calcol 27
run disp1mon
search-forward %curday
2 backward-character
overwrite-string "**"
beginning-of-file
;
;			Do Preceding Month
set %mnum &sub %mnum 1
!if &equ %mnum 0
	set %mnum 12
	set %curyear &sub %curyear 1
!endif
set %ynum %curyear
set %calcol 1
run disp1mon
;
;			Do third month's Calendar
set %mnum    %savemon
set %curyear %saveyear
set %mnum &add %mnum 1
!if &equ %mnum 13
	set %mnum 1
	set %curyear &add %curyear 1
!endif
set %ynum %curyear
set %calcol 53
run disp1mon
;
;restore-window
next-window
run restore-ctx
clear-message-line
;
