;	EPAGE.CMD:	EMACS Macro Programming Page
;			for MicroEMACS 3.9d and above
;			(C)opyright 1987 by Daniel M Lawrence

; set the clean procedure up
store-procedure clean
	delete-buffer "[Macro 10]"
	delete-buffer "[Macro 11]"
	delete-buffer "[Macro 12]"
	delete-buffer "[Macro 13]"
	bind-to-key execute-macro-14 S-FN5
	bind-to-key execute-macro-15 S-FN6
	delete-buffer "[Macro 16]"
	delete-buffer "[Macro 17]"
	bind-to-key execute-macro-18 S-FN9
	bind-to-key execute-macro-19 S-FN0
!endm

; make sure the function key window is up
	set %rcfkeys FALSE
	toggle-fkeys
	write-message "Loading..."

; Write out the page instructions
	save-window
	1 next-window
	beginning-of-file
	set $curcol 25
	overwrite-string " F1 display variable       F2 evaluate expression"
	next-line
	set $curcol 25
	overwrite-string " F3 compile buffer         F4 debug mode [OFF]"
	next-line
	set $curcol 25
	overwrite-string " F5 execute buffer         F6 execute macro"
	next-line
	set $curcol 18
	overwrite-string "EMAC"
	set $curcol 25
	overwrite-string " F7 indent region          F8 undent region"
	next-line
	set $curcol 25
	overwrite-string " F9 execute DOS command   F10 shell to DOS"
	unmark-buffer
	beginning-of-file
	!force restore-window
	update-screen

; display a variable

10	store-macro
	set %rcdebug $debug
	set $debug FALSE
	set %rctmp @&cat &cat "Variable to display[" %rcvar "]: "
	!if &not &seq %rctmp ""
		set %rcvar %rctmp
	!endif
	write-message &cat &cat &cat %rcvar " = [" &ind %rcvar "]"
	set $debug %rcdebug
!endm

;	evaluate expresion

11	store-macro
	delete-buffer "[temp]"
	set %rcbuf $cbufname
	set %cline $cwline
	select-buffer "[temp]"
	insert-string "set %rcval "
	!force insert-string @"EXP: "
	execute-buffer "[temp]"
	unmark-buffer
	select-buffer %rcbuf
	delete-buffer "[temp]"
	%cline redraw-display
	write-message &cat &cat "Value = [" %rcval "]"
!endm

;	compile the current buffer

12	store-macro
;	here is where to add code to handle compiling the current buffer
	write-message "[No Compiler module loaded]"
!endm

;	Toggle debug mode

13	store-macro
	set $debug FALSE
	set %cbuf $cbufname
	set %cline $cwline
	select-buffer "Function Keys"
	beginning-of-file
	next-line
	68 forward-character
	set %rcdebug &equ $curchar 70
	delete-previous-character
	2 delete-next-character
	!if %rcdebug
		insert-string "ON "
	!else
		insert-string "OFF"
	!endif
	unmark-buffer
	select-buffer %cbuf	
	%cline redraw-display
	write-message &cat &cat "[Debug Mode " %rcdebug "]"
	set $debug %rcdebug
!endm

	bind-to-key execute-buffer S-FN5
	bind-to-key execute-macro S-FN6

;	indent region

16	store-macro
	write-message "[Indenting region]"
	set %endline $curline
	set %endpos $cwline
	exchange-point-and-mark
	set $discmd FALSE
	set-mark
	set $discmd TRUE

	!while &gre %endline $curline
		beginning-of-line
		handle-tab
		next-line
	!endwhile

	beginning-of-line	
	set $cwline %endpos
	write-message "[Region indented]"
!endm

;	undent region

17	store-macro
	write-message "[Undenting region]"
	set %endline $curline
	set %endpos $cwline
	exchange-point-and-mark
	set $discmd FALSE
	set-mark
	set $discmd TRUE

	!while &gre %endline $curline
		beginning-of-line
		!if &gre $lwidth 0
			delete-next-character
		!endif
		next-line
	!endwhile

	beginning-of-line	
	set $cwline %endpos
	write-message "[Region undented]"
!endm

	bind-to-key shell-command S-FN9
	bind-to-key i-shell S-FN0
	set %rcvar ""
	write-message "[Program page loaded]"
