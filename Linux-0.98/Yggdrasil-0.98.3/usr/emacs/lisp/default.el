
(global-set-key "\M-Oy" 'end-of-line)
(global-set-key "\M-O5~" 'scroll-down)
(global-set-key "\M-O6~" 'scroll-up)
(global-set-key "\M-O3~" 'delete-char)

;;;; (load "c++-mode") 

;;; Commands added by calc-public-autoloads on Tue Mar 17 23:09:45 1992.
(autoload 'calc-dispatch	   "calc" "Calculator Options" t)
(autoload 'full-calc		   "calc" "Full-screen Calculator" t)
(autoload 'full-calc-keypad	   "calc" "Full-screen X Calculator" t)
(autoload 'calc-eval		   "calc" "Use Calculator from Lisp")
(autoload 'defmath		   "calc" nil t t)
(autoload 'calc			   "calc" "Calculator Mode" t)
(autoload 'quick-calc		   "calc" "Quick Calculator" t)
(autoload 'calc-keypad		   "calc" "X windows Calculator" t)
(autoload 'calc-embedded	   "calc" "Use Calc inside any buffer" t)
(autoload 'calc-embedded-activate  "calc" "Activate =>'s in buffer" t)
(autoload 'calc-grab-region	   "calc" "Grab region of Calc data" t)
(autoload 'calc-grab-rectangle	   "calc" "Grab rectangle of data" t)
(autoload 'edit-kbd-macro	   "macedit" "Edit Keyboard Macro" t)
(autoload 'edit-last-kbd-macro	   "macedit" "Edit Keyboard Macro" t)
(autoload 'read-kbd-macro	   "macedit" "Read Keyboard Macro" t)
(setq load-path (append load-path (list "/usr/local/emacs/calc-2.02")))
(global-set-key "\e#" 'calc-dispatch)
;;; End of Calc autoloads.

