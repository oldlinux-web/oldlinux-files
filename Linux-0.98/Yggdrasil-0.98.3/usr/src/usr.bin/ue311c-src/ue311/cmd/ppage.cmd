;	PPAGE.CMD:	Pascal MENU Page
;			for MicroEMACS 3.9e and above
;			(C)opyright 1988 by Ian Ornstein
;			2-May-1988
;
;    This page was inspired by functionally similar code written
;    for the ASE editor for the p-System by:
;	Dick Karpinski  Manager of Unix Services, UCSF Computer Center
;	UUCP:  ...!ucbvax!ucsfcgl!cca.ucsf!dick        (415) 476-4529 (11-7)
;	BITNET:  dick@ucsfcca or dick@ucsfvm           Compuserve: 70215,1277  
;	USPS:  U-76 UCSF, San Francisco, CA 94143-0704   Telemail: RKarpinski   
;    I think it is only fair to give him the credit for the inspiration.
;    - Ian -                                  
;
;	[I cleaned this up PC and added DOS shelling, CMODE for
;	 fence matching and indentation and put it in the distribution
;			- dan]

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
	overwrite-string " F1 PROCEDURE             F2 FUNCTION           "
	next-line
	set $curcol 25
	overwrite-string " F3 IF Then ELSE          F4 WHILE              "
	next-line
	set $curcol 25
	overwrite-string " F5 REPEAT                F6 CASE               "
	next-line
	set $curcol 18
	overwrite-string "PASC"
	set $curcol 25
	overwrite-string " F7 VALUE                 F8 Reserved words UP  "
	next-line
	set $curcol 25
	overwrite-string " F9 Reserve words CAPS   F10 Shell to DOS       "
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
;       insert-string "CONST~n"
;       insert-string "~n"
;       insert-string "TYPE~n"
;       insert-string "~n"
;       insert-string "VAR~n"
;       insert-string "~n"
        insert-string &cat &cat "BEGIN (* " %proc " *)~n"
        insert-string &cat &cat "END;  (* " %proc " *)~n"
	previous-line
        open-line
        insert-string "   "
	!force set $debug FALSE
!endm
bind-to-key execute-macro-10 FNC

;	Function
11	store-macro
        set %func @"Function Name: "
        insert-string &cat &cat "Function " %func "~n"
        insert-string "   ( VAR ~n"
        insert-string "   )        : BOOLEAN;~n
        insert-string "~n"
        insert-string &cat &cat "BEGIN (* " %func " *)~n"
        insert-string &cat &cat "END;  (* " %func " *)~n"
	previous-line
        open-line
        insert-string "   "
	!force set $debug FALSE
!endm
bind-to-key execute-macro-11 FND

;

12	store-macro
        set %curcol $curcol
        set %cond @"Condition Expr: "
;	set $debug TRUE
        insert-string &cat &cat "IF  " %cond "~n"
        %curcol insert-string " "
        set $curcol %curcol
        insert-string "THEN~n"
        %curcol insert-string " "
        set $curcol %curcol
        insert-string "    BEGIN~n"
        %curcol insert-string " "
        set $curcol %curcol
        insert-string &cat &cat "    END  (* TRUE " %cond " *)~n"
        %curcol insert-string " "
        set $curcol %curcol
        insert-string "ELSE~n"
        %curcol insert-string " "
        set $curcol %curcol
        insert-string "    BEGIN~n"
        %curcol insert-string " "
        set $curcol %curcol
        !if &EQUal  %curcol 3
        	insert-string &cat &cat "    END; (* FALSE " %cond " *)"
	!else
        	insert-string &cat &cat "    END  (* FALSE " %cond " *)"
	!endif
	3 previous-line
        beginning-of-line
        open-line
        %curcol insert-string " "
        insert-string "       "
	!force set $debug FALSE
!endm
bind-to-key execute-macro-12 FNy

;

13	store-macro
        set %cond @"Condition Expr: "
	;set $debug TRUE
        set %curcol $curcol
        insert-string &cat &cat "WHILE " %cond " DO~n"
        %curcol insert-string " "
        set $curcol %curcol
        insert-string "   BEGIN~n"
        %curcol insert-string " "
        set $curcol %curcol
        !if &EQUal  %curcol 3
	        insert-string &cat &cat "   END; (* WHILE " %cond " *)"
	!else
	        insert-string &cat &cat "   END  (* WHILE " %cond " *)"
	!endif
        beginning-of-line
        open-line
        %curcol insert-string " "
        insert-string "      "
	!force set $debug FALSE
!endm
bind-to-key execute-macro-13 A-FN6

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
bind-to-key execute-macro-14 FN^V

15	store-macro
        set %expr @"Case Expr: "
	;set $debug TRUE
        set %curcol $curcol
        insert-string &cat &cat "CASE " %expr " OF~n"
        %curcol insert-string " "
        set $curcol %curcol
        !if &EQUal  %curcol 3
	        insert-string &cat &cat "END; (* Case " %expr " *)"
        !else
	        insert-string &cat &cat "END  (* Case " %expr " *)"
        !endif
        beginning-of-line
        open-line
        %curcol insert-string " "
        insert-string "   "
	!force set $debug FALSE
!endm
bind-to-key execute-macro-15 A-FN5

16	store-macro
;	set $debug TRUE
        set %curcol $curcol
        set %value @"Value: "
        insert-string &cat %value " : BEGIN~n"
        %curcol insert-string " "
        insert-string "    END;~n"
        %curcol insert-string " "
	previous-line
        beginning-of-line
        open-line
        %curcol insert-string " "
        insert-string "       "
	!force set $debug FALSE
!endm
bind-to-key execute-macro-16 FN^B

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
bind-to-key execute-macro-17 ^X8

18	store-macro
;	set $debug TRUE
        set %curcol $curcol
	;save-file
        set-mark
	beginning-of-file
        write-message "[Replacing 'begin']
	replace-string "begin" "Begin"

	beginning-of-file
        write-message "[Replacing 'end']
	replace-string "end" "End"

	beginning-of-file
        write-message "[Replacing ' if']
	replace-string " if" " IF"

	beginning-of-file
        write-message "[Replacing ' else']
	replace-string " else" " Else"

	beginning-of-file
        write-message "[Replacing ' repeat']
	replace-string " repeat" " Repeat"

	beginning-of-file
        write-message "[Replacing ' while']
	replace-string " while" " While"

        exchange-point-and-mark
	!force set $debug FALSE
!endm
bind-to-key execute-macro-18 ^X9

19	store-macro
	i-shell
!endm

; Set up CMODE
	set %oldmode $cmode
	set %oldgmode $gmode
	add-mode CMODE
	add-global-mode CMODE

write-message "[PASCAL MENU page loaded]"
