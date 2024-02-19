set %obuff $cbufname
set $discmd FALSE
list-buffers
select-buffer "[List]"
end-of-file
set %lline $curline
beginning-of-file
4 next-line
!while &less $curline %lline
	beginning-of-line
	22 forward-character
	insert-string "=>"
	update-screen
	set $discmd TRUE
	set %resp &upper @"Kill this buffer? (N/Y)"
	set $discmd FALSE
	set %resp &upper %resp
	2 delete-previous-character
	!if &sequal  "Y" &left %resp 1
		set-mark
		beginning-of-line
		36 forward-character
 		backward-character	
		!while &equal $curchar 32
	 		backward-character	
		!endwhile		
		forward-character
		open-line
		exchange-point-and-mark
		set %bname #[List]
		!if &not &sequal %bname %obuff
			delete-buffer %bname
		!endif
		delete-previous-character
		beginning-of-line
		4 forward-character
		!if &not &sequal %bname %obuff
			overwrite-string "DELETED"
		!else
			overwrite-string "Buffer is being displayed."
	!endif
	next-line
!endwhile
set $discmd TRUE
delete-window
select-buffer %obuff
