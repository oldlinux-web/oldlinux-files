;	FUNC.CMD	Allow mainframes to simulate function
;			keys with ^C<n> and ^C shifted-<n>
;			Also allow the ` key to be META as well as ESC

bind-to-key execute-macro-21 ^C
bind-to-key meta-prefix `

21	store-macro
	!if &not $pending
		write-message "FN-"
	!endif
	set %rcchar &gtkey
	set %rcchar &sindex "1234567890!@#$%^&*()" %rcchar
	!if &equ %rcchar 0
		write-message "[Not Bound]"
		!return
	!endif
	clear-message-line
	set %rctmp "FN"
	!if &gre %rcchar 10
		set %rctmp &cat "S-" %rctmp
	!endif
	set %rcchar &mid "12345678901234567890" %rcchar 1
	execute-command-line &bind &cat %rctmp %rcchar
!endm
