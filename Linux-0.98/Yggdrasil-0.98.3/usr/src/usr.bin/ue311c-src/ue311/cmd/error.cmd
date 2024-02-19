;	ERROR.CMD:	Parse Mark Williams C compilation error files
;			using MicroEMACS 3.9a
;
;	(C)opyright 1987 by Daniel M. Lawrence
;

;	Load up the needed macros
	set $discmd "FALSE"
	write-message "[Setting UP]"

;	page up/down for the Atari 1040ST
	bind-to-key previous-page	FNC
	bind-to-key next-page		FN<

;	cycle to the next error
39	store-macro
	run closelast
!force	1 next-line
	run getnext
!endm
	bind-to-key execute-macro-39	FN1

;	cycle to the previous error
38	store-macro
	run closelast
!force	1 previous-line
	run getnext
!endm
	bind-to-key execute-macro-38	FN2

;	get the next error
store-procedure	getnext

	;check for end of errors
	2 next-window
	!if &seq $line ""
		3 next-window
		write-message "[No more errors]"
		!return
	!endif

	;grab the line number and file name
	delete-next-word
	yank
	set %eline $kill
	search-forward " "
	set-mark
	search-forward ":"
	backward-character
	kill-region
	yank
	set %efile $kill

	;mark the line
	beginning-of-line
	insert-string "["
	end-of-line
	insert-string "]"
	beginning-of-line
	unmark-buffer

;	save the last file if different
	3 next-window
	!if &not &seq $lastfile ""
		!if &not &seq %lastfile %efile
			save-file
			select-buffer "[temp]"
			delete-buffer $lastfile
		!endif
	!endif
	find-file %efile
	%eline goto-line
	update-screen
	write-message &cat &cat &cat &cat "[Error found in " %efile " at line " %eline "]"
!endm

;	close the last error
store-procedure	closelast

	;if at the end.... go away
	2 next-window
	!if &seq $line ""
		!return
	!endif

	;unmark the last error
	beginning-of-line
	delete-next-character
	end-of-line
	delete-previous-character
	beginning-of-line

	;and save the file name
	set %lastfile %efile
!endm

;	Abort!
40	store-macro
	4 exit-emacs
!endm
	bind-to-key execute-macro-40	FN5

;	save current file

	bind-to-key save-file		FN9

;	recompile!!!!

	bind-to-key	quick-exit	FN0


;	set the help window

	1 split-current-window
	select-buffer "[MWC C-Errors]"
	insert-string "Mark Williams C			Compilation Error Scanning~n"
	insert-string "	F1	Go to next error	F9	Save current source file~n"
	insert-string "	F2	Go to last error	F10	Re-compile~n"
	insert-string "			F5	Abort compiler"
	beginning-of-file
	4 resize-window
	add-mode "red
	unmark-buffer

;	set up the error line window

	next-window
	1 split-current-window
	4 resize-window
	add-mode "black"
*getcc
	!if &not &seq &left $cbufname 2 "cc"
		next-buffer
		!goto getcc
	!endif

;	set up the current source window

	next-window
	next-buffer
	add-mode "blue

;	initialize

	set %lastfile ""
	run getnext
	set $gflags 0

;	wait for a command

	set $discmd "TRUE"
