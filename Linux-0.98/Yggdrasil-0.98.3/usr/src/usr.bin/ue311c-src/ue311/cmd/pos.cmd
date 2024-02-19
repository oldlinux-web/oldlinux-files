30	store-macro
	!if &seq $pending TRUE
		update-screen
		!return
	!endif
	set %tline &cat &cat &cat "Line = " $curline " Col = " $curcol
*it
	save-window
	1 next-window
	previous-window
	!if &not &seq $cbufname "Position"
		run pset
		!goto it
	!endif
	beginning-of-file
	overwrite-string &cat %tline "                "
	restore-window
!endm

store-procedure	pset
	1 next-window
	previous-window
	2 split-current-window
	1 resize-window
	1 select-buffer "Position"
	add-mode "black"
	add-mode "GREEN"
	previous-window
	set $cmdhook execute-macro-30
!endm

run pset
