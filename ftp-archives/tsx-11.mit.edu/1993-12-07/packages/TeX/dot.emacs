(setq text-mode-hook 'turn-on-auto-fill)
(setq make-backup-files nil) 
(put 'eval-expression 'disabled nil) 


(defvar cursor-map-1 (make-keymap)
"for ESC-O")
(fset 'Cursor-Map-1 cursor-map-1)
(define-key esc-map "O" 'Cursor-Map-1)


(defvar cursor-map-2 (make-keymap)
"for ESC-[")
(fset 'Cursor-Map-2 cursor-map-2)
(define-key esc-map "[" 'Cursor-Map-2)

(define-key esc-map "OA" 'previous-line)
(define-key esc-map "OB" 'next-line)
(define-key esc-map "OC" 'forward-char)
(define-key esc-map "OD" 'backward-char)
(define-key esc-map "[23~" 'query-replace)
(define-key esc-map "[24~" 'goto-line)
(define-key esc-map "[5~" 'scroll-down)
(define-key esc-map "[6~" 'scroll-up)
(define-key esc-map "[11~" 'help-for-help)
(define-key esc-map "[12~" 'scroll-other-window) 
(define-key esc-map "[13~" 'isearch-forward)
(define-key esc-map "[14~" 'query-replace-regexp)
(define-key esc-map "[15~" 'calc-dispatch)
(define-key esc-map "[17~" 'calc-embedded)
(define-key esc-map "[18~" 'load-library)
(define-key esc-map "[19~" 'math-mode)
(define-key esc-map "[20~" 'save-buffer)
(define-key esc-map "[21~" 'buffer-menu)
(define-key esc-map "[2~" 'set-mark-command)
(define-key esc-map "Ow" 'beginning-of-buffer)
(define-key esc-map "Oq" 'end-of-buffer)

(define-key global-map "\C-h" 'backward-delete-char-untabify)
(global-set-key "\177" 'delete-char)
(define-key lisp-interaction-mode-map "\177" nil)
(define-key emacs-lisp-mode-map "\177" nil)
(define-key c-mode-map "\177" nil)
(define-key lisp-interaction-mode-map "\C-h" 'backward-delete-char-untabify)
(define-key emacs-lisp-mode-map "\C-h" 'backward-delete-char-untabify)
(define-key c-mode-map "\C-h" 'backward-delete-char-untabify)
(setq search-delete-char ?\b)


  (autoload 'ispell-word "ispell"
    "Check the spelling of word in buffer." t)
  (global-set-key "\e$" 'ispell-word)
  (autoload 'ispell-region "ispell"
    "Check the spelling of region." t)
  (autoload 'ispell-buffer "ispell"
    "Check the spelling of buffer." t)
  (autoload 'ispell-complete-word "ispell"
    "Look up current word in dictionary and try to complete it." t)
  (autoload 'ispell-change-dictionary "ispell"
    "Change ispell dictionary." t)





(autoload 'calc-dispatch "calc" "Calculator Options" t)
(autoload 'calc-grab-region "calc" "grab region of calc data" t)
(autoload 'calc-grab-rectangle "calc" "grab rectangle of calc data" t)
(autoload 'calc-embedded "calc" "Calculator in any buffer" t)


;;; Mode settings stored by Calc on Sun Aug 23 17:50:29 1992
(setq calc-complex-format 'i)
(setq calc-angle-mode 'rad)
(setq calc-language 'tex)
;;; End of mode settings

     ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
     ;;
     ;; Make AUC TeX be loaded automatically when one of the following
     ;; commands are invoked.
     
     (autoload 'tex-mode "auc-tex" "Automatic select TeX or LaTeX mode" t)
     (autoload 'plain-tex-mode "auc-tex" "Mode for Plain TeX" t)
     (autoload 'latex-mode "auc-tex" "Mode for LaTeX" t)
     (autoload 'LaTeX-math-mode    "tex-math"      "Math mode for TeX." t)
     (autoload 'outline-minor-mode "min-out" "Minor Outline Mode." t)
     
     ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
     ;;
     ;; Let tex-mode be called, if a `.tex' file is loaded.  The function
     ;; tex-mode will automatically select either plain-TeX or LaTeX mode
     
     (setq auto-mode-alist (append '(("\\.tex$" . tex-mode))
                                   auto-mode-alist))


     ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
     ;;
     ;; Note:  C-c <letter> sequences are reserved for users own bindings
     ;;
     
     (defun Ctl-C-prefix ()                  ; Use Ctl-C as a command prefix
       Ctl-C-keymap)
     (setq Ctl-C-keymap (make-keymap))       ; allocate Ctl-C keymap table
     (global-set-key "\C-c" (Ctl-C-prefix))  ; define Ctl-C function
     
     (define-key Ctl-C-keymap "m" 'LaTeX-math-mode)
     
     ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
     ;;
     ;;  Setup outline-minor-mode
     ;;
     
     (define-key Ctl-C-keymap "o" 'outline-minor-mode)
     (make-variable-buffer-local 'outline-prefix-char)
     (setq-default outline-prefix-char "\C-l")
     (make-variable-buffer-local 'outline-regexp)
     (setq-default outline-regexp "[*\^l]+") ;; this is purely a default
     (make-variable-buffer-local 'outline-level-function)
     (setq-default outline-level-function 'outline-level-default)
     
     ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
     ;;
     ;; Personal defaults for AUC-TeX mode
     ;;
     
     (setq LaTeX-figure-label "fig:")
     (setq LaTeX-table-label "tab:")
     (setq LaTeX-section-label "sec:")
     (setq LaTeX-default-style-options "11pt")
     (setq LaTeX-float "hbtp")



(if (boundp 'epoch::version) 
 (progn (rebind-key "Prior" nil "\C-^k1")
        (global-set-key "\C-^k1" 'scroll-down)
        (rebind-key "Next" nil "\C-v") 
        (rebind-key "Home" nil "\C-a")
        (rebind-key "End" nil "\C-e")
   ;;viewer stuff
    (autoload 'math-mode "math-mode" "Minor mode for typing math in TeX" t)
       (load "viewer/autoload")
       (setq fmath-mode-user-context-map
             '(("s" "@" "\345_{i=}^{}")))
       (setq viewer::TeX-ext t)
       (setq LaTeX::viewer-user-context-map
             '(
               ("\275" "c" "\275{corollary}")
               ("\275" "d" "\275{definition}")
               ("\275" "D" "\275{description}")
               ("\275" "l" "\275{lemma}")
               ("\275" "p" "\275{property}")
               ("\275" "P" "\275{proof}")
               ("\275" "t" "\275{theorem}")
               ))
   ;;end viewer stuff

   (setq epoch-mode-alist
      (append (list
          (cons 'c-mode
            (list
              (cons 'geometry "77x40")
              (cons 'cursor-glyph 94)))
          (cons 'TeX-mode
            (list
              (cons 'geometry "77x30")
              (cons 'cursor-glyph 58)))
          (cons 'tex-mode
            (list
              (cons 'geometry "77x30")
              (cons 'cursor-glyph 58)))
          (cons 'text-mode
            (list
              (cons 'geometry "77x30")
              (cons 'cursor-glyph 38))))
        epoch-mode-alist))
  )
(load "x-mouse")
(defun x-window-menu-a (arg)
        (let ((menu-selection
                (x-popup-menu
                 arg
                 '("MaryAlice's menu"
                   ("Editing"
                    ("insert-italic" . TeX-italic)
                    ("insert-bold" . TeX-bold)
                    ("end-group" . up-list)
                    ("search" . isearch-forward)
                    ("replace" . query-replace)
                    ("undo" . undo))
                   ("Formatting"
                    ("spell" . ispell-buffer)
                    ("format" . TeX-buffer)
                    ("preview" . TeX-preview)
                    ("save" . save-buffer)
                    ("apropos" . command-apropos)
                    ("Tutorial" . ehelp-with-tutorial))))))
          (if (and menu-selection (x-mouse-select arg))
              (call-interactively menu-selection))))
(define-key mouse-map x-button-c-s-left 'x-buffer-menu)
(define-key mouse-map x-button-c-s-right 'x-window-menu-a)
(define-key mouse-map x-button-c-s-middle 'x-help)

(define-key mouse-map x-button-c-left 'x-cut-text)
(define-key mouse-map x-button-c-right 'x-window-menu-a)
(define-key mouse-map x-button-c-middle 'x-paste-text)

(define-key mouse-map x-button-s-left 'x-cut-and-wipe-text)
(define-key mouse-map x-button-s-right 'x-window-menu-a)
(define-key mouse-map x-button-s-middle 'x-paste-text)

(define-key mouse-map x-button-left 'x-mouse-set-point)
(define-key mouse-map x-button-right 'x-window-menu-a)
(define-key mouse-map x-button-middle 'x-paste-text)
)

