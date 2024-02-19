;	SMOOTH.CMD:	add the smooth paging option


	set $discmd FALSE

;	toggle smooth paging mode
 
30	store-macro
	set $discmd FALSE
	!if %rcspage
		set %rcspage FALSE
		set %rctmp "OFF"
		set $sscroll FALSE
	!else
		set %rcspage TRUE
		set %rctmp "ON "
		set $sscroll TRUE
	!endif
	set %cbuf $cbufname
	set %cline $cwline
	select-buffer "Function Keys"
	beginning-of-file
	search-forward "Spg["
	3 delete-next-character
	insert-string %rctmp
 
	unmark-buffer
	select-buffer %cbuf	
	%cline redraw-display
	set $discmd TRUE
	write-message &cat &cat "[Smooth Paging " %rctmp "]"
!endm
bind-to-key execute-macro-30 FN6

;set up new function keys window
set %cbuf $cbufname
set %cline $cwline
select-buffer "Function Keys"
beginning-of-file
search-forward "f6 "
8 delete-next-character
insert-string "Spg[OFF]"
unmark-buffer
select-buffer %cbuf
%cline redraw-display
set %rcspage FALSE
print "[Smooth Paging loaded]"
set $discmd TRUE

