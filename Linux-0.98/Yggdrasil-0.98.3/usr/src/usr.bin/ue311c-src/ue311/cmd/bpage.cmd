;	BPAGE.CMD:	Box Macro and rectangualr region page
;			for MicroEMACS 3.9d and above
;			(C)opyright 1987 by Suresh Konda and Daniel M Lawrence
;			Last Update: 11/02/87

; make sure the function key window is up
	set %rcfkeys FALSE
	toggle-fkeys
	write-message "Loading..."

; set the clean procedure up
store-procedure clean
	delete-buffer "[Macro 10]"
	delete-buffer "[Macro 11]"
	delete-buffer "[Macro 12]"
	delete-buffer "[getblock]"
	delete-buffer "[putblock]"
	delete-buffer "[Macro 13]"
	delete-buffer "[Macro 14]"
	delete-buffer "[Macro 15]"
	delete-buffer "[Macro 16]"
	delete-buffer "[Macro 17]"
	delete-buffer "[Macro 18]"
	delete-buffer "[Macro 19]"
	delete-buffer "[drawbox]"
	delete-buffer "[setpoints]"
	delete-buffer "[horizontal]"
	delete-buffer "[vertical]"
	delete-buffer "[horline]"
	delete-buffer "[vertline]"
	delete-buffer "[delcol]"
	delete-buffer "[iline]"
!endm

; Write out the page instructions
	save-window
	1 next-window
	beginning-of-file
	set $curcol 25
	overwrite-string " F1 Line type [DOUBLE]    F2 kill block   "
	next-line
	set $curcol 25
	overwrite-string " F3 draw box              F4 copy block   "
	next-line
	set $curcol 25
	overwrite-string " F5 insert line           F6 yank block   "
	next-line
	set $curcol 18
	overwrite-string "BOX "
	set $curcol 25
	overwrite-string " F7 insert space          F8 insert block "
	next-line
	set $curcol 25
	overwrite-string "                                          "
	unmark-buffer
	beginning-of-file
	!force restore-window
	update-screen

; this sets overwrite mode to off.  to change it, set rcinsert to 1
set %rcinsert 0

;	change line type

10	store-macro
	!if &equ %rcltype 1
		set %rcltype 2
		set %rctmp "DOUBLE"
	!else
		!if &equ %rcltype 2
			set %rcltype 3
			set %rctmp "C-CMNT"
		!else
			set %rcltype 1
			set %rctmp "SINGLE"
		!endif
	!endif
	set %cbuf $cbufname
	set %cline $cwline
	select-buffer "Function Keys"
	beginning-of-file
	1 goto-line
	40 forward-character
	6 delete-next-character
	insert-string %rctmp
	unmark-buffer
	select-buffer %cbuf	
	%cline redraw-display
	!return
!endm

;	Draw a box

12	store-macro
	!if &equal %rcltype  1
		set %c1 "Ú"
		set %c2 "Ä"
		set %c3 "¿"
		set %c4 "À"
		set %c5 "Ù"
		set %c6 "³"
	!else
		!if &equal %rcltype 2
			set %c1 "É"
			set %c2 "Í"
			set %c3 "»"
			set %c4 "È"
			set %c5 "¼"
			set %c6 "º"
		!else
			set %c1 "/"
			set %c2 "*"
			set %c3 "\"
			set %c4 "\"
			set %c5 "/"
			set %c6 "*"
		!endif
	!endif
	run drawbox	
!endm

;	insert a line in a box

14	store-macro
	run iline
!endm

;	insert a blank line in a box

16	store-macro
	set %rctmp %rcltype
	set %rcltype 0
	run iline
	set %rcltype %rctmp
!endm

store-procedure	iline
	run setpoints
	!if &equal %pcol %mcol
		run vertical
	!else
		!if &equal %pline %mline
			run horizontal
		!else
			write-message "Illegal point and mark for lines"
		!endif
	!endif
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

store-procedure drawbox
	run setpoints
	set $curline %mline
	set $curcol %mcol
;draw top horizontal line
	insert-string %c1
;	set %width &sub &sub %pcol %mcol 1
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
;	set $curcol %mcol
	insert-string %c4
	%width insert-string %c2
	insert-string %c5
; bump pline 
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
	%width forward-character
	6 forward-character
!endm

; user procedure to draw a horizontal from mark to point making spaces for
; the characters.
store-procedure horizontal
	set %s1 "º"
	set %s2 "³"
	set %s3 "*"
	!if &equal %rcltype  0
