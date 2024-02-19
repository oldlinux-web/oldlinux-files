;	LPAGE.CMD:	LISP language MENU Page
;			for MicroEMACS 3.11 and above
;			(C)opyright 1989 by Gregory Wilcox
;			modified by Daniel Lawrence

; set the clean procedure up
store-procedure clean
	delete-buffer "[Macro 10]"
	delete-buffer "[Macro 11]"
	delete-buffer "[Macro 12]"
	delete-buffer "[Macro 13]"
	delete-buffer "[Macro 14]"
	delete-buffer "[Macro 15]"
	delete-buffer "[Macro 16]"
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
	overwrite-string " F1 forward sexpr          F2 previous sexpr        "
	next-line
	set $curcol 25
	overwrite-string " F3 forward function       F4 previous function     "
	next-line
	set $curcol 25
	overwrite-string " F5 mark sexpr             F6 indent sexpr          "
	next-line
	set $curcol 18
	overwrite-string "LISP"
	set $curcol 25
	overwrite-string " F7 find unmatched parens  F8                       "
	next-line
	set $curcol 25
	overwrite-string " F9                       F10                       "
	unmark-buffer
	beginning-of-file
	!force restore-window
	update-screen

set %tab 9
set %newline 13
set %space 32
set %lparen 40
set %rparen 41
set %indent "  " 

; forward-sexpr
; this function (and its twin, backward-sexpr) only work on lists -
; it would be difficult to make them work on atoms,
; since forward-word and backward-word skip over parentheses
10	store-macro
	!if &not &equal $curchar %lparen
		search-forward "("
		backward-character
	!endif
	goto-matching-fence	
!endm

; backward-sexpr
; unfortunately, this won't work on a top-level sexpr
; there must be some stupid C rule about braces not allowed in column 1
; thus, when you goto-matching-fence, it can't be found
; hence the !force and following !if clause
11	store-macro
	!if &not &equal $curchar %rparen
		search-reverse ")"
	!endif
	!force goto-matching-fence	
	!if &seq $status FALSE
		search-reverse "~n("
	!endif
!endm

; forward-function
12	store-macro
	!force search-forward "~n("
	!if &seq $status FALSE
		end-of-file
		write-message "Last function"
	!else
		backward-character
	!endif
!endm

; backward-function
13	store-macro
	!force search-reverse "~n("
	!if &seq $status FALSE
		beginning-of-file
		write-message "First function"
	!else
	forward-character
	!endif
!endm

; mark-sexpr
14	store-macro
	!if &not &equal $curchar %lparen
		search-reverse "("
	!endif
	set-mark
	goto-matching-fence
	forward-character
	exchange-point-and-mark
!endm

; indent-sexpr
15	store-macro
	write-message "Indenting S-expression. . ."
	forward-character
	execute-macro-26  ; backward-function
	; are we really here?
	!if &not &equal $curchar %lparen
		write-message "Can't find S-expression."
		!return
	!endif
	set %parens 0
*nest
	!if &equal $curchar %lparen
		; if sexpr is on one line, swallow it
		set %cline $curline
		set %ccol $curcol
		goto-matching-fence
		!if &not &equal %cline $curline
			set $curline %cline
			set $curcol %ccol
			set %parens &add %parens 1
		!endif
	!else
		!if &equal $curchar %rparen
			set %parens &sub %parens 1
		!endif
	!endif
	!if &equal $curchar %newline
		forward-character
		; trim leading whitespace
		!while &or &equ $curchar %space &equ $curchar %tab
			delete-next-character
		!endwhile		
		; now do the indenting
		set %count %parens
		!while &not &equ %count 0
			insert-string %indent
			set %count &sub %count 1
		!endwhile
		backward-character
	!endif
	forward-character
	!if &not &equ %parens 0
		!goto nest
	!endif
	write-message "Finished."
!endm

; find-unbalanced-parentheses
16	store-macro
	; save cursor position
	set %cline $curline
	set %ccol $curcol
	write-message "Searching forward for mismatched parentheses. . ."
	beginning-of-file
*loop	
	!force search-forward "("
	!if &seq $status FALSE 
		!goto misup
	!endif
	backward-character
	!force goto-matching-fence
	!if &seq $status FALSE
		write-message "Too few close parentheses in this function."
		!return
	!endif
	!goto loop
*misup
	write-message "Searching backward for mismatched parentheses. . ."
	end-of-file
*loopr	
	!force search-reverse ")"
	!if &seq $status FALSE 
		!goto ok 
	!endif
	; forward-character not needed - asymmetry here
	!force goto-matching-fence
	!if &seq $status FALSE
		write-message "Too many close parentheses in this function."
		!return
	!endif
	!goto loopr
*ok	
	write-message "All parentheses appear balanced."
	; restore cursor position
	set $curline %cline
	set $curcol %ccol
!endm

bind-to-key execute-macro-22 A-F	; forward-sexpr
bind-to-key execute-macro-23 A-B	; backward-sexpr
bind-to-key execute-macro-24 ^@		; mark-sexpr
bind-to-key execute-macro-25 A-E	; forward-function
bind-to-key execute-macro-26 A-A	; backward-function
bind-to-key execute-macro-27 A-I	; indent-sexpr
bind-to-key execute-macro-28 A-U	; find-unmatched-parentheses

write-message "[LISP page loaded]"
