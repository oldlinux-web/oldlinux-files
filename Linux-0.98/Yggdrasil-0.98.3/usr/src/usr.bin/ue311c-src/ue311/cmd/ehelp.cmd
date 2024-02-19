store-procedure	init-help

	clear-message-line
	print "                [Loading Help System]"
	set %oldscreen $scrname		;remember the screen we were on
	set %oldmode $modeflag		;remember the original $modeflag
	set $discmd FALSE
	find-screen "HELP"		;switch to the help screen

	set %oldres $sres
	!if &or &or &or &seq $sres "EGA" &seq $sres "VGA" &seq $sres "CGA40" &seq $sres "VGA12"
		set $sres "CGA"
	!endif

	!if &seq $sres "CGA"
		set $orgrow 0
		set $orgcol 0
		set $curwidth 80
		set $pagelen 25
	!endif

	set $modeflag FALSE
	set %done FALSE			;not done with help
	view-file &find ehelp1.txt	;read in the help file!
	set %helpfile 1			;mark which help file we are in
	delete-mode "view"		;don't lock the help file
	sethscreen
	set %helpscreen "Main Menu"	;start at the main menu
	gethscreen
!endm

store-procedure gethscreen	;switch the current help screen

*gethbegin

	;build the window ID line
	1 next-window
	beginning-of-file
	set-mark
	end-of-file
	kill-region
	!if &gre &len $version 5
		insert-string "MicroEMACS version "
	!else
		insert-string "	MicroEMACS version "
	!endif
	insert-string $version
	insert-string "		Help System	"
	insert-string %helpscreen
	beginning-of-line
	unmark-buffer

	;find and display the named helpscreen
	2 next-window
	beginning-of-file
	set %tmp &cat "~n=>" %helpscreen
	!force search-forward %tmp
	!if &seq $status FALSE
		!if &equ %helpfile 1
			view-file &find ehelp2.txt ;read in the help file!
			set %helpfile 2		;mark which help file we are in
		!else
			view-file &find ehelp1.txt ;read in the help file!
			set %helpfile 1		;mark which help file we are in
		!endif
		delete-mode "view"		;don't lock the help file
			
		beginning-of-file
		!force search-forward %tmp
		!if &seq $status FALSE
			set %helpscreen "Main Menu"
			print "          [Attempt to fetch illegal screen]"
			!goto gethbegin
		!endif
	!endif
	1 next-line
	beginning-of-line
	!if &equ $curchar 61
		1 forward-character
		set-mark
		end-of-line
		copy-region
		set %helpscreen $kill
		!goto gethbegin
	!endif
	1 next-line
	beginning-of-line
	1 redraw-display
	update-screen
	set %topline $curline
	set %bottomline &add $curline 20

	firstsel	;position us at the first selection

!endm

store-procedure sethscreen	;set up the windows on the help screen

	find-screen "HELP"
	delete-other-windows
	beginning-of-file
	split-current-window
	0 resize-window
	add-mode "red"
	add-mode WHITE
	select-buffer "Window ID"
	next-window
	split-current-window
	19 resize-window
	add-mode "blue"
	add-mode WHITE
	next-window
	add-mode "red"
	add-mode WHITE
	beginning-of-file
	!if &les $pagelen 25
		4 next-line
		1 redraw-display
	!endif
!endm

store-procedure firstsel	;position us at the first selection

	9 set-mark		;remember where we started from
	set %endscreen &add $curline 20
	search-forward "< >"	;look for a selection
	!if &gre $curline %endscreen

		;no selections on this screen!
		9 goto-mark
		set %selflag FALSE
	!else
		2 backward-character
		set %selflag TRUE
		8 set-mark
	!endif

!endm

store-procedure setsel		;highlight the current selection

	!if &seq %selflag TRUE
		delete-next-character
		insert-string "*"
		backward-character
		unmark-buffer
	!endif
!endm

store-procedure clearsel		;clear the current selection

	!if &seq %selflag TRUE
		delete-next-character
		insert-string " "
		backward-character
		unmark-buffer
	!endif
!endm

store-procedure nextcmd

	!if &seq $pending FALSE
		update-screen
	!endif
	set %cmd &gtcmd
	set %cmd &bind %cmd
	clear-message-line

!endm

store-procedure execloop	;execute a user screen

	setsel		;mark the current selection
	nextcmd		;get the command
	clearsel	;clear the selection
	execcmd		;execute the command

!endm

store-procedure execcmd		;execute the current command

	!if &seq %cmd "set"
		!if &seq $debug TRUE
			set $debug FALSE
		!else
			set $debug TRUE
		!endif
		!return
	!endif

	!if &seq %selflag TRUE

		!if &seq %cmd "forward-character"
			forward-selection
			!return
		!endif

		!if &seq %cmd "backward-character"
			backward-selection
			!return
		!endif

		!if &seq %cmd "next-line"
			next-selection
			!return
		!endif

		!if &seq %cmd "previous-line"
			previous-selection
			!return
		!endif

		!if &seq %cmd "newline"
			select-this
			!return
		!endif

	!endif

	!if &seq %cmd "next-page"
		next-hscreen
		!return
	!endif

	!if &seq %cmd "newline"
		next-hscreen
		!return
	!endif

	!if &seq %cmd "previous-page"
		previous-hscreen
		!return
	!endif

	!if &seq %cmd &bind "FN6"
		set %helpscreen "Index"
		gethscreen
		!return
	!endif

	!if &seq %cmd "beginning-of-file"
		set %helpscreen "Main Menu"
		gethscreen
		!return
	!endif

	!if &seq %cmd "exit-emacs"
		exit-help
		!return
	!endif

	!if &seq %cmd &bind "MSa"
		select-mouse
		!return
	!endif

	!if &seq %cmd &bind "MSb"
		!return
	!endif

	!if &seq %cmd "abort-command"
		print "[Help Aborted right HERE!]"
		stop
	!endif

	print "              [Not a legal command at this time]"