;	then insert blanks
		set %c1 "º"
		set %c2 "³"
		set %c3 " "
		set %c4 "º"
		set %c5 "³"
		set %c6 "º"
		set %c7 "³"
		set %c8 "*"
	!else
		!if &equal %rcltype  1
;		then insert a single line
			set %c1 "Ç"
			set %c2 "Ã"
			set %c3 "Ä"
			set %c4 "×"
			set %c5 "Å"
			set %c6 "¶"
			set %c7 "´"
			set %c8 "*"
		!else
			!if &equal %rcltype 2
;		then insert a double line
				set %c1 "Ì"
				set %c2 "Æ"
				set %c3 "Í"
				set %c4 "Î"
				set %c5 "Ø"
				set %c6 "¹"
				set %c7 "µ"
				set %c8 "*"
			!else
				set %c1 "*"
				set %c2 "*"
				set %c3 "*"
				set %c4 "*"
				set %c5 "*"
				set %c6 "*"
				set %c7 "*"
				set %c8 "*"
			!endif
		!endif
	!endif
	run horline
!endm

store-procedure vertical
	set %s1 "Í"
	set %s2 "Ä"
	set %s3 "*"
	!if &equal %rcltype  0
		set %c1 "Í"
		set %c2 "Ä"
		set %c3 " "
		set %c4 "Í"
		set %c5 "Ä"
		set %c6 "Í"
		set %c7 "Ä"
		set %c8 "*"
	!else
		!if &equal %rcltype  1
			set %c1 "Ñ"
			set %c2 "Â"
			set %c3 "³"
			set %c4 "Ø"
			set %c5 "Å"
			set %c6 "Ï"
			set %c7 "Á"
			set %c8 "*"
		!else
			!if &equal %rcltype 2
				set %c1 "Ë"
				set %c2 "Ò"
				set %c3 "º"
				set %c4 "Î"
				set %c5 "×"
				set %c6 "Ê"
				set %c7 "Ð"
				set %c8 "*"
			!else
				set %c1 "*"
				set %c2 "*"
				set %c3 "*"
				set %c4 "*"
				set %c5 "*"
				set %c6 "*"
				set %c7 "*"
				set %c8 "*"
			!endif
		!endif
	!endif
	run verline
!endm

store-procedure horline
; procedure to draw a line from beginning of line to point
	!if &equal %mcol %pcol
		!return
	!endif
	set $curline %pline
	set $curcol %pcol
	!if &less %pcol %mcol
;	then point was to left of mark.  exchange and reset variables
		exchange-point-and-mark
		run setpoints
	!endif
	!if %rcinsert
		set $curcol %mcol
	!else
		beginning-of-line
		newline
		previous-line
;		end-of-line
;		newline
		; move to under mark
		%mcol insert-string " "
	!endif
; see if first char is a vertical line
	previous-line
	set %char &chr $curchar
	next-line
	%rcinsert delete-next-character
	!if &sequal %char %s1
			insert-string %c1
	!else
		!if &sequal %char %s2
			insert-string %c2
		!else
			!if &sequal %char %s3
				insert-string %c8
			!else
				insert-string %c3
			!endif
		!endif
	!endif
; now for all chars but the last character i.e., char at point
	!while &less $curcol %pcol
		previous-line
		set %char  &chr $curchar
		next-line
		%rcinsert delete-next-character
		!if &sequal %char %s1
			insert-string %c4
        !else 
			!if &sequal %char %s2
				insert-string %c5
			!else
				!if &sequal %char %s3
					insert-string %c8
				!else
					insert-string %c3
				!endif
			!endif
		!endif
	!endwhile
; see if last char is a vertical line
	previous-line
	set %char  &chr $curchar
	next-line
	%rcinsert delete-next-character
	!if &sequal %char %s1
			insert-string %c6
	!else
		!if &sequal %char %s2
			insert-string %c7
		!else
			!if &sequal %char %s3
				insert-string %c8
			!else
				insert-string %c3
			!endif
		!endif
	!endif
!endm

store-procedure verline
;  proc to draw vertical line from mark to point.  mark should be above point.
	!if &equal %mline %pline
		!return
	!endif
;	if point was above mark exchange and reset variables
	!if &less %pline %mline
		exchange-point-and-mark
		run setpoints
	!endif
