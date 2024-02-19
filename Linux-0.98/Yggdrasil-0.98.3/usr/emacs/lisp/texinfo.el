;;;; texinfo.el

;;; Major mode for editing Texinfo files.

;;; Version 2.11   14 November 1991
;;; Robert J. Chassell          
;;; Please send bug reports to:  bob@gnu.ai.mit.edu

;; Copyright (C) 1985, 1988, 1989, 1990, 1991 Free Software Foundation, Inc.


;;; This file is part of GNU Emacs.

;; GNU Emacs is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 1, or (at your option)
;; any later version.

;; GNU Emacs is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GNU Emacs; see the file COPYING.  If not, write to
;; the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.


;;; Autoloads

(autoload 'texinfo-format-region
          "texinfmt"
  "Convert the current region of the Texinfo file to Info format.
This lets you see what that part of the file will look like in Info.
The command is bound to \\[texinfo-format-region].  The text that is
converted to Info is stored in a temporary buffer."
          t nil)

(autoload 'texinfo-format-buffer
          "texinfmt"
  "Process the current buffer as texinfo code, into an Info file.
The Info file output is generated in a buffer visiting the Info file
names specified in the @setfilename command.

Non-nil argument (prefix, if interactive) means don't make tag table
and don't split the file if large.  You can use Info-tagify and
Info-split to do these manually."
          t nil)

(autoload 'makeinfo-region
          "makeinfo"
  "Make Info file from region of current Texinfo source file.

The \\[next-error] command can be used to move to the next error \(if
any are found\)."
          t nil)

(autoload 'makeinfo-buffer
          "makeinfo"
  "Make Info file from current buffer.

The \\[next-error] command can be used to move to the next error 
\(if any are found\)."
          t nil)

(autoload 'kill-compilation
          "compile"
  "Kill the process made by the \\[compile] command."
          t nil)

(autoload 'makeinfo-recenter-compilation-buffer
          "makeinfo"
  "Redisplay *compilation* buffer so that most recent output can be seen.
The last line of the buffer is displayed on
line LINE of the window, or centered if LINE is nil."
          t nil)

(autoload 'texinfo-tex-region
          "texnfo-tex"
  "Run tex on the current region. 

A temporary file is written in the default directory, and tex is run
in that directory.  The first line of the file is copied to the
temporary file; and if the buffer has a header, it is written to the
temporary file before the region itself.  The buffer's header is all
lines between the strings defined by texinfo-start-of-header and
texinfo-end-of-header inclusive.  The header must start in the first 100
lines.  The value of texinfo-tex-trailer is appended to the temporary file
after the region."
          t nil)

(autoload 'texinfo-tex-buffer
          "texnfo-tex"
  "Run TeX on current buffer.
After running TeX the first time, you may have to run \\[texinfo-texindex]
and then \\[texinfo-tex-buffer] again."
          t nil)

(autoload 'texinfo-kill-tex-job
          "texnfo-tex"
  "Kill the currently running TeX job."
          t nil)

(autoload 'texinfo-recenter-tex-output-buffer
          "texnfo-tex"
  "Redisplay buffer of TeX job output so that most recent output can be seen.
The last line of the buffer is displayed on
line LINE of the window, or centered if LINE is nil."
          t nil)

