;;; Spelling correction interface for GNU EMACS "ispell"
;;; $Id: ispell.el,v 2.19 1992/01/10 10:54:08 geoff Exp $
;;;
;;; $Log: ispell.el,v $
;;; Revision 2.19  1992/01/10  10:54:08  geoff
;;; Make another attempt at fixing the "Bogus, dude" problem.  This one is
;;; less elegant, but has the advantage of working.
;;;
;;; Revision 2.18  1992/01/07  10:04:52  geoff
;;; Fix the "Bogus, Dude" problem in ispell-word.
;;;
;;; Revision 2.17  91/09/12  00:01:42  geoff
;;; Add some changes to make ispell-complete-word work better, though
;;; still not perfectly.
;;; 
;;; Revision 2.16  91/09/04  18:00:52  geoff
;;; More updates from Sebastian, to make the multiple-dictionary support
;;; more flexible.
;;; 
;;; Revision 2.15  91/09/04  17:30:02  geoff
;;; Sebastian Kremer's tib support
;;; 
;;; Revision 2.14  91/09/04  16:19:37  geoff
;;; Don't do set-window-start if the move-to-window-line moved us
;;; downward, rather than upward.  This prevents getting the buffer all
;;; confused.  Also, don't use the "not-modified" function to clear the
;;; modification flag;  instead use set-buffer-modified-p.  This prevents
;;; extra messages from flashing.
;;; 
;;; Revision 2.13  91/09/04  14:35:41  geoff
;;; Fix a spelling error in a comment.  Add code to handshake with the
;;; ispell process before sending anything to it.
;;; 
;;; Revision 2.12  91/09/03  20:14:21  geoff
;;; Add Sebastian Kremer's multiple-language support.
;;; 

;;; Walt Buehring
;;; Texas Instruments - Computer Science Center
;;; ARPA:  Buehring%TI-CSL@CSNet-Relay
;;; UUCP:  {smu, texsun, im4u, rice} ! ti-csl ! buehring

;;; ispell-region and associated routines added by
;;; Perry Smith
;;; pedz@bobkat
;;; Tue Jan 13 20:18:02 CST 1987

;;; extensively modified by Mark Davies and Andrew Vignaux
;;; {mark,andrew}@vuwcomp
;;; Sun May 10 11:45:04 NZST 1987

;;; This file has overgone a major overhaul to be compatible with ispell
;;; version 2.1.  Most of the functions have been totally rewritten, and
;;; many user-accessible variables have been added.  The syntax table has
;;; been removed since it didn't work properly anyway, and a filter is
;;; used rather than a buffer.  Regular expressions are used based on
;;; ispell's internal definition of characters (see ispell(4)).
;;; Ken Stevens 	ARPA: stevens@hplabs.hp.com	UUCP: hplabs!stevens
;;; Tue Jan  3 16:59:07 PST 1989
;;; Some new updates:
;;; - Updated to version 3.0 to include terse processing.
;;; - Added a variable for the look command.
;;; - Fixed a bug in ispell-word when cursor is far away from the word
;;;   that is to be checked.
;;; - Ispell places the incorrect word or guess in the minibuffer now.
;;; - fixed a bug with 'l' option when multiple windows are on the screen.
;;; - lookup-words just didn't work with the process filter.  Fixed.
;;; - Rewrote the process filter to make it cleaner and more robust
;;;   in the event of a continued line not being completed.
;;; - Made ispell-init-process more robust in handling errors.
;;; - Fixed bug in continuation location after a region has been modified by
;;;   correcting a misspelling.
;;; Mon 17 Sept 1990

;;; Sebastian Kremer <sk@thp.uni-koeln.de>
;;; Wed Aug  7 14:02:17 MET DST 1991
;;; - Ported ispell-complete-word from Ispell 2 to Ispell 3.
;;; - Added ispell-kill-ispell command.
;;; - Added ispell:dictionary and ispell:dictionary-alist variables to
;;;   support other than default language.  See their docstrings and
;;;   command ispell-change-dictionary.
;;; - (ispelled it :-)
;;; - Added ispell:check-tib variable to support the tib bibliography
;;;   program.

;;; To fully install this, add this file to your GNU lisp directory and 
;;; compile it with M-X byte-compile-file.  Then add the following to the
;;; appropriate init file:

