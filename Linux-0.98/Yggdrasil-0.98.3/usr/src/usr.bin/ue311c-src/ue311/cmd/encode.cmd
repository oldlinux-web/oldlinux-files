;	ENCODE:	Captain Midnight strikes again!!!!!
;		This will do a simple substitution cypher

set %set1 "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
set %set2 "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM"

30	store-macro
	beginning-of-file
*nxt1
	set $line &xlate $line %set1 %set2
	!force next-line
	!if &seq $status "TRUE"
		!goto nxt1
	!endif
	beginning-of-file
	write-message "[Buffer ENCODED]"
!endm

31	store-macro
	beginning-of-file
*nxt2
	set $line &xlate $line %set2 %set1
	!force next-line
	!if &seq $status "TRUE"
		!goto nxt2
	!endif
	beginning-of-file
	write-message "[Buffer DECODED]"
!endm

bind-to-key execute-macro-30 FN1
bind-to-key execute-macro-31 FN2

write-message "[All set for good stuff!]"

