;	CPAGE.CMD:	C language MENU Page
;			for MicroEMACS 3.9e and above
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
	delete-buffer "[drawbox]"
	delete-buffer "[setpoints]"
	delete-buffer "[findcom]"
	set $cmode %oldmode
	set $gmode %oldgmode
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
	overwrite-string " F1 PROCEDURE             F2 function           "
	next-line
	set $curcol 25
	overwrite-string " F3 if then else          F4 while              "
	next-line
	set $curcol 25
	overwrite-string " F5 REPEAT                F6 switch             "
	next-line
	set $curcol 18
	overwrite-string "C   "
	set $curcol 25
	overwrite-string " F7 VALUE                 F8 Reserved words UP  "
	next-line
	set $curcol 25
	overwrite-string " F9 chk comment matching F10 Comment block      "
	unmark-buffer
	beginning-of-file
	!force restore-window
	update-screen

;

10	store-macro
;	set $debug TRUE
        set %proc @"Procedure Name: "
        insert-string &cat &cat "Procedure " %proc "~n"
        insert-string "   ( VAR ~n"
        insert-string "   );~n"
        insert-string "~n"
        insert-string &cat &cat "{ (* " %proc " *)~n"
        insert-string &cat &cat "};  (* " %proc " *)~n"
	previous-line
        open-line
        insert-string "   "
	!force set $debug FALSE
!endm
bind-to-key execute-macro-10 S-FN1

;	Function
11	store-macro
        set %func @"Function Name: "
        set %type @"Return type: "
	insert-string %type
	!if &not &sequal &right %type 1 "*"
		insert-string " "
	!endif                            
        insert-string &cat %func "()~n~n{~n	~n}~n"
        3 backward-character
	!force set $debug FALSE
!endm
bind-to-key execute-macro-11 S-FN2

;

12	store-macro
        set %cond @"Condition Expr: "
        insert-string &cat &cat "if (" %cond ") {"
	newline-and-indent
	newline-and-indent
        insert-string "} else {"
	newline-and-indent
	newline-and-indent
       	insert-string &cat &cat "}  /* if (" %cond ") */"
	1 previous-line
        end-of-line
        handle-tab
	2 previous-line
        end-of-line
        handle-tab
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
	run findcom
!endm
bind-to-key execute-macro-18 S-FN9

store-procedure	findcom
;	This hunts down mismatched comment problems in C

;	start from the current position in the file

	!force search-forward "/*"
	!if &seq $status FALSE
		!goto nend
	!endif

*nxtopen
	;record the position of the open comment
	update-screen
	set %oline $curline
	set %opos $curcol

	;find the first close comment
	!force search-forward "*/"
	!if &seq $status FALSE
		write-message "%%No close to this open comment"
		!return
	!endif

	;record the position of the close comment
	set %cline $curline
	set %cpos $curcol

	;go back to the open and find the next open
	set $curline %oline
	set $curcol %opos

	;and now find the next open
	!force search-forward "/*"
	!if &seq $status FALSE
		write-message "No errors"
		!return
	!endif
	set %nline $curline
	set %npos $curcol

	;compare the close to the next open
	!if &less %cline %nline
		!goto getnext
	!endif
	!if &and &equ %cline %nline &less %cpos %npos
		!goto getnext
	!endif

	;report a mismatch
	set $curline %oline
	set $curcol %opos
	write-message "%%This comment does not terminate properly"
	!return

*getnext
	set $curline %nline
	set $curcol %npos
	!goto nxtopen 
!endm

19	store-macro
	set %c1 "/"
	set %c2 "*"
	set %c3 "\"
	set %c4 "\"
	set %c5 "/"
	set %c6 "*"
	run drawbox	
!endm
bind-to-key execute-macro-19 S-FN0
bind-to-key execute-macro-19 A-FN0

store-procedure drawbox
	run setpoints
	set $curline %mline
	set $curcol %mcol
;draw top horizontal line
	insert-string %c1
	set %width &add 2 &sub %pcol %mcol
	%width insert-string %c2
 	insert-string %c3
	newline-and-indent
;draw bottom horizontal line
	%pline goto-line
	next-line
	end-of-line
   	newline
	%mcol insert-string " "
	insert-string %c4
	%width insert-string %c2
	insert-string %c5
;bump pline 
	set %pline &add %pline 1
;draw verticals -- go to top and work our way down
	%mline goto-line
	!while &less $curline %pline
		next-line
		end-of-line
		!if &less $curcol %pcol
			&sub %pcol $curcol insert-string " "
		!endif
		set $curcol %pcol
		insert-string " "
		insert-string %c6
		set $curcol %mcol
		insert-string %c6
		insert-string " "
	!endwhile
;return to point
	%pline goto-line
	next-line
	beginning-of-line
	&add 6 %width forward-character
!endm

store-procedure setpoints
; procedure will set pcol, pline, mcol and mline. currently at point
; it will also detab the region
	set %pcol $curcol
	set %pline $curline
	exchange-point-and-mark
	set %mcol $curcol
	set %mline $curline
	exchange-point-and-mark
	detab-region
	set $curline %pline
	set $curcol %pcol
!endm
bind-to-key execute-macro-19 S-FN0

; Set up CMODE
	set %oldmode $cmode
	set %oldgmode $gmode
	add-mode CMODE
	add-global-mode CMODE

write-message "[C MENU page loaded]"
