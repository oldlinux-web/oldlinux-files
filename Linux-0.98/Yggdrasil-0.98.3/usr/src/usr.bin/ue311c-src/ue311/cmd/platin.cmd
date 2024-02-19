;	PLATIN.CMD:	Pig latin translator
;
;	This file creates a macro to assist you in translating
;	english to pig latin.
;
;	<F1>	Translate the current word (from the point)
;	<F2>	Move to the next word

20	store-macro
	end-of-word
	previous-word
	set-mark
	end-of-word
	kill-region
	set %ucase FALSE
	!if &less &asc $kill 96
		set %ucase TRUE
	!endif
	!if &not &equ &sindex "aeiou" &left $kill 1 0
		yank
		insert-string "way"
	!else
		yank
		insert-string &left $kill 1
		insert-string "ay"
		previous-word
		delete-next-character
	!endif
	next-word
	previous-word
	!if %ucase
		case-word-capitalize
	!else
		case-word-lower
	!endif
	next-word
!endm

bind-to-key execute-macro-20 FN1
bind-to-key next-word FN2
