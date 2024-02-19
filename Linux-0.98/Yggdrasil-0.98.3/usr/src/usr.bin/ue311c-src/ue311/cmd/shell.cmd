;	Shell.cmd:	MSDOS shell within MicroEMACS
;			written 1987 by Daniel Lawrence

	write-message "[Setting up SHELL window]"

store-procedure	getdir
	shell-command "cd > eshell"
	set %shtmp $cbufname
	!force delete-buffer eshell
	find-file eshell
	beginning-of-file
	set %shdir $line
	select-buffer %shtmp
	delete-buffer eshell
!endm

store-procedure prompt
	!if &not &equ $curcol 0
		newline
	!endif
	!if &not &seq $line ""
		open-line
	!endif
	insert-string &cat %shdir ">"
!endm

store-procedure getline
	beginning-of-line
	set %shtmp $curline
	!force search-forward ">"
	!if &not &equ $curline %shtmp
		set $curline %shtmp
	!endif
	!if &equ $curchar 10
		set %shline ""
	!else
		kill-to-end-of-line
		yank
		set %shline $kill
	!endif
	newline
!endm

store-procedure execline
	shell-command &cat %shline " > shtmp"
	!force insert-file shtmp
!endm

;	prompt and execute a command

4	store-macro
	run getline
	!if &not &seq %shline ""
		!if &or &seq &left %shline 2 "cd" &seq &right %shline 1 ":"
			shell-command %shline
			run getdir
		!else
			run execline
		!endif
	!endif
	run prompt
!endm

store-procedure checkmode
	!if &and %shmode &not &seq $cbufname "[I-SHELL]"
		set $discmd FALSE
		bind-to-key newline ^M
		add-mode blue
		add-mode WHITE
		write-message "[Exiting Shell window]"
		set %shmode FALSE
		set $discmd TRUE
	!endif
	!if &and &not %shmode &seq $cbufname "[I-SHELL]"
		set $discmd FALSE
		write-message "[Entering Shell window]"
		bind-to-key execute-macro-4 ^M
		run getdir
		add-mode black
		add-mode GREEN
		end-of-file
		run prompt
		set %shmode TRUE
		set $discmd TRUE
	!endif
!endm

;	window movement (and deactivate us)

5	store-macro
	next-window
	run checkmode
!endm

6	store-macro
	previous-window
	run checkmode
!endm

7	store-macro
	delete-window
	run checkmode
!endm

8	store-macro
	find-file @"Find file:"
	run checkmode
!endm

9	store-macro
	next-buffer
	run checkmode
!endm

store-procedure openshell
	set $discmd FALSE
	bind-to-key execute-macro-5 ^XO
	bind-to-key execute-macro-6 ^XP
	bind-to-key execute-macro-7 ^X0
	bind-to-key execute-macro-8 ^X^F
	bind-to-key execute-macro-9 ^XX
	select-buffer "[I-SHELL]"
	run checkmode
	set $discmd TRUE
!endm

	set %shmode FALSE
	run openshell
