;By: thomas dunbar <GSTD@VTVM2.CC.VT.EDU>
;.emacs sample
;Date: Mon, 13 Apr 1992 23:58:41 GMT

(setq text-mode-hook 'turn-on-auto-fill)
(setq make-backup-files nil)
(put 'eval-expression 'disabled nil)

;;;;;;;;;;;;;;;;
; depending on your 1) linux version 2) termcap file 3) TERM setting
;  you may need to change some of the key codes - in particular
;  the ones for the cursor keys..use Ctrl-q  to find the key code.
;  when i changed fm v 0.12 to 0.95c+, i had to add the ESC-O map
;  for the cursor keys (used ESC-[ before)

(defvar cursor-map-1 (make-keymap)
"for ESC-O")
(fset 'Cursor-Map-1 cursor-map-1)
(define-key esc-map "O" 'Cursor-Map-1)

(defvar cursor-map-2 (make-keymap)
"for ESC-[")
(fset 'Cursor-Map-2 cursor-map-2)
(define-key esc-map "[" 'Cursor-Map-2)

(setq exec-directory "/usr/local/emacs/etc/")
(setq load-directory "/usr/local/emacs/lisp/")
(define-key esc-map "OA" 'previous-line)        ;cursor keys
(define-key esc-map "OB" 'next-line)
(define-key esc-map "OC" 'forward-char)
(define-key esc-map "OD" 'backward-char)
(define-key esc-map "OH" 'beginning-of-line)
(define-key esc-map "OY" 'end-of-line)
(define-key esc-map "O5^" 'scroll-down)
(define-key esc-map "O6^" 'scroll-up)
(define-key esc-map "[[A" 'help-for-help)           ;function keys
(define-key esc-map "[[B" 'delete-other-windows)
(define-key esc-map "[[C" 'isearch-forward)
(define-key esc-map "[[D" 'query-replace-regexp)
(define-key esc-map "[[E" 'eval-defun)
(define-key esc-map "[[F" 'eval-current-buffer)
(define-key esc-map "[[G" 'buffer-menu)
(define-key esc-map "[[H" 'global-set-key)
(define-key esc-map "[[I" 'save-buffer)
(define-key esc-map "[[J" 'save-buffers-kill-emacs)
(define-key esc-map "O2^" 'set-mark-command)   ; insert key
(define-key esc-map "O3^" 'delete-char)        ; del key
(define-key esc-map "Ow" 'beginning-of-buffer) ; gray home key
(define-key esc-map "Oq" 'end-of-buffer)       ; gray end  key
