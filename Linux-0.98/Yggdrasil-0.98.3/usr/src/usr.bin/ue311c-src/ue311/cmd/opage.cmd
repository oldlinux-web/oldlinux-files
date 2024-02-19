;	OPAGE.CMD:	COBOL language MENU Page
;			for MicroEMACS 3.11 and above
;			(C)opyright 1988 by Daniel Lawrence
;

; set up the "clean" procedure
store-procedure clean
	delete-buffer "[Macro 10]"
	delete-buffer "[Macro 11]"
	delete-buffer "[Macro 12]"
	delete-buffer "[Macro 13]"
	delete-buffer "[Macro 14]"
	delete-buffer "[Macro 15]"
	delete-buffer "[Macro 16]"
	delete-buffer "[Macro 17]"
	delete-buffer "[Macro 18]"
	delete-buffer "[Macro 19]"
	bind-to-key handle-tab ^I
	delete-buffer "[cobol-tab]"
	set $cmode %oldmode
	set $gmode %oldgmode
	0 handle-tab
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
	overwrite-string " F1 Make IDENT division   F2 UPPERcase buffer   "
	next-line
	set $curcol 25
	overwrite-string " F3 Make ENVIR division   F4 while              "
	next-line
	set $curcol 25
	overwrite-string " F5 REPEAT                F6 switch             "
	next-line
	set $curcol 18
	overwrite-string "COBL"
	set $curcol 25
	overwrite-string " F7 VALUE                 F8 Reserved words UP  "
	next-line
	set $curcol 25
	overwrite-string " F9 chk comment matching F10 Shell to DOS       "
	unmark-buffer
	beginning-of-file
	!force restore-window
	update-screen

;

10	store-macro
	beginning-of-file

	set %rctemp @"Program Name: "
	insert-string "       IDENTIFICATION DIVISION.~n"
	insert-string &cat &cat "       PROGRAM-ID. " %rctemp ".~n"
	update-screen
	set %rctemp @"Author: "
	insert-string &cat &cat "       AUTHOR. " %rctemp ".~n"
	insert-string &cat &cat "       DATE-WRITTEN. " $time ".~n"
	insert-string "       DATE-COMPILED.~n"
	insert-string "       REMARKS.~n~n"
!endm
bind-to-key execute-macro-10 S-FN1

;	Function
11	store-macro
	9 set-mark
	beginning-of-file
	set-mark
	end-of-file
	case-region-upper
	9 goto-mark
!endm
bind-to-key execute-macro-11 S-FN2

;

12	store-macro
	insert-string "       ENVIRONMENT DIVISION.~n"
	insert-string "       CONFIGURATION SECTION.~n"
	insert-string "       SOURCE-COMPUTER. IBM-3090.~n"
	insert-string "       OBJECT-COMPUTER. IBM-3090.~n"
	insert-string "       SPECIAL-NAMES.~n"
	insert-string "       INPUT-OUTPUT SECTION.~n"
	insert-string "       FILE-CONTROL.~n"
!endm
bind-to-key execute-macro-12 S-FN3

;

13	store-macro
        set %cond @"Condition Expr: "
	;set $debug TRUE
        insert-string &cat &cat "while (" %cond ") {"
	newline-and-indent
	newline-and-indent
        insert-string &cat &cat "}  /* while (" %cond ") */"
	1 previous-line
        end-of-line
	handle-tab
!endm
bind-to-key execute-macro-13 S-FN4

14	store-macro
        set %cond @"Condition Expr: "
	;set $debug TRUE
        set %curcol $curcol
        insert-string &cat &cat "REPEAT~n"
        %curcol insert-string " "
        set $curcol %curcol
        !if &EQUal  %curcol 3
	        insert-string &cat &cat "UNTIL " %cond ";"
	!else
	        insert-string &cat      "UNTIL " %cond 
	!endif
        beginning-of-line
        open-line
        %curcol insert-string " "
        insert-string "   "
	!force set $debug FALSE
!endm
bind-to-key execute-macro-14 S-FN5

15	store-macro
        set %expr @"Switch Expr: "
	;set $debug TRUE
        set %curcol $curcol
        insert-string &cat &cat "switch (" %expr ") {"
	newline-and-indent
	insert-string "}"
	previous-line
	end-of-line
	newline-and-indent
	handle-tab
	insert-string "case "
	!force set $debug FALSE
!endm
bind-to-key execute-macro-15 S-FN6

16	store-macro
;	set $debug TRUE
        set %curcol $curcol
        set %value @"Value: "
        insert-string &cat %value " : {~n"
        %curcol insert-string " "
        insert-string "    };~n"
        %curcol insert-string " "
	previous-line
        beginning-of-line
        open-line
        %curcol insert-string " "
        insert-string "       "
	!force set $debug FALSE
!endm
bind-to-key execute-macro-16 S-FN7

17	store-macro
;	set $debug TRUE
        set %curcol $curcol
	;save-file
        set-mark
	beginning-of-file
        write-message "[Replacing 'begin']
	replace-string "begin" "BEGIN"

	beginning-of-file
        write-message "[Replacing 'end']
	replace-string "end" "END"

	beginning-of-file
        write-message "[Replacing ' if']
	replace-string " if" " IF"

	beginning-of-file
        write-message "[Replacing ' else']
	replace-string " else" " ELSE"

	beginning-of-file
        write-message "[Replacing ' repeat']
	replace-string " repeat" " REPEAT"

	beginning-of-file
        write-message "[Replacing ' while']
	replace-string " while" " WHILE"

        exchange-point-and-mark
	!force set $debug FALSE
!endm
bind-to-key execute-macro-17 S-FN8

18	store-macro
	beginning-of-file
	source findcom.cmd
!endm
bind-to-key execute-macro-18 S-FN9

19	store-macro
	i-shell
!endm
bind-to-key execute-macro-19 S-FN0

;	This macro assumes we are on an 01 entry line and
;	moves all the datanames to the DATANAMES buffer

store-procedure getrecnames
	set %orbuf $cbufname
	!force delete-buffer DATANAMES
	9 set-mark
*grn	beginning-of-line
	next-line
	8 forward-character
	!if &not &equ $curchar 32 
		9 goto-mark
		write-message "[data names recorded]"
		!return
	!endif

!endm

; modify the tabbing behavior to match COBOL

store-procedure	cobol-tab
	!if &less $curcol 7
		set %rctmp &sub 7 $curcol
	!else
		set %rctmp &sub &add 3 &band 252 &add 1 $curcol $curcol
	!endif
	%rctmp insert-space
	%rctmp forward-character
!endm
macro-to-key cobol-tab ^I

; Set up CMODE
	set %oldmode $cmode
	set %oldgmode $gmode
	add-mode CMODE
	add-global-mode CMODE
	4 handle-tab

write-message "[COBOL MENU page loaded]"
