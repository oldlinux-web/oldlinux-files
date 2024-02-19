;	NEWPAGE.CMD:	Startup page Loader
;			for MicroEMACS 3.9d and above
;			(C)opyright 1987 by Daniel M Lawrence

; Get rid of the last page
	set $discmd FALSE
	write-message [Cleaning]
	!force run clean

!force	delete-buffer "[clean]"

; make sure the function key window is up
	set %rcfkeys FALSE
	toggle-fkeys

!if &not $pending

	; Write out the page load instructions
	save-window
	1 next-window
	beginning-of-file
	set $curcol 25
	overwrite-string "           Available Pages to Load:                 "
	next-line
	set $curcol 25
	overwrite-string "    [W]  WORDprocessing  [P]  Pascal  [L] Lisp      "
	next-line
	set $curcol 25
	overwrite-string "    [E]  EMACS macroes   [C]  C                     "
	next-line
	set $curcol 18
	overwrite-string "    "
	set $curcol 25
	overwrite-string "    [B]  Block editing   [O]  Cobol                 "
	next-line
	set $curcol 25
	overwrite-string "[space]  to load a user page                        "
	unmark-buffer
	beginning-of-file
	!force restore-window
	update-screen


; prompt for the page

	write-message "Page to load: "
!endif

*prompt
	set %rctmp &gtcmd
	clear-message-line

; check for mouse press

	!if &seq &left %rctmp 2 "MS"
		save-window
		!force mouse-move-down

		;eat the mouse up
		set %junk &gtcmd

		;If not in the function key window... don't bother
		!if &not &sequal $cbufname "Function Keys"
			restore-window
			!goto prompt
		!endif

		forward-character
		!force search-reverse "["
		!if &seq $status FALSE
			restore-window
			!goto prompt
		!endif

		!force forward-character
		set %rctmp &chr $curchar
		!if &seq %rctmp "s"
			set %rctmp " "
		!endif
		set %rctmp &lower %rctmp
		restore-window
	!endif

; check for an abort
	!if &seq %rctmp "^G"
		write-message "[Aborted]"
		save-window
		1 next-window
		beginning-of-file
		set $curcol 25
		overwrite-string "   MicroEMACS:  Text Editor                         "
		next-line
		set $curcol 25
		overwrite-string "                                                    "
		next-line
		set $curcol 25
		overwrite-string "  Available function key Pages include:             "
		next-line
		set $curcol 25
		overwrite-string "    WORD  BOX  EMACS  PASCAL  C                     "
		next-line
		set $curcol 25
		overwrite-string "  [use the f8 key to load Pages]                    "
		unmark-buffer
		beginning-of-file
		!force restore-window
		set $discmd TRUE
		!return
	!endif
	
; if it is an unlisted page, get it's name
	!if &seq %rctmp " "
		set %rcfile &cat @"Name of Page to load: " ".cmd"
	!else
		set %rcfile &cat %rctmp "page.cmd"
	!endif

; see if this is a legit file
	!if &seq &find %rcfile ""
		write-message "%No Such Page, Page to load: "
		!goto prompt
	!endif

;and lastly, execute it

	execute-file %rcfile
	set $discmd TRUE
