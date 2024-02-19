;	SCAN.CMD:	MicroEMACS macro for scanning
;			SPELL.LST from MicroSPELL 1.0
;			(C)opyright 1987 by Daniel M Lawrence

;*****	Load up the needed procedures *****

store-procedure	getnext
;	Go to the next entry in the spell list

*next
	set %line &add #%slist 1
	set %col #%slist

	; user file name?
	!if &equ %line -2
		set %userlist %col
		!goto next
	!endif

	; if we are all done...
	!if &equ %line -1
		set %done TRUE
		!return
	!endif

	; if we are starting a new file.....
	!if &equ %line 0
		set %cfile %col
		write-message &cat &cat "[Reading " %cfile "]"
		find-file %cfile
		set %cbuffer $cbufname
		set %cfnum &add %cfnum 1
		!goto next
	!endif

	;reset column offset if needed
	!if &not &equ %lastline %line
		set %offset 0
	!endif
	set %lastline %line

	;go to it!!!
	set $curline %line
	&add %col %offset forward-character
	set %cwnum &add %cwnum 1
!endm

store-procedure doopt
;Macro for asking and resolving options

	;grab the word
	0 delete-next-word
	yank
	set %word $kill
!force	previous-word

	;next check to see if this should be ignored....
	select-buffer "[ignore]
	beginning-of-file
!force	search-forward &cat &cat "~n" %word "~n"
	set %sstatus $status
	select-buffer %cbuffer
	!if &seq %sstatus TRUE
		!return
	!endif

	;see if this word has been replaced in the past
	select-buffer "[source]
	beginning-of-file
!force	search-forward &cat &cat "~n" %word "~n"
	set %sstatus $status

	!if &seq %sstatus TRUE
		set %repline &sub $curline 2
		select-buffer "[replace]"
		set $curline %repline
		set %defrep #"[replace]"
		select-buffer %cbuffer
		set %defflag "*"
		;check to see if this is a global replacement
		!if &equ &asc %defrep 19
			set %defrep &mid %defrep 2 255
			!goto globalrep
		!endif
	!else
		select-buffer %cbuffer
		set %defrep ""
		set %defflag ""
	!endif

*askopt
	run upstat	;update the dialog window
	clear-message-line
	1 next-window		;skip to the dialog window
	set $curline 3		;update the current word & default
	3 redraw-display
	set $curcol 21
	20 delete-next-character
	&sub 20 &len %word insert-space
	insert-string %word
	set $curline 4
	set $curcol 21
	20 delete-next-character
	&sub 20 &len %defrep insert-space
	insert-string %defrep

	set $curline 6
	15 delete-next-character
	insert-string "       Option: "
	beginning-of-file
	2 next-window
	update-screen
	set %resp &gtkey
	1 next-window
	set $curline 6
	15 delete-next-character
	15 insert-space
	2 next-window
	update-screen

	;skip just this occurence....
	!if &seq %resp "s"
		!return
	!endif

	;Ignore this mismatch and all like it
	!if &seq %resp "i"
		select-buffer "[ignore]"
		end-of-file
		insert-string %word
		select-buffer %cbuffer
		!return
	!endif

	;Replace this string with the default string
	!if &seq %resp "d"
*globalrep
		0 delete-next-word
		insert-string %defrep
		update-screen
		set %offset &add &sub %offset &len %word &len %repstring
		!return
	!endif

	;Replace this string with another.....
	!if &or &seq %resp "c" &seq %resp "g"
		set $discmd TRUE
		set %repstring ""
		!if &seq %resp "g"
			set %repstring "Globally "
		!endif
		!if &seq %defflag ""
			set %repstring @&cat %repstring "Replace with: "
		!else
			set %repstring @&cat &cat &cat %repstring "Replace with[" %defrep "]: "
			!if &seq %repstring ""
				set %repstring %defrep
			!endif
		!endif
		set $discmd FALSE
		0 delete-next-word
		insert-string %repstring
		update-screen
		set %offset &add &sub %offset &len %word &len %repstring
		!if &seq %defflag ""
			select-buffer "[source]"
			end-of-file
			insert-string %word
			select-buffer "[replace]"
			end-of-file
			;if it is a global replace, add this marker
			!if &seq %resp "g"
				insert-string &chr 19
			!endif
			insert-string %repstring
			select-buffer %cbuffer
		!endif
		!return
	!endif

	;if we are adding this word and it is not to upper case,
	;...lower case it
	!if &seq %resp "a"
		set %resp "u"
		set %word &lower %word
	!endif

	;Add this word to a user dictionary and ignore it
	!if &seq %resp "u"
		select-buffer "[ignore]"
		end-of-file
		insert-string %word
		select-buffer "[add]"
		end-of-file
		insert-string %word
		select-buffer %cbuffer
		set %addflag TRUE
		!return
	!endif

	;Abort the spell check run
	!if &or &seq %resp "" &seq %resp "q"
		set %done ABORT
		!return
	!endif

	write-message ""
	!goto askopt