(autoload 'texinfo-delete-from-tex-print-queue
          "texnfo-tex"
  "Delete job from the line printer spooling queue.
You are prompted for the job number (shown by a previous
\\[texinfo-show-tex-print-queue] command."
          t nil)

(autoload 'texinfo-show-tex-print-queue
          "texnfo-tex"
  "Show the print queue that \\[texinfo-tex-print] put your job on.
Runs the shell command defined by texinfo-show-tex-queue-command."
          t nil)

(autoload 'texinfo-tex-print
          "texnfo-tex"
  "Print .dvi file made by \\[texinfo-tex-region] or \\[texinfo-tex-buffer].
Runs the shell command defined by texinfo-tex-dvi-print-command."
          t nil)

(autoload 'texinfo-texindex
          "texnfo-tex"
  "Run texindex on unsorted index files.
The index files are made by \\[texinfo-tex-region] or \\[texinfo-tex-buffer].
Runs the shell command defined by texinfo-texindex-command."
          t nil)

(autoload 'texinfo-make-menu
          "texnfo-upd"
  "Without any prefix argument, make or update a menu.
Make the menu for the section enclosing the node found following point.

Non-nil argument (prefix, if interactive) means make or update menus
for nodes within or part of the marked region.

Whenever a menu exists, and is being updated, the descriptions that
are associated with node names in the pre-existing menu are
incorporated into the new menu.  Otherwise, the nodes' section titles
are inserted as descriptions."
          t nil)

(autoload 'texinfo-update-node
          "texnfo-upd"
  "Without any prefix argument, update the node in which point is located.
Non-nil argument (prefix, if interactive) means update the nodes in the
marked region.

The functions for creating or updating nodes and menus, and their
keybindings, are:

    texinfo-update-node (&optional region-p)    \\[texinfo-update-node]
    texinfo-every-node-update ()                \\[texinfo-every-node-update]
    texinfo-sequential-node-update (&optional region-p)

    texinfo-make-menu (&optional region-p)      \\[texinfo-make-menu]
    texinfo-all-menus-update ()                 \\[texinfo-all-menus-update]
    texinfo-master-menu ()

    texinfo-indent-menu-description (column &optional region-p)

The `texinfo-column-for-description' variable specifies the column to
which menu descriptions are indented. Its default value is 24."
          t nil)

(autoload 'texinfo-every-node-update
          "texnfo-upd"
  "Update every node in a Texinfo file."
          t nil)

(autoload 'texinfo-all-menus-update
          "texnfo-upd"
  "Update every regular menu in a Texinfo file.
Remove pre-existing master menu, if there is one.

If called with a non-nil argument, this function first updates all the
nodes in the buffer before updating the menus."
          t nil)

(autoload 'texinfo-master-menu
          "texnfo-upd"
  "Make a master menu for a whole Texinfo file.
Non-nil argument (prefix, if interactive) means first update all
existing nodes and menus.  Remove pre-existing master menu, if there is one.

This function creates a master menu that follows the top node.  The
master menu includes every entry from all the other menus.  It
replaces any existing ordinary menu that follows the top node.

If called with a non-nil argument, this function first updates all the
menus in the buffer (incorporating descriptions from pre-existing
menus) before it constructs the master menu.

The function removes the detailed part of an already existing master
menu.  This action depends on the pre-exisitng master menu using the
standard `texinfo-master-menu-header'.

The master menu has the following format, which is adapted from the
recommendation in the Texinfo Manual:

   * The first part contains the major nodes in the Texinfo file: the
     nodes for the chapters, chapter-like sections, and the major
     appendices.  This includes the indices, so long as they are in
     chapter-like sections, such as unnumbered sections.

   * The second and subsequent parts contain a listing of the other,
     lower level menus, in order.  This way, an inquirer can go
     directly to a particular node if he or she is searching for
     specific information.

Each of the menus in the detailed node listing is introduced by the
title of the section containing the menu."
          t nil)

(autoload 'texinfo-indent-menu-description
          "texnfo-upd"
  "Indent every description in menu following point to COLUMN.  
Non-nil argument (prefix, if interactive) means indent every
description in every menu in the region.  Does not indent second and
subsequent lines of a multi-line description."
          t nil)

(autoload 'texinfo-sequential-node-update
          "texnfo-upd"
  "Update one node (or many) in a Texinfo file with sequential pointers.

This function causes the `Next' or `Previous' pointer to point to the
immediately preceding or following node, even if it is at a higher or
lower hierarchical level in the document.  Continually pressing `n' or
`p' takes you straight through the file.

Without any prefix argument, update the node in which point is located.
Non-nil argument (prefix, if interactive) means update the nodes in the
marked region.

This command makes it awkward to navigate among sections and
subsections; it should be used only for those documents that are meant
to be read like a novel rather than a reference, and for which the
Info `g*' command is inadequate."
          t nil)

(autoload 'texinfo-insert-node-lines
          "texnfo-upd"
  "Insert missing `@node' lines in region of Texinfo file.
Non-nil argument (prefix, if interactive) means also to insert the
section titles as node names; and also to insert the section titles as
node names in pre-existing @node lines that lack names."
          t nil)

(autoload 'texinfo-multiple-files-update
          "texnfo-upd"
  "Update first node pointers in each file included in OUTER-FILE;
create or update main menu in the outer file that refers to such nodes. 
This does not create or update menus or pointers within the included files.

With optional MAKE-MASTER-MENU argument (prefix arg, if interactive),
insert a master menu in OUTER-FILE.  This does not create or update
menus or pointers within the included files.

With optional UPDATE-EVERYTHING argument (numeric prefix arg, if
interactive), update all the menus and all the `Next', `Previous', and
`Up' pointers of all the files included in OUTER-FILE before inserting
a master menu in OUTER-FILE.

The command also updates the `Top' level node pointers of OUTER-FILE.

Notes: 

  * this command does NOT save any files--you must save the
    outer file and any modified, included files.

  * except for the `Top' node, this command does NOT handle any
    pre-existing nodes in the outer file; hence, indices must be
    enclosed in an included file.

Requirements:

  * each of the included files must contain exactly one highest
    hierarchical level node, 
  * this highest node must be the first node in the included file,
  * each highest hierarchical level node must be of the same type.

Thus, normally, each included file contains one, and only one,
chapter."
          t nil)


;;; Syntax table

(defvar texinfo-mode-syntax-table nil)

(if texinfo-mode-syntax-table
    nil
  (setq texinfo-mode-syntax-table (make-syntax-table))
  (modify-syntax-entry ?\" " " texinfo-mode-syntax-table)
  (modify-syntax-entry ?\\ " " texinfo-mode-syntax-table)
  (modify-syntax-entry ?@ "\\" texinfo-mode-syntax-table)
  (modify-syntax-entry ?\^q "\\" texinfo-mode-syntax-table)
  (modify-syntax-entry ?\[ "(]" texinfo-mode-syntax-table)
  (modify-syntax-entry ?\] ")[" texinfo-mode-syntax-table)
  (modify-syntax-entry ?{ "(}" texinfo-mode-syntax-table)
  (modify-syntax-entry ?} "){" texinfo-mode-syntax-table)
  (modify-syntax-entry ?\' "w" texinfo-mode-syntax-table))


;;; Keybindings
(defvar texinfo-mode-map nil)

;;; Keys common both to Texinfo mode and to TeX shell.

(defun texinfo-define-common-keys (keymap)
  "Define the keys both in Texinfo mode and in the texinfo-tex-shell."
  (define-key keymap "\C-c\C-t\C-k"    'texinfo-kill-tex-job)
  (define-key keymap "\C-c\C-t\C-x"    'texinfo-quit-tex-job)
  (define-key keymap "\C-c\C-t\C-l"    'texinfo-recenter-tex-output-buffer)
  (define-key keymap "\C-c\C-t\C-d"    'texinfo-delete-from-tex-print-queue)
  (define-key keymap "\C-c\C-t\C-q"    'texinfo-show-tex-print-queue)
  (define-key keymap "\C-c\C-t\C-p"    'texinfo-tex-print)
  (define-key keymap "\C-c\C-t\C-i"    'texinfo-texindex)

  (define-key keymap "\C-c\C-t\C-r"    'texinfo-tex-region)
  (define-key keymap "\C-c\C-t\C-b"    'texinfo-tex-buffer))

;; Mode documentation displays commands in reverse order 
;; from how they are listed in the texinfo-mode-map.

(if texinfo-mode-map
    nil
  (setq texinfo-mode-map (make-sparse-keymap))

  ;; bindings for `texnfo-tex.el'
  (texinfo-define-common-keys texinfo-mode-map)

  ;; bindings for `texinfmt.el'
  (define-key texinfo-mode-map "\C-c\C-e\C-r"    'texinfo-format-region)
  (define-key texinfo-mode-map "\C-c\C-e\C-b"    'texinfo-format-buffer)

  ;; bindings for `makeinfo.el'
  (define-key texinfo-mode-map "\C-c\C-m\C-k" 'kill-compilation)
  (define-key texinfo-mode-map "\C-c\C-m\C-l" 
    'makeinfo-recenter-compilation-buffer)
  (define-key texinfo-mode-map "\C-c\C-m\C-r" 'makeinfo-region)
  (define-key texinfo-mode-map "\C-c\C-m\C-b" 'makeinfo-buffer)

  ;; bindings for updating nodes and menus

  (define-key texinfo-mode-map "\C-c\C-um"   'texinfo-master-menu)

  (define-key texinfo-mode-map "\C-c\C-u\C-m"   'texinfo-make-menu)
  (define-key texinfo-mode-map "\C-c\C-u\C-n"   'texinfo-update-node)
  (define-key texinfo-mode-map "\C-c\C-u\C-e"   'texinfo-every-node-update)
  (define-key texinfo-mode-map "\C-c\C-u\C-a"   'texinfo-all-menus-update)

  (define-key texinfo-mode-map "\C-c\C-s"     'texinfo-show-structure)

  (define-key texinfo-mode-map "\C-c\C-c}"    'up-list)
  (define-key texinfo-mode-map "\C-c\C-c]"    'up-list) ; easier to type
  (define-key texinfo-mode-map "\C-c\C-c{"    'texinfo-insert-braces)

  ;; bindings for inserting strings

  (define-key texinfo-mode-map "\C-c\C-cv"    'texinfo-insert-@var)
  (define-key texinfo-mode-map "\C-c\C-ct"    'texinfo-insert-@table)
  (define-key texinfo-mode-map "\C-c\C-cs"    'texinfo-insert-@samp)
  (define-key texinfo-mode-map "\C-c\C-co"    'texinfo-insert-@noindent)
  (define-key texinfo-mode-map "\C-c\C-cn"    'texinfo-insert-@node)
  (define-key texinfo-mode-map "\C-c\C-ck"    'texinfo-insert-@kbd)
  (define-key texinfo-mode-map "\C-c\C-ci"    'texinfo-insert-@item)
  (define-key texinfo-mode-map "\C-c\C-cf"    'texinfo-insert-@file)
  (define-key texinfo-mode-map "\C-c\C-cx"    'texinfo-insert-@example)
  (define-key texinfo-mode-map "\C-c\C-ce"    'texinfo-insert-@end)
  (define-key texinfo-mode-map "\C-c\C-cd"    'texinfo-insert-@dfn)
  (define-key texinfo-mode-map "\C-c\C-cc"    'texinfo-insert-@code))


;;; Texinfo mode

(defvar texinfo-chapter-level-regexp 
  "chapter\\|unnumbered \\|appendix \\|majorheading\\|chapheading"
  "*Regexp matching chapter-level headings (but not the top node).")

(defun texinfo-mode ()
  "Major mode for editing Texinfo files.

  It has these extra commands:
\\{texinfo-mode-map}

  These are files that are used as input for TeX to make printed manuals
and also to be turned into Info files by \\[texinfo-format-buffer] or
`makeinfo'.  These files must be written in a very restricted and
modified version of TeX input format.

  Editing commands are like text-mode except that the syntax table is
set up so expression commands skip Texinfo bracket groups.  To see
what the Info version of a region of the Texinfo file will look like,
use \\[texinfo-format-region].  This command runs Info on the current region
of the Texinfo file and formats it properly.

  You can show the structure of a Texinfo file with \\[texinfo-show-structure].
This command shows the structure of a Texinfo file by listing the
lines with the @-sign commands for @chapter, @section, and the like.
These lines are displayed in another window called the *Occur* window.
In that window, you can position the cursor over one of the lines and
use \\[occur-mode-goto-occurrence], to jump to the corresponding spot
in the Texinfo file.

  In addition, Texinfo mode provides commands that insert various
frequently used @-sign commands into the buffer.  You can use these
commands to save keystrokes.  And you can insert balanced braces with
\\[texinfo-insert-braces] and later use the command \\[up-list] to
move forward past the closing brace.

Also, Texinfo mode provides functions for automatically creating or
updating menus and node pointers.  These functions

  * insert the `Next', `Previous' and `Up' pointers of a node,
  * insert or update the menu for a section, and
  * create a master menu for a Texinfo source file.

Here are the functions:

    texinfo-update-node                \\[texinfo-update-node]
    texinfo-every-node-update          \\[texinfo-every-node-update]
    texinfo-sequential-node-update 

    texinfo-make-menu                  \\[texinfo-make-menu]
    texinfo-all-menus-update           \\[texinfo-all-menus-update]
    texinfo-master-menu

    texinfo-indent-menu-description (column &optional region-p)

The `texinfo-column-for-description' variable specifies the column to
which menu descriptions are indented. 

Passed an argument (a prefix argument, if interactive), the
`texinfo-update-node' and `texinfo-make-menu' functions do their jobs
in the region.

To use the updating commands, you must structure your Texinfo file
hierarchically, such that each `@node' line, with the exception of the
Top node, is accompanied by some kind of section line, such as an
`@chapter' or `@section' line.

If the file has a `top' node, it must be called `top' or `Top' and
be the first node in the file.

Entering Texinfo mode calls the value of text-mode-hook, and then the
value of texinfo-mode-hook."
  (interactive)
  (text-mode)
  (setq mode-name "Texinfo")
  (setq major-mode 'texinfo-mode)
  (use-local-map texinfo-mode-map)
  (set-syntax-table texinfo-mode-syntax-table)
  (make-local-variable 'page-delimiter)
  (setq page-delimiter 
        (concat 
         "^@node [ \t]*[Tt]op\\|^@\\(" 
         texinfo-chapter-level-regexp 
         "\\)"))
  (make-local-variable 'require-final-newline)
  (setq require-final-newline t)
  (make-local-variable 'indent-tabs-mode)
  (setq indent-tabs-mode nil)
  (make-local-variable 'paragraph-separate)
  (setq paragraph-separate (concat "^\b\\|^@[a-zA-Z]*[ \n]\\|" paragraph-separate))
  (make-local-variable 'paragraph-start)
  (setq paragraph-start (concat "^\b\\|^@[a-zA-Z]*[ \n]\\|" paragraph-start))
  (make-local-variable 'fill-column)
  (setq fill-column 72)
  (make-local-variable 'comment-start)
  (setq comment-start "@c ")
  (make-local-variable 'comment-start-skip)
  (setq comment-start-skip "@c +")
  (make-local-variable 'words-include-escapes)
  (setq words-include-escapes t)
  (make-local-variable 'texinfo-start-of-header)
  (setq texinfo-start-of-header "%**start")
  (make-local-variable 'texinfo-end-of-header)
  (setq texinfo-end-of-header "%**end")
  (run-hooks 'text-mode-hook 'texinfo-mode-hook))


;;; Define start and end of header expressions 

;; The start-of-header and end-of-header expressions are defined here
;; since they used by both by `texinfmt.el' and by `texnfo-tex.el',
;; only one of which is likely to be loaded at one time.

(defvar texinfo-start-of-header "%**start"
  "String to mark start of header for texinfo-tex-region.")

(defvar texinfo-end-of-header "%**end"
  "String to mark end of header for texinfo-tex-region.")


;;; Insert string commands

(defconst texinfo-environment-regexp
  "^@\\(f?table\\|enumerate\\|itemize\\|ifinfo\\|iftex\\|example\\|quotation\\|lisp\\|smallexample\\|smalllisp\\|display\\|format\\|flushleft\\|flushright\\|ignore\\|group\\|tex\\)"
  "Regexp for environment-like TexInfo list commands.
Subexpression 1 is what goes into the corresponding `@end' statement.")

;; The following texinfo-insert-@end command not only inserts a SPC
;; after the @end, but tries to find out what belongs there.  It is
;; not very smart: it does not understand nested lists.

(defun texinfo-insert-@end ()
  "Insert the matching `@end' for a @table etc. in a texinfo buffer.
BUG: Does not understand nested lists."
  (interactive)
  (let ((string (save-excursion
		(if (re-search-backward
		     texinfo-environment-regexp nil t) 
		    (buffer-substring (match-beginning 1)
				      (match-end 1))))))
    (insert "@end ")
    (if string (insert string))))

; Earlier version of texinfo-insert-@end
; (defun texinfo-insert-@end ()
;   "Insert the string `@end ' (end followed by a space) in a texinfo buffer."
;   (interactive)
;   (insert "@end "))

;; The following insert commands accept a prefix arg N, which is the
;; number of words (actually s-exprs) that should be surrounded by
;; braces.  Thus you can first paste a variable name into a .texinfo
;; buffer, then say C-u 1 C-c C-c v at the beginning of the just
;; pasted variable name to put @var{...} *around* the variable name.
;; Operate on previous word or words with negative arg.

;; These commands use texinfo-insert-@-with-arg
(defun texinfo-insert-@-with-arg (string &optional arg)
  (if arg 
      (progn
	(setq arg (prefix-numeric-value arg))
	(if (< arg 0)
	    (progn
	      (skip-chars-backward " \t\n\r\f")
	      (save-excursion
		(forward-sexp arg)
		(insert "@" string "{"))
	      (insert "}"))
	  (skip-chars-forward " \t\n\r\f")
	  (insert "@" string "{")
	  (forward-sexp arg)
	  (insert "}")))
    (insert "@" string "{}")
    (backward-char)))

(defun texinfo-insert-braces ()
  "Make a pair of braces and be poised to type inside of them.
Use \\[up-list] to move forward out of the braces."
  (interactive)
  (insert "{}")
  (backward-char))

; (defun texinfo-insert-@code ()
;   "Insert the string @code in a texinfo buffer."
;   (interactive)
;   (insert "@code{}")
;   (backward-char))

(defun texinfo-insert-@code (&optional arg)
  "Insert the string @code in a texinfo buffer."
  (interactive "P")
  (texinfo-insert-@-with-arg "code" arg))

; (defun texinfo-insert-@dfn ()
;   "Insert the string @dfn in a texinfo buffer."
;   (interactive)
;   (insert "@dfn{}")
;   (backward-char))

(defun texinfo-insert-@dfn (&optional arg)
  "Insert the string @dfn in a texinfo buffer."
  (interactive "P")
  (texinfo-insert-@-with-arg "dfn" arg))

(defun texinfo-insert-@example ()
  "Insert the string @example in a texinfo buffer."
  (interactive)
  (insert "@example\n"))

(defun texinfo-insert-@file (&optional arg)
  "Insert the string @file in a texinfo buffer."
  (interactive "P")
  (texinfo-insert-@-with-arg "file" arg))

; (defun texinfo-insert-@item ()
;   "Insert the string @item in a texinfo buffer."
;   (interactive)
;   (insert "@item")
;   (newline))

(defun texinfo-insert-@item ()
  "Insert the string @item in a texinfo buffer."
  (interactive)
  (insert "@item")
  (newline))

; (defun texinfo-insert-@kbd ()
;   "Insert the string @kbd in a texinfo buffer."
;   (interactive)
;   (insert "@kbd{}")
;   (backward-char))

(defun texinfo-insert-@kbd (&optional arg)
  "Insert the string @kbd in a texinfo buffer."
  (interactive "P")
  (texinfo-insert-@-with-arg "kbd" arg))

; (defun texinfo-insert-@node ()
;   "Insert the string @node in a texinfo buffer, 
; along with a comment indicating the arguments to @node."
;   (interactive)
;   (insert "@node \n@comment  node-name,  next,  previous,  up")
;   (forward-line -1)
;   (forward-char 6))

(defun texinfo-insert-@node ()
  "Insert the string @node in a texinfo buffer, 
along with a comment indicating the arguments to @node."
  (interactive)
  (insert "@node \n@comment  node-name,  next,  previous,  up")
  (forward-line -1)
  (forward-char 6))

; (defun texinfo-insert-@noindent ()
;   "Insert the string @noindent in a texinfo buffer."
;   (interactive)
;   (insert "@noindent\n"))

(defun texinfo-insert-@noindent ()
  "Insert the string @noindent in a texinfo buffer."
  (interactive)
  (insert "@noindent\n"))

; (defun texinfo-insert-@samp ()
;   "Insert the string @samp in a texinfo buffer."
;   (interactive)
;   (insert "@samp{}")
;   (backward-char))

(defun texinfo-insert-@samp (&optional arg)
  "Insert the string @samp in a texinfo buffer."
  (interactive "P")
  (texinfo-insert-@-with-arg "samp" arg))

(defun texinfo-insert-@table (&optional arg)
  "Insert the string @table in a texinfo buffer."
  (interactive "P")
  (insert "@table "))

; (defun texinfo-insert-@var ()
;   "Insert the string @var in a texinfo buffer."
;   (interactive)
;   (insert "@var{}")
;   (backward-char))

(defun texinfo-insert-@var (&optional arg)
  "Insert the string @var in a texinfo buffer."
  (interactive "P")
  (texinfo-insert-@-with-arg "var" arg))


;;; Texinfo file structure

; The following is defined in `texnfo-upd.el'
; (defvar texinfo-section-types-regexp
;   "^@\\(chapter \\|sect\\|subs\\|subh\\|unnum\\|major\\|chapheading \\|heading \\|appendix\\)"
;   "Regexp matching chapter, section, other headings (but not the top node).")

(defun texinfo-show-structure (&optional nodes-too) 
  "Show the structure of a Texinfo file.
List the lines in the file that begin with the @-sign commands for
@chapter, @section, and the like.

With optional argument (prefix if interactive), list both the lines
with @-sign commands for @chapter, @section, and the like, and list
@node lines.

Lines with structuring commands beginning in them are displayed in
another window called the *Occur* window.  In that window, you can
position the cursor over one of the lines and use
\\[occur-mode-goto-occurrence], 
to jump to the corresponding spot in the Texinfo file."

  (interactive "P")
  (require 'texnfo-upd)
  (save-excursion 
    (goto-char (point-min))
    (if nodes-too
        (occur (concat "\\(^@node\\)\\|" texinfo-section-types-regexp))
      (occur texinfo-section-types-regexp)))
  (pop-to-buffer "*Occur*")
  (goto-char (point-min))
  (flush-lines "-----"))


;;; Place provide at end of file.
(provide 'texinfo)
;;;;;;;;;;;;;;;; end texinfo.el ;;;;;;;;;;;;;;;;