!endm

store-procedure forward-selection

	search-forward "< >"
	!if &gre $curline %bottomline
		set $curline %topline
		search-forward "< >"
	!endif
	2 backward-character
	8 set-mark

!endm

store-procedure backward-selection

	search-reverse "< >"
	!if &less $curline %topline
		set $curline %bottomline
		search-reverse "< >"
	!endif
	1 forward-character
	8 set-mark

!endm

store-procedure next-selection

	set %target $curcol
	forward-selection
	!while &not &equ %target $curcol
		forward-selection
	!endwhile

!endm

store-procedure previous-selection

	set %target $curcol
	backward-selection
	!while &not &equ %target $curcol
		backward-selection
	!endwhile

!endm

store-procedure select-this	;select the currently highlighted selection

	next-word		;jump to the beginning of the selection
	7 set-mark		;mark it so we can come back!
	set-mark
	set %tmpline $curline	;remember what line we are on
	search-forward "< >"	;find the beginning of the next selection
	!if &not &equ %tmpline $curline

		;we are on a new line, try for a <tab>!
		7 goto-mark
		search-forward "~t"
		!if &not &equ %tmpline $curline

			;we are no longer on the same line, select the whole line
			7 goto-mark
			end-of-line
		!else
			;backup over the tab
			1 backward-character
		!endif
	!else

		;select up to this selection
		4 backward-character
	!endif

	;get the selection
	copy-region
	set %selection &trim $kill
;	print &cat &cat "[selecting :" %selection ":]"
	set %helpscreen %selection
	gethscreen
!endm

store-procedure select-mouse

	9 set-mark
	mouse-move-down

	!if &and &gre $ypos 20 &less $ypos 24

		; a mouse click on a key in the key window/find the key
		1 forward-character
		!force search-reverse "["
		!if &seq $status FALSE
			search-forward "["
		!else
			1 forward-character
		!endif
		set-mark
		search-forward "]"
		1 backward-character
		copy-region
		2 next-window
		set %cmd $kill

		!if &and &seq %selflag TRUE &seq %cmd "ENTER"
			select-this
			!return
		!endif

		!if &seq %cmd "F10" 
			exit-help
			!return
		!endif

		!if &seq %cmd "I"
		!endif

		!if &seq %cmd "PG UP"
			previous-hscreen
			!return
		!endif

		!if &seq %cmd "PG DOWN"
			next-hscreen
			!return
		!endif

		!if &seq %cmd "F6"
			set %helpscreen "Index"
			gethscreen
			!return
		!endif

		!if &seq %cmd "HOME"
			set %helpscreen "Main Menu"
			gethscreen
			!return
		!endif
	!endif

	!if &or &seq %selflag FALSE &or &gre $ypos 24 &equ $ypos 1
		2 next-window
		print "              [Not a legal command at this time]"
		9 goto-mark
		!return
	!endif

	3 forward-character
	backward-selection
	select-this

!endm

store-procedure next-screen

	search-reverse "NEXT["
	set-mark
	search-forward "]"
	backward-character
	copy-region
	set %helpscreen $kill
	gethscreen

!endm

store-procedure next-hscreen

	search-reverse "NEXT["
	5 forward-character
	set-mark
	search-forward "]"
	backward-character
	copy-region
	set %helpscreen $kill
	gethscreen

!endm

store-procedure previous-hscreen

	search-reverse "PREV["
	5 forward-character
	set-mark
	search-forward "]"
	backward-character
	copy-region
	set %helpscreen $kill
	gethscreen

!endm

store-procedure exit-help

	!if &not &seq $sres %oldres
		set $sres %oldres
	!endif
	find-screen %oldscreen
	set $modeflag %oldmode
	delete-buffer "[init-help]"
	delete-buffer "[gethscreen]"
	delete-buffer "[sethscreen]"
	delete-buffer "[firstsel]"
	delete-buffer "[setsel]"
	delete-buffer "[clearsel]"
	delete-buffer "[nextcmd]"
	delete-buffer "[forward-selection]"
	delete-buffer "[backward-selection]"
	delete-buffer "[next-selection]"
	delete-buffer "[previous-selection]"
	delete-buffer "[select-this]"
	delete-buffer "[next-screen]"
	delete-buffer "[next-hscreen]"
	delete-buffer "[previous-hscreen]"
	delete-screen HELP
	!force delete-buffer ehelp1.txt
	!force delete-buffer ehelp2.txt
	set $discmd TRUE
	set %done TRUE

!endm

;	Help main program

init-help
!while &not %done
	execloop
!endwhile

delete-buffer "[select-mouse]"
delete-buffer "[execloop]"
delete-buffer "[execcmd]"
delete-buffer "[exit-help]"
