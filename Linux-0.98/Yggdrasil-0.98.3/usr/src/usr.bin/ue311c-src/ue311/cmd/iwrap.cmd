;	IWRAP.CMD:	Some useful macroes when typing in adventures
;			(C)opyright 1987 by Daniel M Lawrence

add-mode wrap
add-global-mode wrap

21	store-macro
	wrap-word
	beginning-of-line
	handle-tab
	end-of-line
!endm
bind-to-key execute-macro-21 M-S-FN4
