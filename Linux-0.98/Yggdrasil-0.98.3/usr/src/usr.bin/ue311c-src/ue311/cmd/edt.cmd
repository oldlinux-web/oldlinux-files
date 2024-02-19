; EDT Keypad Emulator (first order approximation - partial implementation)
; For MicroEmacs 3.10_J (version which allows multiple bindings of meta-prefix)
; VMS and ATARI ST version only.
; by Roy Lomicka February 2, 1988

; Name the keys.  Note with V3.10_J, there are very few differences between
; the Atari and VMS versions

;turn keypad commands on with ^X3, off with ^X4 (ANSI versions only)

bind-to-key execute-macro-39 ^X3
39 store-macro
write-message &cat &chr 27 "= keypad on"
!endm

bind-to-key execute-macro-40 ^X4
40 store-macro
write-message &cat &chr 27 "> keypad off"
!endm

; Currently uses macros 31-38

bind-to-key meta-prefix	FN^(
set $sterm FN^E

; Unbind currently unimplemented EDT keys:
!force unbind-key	M-FN^(
!force unbind-key	FN^)		; Help
!force unbind-key	M-FN^)
!force unbind-key	FN^9		; Append
!force unbind-key	M-FN^9		; Replace
!force unbind-key	M-FN^1		; Change case
!force unbind-key	M-FN^3		; Special insert
!force unbind-key	M-FN^.		; Reset
!force unbind-key	FN^E		; Enter
!force unbind-key	M-FN^E		; Substitute

bind-to-key execute-macro-31		FN^4	; Advance
31 store-macro
set $discmd FALSE
bind-to-key search-forward		M-FN^/	; Find
bind-to-key hunt-forward		FN^/	; Find next
bind-to-key execute-macro-33		FN^7	; Page (next-ff)
bind-to-key next-page			FN^8	; Section
bind-to-key next-word			FN^1	; Word
bind-to-key forward-character		FN^3	; Char
bind-to-key execute-macro-35		FN^0	; Beginning of line (next-bol)
bind-to-key execute-macro-37		FN^2	; End of line (next-eol)
set $discmd TRUE
write-message "EDT Directional Mode = Advance"
!endm

execute-macro-31

bind-to-key execute-macro-32		FN^5	; Backup
32 store-macro
set $discmd FALSE
bind-to-key search-reverse		M-FN^/	; Find
bind-to-key hunt-backward		FN^/	; Find next
bind-to-key execute-macro-34		FN^7	; Page (prev-ff)
bind-to-key previous-page		FN^8	; Section
bind-to-key previous-word		FN^1	; Word
bind-to-key backward-character		FN^3	; Char
bind-to-key execute-macro-36		FN^0	; Beginning of line (next-bol)
bind-to-key execute-macro-38		FN^2	; End of line (next-eol)
set $discmd TRUE
write-message "EDT Directional Mode = Backup"
!endm

; Caution - Delete eol, Delete line, and Delete word
;	    use the same buffer as Cut, so Paste and
;	    the Undelete keys are bound identically. 
;	    Note that although Delete char is defined 
;	    in this group, it does not participate in 
;	    the Cut/Paste/Delete/Undelete process.

bind-to-key kill-to-end-of-line M-FN^2	; Delete eol
bind-to-key kill-to-end-of-line		FN^*	; Delete line
bind-to-key delete-next-word		FN^-	; Delete word
bind-to-key delete-next-character	FN^,	; Delete char
bind-to-key yank			M-FN^*	; Undelete line
bind-to-key yank			M-FN^-	; Undelete word
bind-to-key yank			M-FN^,	; Undelete char

bind-to-key execute-named-command M-FN^7 ; Command
bind-to-key fill-paragraph		M-FN^8	; Fill

bind-to-key end-of-file			M-FN^4	; Bottom
bind-to-key beginning-of-file		M-FN^5	; Top

bind-to-key kill-region			FN^6	; Cut
bind-to-key yank			M-FN^6	; Paste

bind-to-key open-line			M-FN^0	; Open line

bind-to-key set-mark			FN^.	; Select

; next-ff
33 store-macro
set %search $search
!force search-forward "~f"
set $search %search
!endm

; prev-ff
34 store-macro
set %search $search
!force search-reverse "~f"
set $search %search
!endm

; next-bol
35 store-macro
  beginning-of-line
  next-line
!endm

; prev-bol
36 store-macro
  backward-character
  beginning-of-line
!endm


; next-eol
37 store-macro
  forward-character
  end-of-line
!endm

; prev-eol
38 store-macro
  previous-line                             
  end-of-line
!endm