;;;  (autoload 'ispell-word "ispell"
;;;    "Check the spelling of word in buffer." t)
;;;  (global-set-key "\e$" 'ispell-word)
;;;  (autoload 'ispell-region "ispell"
;;;    "Check the spelling of region." t)
;;;  (autoload 'ispell-buffer "ispell"
;;;    "Check the spelling of buffer." t)
;;;  (autoload 'ispell-complete-word "ispell"
;;;    "Look up current word in dictionary and try to complete it." t)
;;;  (autoload 'ispell-change-dictionary "ispell"
;;;    "Change ispell dictionary." t)


;;; **********************************************************************
;;; The following variables should be set according to personal preference
;;; and location of binaries:
;;; **********************************************************************


;;;  ******* THIS FILE IS WRITTEN FOR ISPELL VERSION 3.0


;;; Highlighting can slow down display at slow baud and emacs in
;;; X11 windows cannot take advantage of highlighting (yet).
(defconst ispell:highlight-p t
  "*When not nil, spelling errors will be highlighted.")

(defvar ispell:check-comments nil
  "*When true, the spelling of comments in region is checked.")

(defvar ispell:check-tib nil
  "*If non-nil, the spelling of references for the tib(1) bibliography
program is checked.  Else any text between strings matching the regexps
ispell:tib-ref-beginning and ispell:tib-ref-end is ignored, usually what
you want.")

(defvar ispell:tib-ref-beginning "\\(\\[\\.\\)\\|\\(<\\.\\)"
  "Regexp matching the beginning of a Tib reference.")

(defvar ispell:tib-ref-end "\\(\\.\\]\\)\\|\\(\\.\>\\)"
  "Regexp matching the end of a Tib reference.")

(defvar ispell:keep-choices-win t
  "*When true, the *Choices* window remains for spelling session.")

(defvar ispell:program-name "ispell"
  "Program invoked by ispell-word and ispell-region commands.")

(defvar ispell:alternate-dictionary "/usr/dict/web2"
  "Alternate dictionary for spelling help.")

(defvar ispell:grep-command "/usr/bin/egrep"
  "Name of the grep command for search processes.")

(defvar ispell:look-command "/usr/bin/look"
  "Name of the look command for search processes.")

(defvar ispell:dictionary nil
  "If non-nil, a dictionary to use instead of the default one.
This is passed to the ispell process using the \"-d\" switch and is
used as key in ispell:dictionary-alist (which see).

You should set this variable before your first call to ispell (e.g. in
your .emacs), or use the \\[ispell-change-dictionary] command to
change it, as changing this variable only takes effect in a newly
started ispell process.")

(defvar ispell:dictionary-alist		; sk  9-Aug-1991 18:28
  '((nil				; default (english.aff) 
     "[A-Za-z]" "[^A-Za-z]" "[---']" nil nil)
    ("german"				; german.aff
     "[A-Za-z]" "[^A-Za-z]" "[---'\"]" t ("-C")) 
    ;; add more dicts before this line
    )
  "An alist of dictionaries and their associated parameters.

Each element of this list is also a list:

    \(DICTIONARY-NAME
        CASECHARS NOT-CASECHARS OTHERCHARS MANY-OTHERCHARS-P
        ISPELL-ARGS\)

DICTIONARY-NAME is a possible value of variable ispell:dictionary, nil
means the default dictionary.

CASECHARS is a regular expression of valid characters that comprise a
word.

NOT-CASECHARS is the opposite regexp of CASECHARS.

OTHERCHARS is a regular expression of other characters that are valid
in word constructs.  Otherchars cannot be adjacent to each other in a
word, nor can they begin or end a word.  This implies we can't check
\"Stevens'\" as a correct possessive and other correct formations.

Hint: regexp syntax requires the hyphen to be declared first here.

MANY-OTHERCHARS-P is non-nil if many otherchars are to be allowed in a
word instead of only one.

ISPELL-ARGS is a list of additional arguments passed to the ispell
subprocess.

Note that the CASECHARS and OTHERCHARS slots of the alist should
contain the same character set as casechars and otherchars in the
language.aff file (e.g., english.aff).")

(defun ispell:get-casechars ()
  (nth 1 (assoc ispell:dictionary ispell:dictionary-alist)))
(defun ispell:get-not-casechars ()
  (nth 2 (assoc ispell:dictionary ispell:dictionary-alist)))
(defun ispell:get-otherchars ()
  (nth 3 (assoc ispell:dictionary ispell:dictionary-alist)))
(defun ispell:get-many-otherchars-p ()
  (nth 4 (assoc ispell:dictionary ispell:dictionary-alist)))
(defun ispell:get-ispell-args ()
  (nth 5 (assoc ispell:dictionary ispell:dictionary-alist)))


;;; **********************************************************************
;;; The following are used by ispell, and should not be changed.
;;; **********************************************************************


(defvar ispell-process nil
  "Holds the process object for 'ispell'")

(defvar ispell:pdict-modified-p nil
  "T when the personal dictionary has modifications that need to be written.")

(defvar ispell:quit nil
  "Set to t when user want to abort ispell session.")

(defvar ispell:look-p t
  "Use look. Automatically reset if look not available")

(defvar ispell:filter nil
  "Output filter from piped calls to ispell.")

(defvar ispell:filter-continue nil
  "Control variable for ispell filter function.")




(defun ispell-word (&optional preceding quietly)
  "Check spelling of word under or following the cursor.
If word not found in dictionary, display possible corrections in a window 
and let user select.
  Optional argument PRECEDING set for checking preceding word when not
over a word, and QUIETLY suppresses messages when word is correct.
  Word syntax described by ispell:dictionary-alist (which see)."
  (interactive)
  (let* ((ispell-casechars (ispell:get-casechars))
	 (ispell-not-casechars (ispell:get-not-casechars))
	 (ispell-otherchars (ispell:get-otherchars))
	 (ispell-many-otherchars-p (ispell:get-many-otherchars-p))
	 (word-regexp (concat ispell-casechars
			      "+\\("
			      ispell-otherchars
			      "?"
			      ispell-casechars
			      "+\\)"
			      (if ispell-many-otherchars-p "*" "?")))
	 did-it-once
	 ispell:keep-choices-win	; override global to force creation
	 start end word poss replace)
    (save-excursion
      ;; find the word
      (if (not (looking-at ispell-casechars))
	  (if preceding
	      (re-search-backward ispell-casechars (point-min) t)
	    (re-search-forward ispell-casechars (point-max) t)))
      ;; move to front of word
      (re-search-backward ispell-not-casechars (point-min) 'start)
      (while (and (looking-at ispell-otherchars)
		  (not (bobp))
		  (or (not did-it-once)
		      ispell-many-otherchars-p))
	(progn
	  (setq did-it-once t)
	  (backward-char 1)
	  (if (looking-at ispell-casechars)
	      (re-search-backward ispell-not-casechars (point-min) t)
	    (backward-char -1))))
      ;; Now mark the word and save to string.
      (or (re-search-forward word-regexp (point-max) t)
	  (error "No word found to check!"))
      (setq start (match-beginning 0)
	    end (match-end 0)
	    word (buffer-substring start end)))
    (goto-char start)
    ;; now check spelling of word.
    (or quietly (message "Checking spelling of %s..." (upcase word)))
    (ispell-init-process)		; erases ispell output buffer
    (process-send-string ispell-process "%\n") ;put in verbose mode
    (process-send-string ispell-process (concat "^" word "\n"))
    ;; wait until ispell has processed word
    (while (progn
	     (accept-process-output ispell-process)
	     (not (string= "" (car ispell:filter)))))
    (process-send-string ispell-process "!\n") ;back to terse mode.
    (setq ispell:filter (cdr ispell:filter))
    (if (listp ispell:filter)
	(setq poss (ispell-parse-output (car ispell:filter))))
    (cond ((eq poss t)
	   (or quietly (message "Found %s" (upcase word))))
	  ((stringp poss)
	   (or quietly (message "Found %s because of root %s" (upcase word) (upcase poss))))
	  ((null poss) (message "Error in ispell process"))
	  (t
	   (unwind-protect
	       (progn
		 (if ispell:highlight-p
		     (highlight-spelling-error start end t)) ; highlight word
		 (setq replace (ispell-choose (car (cdr (cdr poss)))
					      (car (cdr (cdr (cdr poss))))
					      (car poss)))
		 ;; update ispell:pdict-modified-p
		 (if (listp ispell:pdict-modified-p)
		     (setq ispell:pdict-modified-p
			   (car ispell:pdict-modified-p))))
	     ;; protected
	     (if ispell:highlight-p  ; clear highlight
		 (highlight-spelling-error start end)))
	   (cond (replace
		  (goto-char end)
		  (delete-region start end)
		  (if (atom replace)
		      (insert-string replace)
		    (insert-string (car replace)) ; replacement string, recheck spelling.
		    (ispell-word t quietly))))
	   (if (get-buffer "*Choices*")
	       (kill-buffer "*Choices*"))))
    (ispell-pdict-save)
    (if ispell:quit (setq ispell:quit nil))))


(defun ispell-pdict-save ()
  "Check to see if the personal dictionary has been modified.
  If so, ask if it needs to be saved."
  (interactive)
  (if ispell:pdict-modified-p
      (if (y-or-n-p "Personal dictionary modified.  Save? ")
	  (process-send-string ispell-process "#\n")))
  (setq ispell:pdict-modified-p nil))		; unassert variable, even if not saved to avoid questioning.


;;; Global ispell:pdict-modified-p is used to track changes in the dictionary.
;;;   The global becomes a list when we either accept or insert word into the dictionary.
;;;   The value of the only element in the list is the state of whether the dictionary
;;;   needs to be saved.
(defun ispell-choose (miss guess word)
  "Display possible corrections from list MISS.
  GUESS lists possibly valid affix construction of WORD.
  Returns nil to keep word.
          string for new chosen word.
          list for new replacement word (needs rechecking).
  Global ispell:pdict-modified-p becomes a list where the only value indicates
   whether the dictionary has been modified when option a or i is used.  This
   must be returned to an atom by the calling program."
  (unwind-protect
      (save-window-excursion
	(let ((count 0)
	      (line 2)
	      (choices miss)
	      (window-min-height 2)
	      char num result)
	  (save-excursion
	    (if ispell:keep-choices-win
		(select-window (previous-window))
	      (set-buffer (get-buffer-create "*Choices*"))
	      (setq mode-line-format "--  %b  --"))
	    (if (equal (get-buffer "*Choices*") (current-buffer))
		(erase-buffer)
	      (error "Bogus, dude!  I should be in the *Choices* buffer, but I'm not!"))
	    (if guess
		(progn
		  (insert "\tAffix rules generate and capitalize this word as shown below:\n")
		  (while guess
		    (if (> (+ 4 (current-column) (length (car guess)))
			   (window-width))
			(progn
			  (insert "\n")
			  (setq line (1+ line))))
		    (insert (car guess) "    ")
		    (setq guess (cdr guess)))
		  (insert "\nUse option \"i\" if this is a correct composition from the derivative root.\n\n")
		  (setq line (+ line 4))))
	    (while choices
	      (if (> (+ 7 (current-column) (length (car choices)))
		     (window-width))
		  (progn
		    (insert "\n")
		    (setq line (1+ line))))
	      ;; not so good if there are over 20 or 30 options, but then, if
	      ;; there are that many you don't want to have to scan them all anyway...
	      (insert "(" (+ count ?0) ") " (car choices) "  ")
	      (setq choices (cdr choices)
		    count (if (memq count '(14 48 56 59 64 71))	; skip command characters.
			      (if (= count 64)
				  (+ count 3)
				(+ count 2))
			    (1+ count)))))
	  (if ispell:keep-choices-win
	      (if (> line ispell:keep-choices-win)
		  (progn
		    (switch-to-buffer "*Choices*")
		    (select-window (next-window))
		    (save-excursion
		      (let ((cur-point (point)))
			(move-to-window-line (- line ispell:keep-choices-win))
			(if (<= (point) cur-point)
			    (set-window-start (selected-window) (point)))))
		    (select-window (previous-window))
		    (enlarge-window (- line ispell:keep-choices-win))
		    (goto-char (point-min))))
	    (overlay-window line))
	  (switch-to-buffer "*Choices*")
	  (select-window (next-window))
	  (while (eq t
		     (setq result
			   (progn
			     (message "^h or ? for more options; Space to leave unchanged, Character to replace word")
			     (setq char (read-char))
			     (setq num (- char ?0))
			     (cond ((< num 15))	; hack to map num to choices, avoiding command characters.
				   ((< num 49) (setq num (- num 1)))
				   ((< num 57) (setq num (- num 2)))
				   ((< num 60) (setq num (- num 3)))
				   ((< num 65) (setq num (- num 4)))
				   ((< num 72) (setq num (- num 6)))
				   (t (setq num (- num 7))))
			     (cond ((= char ? ) nil) ; accept word this time only
				   ((= char ?i)	; accept and insert word into personal dictionary
				    (process-send-string ispell-process (concat "*" word "\n"))	; no return value
				    (setq ispell:pdict-modified-p '(t))
				    nil)
				   ((= char ?a)	; accept word, don't insert in dictionary
				    (process-send-string ispell-process (concat "@" word "\n"))	; no return value
				    (setq ispell:pdict-modified-p (list ispell:pdict-modified-p))
				    nil)
				   ((= char ?r)	; type in replacement
				    (cons (read-string "Replacement: " word) nil))
				   ((or (= char ??) (= char help-char) (= char ?\C-h))
				    (ispell-choose-help)
				    t)
				   ((= char ?x)
				    (setq ispell:quit t) nil)
				   ((= char ?q)
				    (if (y-or-n-p "Really quit ignoring changes? ")
					(progn
					  (setq ispell:quit t)
					  (process-send-eof ispell-process) ; terminate process.
					  (setq ispell:pdict-modified-p nil))))
				   ;; Cannot return to initial state after this....
				   ((= char ?l)
				    (let ((new-word (read-string "Lookup string ('*' is wildcard): " word))
					  (new-line 2))
				      (cond (new-word
					     (save-excursion
					       (setq count 0)
					       (set-buffer (get-buffer-create "*Choices*")) (erase-buffer)
					       (setq mode-line-format "--  %b  --")
					       (setq miss (lookup-words new-word))
					       (setq choices miss)
					       (while choices
						 (if (> (+ 7 (current-column) (length (car choices)))
							(window-width))
						     (progn
						       (insert "\n")
						       (setq new-line (1+ new-line))))
						 (insert "(" (+ count ?0) ") " (car choices) "  ")
						 (setq choices (cdr choices)
						       count (if (memq count '(14 48 56 59 64 71)) ; skip commands
								 (if (= count 64)
								     (+ count 3)
								   (+ count 2))
							       (1+ count)))))
					     (select-window (previous-window))
					     (if (/= new-line line)
						 (if (> new-line line)
						     (enlarge-window (- new-line line))
						   (shrink-window (- line new-line))))
					     (select-window (next-window)))))
				    t)
				   ((and (>= num 0) (< num count))
				    (nth num miss))
				   ((= char ?\C-l)
				    (redraw-display) t)
				   ((= char ?\C-r)
				    (save-excursion (recursive-edit)) t)
				   ((= char ?\C-z)
				    (suspend-emacs) t)
				   (t (ding) t))))))
	  result))
    (if (not ispell:keep-choices-win) (bury-buffer "*Choices*"))))

(defun ispell-choose-help ()
  (let ((help-1 "[r]eplace word;  [a]ccept for this session;  [i]nsert into private dictionary;")
	(help-2 "[l]ook a word up in alternate dictionary;  e[x]it;  [q]uit session."))
    (if (and (boundp 'epoch::version)
	     (equal epoch::version
		    "Epoch 3.1"))
	;; Enlarging the minibuffer crashes Epoch 3.1
	(with-output-to-temp-buffer "*Ispell Help*"
	  (princ help-1)
	  (princ "\n")
	  (princ help-2))
      (save-window-excursion
	(select-window (minibuffer-window))
	(save-excursion
	  (message help-2)
	  (enlarge-window 1)
	  (message help-1)
	  (sit-for 5)
	  (erase-buffer)))))
  )

(defun lookup-words (word)
  "Look up word in dictionary contained in the
  ispell:alternate-dictionary variable.  A '*' is used for wild cards.
  If no wild cards, LOOK is used if it exists.
  Otherwise the variable ispell:grep-command contains the command used to search
  for the words (usually egrep)."
  ;; We need a new copy of the filter to not overwrite the old copy that may currently be
  ;; utilized for another spelling operation.
  (let ((save-ispell-filter ispell:filter) results)
    (setq ispell:filter nil)			; flush output filter if currently running
    (if (and ispell:look-p
	     (not (string-match "\\*" word)))	; Only use look for an exact match.
	(let (temp-ispell-process)
	  (message "Starting \"look\" process...")
	  (sit-for 0)
	  (setq temp-ispell-process
		(start-process "look" nil
			       ispell:look-command "-df" word ispell:alternate-dictionary))
	  (set-process-filter temp-ispell-process 'ispell-filter)
	  (while (progn
		   (accept-process-output temp-ispell-process)
		   (eq (process-status temp-ispell-process) 'run)))
	  (if (zerop (length ispell:filter))	; assure look worked.
	      (progn
		(sit-for 1)			; Hope this is enough ....
		(accept-process-output temp-ispell-process)
		;; See callproc.c for this error message in function child_setup.
		;; This is passed when the program couldn't be found (no "look" here).
		;; Must recheck using grep if look failed.
		(if (not (string-match "Couldn't exec the program "
				       (car ispell:filter)))
		    (setq ispell:filter nil)	; look failed, and there was no error.  No match!
		  (message "Look failed, starting \"egrep\" process...")
		  (sit-for 0)
		  (setq ispell:look-p nil	; No look, disable it from now on.
			ispell:filter nil
			ispell:filter-continue nil) ; Above message DOESN'T send linefeed!
		  (setq temp-ispell-process	; Search for word using ispell:grep-command
			(start-process "egrep" nil ispell:grep-command
				       "-i" (concat "^" word "$") ispell:alternate-dictionary))
		  (set-process-filter temp-ispell-process 'ispell-filter)
		  (while (progn
			   (accept-process-output temp-ispell-process)
			   (eq (process-status temp-ispell-process) 'run)))))))
      (message "Starting \"egrep\" process...")
      (sit-for 0)
      (let ((start 0)				; Format correctly for egrep search.
	    new-word end)
	(while (progn
		 (if (setq end (string-match "\\*" word start))
		     (progn
		       (setq new-word (concat new-word (substring word start end) ".*"))
		       (setq start (1+ end)))
		   (setq new-word (concat new-word (substring word start)))
		   nil)))
	(setq word (concat "^" new-word "$")))
      (let ((temp-ispell-process (start-process "egrep" nil ispell:grep-command
						"-i" word ispell:alternate-dictionary)))
	(set-process-filter temp-ispell-process 'ispell-filter)
	(while (progn
		 (accept-process-output temp-ispell-process)
		 (eq (process-status temp-ispell-process) 'run)))))
    (setq results ispell:filter ispell:filter save-ispell-filter) ; Restore ispell:filter value.
    (nreverse results)))			; return filtered output.


;;; "ispell:filter" is a list of output lines from the generating function.
;;;   Each full line (ending with \n) is a separate item on the list.
;;; "output" can contain multiple lines, part of a line, or both.
;;; "start" and "end" are used to keep bounds on lines when "output" contains
;;;   multiple lines.
;;; "ispell:filter-continue" is true when we have received only part of
;;;   a line as output from a generating function ("output" did not end with a \n).
;;; NOTE THAT THIS FUNCTION WILL FAIL IF THE PROCESS OUTPUT DOESN'T END WITH A \n!
;;;   This is the case when a process dies or fails -- see lookup-words.
;;;   the default behavior in this case is to treat the next input as fresh input
(defun ispell-filter (process output)
  "Output filter function for ispell, grep, and look."
  (let ((start 0)
	(continue t)
	end)
    (while continue
      (setq end (string-match "\n" output start)) ; get text up to the newline.
      ;; If we get out of sync and ispell:filter-continue is asserted when we are not
      ;; continuing, treat the next item as a separate list.
      ;; When ispell:filter-continue is asserted, ispell:filter *should* always be a list!
      (if (and ispell:filter-continue ispell:filter (listp ispell:filter)) ; Continue with same line (item)?
	  (setcar ispell:filter (concat (car ispell:filter) ;Add it to the prev item
					(substring output start end)))
	(setq ispell:filter (cons (substring output start end) ; This is a new line and item.
				  ispell:filter)))
      (if (null end)				; We've completed reading the output.
	  (setq ispell:filter-continue t continue nil) ; We didn't finish with the line.
	(setq ispell:filter-continue nil end (1+ end)) ; Get new item next time.
	(if (= end (length output))		; No more lines in output
	    (setq continue nil)			;  so we can exit the filter.
	  (setq start end))))))			; Move start to next line of input.


(defun highlight-spelling-error (start end &optional highlight)
  "Highlight a word by toggling inverse-video.
  highlights word from START to END.
  When the optional third arg HIGHLIGHT is set, the word is drawn in inverse
  video, otherwise the word is drawn in normal video mode."
  (if (string-match "^19\\." emacs-version)
      (highlight-spelling-error-v19 start end highlight)
    ;; else 
  (let ((modified (buffer-modified-p))		; leave buffer unmodified if highlight modifies it.
	(text (buffer-substring start end))	; Save highlight region
	(inhibit-quit t)			; don't process interrupt until this function exits
	(buffer-undo-list nil))			; We're not doing anything permanent here, so dont'
    						;  clutter the undo-list with it.
    (delete-region start end)
    (insert-char ?  (- end start))		; white out region to mimimize amount of redisplay
    (sit-for 0)					; update display
    (if highlight (setq inverse-video (not inverse-video))) ; toggle inverse-video
    (delete-region start end)			; delete whitespace
    (insert text)				; insert text in inverse video.
    (sit-for 0)					; update display showing inverse video.
    (if highlight (setq inverse-video (not inverse-video))) ; toggle inverse-video
    (set-buffer-modified-p modified))))		; don't modify if flag not set.

(defun highlight-spelling-error-v19 (start end &optional highlight)
  (if highlight
      (setq ispell-saved-selection (cons selection-begin selection-end)
	    selection-begin (set-marker (make-marker) start)
	    selection-end (set-marker (make-marker) end))
    (setq selection-begin (car ispell-saved-selection)
	  selection-end (cdr ispell-saved-selection)
	  ispell-saved-selection nil))
  (sit-for 0))


(defun overlay-window (height)
  "Create a (usually small) window with HEIGHT lines and avoid
recentering."
  (save-excursion
    (let ((oldot (save-excursion (beginning-of-line) (point)))
	  (top (save-excursion (move-to-window-line height) (point)))
	  newin)
      (if (< oldot top) (setq top oldot))
      (setq newin (split-window-vertically height))
      (set-window-start newin top))))


(defun ispell-parse-output (output)
  "Parse the OUTPUT string of 'ispell' and return:
 1) T for an exact match.
 2) A string containing the root word for a match via suffix removal.
 3) A list of possible correct spellings of the format:
    '(\"original-word\" offset miss-list guess-list)
    original-word is a string of the possibly misspelled word.
    offset is an integer of the line offset of the word.
    miss-list and guess-list are possibly null list of guesses and misses."
  (cond
   ((string= output "") t)			; for startup with pipes...
   ((string= output "*") t)			; exact match
   ((string= (substring output 0 1) "+")	; found cuz of rootword
    (substring output 2))			; return root word
   (t						; need to process &,?, and #'s
    (let ((type (substring output 0 1))		; &, ?, or #
	  (original-word (substring output 2 (string-match " " output 2)))
	  (cur-count 0)				; contains current number of misses + guesses
	  count miss-list guess-list)
      (setq output (substring output (match-end 0))) ; skip over original misspelling
      (if (string= type "#")
	  (setq count 0)			; no misses for type #
	(setq count (string-to-int output))	; get number of misses.
	(setq output (substring output (1+ (string-match " " output 1)))))
      (setq offset (string-to-int output))
      (if (string= type "#")			; No miss or guess list.
	  (setq output nil)
	(setq output (substring output (1+ (string-match " " output 1)))))
      (while output
	(let ((end (string-match ",\\|\\($\\)" output))) ; end of next miss/guess.
	  (setq cur-count (1+ cur-count))
	  (if (> cur-count count)
	      (setq guess-list (cons (substring output 0 end) guess-list))
	    (setq miss-list (cons (substring output 0 end) miss-list)))
	  (if (match-end 1)			; True only when at end of line.
	      (setq output nil)			; no more misses or guesses
	    (setq output (substring output (+ end 2))))))
      (list original-word offset miss-list guess-list)))))


(defun ispell-init-process ()
  "Check status of 'ispell' process and start if necessary."
  (if (and ispell-process
	   (eq (process-status ispell-process) 'run))
      (setq ispell:filter nil ispell:filter-continue nil)
    (message "Starting new ispell process...")
    (sit-for 0)
    (setq ispell-process
	  (apply 'start-process
		 "ispell" nil ispell:program-name
		 "-a";; accept single input lines
		 "-m";; make root/affix combinations not in the dict
		 (let ((args (ispell:get-ispell-args)))
		   (if ispell:dictionary ; maybe use other dict
		       (setq args
			     (append (list "-d" ispell:dictionary)
				     args)))
		   args))
	  ispell:filter nil
	  ispell:filter-continue nil)
    (set-process-filter ispell-process 'ispell-filter)
    (accept-process-output ispell-process)	; Get version ID line
    (setq ispell:filter nil)			; Discard version ID line
    (process-send-string ispell-process "!\n")	; Put into terse mode -- save processing & parsing time!
    (process-kill-without-query ispell-process)))

(defun ispell-kill-ispell (&optional no-error)
  "Kill current ispell process (so that you may start a fresh one)."
  ;; With NO-ERROR, just return non-nil if there was no ispell
  ;; running.
  (interactive)
  (if (not (and ispell-process
		(eq (process-status ispell-process) 'run)))
      (or no-error
	  (error "There is no ispell process running!"))
    (kill-process ispell-process)
    (message "Killed ispell process.")
    nil))

(defun ispell-change-dictionary (dict)
  "Change ispell:dictionary (q.v.) and kill old ispell process.
A new one will be started as soon as necessary.

By just answering RET you can find out what the current dictionary is."
  (interactive
   (list (completing-read "Use new ispell dictionary (type SPC to complete): "
			  ispell:dictionary-alist
			  nil t)))
  ;; Like info.el, we also rely on completing-read's bug of returning
  ;; "" even if this is not in the table:
  (if (equal dict "")
      (setq dict nil))
  (if (equal dict ispell:dictionary)
      (message "(No change, using %s dictionary)"
	       (if dict dict "default"))
    (setq ispell:dictionary dict)
    (ispell-kill-ispell t)
    (message "(Next ispell command will use %s dictionary)"
	     (if dict dict "default")))
  )

;;; Requires ispell version 2.1.02 or later.
;;; Ispell processes the file and no UNIX filters are used.
;;; This allows tex and nroff files to be processed well (ispell knows about them).
;;; Spelling of comments are checked when ispell:check-comments is non-nil.
(defun ispell-region (reg-start reg-end)
  "Interactively check a region for spelling errors."
  (interactive "*r")
  (ispell-init-process)
  (if (memq major-mode '(plain-TeX-mode plain-tex-mode TeX-mode tex-mode LaTeX-mode latex-mode))
      (process-send-string ispell-process "+\n")	; set ispell mode to tex
    (process-send-string ispell-process "-\n"))		; set ispell mode to normal (nroff)
  (unwind-protect
  (save-excursion
    (message "Spelling %s..."
	     (if (and (= reg-start (point-min)) (= reg-end (point-max)))
		 (buffer-name) "region"))
    (sit-for 0)
    ;; must be top level now, not inside ispell-choose for keeping window around.
    (save-window-excursion
    (if ispell:keep-choices-win
	(let ((window-min-height 2))
	  (setq ispell:keep-choices-win 2)	; This now keeps the window size.
	  (overlay-window 2)
	  (switch-to-buffer (get-buffer-create "*Choices*"))
	  (setq mode-line-format "--  %b  --")
	  (erase-buffer)
	  (select-window (next-window))))
    (goto-char reg-start)
    (while (and (not ispell:quit) (< (point) reg-end))
      (let ((start (point))
	    (offset-change 0)
	    (end (save-excursion (end-of-line) (min (point) reg-end)))
	    (ispell-casechars (ispell:get-casechars))
	    string)
	(cond ((eolp)				; if at end of line, just go to next.
	       (forward-char 1))
	      ((and (null ispell:check-comments)
		    comment-start		; skip comments that start on the line.
		    (search-forward comment-start end t)) ; a comment is on this line.
	       (if (= (- (point) start) (length comment-start)) ; comments starts line.
		   (if (string= "" comment-end) ; skip to next line over comment
		       (beginning-of-line 2)
		     (search-forward comment-end reg-end 'limit)) ; Skip to end of comment
		 ;; Comment starts later on line.
		 ;; Only send string if it contains "casechars" before comment.
		 (let ((limit (- (point) (length comment-start)))) 
		   (goto-char start)
		   (if (re-search-forward ispell-casechars limit t)
			 (setq string (concat "^" (buffer-substring start limit) "\n")))
		   (goto-char limit))))
	      ((and (null ispell:check-tib)
		    (re-search-forward ispell:tib-ref-beginning end t))
	       ;; Skip to end of tib ref, not necessarily on this line
	       (or (re-search-forward ispell:tib-ref-end reg-end 'move)
		   (error "No end for tib reference %s"
			  (buffer-substring (point) end)))
	       (let (limit)
		 (goto-char (match-end 0)) ; end of tib ref
		 (skip-chars-forward " \t\f\n" reg-end)
		 ;; maybe we skipped several lines, need new `start' and `end'
		 (setq start (point)
		       end (save-excursion (end-of-line)
					   (min (point) reg-end))
		       limit (if (re-search-forward ispell:tib-ref-beginning
						    end 'move)
				 (match-beginning 0)
			       end))
		 ;; Only send string if it contains "casechars"
		 (goto-char start)
		 (if (re-search-forward ispell-casechars limit t)
		     (progn
		       (setq string (concat "^"
					    (buffer-substring start
							      limit)
					    "\n"))
		       (goto-char limit)))))
	      ((looking-at "[---#@*+!%~^]")	; looking at the special ispell characters..
	       (forward-char 1))		; skip over it.
	      ((re-search-forward ispell-casechars end t) ; text exists...
	       (setq string (concat "^" (buffer-substring start end) "\n"))
	       (goto-char end))
	      (t (beginning-of-line 2)))	; empty line, skip it.
	(setq end (point))			; use "end" to track end of region to check.
	(if string				; there is something to spell!
	    (let (poss)
	      ;; send string to spell process and get input.
	      (process-send-string ispell-process string)
	      (while (progn
		       (accept-process-output ispell-process)
		       (not (string= "" (car ispell:filter)))))	;Last item of output contains a blank line.
	      ;; parse all inputs from the stream one word at a time.
	      (setq ispell:filter (nreverse (cdr ispell:filter))) ; remove blank item.
	      (while (and (not ispell:quit) ispell:filter)
		(setq poss (ispell-parse-output (car ispell:filter)))
		(if (listp poss)		; spelling error occurred.
		    (let* ((word-start (+ start offset-change (car (cdr poss))))
			   (word-end (+ word-start (length (car poss))))
			   replace)
		      ;; debug debug debug
		      (if ispell:keep-choices-win (sit-for 0))
		      (goto-char word-start)
		      (if (/= word-end (progn
					 (re-search-forward (car poss) word-end t)
					 (point)))
			  ;; This usually occurs due to pipe problems with the filter.
			  (error "***ispell misalignment: word \"%s\" point %d; please retry."
				 (car poss) word-start))
		      (unwind-protect
			  (progn
			    (if ispell:highlight-p
				(highlight-spelling-error word-start word-end t) ; highlight word
			      (sit-for 0))	; otherwise, update screen.
			    (setq replace (ispell-choose (car (cdr (cdr poss)))
							 (car (cdr (cdr (cdr poss))))
							 (car poss))))
			;; protected
			(if ispell:highlight-p
			    (highlight-spelling-error word-start word-end))) ; un-highlight
		      (goto-char word-start)
		      (if replace
			  (if (listp replace)	; re-check all list replacements; otherwise exit.
			      (progn
				;; quit parsing this line, redo rest when re-checking new word.
				(setq ispell:filter nil)
				;; adjust regions
				(let ((change (- (length (car replace)) (length (car poss)))))
				  (setq reg-end (+ reg-end change))
				  (setq offset-change (+ offset-change change)))
				(delete-region word-start word-end)
				(insert (car replace))
				(backward-char (length (car replace)))
				(setq end (point))) ; reposition within region to recheck spelling.
			    (delete-region word-start word-end)
			    (insert replace)
			    (let ((change (- (length replace) (length (car poss)))))
			      (setq reg-end (+ reg-end change)
				    offset-change (+ offset-change change)
				    end (+ end change))))
			;; This prevents us from pointing out the word that was just accepted
			;; (via 'i' or 'a') if it follows on the same line. (The one drawback of
			;; processing an entire line.)  Redo check following the accepted word.
			(cond ((and (not (null ispell:pdict-modified-p)) (listp ispell:pdict-modified-p))
			       ;; We have accepted or inserted a word.  Re-check line.
			       (setq ispell:pdict-modified-p (car ispell:pdict-modified-p)) ; fix update flag
			       (setq ispell:filter nil) ; don't continue check.
			       (setq end word-end)))) ; reposition to check line following accepted word.
		      (message "continuing spelling check...")
		      (sit-for 0)))
		(setq ispell:filter (cdr ispell:filter))))) ; finished with this check.
	(goto-char end)))))
  (progn
    (if (get-buffer "*Choices*")
	(kill-buffer "*Choices*"))
    (ispell-pdict-save)
    (if ispell:quit (setq ispell:quit nil))
    (message "Spell done."))))


(defun ispell-buffer () 
  "Check the current buffer for spelling errors interactively."
  (interactive)
  (ispell-region (point-min) (point-max)))

;; Interactive word completion.
;; Some code and many ideas tweaked from Peterson's spell-dict.el.
;; Ashwin Ram <Ram@yale>, 8/14/87.

;; Ported from ispell 2 to ispell 3 by Sebastian Kremer <sk@thp.uni-koeln.de>
;; 7-Aug-1991 13:44

(defvar ispell-words-file "/usr/dict/words"
   "*File used for ispell-complete-word command.  On 4.3bsd systems, try
using \"/usr/dict/web2\" for a larger selection.  Apollo users may want to
try \"/sys/dict\".")

(defun ispell-complete-word ()
   "Look up word before point in dictionary (see the variable
ispell-words-file) and try to complete it.  If in the middle of a word,
replace the entire word."
   (interactive)
   (let* ((current-word (buffer-substring (save-excursion
					    (forward-word -1) (point))
                                          (point)))
          (in-word (looking-at "\\w"))
	  (ispell:filter-continue t)
          (possibilities
	     (or (string= current-word "") ; Will give you every word
		 (setq ispell:filter (lookup-words current-word))
		 (if (not (null ispell:filter))
		     (ispell-parse-output (car ispell:filter))
		   '())))

	  (ispell:keep-choices-win nil)
	  (replacement (ispell-choose possibilities nil current-word)))
     (cond (replacement
	    (if in-word (kill-word 1));; Replace the whole word.
	    (search-backward current-word)
	    (replace-match replacement)))));; To preserve capitalization etc.
