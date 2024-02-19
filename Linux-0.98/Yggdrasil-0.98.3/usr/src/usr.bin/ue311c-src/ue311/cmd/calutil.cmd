;       CALUTIL.CMD:    Utilities for Calendar Preparation
;			(C)opyright 1989 by Ian Ornstein
;			Last Update Fri Feb 17 1989
;    Permission is granted to copy, distribute and use this software
;    wherever Daniel M Lawrence's MicroEMACS is used.
;
;
store-procedure save-ctx
	set %savebuf	$cbufname
	set %savecol	$curcol
	set %saverow	$curline
!endm
store-procedure restore-ctx
	select-buffer	%savebuf
	set $curcol	%savecol
	set $curline	%saverow
!endm
;
store-procedure get-date
	select-buffer "[CalWork]"
	beginning-of-line
	!force kill-to-end-of-line
	insert-string $time
;
	beginning-of-line
	next-word
	delete-next-word
	set %curmon  &left $kill 3
;
	end-of-line
	4 backward-character
	delete-next-word
	set %curyear &left $kill 4
;
	beginning-of-line
	next-word
	delete-next-word
        set %curday &left $kill 2
	!if &equal &left %curday 1 0
		set %curday &cat " " &right %curday 1
	!endif
;
; SAVE  %mnum and %curyear
        run month-number
	set %savemon  %mnum
 	set %saveyear %curyear
	unmark-buffer
!endm
;
store-procedure get-window
        save-window
	1 next-window
	!if &sequal $cbufname "Function Keys"
		delete-window
	!endif
	!if &sequal $cbufname "*Calendar*"
		beginning-of-file
		set-mark
		end-of-file
		kill-region
	!else
		1 split-current-window
		select-buffer "*Calendar*"
		!force 8 resize-window
		beginning-of-file
		set-mark
		end-of-file
		!force kill-region
	!endif
	!while &less $curline 8
		78 insert-string " "
		insert-string "~n"
 	!endwhile
	78 insert-string " "  
	1 goto-line
!endm
;	*Calendar* Tables
;
store-procedure days-per-mon
;
;	DPM	- days per month
;
	set %M1 	31
	set %M2		28
	set %M3 	31
;
	set %M4 	30
	set %M5 	31
	set %M6  	30
;	
	set %M7 	31
	set %M8 	31
	set %M9 	30
;
	set %M10	31
	set %M11	30
	set %M12	31
;
	set %index &cat "%M" %mnum  
	set %dpm &ind %index
!endm
;
store-procedure month-name
;
;	MONAME	- month name
;
	set %M1 	"January  "
	set %M2 	"February "
	set %M3 	"March    "
;
	set %M4 	"April    "
	set %M5 	"May      "
	set %M6 	"June     "
;	
	set %M7 	"July     "
	set %M8 	"August   "
	set %M9 	"September"
;
	set %M10	"October  "
	set %M11	"November "
	set %M12	"December "
;
	set %index &cat "%M" %mnum   
	set %moname &ind %index
!endm
;
store-procedure month-number
;
;	MNUM
;
	set %Jan	1
	set %Feb	2
	set %Mar	3
;
	set %Apr	4
	set %May	5
	set %Jun	6
;	
	set %Jul	7
	set %Aug	8
	set %Sep	9
;
	set %Oct	10
	set %Nov	11
	set %Dec	12
;
	set %index &cat "%" %curmon
	set %mnum &ind %index
!endm
;
store-procedure day-of-week
	set %p1 &add  %mnum 10
	set %p2 &div  %p1   13
	set %p3 &tim  %p2   12
	set %p4 &sub  %p1   %p3
	set %p5 &tim  13    %p4
	set %p5 &sub  %p5   1
	;
	set %p5 &div  %p5   5
	set %p5 &add  &add  %p5   %dnum   77
	;
	set %p6 &sub  %mnum 14
	set %p6 &div  %p6   12
	set %p6 &add  %ynum %p6                    
	set %p7 &tim  5     %p6
	set %p8 &div  %p7   4
	set %p9 &add &add %p5 %p8 6
	;
	set %dow &div %p9  7
	set %dow &tim %dow 7
	set %dow &sub %p9  %dow 
!endm
;
;	DISP1MON	Displays one Calendar Month
;			Requires:
;			    %calcol	column to start month display
;			    %mnum	month number
;			    %ynum	four digit year (1989)
;
store-procedure disp1mon
	run month-name
	set $curcol %calcol
	overwrite-string &cat &CAT &cat "    " %moname " " %ynum
	next-line
	set $curcol %calcol
	overwrite-string &cat  " S  M  Tu W  Th F  S"
	run days-per-mon
;
	set %d1  "                     1  2  3  4  5  6  7  8  9 10"
	set %d2  " 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27"
	set %dfeb " 28                  "
	set %dleap " 28 29                  "
	set %d30 " 28 29 30                  "
	set %d31 " 28 29 30 31               "
	set %days &cat %d1 %d2
;
	!if &equ %dpm 30
		set %days &cat %days %d30
	!else
		!if &equ %dpm 31
			set %days &cat %days %d31
		!else
			!if &equ %mnum 2
			    !if &or &seq %curyear "1992" &seq %curyear "1996"
			  	    set %days &cat %days %dleap
			    !else
				    set %days &cat %days %dfeb
			    !endif
			!endif
		!endif
	!endif
;
	set %dnum 1
	run day-of-week
	set %pos &tim %dow 3
	set %pos &sub 21 %pos
;
	set %wkline 0
	!while &les %wkline 6
		set %wkline &add %wkline 1
		set %dispwk &mid %days %pos 21
		next-line
		set $curcol %calcol
		overwrite-string %dispwk
		set %pos &add %pos 21
	!endwhile
;
	beginning-of-file
	unmark-buffer
!endm