;top line
	%mline goto-line
	set $curcol %pcol
	backward-character
	set %char &chr $curchar
	forward-character
	%rcinsert delete-next-character
	!if &sequal %char %s1
		insert-string %c1
	!else
		!if &sequal %char %s2
			insert-string %c2
		!else
			!if &sequal %char %s3
				insert-string %c8
			!else
				insert-string %c3
			!endif
		!endif
	!endif
;all but pline
	!while &less $curline &sub %pline 1
		next-line
		beginning-of-line
		set $curcol %pcol
		backward-character
		set %char &chr $curchar
		forward-character
		%rcinsert delete-next-character
		!if &sequal %char %s1
			insert-string %c4
		!else
			!if &sequal %char %s2
				insert-string %c5
			!else
				!if &sequal %char %s3
					insert-string %c8
				!else
					insert-string %c3
				!endif
			!endif
		!endif
	!endwhile
; bottom line
	next-line
	beginning-of-line
	set $curcol %pcol
	backward-character
	set %char &chr $curchar
	forward-character
	%rcinsert delete-next-character
	!if &sequal %char %s1
		insert-string %c6
	!else
		!if &sequal %char %s2
			insert-string %c7
		!else
			!if &sequal %char %s3
				insert-string %c8
			!else
				insert-string %c3
			!endif
		!endif
	!endif
!endm

store-procedure delcol 
; proc to delete column.  we will use the getblock procedure with the column of
; the point set to one beyond the column point
	set-points
	!if &equal %mcol %pcol
		; same columns
		forward-character
		run getblock
		!return
	!else
		!if &equal %mline %pline
		run getblock
		!return
	!endif
!endm

;	delete a rectangular block of text

11	store-macro
	set %bkcopy FALSE
	run getblock
	write-message "[Block deleted]"
!endm

;	copy a rectangular region

13	store-macro
	set %bkcopy TRUE
	run getblock
	write-message "[Block copied]"
!endm

;	yank a rectangular region

15	store-macro
	set %bkcopy TRUE
	run putblock
!endm

;	insert a rectangular region

17	store-macro
	set %bkcopy FALSE
	run putblock
!endm

store-procedure getblock
	;set up needed variables
	set $discmd FALSE
	delete-buffer "[block]"
	set %rcbuf $cbufname
	set %cline $cwline

	;save block boundries
	set %endpos $curcol
	set %endline $curline
	detab-region
	exchange-point-and-mark
	set %begpos $curcol
	set %begline $curline
	set %blwidth &sub %endpos %begpos

	;scan through the block
	set $curline %begline
	!while &less $curline &add %endline 1
		;grab the part of this line needed
		!force set $curcol %begpos
		set-mark
		!force set $curcol %endpos
		kill-region

		;bring it back if this is just a copy
		!if %bkcopy
			yank
		!endif

		;put the line in the block buffer
		select-buffer "[block]"
		yank

		;and pad it if needed
		!if &less $curcol %blwidth
			&sub %blwidth $curcol insert-space
			end-of-line
		!endif
		forward-character

		;onward...
		select-buffer %rcbuf
		next-line
	!endwhile

        ;unmark the block
        select-buffer "[block]"
        unmark-buffer
        select-buffer %rcbuf
        previous-line
        %cline redraw-display
	set $discmd TRUE
!endm

;	insert/overlay a rectangular block of text

store-procedure putblock
	;set up needed variables
	set $discmd FALSE
	set %rcbuf $cbufname
	set %cline $cwline

	;save block boundries
	set %begpos $curcol
	set %begline $curline

	;scan through the block
	select-buffer "[block]"
	beginning-of-file
	set %endpos &add %begpos $lwidth
	!while &not &equ $lwidth 0

		;pad the destination if it is needed
		select-buffer %rcbuf
		beginning-of-line
		!if &not &equ $lwidth 0
			1 detab-line
			previous-line
		!endif
		!force set $curcol %begpos
		!if &less $curcol %begpos
			&sub %begpos $curcol insert-space
			end-of-line
		!endif

		;delete some stuff if this should overlay
		!if %bkcopy
			set-mark
			!force set $curcol %endpos
			kill-region
		!endif

		;grab the line from the block buffer
		select-buffer "[block]"
		beginning-of-line
		set-mark
		end-of-line
		copy-region
		forward-character

		;put the line in the destination position
		select-buffer %rcbuf
		yank
		next-line

		;onward...
		select-buffer "[block]"
	!endwhile

	select-buffer %rcbuf
	set $curline %begline
	set $curcol %begpos
	%cline redraw-display
	set $discmd TRUE
!endm

	; and init some variables
	set %rcltype 2
	write-message "[Block mode loaded]"