!endm

store-procedure cleanup
;	Add the requested user words to a user dictionary

	!if &seq %addflag TRUE
		!if &seq %userlist ""
			set %userlist @"User dictionary name: "
		!endif
		write-message "[Updating User Dictionary]"
	!force	find-file %userlist
		set %bdict $cbufname
		select-buffer "[add]"
		beginning-of-file
		set-mark
		end-of-file
		kill-region
		select-buffer %bdict
		end-of-file
		yank
		delete-blank-lines
		save-file
	!endif

	select-buffer "[ignore]"
	unmark-buffer
	select-buffer "[add]"
	unmark-buffer
	select-buffer "[source]"
	unmark-buffer
	select-buffer "[replace]"
	unmark-buffer
	select-buffer "Dialog Window"
	unmark-buffer

	select-buffer %cbuffer
	beginning-of-file
	update-screen
!endm

store-procedure upstat
;	update the dialog window stats

	1 next-window
	beginning-of-file
	set $curcol 44
	&len %cfnum delete-previous-character
	insert-string %cfnum
	set $curcol 48
	&len %tfnum delete-previous-character
	insert-string %tfnum
	set $curcol 61
	&len %cwnum delete-previous-character
	insert-string %cwnum
	set $curcol 67
	&len %twnum delete-previous-character
	insert-string %twnum
	next-window
!endm

;	init for spell check run

	add-global-mode "blue"
	add-mode "blue"

!force	find-file "spell.lst"
	!if &not &seq $status TRUE
		write-message "[No file to scan]~n"
		exit-emacs
	!endif
	write-message "[Preparing to Scan]"

	select-buffer "[source]"
	unmark-buffer
	select-buffer "[replace]"
	delete-buffer "[source]"
	unmark-buffer
	select-buffer "[add]"
	delete-buffer "[replace]"
	unmark-buffer
	select-buffer "[ignore]"
	delete-buffer "[add]"
	unmark-buffer
	select-buffer "[add]"
	delete-buffer "[ignore]"
	select-buffer "[source]"
	newline
	select-buffer "[replace]"
	select-buffer "[ignore]"
	newline

	set $discmd FALSE
	set %slist "spell.lst"
	set %userlist ""
	set %done FALSE
	set %cfile ""
	set %cbuffer ""
	set %lastline -1
	set %addflag FALSE
	delete-other-windows
	select-buffer "spell.lst"
	beginning-of-file

	;init data for the dialog window
	set %cfnum 0
	set %tfnum 0
*nxtfile
!force	search-forward "-1~n"
	!if &seq $status TRUE
		set %tfnum &add %tfnum 1
		!goto nxtfile
	!endif
	set %cwnum 0
	end-of-file
	set %twnum &sub &sub &div &sub $curline 1 2 %tfnum 1
	beginning-of-file

	;set up the dialog window
	1 split-current-window
	7 resize-window
	add-mode "red"
	select-buffer "Dialog Window"
	beginning-of-file
	set-mark
	end-of-file
	kill-region
	insert-string "MicroSPELL 1.0 spell scan           File    /      Word      /     ~n~n"
	insert-string "       Suspect word:                        (S)kip         (C)hange word~n"
	insert-string "Default Replacement:                        (I)gnore       (D)efault replace~n"
	insert-string "                                            (A)dd word     (G)lobal change~n"
	insert-string "                                Add word as (U)pper case   (Q)uit"
	beginning-of-file
	next-window
	clear-message-line

*nxtword

	run getnext
	!if &seq %done TRUE
		!goto alldone
	!endif
	run doopt
	!if &seq %done FALSE
		!goto nxtword
	!endif

*alldone

	!if &seq %done "ABORT"
		;edit up the spell.lst file so we could continue..
		select-buffer "spell.lst"
		2 previous-line
		set-mark
		beginning-of-file
		kill-region
		insert-string &cat &cat "-1~n" %cfile "~n"
		save-file
		select-buffer %cbuffer
		run cleanup
		write-message "[Spell check ABORTED]"
		!if &not &seq %rcspell TRUE
			set $discmd TRUE
			save-file
			update-screen
			exit-emacs
		!endif
	!else
		run cleanup
		;MACHINE DEPENDANT!!!!!!!!!!!!
		shell-command "del spell.lst"
		write-message "[Spell check run complete]~n"
		set $discmd TRUE
		!if &not &seq %rcspell TRUE
			quick-exit
		!endif
	!endif
	set $discmd TRUE
!return
