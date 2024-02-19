;; macro 29 next sentence -- defn:  go to beginning of word following either a
;; "." or \n\n
29 store-macro
	add-mode "magic"
	set %ccol $curcol
	set %cline $curline
	!force search-forward "\."
	!if $status 
		set %pcol $curcol
		set %pline $curline
	!else
 		set %pcol 0
		end-of-file
		set %pline $curline
	!endif
	set $curline %cline
	set $curcol %ccol
	!force search-forward "~n *~n"
	!if $status 
		set %ncol $curcol
		set %nline $curline
	!else
		set %ncol 0
		end-of-file
		set %nline $curline
	!endif
	set $curline %cline
	set $curcol %ccol
	delete-mode "magic"
	!if &less %pline %nline
;;		period before newlines
		set $curline %pline
		set $curcol %pcol
		next-word

	!else
		!if &and &equal %nline %pline &less %pcol %ncol
;;		here is both on same line and new line after. so .
			set $curline %pline
			set $curcol %pcol
			next-word
		!else
			set $curline %nline
			set $curcol %ncol
		!endif
	!endif

!endm
;; macro 28 previous sentence -- defn:  go to beginning of word following either a
;; "." or \n\n after a reverse search
28	store-macro
	set %scol $curcol
	set %sline $curline
*rep28
	set %ccol $curcol
	set %cline $curline
	add-mode "magic"
	!force search-reverse "\."
	!if $status 
		set %pcol $curcol
		set %pline $curline
	!else
		set %pcol 0
		set %pline 1
	!endif
	set $curline %cline
	set $curcol %ccol
	!force search-reverse "~n *~n"
	!if $status 
		set %ncol $curcol
		set %nline $curline
	!else
		set %ncol 0
		set %nline 1
	!endif
	delete-mode "magic"
	!if &less %nline %pline
;;		period after newlines so .
		set $curline %pline
		set $curcol %pcol
		next-word

	!else
		!if &and &equal %nline %pline &less %ncol %pcol
;;		here both on same line and new line before ncol. so period.
			set $curline %pline
			set $curcol %pcol
			next-word
		!else
;;	use newlines
			set $curline %nline
			set $curcol %ncol
			!if &and &equal $curcol 0 &equal $curline 1
				!return
			!else
				2 forward-character
			!endif
		!endif
	!endif
;;	now see if the current position is at or to the right of the starting
;;	position.  if yes, then return
	!if &less $curline %sline
		!return
	!else
		!if &less $curcol %scol
			!return
		!endif
	!endif
	previous-word
	!goto rep28
!endm
bind-to-key execute-macro-29 M-FNF
bind-to-key execute-macro-28 M-FNB
