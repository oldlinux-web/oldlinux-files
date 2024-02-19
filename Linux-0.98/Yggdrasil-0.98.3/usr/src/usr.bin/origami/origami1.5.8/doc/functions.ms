.NH 1
All Origami functions
.LP
This chapter lists all Origami functions with their keybind names.  Most
of them have an Emacs-like and a TDS name.  If you want to create a new
keybinding, regard the Emacs-like and the TDS binding as examples.
.\"{{{  moving around
.NH 2
Moving around
.nr PP .25v
.\"{{{  backward-character
.sp .5v
.LP
backward-character, O_LEFT
.\"}}}
.\"{{{  forward-character
.sp .5v
.LP
forward-character, O_RIGHT
.\"}}}
.\"{{{  previous-line
.sp .5v
.LP
previous-line, O_UP
.\"}}}
.\"{{{  next-line
.sp .5v
.LP
next-line, O_DOWN
.\"}}}
.\"{{{  beginning-of-line
.sp .5v
.LP
beginning-of-line, O_START_OF_LINE
.\"}}}
.\"{{{  end-of-line
.sp .5v
.LP
end-of-line, O_END_OF_LINE
.\"}}}
.\"{{{  previous-word
.sp .5v
.LP
previous-word, O_WORD_LEFT
.\"}}}
.\"{{{  next-word
.sp .5v
.LP
next-word, O_WORD_RIGHT
.\"}}}
.\"{{{  previous-page
.sp .5v
.LP
previous-page, O_PAGE_UP
.\"}}}
.\"{{{  next-page
.sp .5v
.LP
next-page, O_PAGE_DOWN
.\"}}}
.\"{{{  beginning-of-fold
.sp .5v
.LP
beginning-of-fold, O_TOP_OF_FOLD
.IP "" .75c
Moves to the beginning of the entered fold.
.\"}}}
.\"{{{  end-of-fold
.sp .5v
.LP
end-of-fold, O_BOT_OF_FOLD
.IP "".75c
Moves to the end of the entered fold
.\"}}}
.\"{{{  goto-line
.sp .5v
.LP
goto-line, O_GOTO_LINE
.IP "" .75c
Prompts for a line number and moves cursor to this line.  If needed,
Origami opens closed folds to reach this line.
.br
Warning: goto-line counts lines from the beginning of an entered fold.
This may be not the beginning of the file.
.\"}}}
.nr PP .75v
.\"}}}
.\"{{{  folding
.NH 2
Folding
.nr PP .25v
.\"{{{  open-fold
.sp .5v
.LP
open-fold, O_OPEN_FOLD
.IP "" .75c
Open fold, and show the contents in the context of the current level.
You can not open file-folds, only entering them is allowed.
.\"}}}
.\"{{{  close-fold
.sp .5v
.LP
close-fold, O_CLOSE_FOLD
.IP "" .75c
If the cursor is located within an opened fold, close this fold.
.\"}}}
.\"{{{  enter-fold
.sp .5v
.LP
enter-fold, O_ENTER_FOLD
.IP "" .75c
Enter the fold at the current line.  Entering a filed-fold saves the
current file.
.\"}}}
.\"{{{  exit-fold
.sp .5v
.LP
exit-fold, O_EXIT_FOLD
.IP "" .75c
Exit fold.  Exiting a filed fold saves it.
.\"}}}
.\"{{{  unfold-fold
.sp .5v
.LP
unfold-fold, O_REMOVE_FOLD
.IP "" .75c
Remove fold at the current line \(em deletes the fold structure and
inserts the contents of the fold instead.
.\"}}}
.\"{{{  create-fold
.sp .5v
.LP
create-fold, O_CREATE_FOLD
.IP "" .75c
Create fold.  Use twice \(em first to indicate the top of a new fold,
then to indicate the bottom of it.  Lines to be included within the fold
must not be indented less than the fold itself.  Several other keys are
disabled while Create Fold is in operation.
.\"}}}
.\"{{{  create-auto-fold
.sp .5v
.LP
create-auto-fold, O_AUTO_FOLD
.IP "" .75c
Autofold: takes all lines to the first blank line, folds them up,
inserts the first line as the fold header, and positions the cursor on
the next non-blank line.
.\"}}}
.\"{{{  auto-header
.sp .5v
.LP
auto-header, O_FIRST_LINE_TO_FOLD_HEADER
.IP "" .75c
When the cursor is on a closed fold this key will copy the first line
enclosed by that fold onto the fold line.
.\"}}}
.\"{{{  toggle-create-fild-fold
.sp .5v
.LP
toggle-create-file-fold, O_FILE_FOLD
.IP "" .75c
File (create new file) or unfile a fold.
.br
Creating a file writes the contents of the fold into a new file named by
the fold-comment.  Unlike TDS origami does not create a new filename, if
the choosen one is already used.
.br
Unfile a filed-fold removes the link and puts the file-contents into the
text.  Unlike TDS origami doesn't remove the file.
.\"}}}
.\"{{{  open-file-fold
.sp .5v
.LP
open-file-fold, O_ATTACH_FILE
.IP "" .75c
Attach a file to a fold.  This function open a existing file instead of
creating a new one (and thus differs from O_FILE_FOLD).  The filename is
taken from the comment of the empty fold the cursor is placed.
.\"}}}
.nr PP .75v
.\"}}}
.\"{{{  editing
.NH 2
Editing
.nr PP .25v
.\"{{{  delete-to-end-of-line, O_DEL_TO_EOL
.sp .5v
.LP
delete-to-end-of-line, O_DEL_TO_EOL
.IP "" .75c
Delete to end of line. There is no `undo' for this operation.
.br
.\"}}}
.\"{{{  delete-line, O_DEL_LINE
.sp .5v
.LP
delete-line, O_DEL_LINE
.IP "" .75c
Transfers the current line to the `delete buffer', and overwrite the old
contents of the delete buffer.
.\"}}}
.\"{{{  undo-delete-line, O_UNDEL_LINE
.sp .5v
.LP
undo-delete-line, O_UNDEL_LINE
.IP "" .75c
Transfers the line currently stored in the delete buffer, back into the
file before the current line.
.\"}}}
.\"{{{  delete-character, O_DEL_CHAR_RIGHT
.sp .5v
.LP
delete-character, O_DEL_CHAR_RIGHT
.IP "" .75c
Delete character under the cursor.  If the cursor was at the right
margin, the next line will be appended to this line.
.\"}}}
.\"{{{  delete-previous-character, O_DELETE
.sp .5v
.LP
delete-previous-character, O_DELETE
.IP "" .75c
Deletes the character to the left of the cursor.  If the cursor was at
the left margin, this line will be appended to the previous line.
.\"}}}
.\"{{{  undo-delete-character
.sp .5v
.LP
undo-delete-character
.IP "" .75c
Undeletes the last deleted character.
.\"}}}
.\"{{{  transpose-characters
.sp .5v
.LP
transpose-characters
.IP "" .75c
transposes the character left of the cursor with the character under the
cursor and moves the cursor right one character.
.\"}}}
.\"{{{  case-word-capitalize
.sp .5v
.LP
case-word-capitalize
.IP "" .75c
Capitalizes the character under the cursor and puts the rest of the
word to lower case.  If the character und the cursor is no letter,
nothing happens, otherwise it moves to the next non-letter after the
word.
.\"}}}
.\"{{{  case-word-upper
.sp .5v
.LP
case-word-upper
.IP "" .75c
Same as case-word-capitalize, but changes all letters to capitals.
.\"}}}
.\"{{{  case-word-lower
.sp .5v
.LP
case-word-lower
.IP "" .75c
Same as case-word-capitalize, but changes all letters to small ones.
.\"}}}
.\"{{{  newline-and-indent, O_RETURN
.sp .5v
.LP
newline-and-indent, O_RETURN
.IP "" .75c
Opens the line at cursor position and indents the right part like the
left part.  This does not work for the fold comment.
.\"}}}
.\"{{{  kill-line, O_PICK
.sp .5v
.LP
kill-line, O_PICK
.IP "" .75c
Removes a line from the file and adds it to the end of the `pick
buffer'.  Note that the `pick buffer' is lost on exiting Origami, unlike
TDS.
.\"}}}
.\"{{{  copy-to-kill-buffer, O_COPY_PICK
.sp .5v
.LP
copy-to-kill-buffer, O_COPY_PICK
.IP "" .75c
Copies a line into the end of the `pick buffer' without deleting it from
the file.  Note that the `pick buffer' is lost on exiting Origami,
unlike TDS.
.\"}}}
.\"{{{  fold-kill-buffer, O_PUT_PICK
.sp .5v
.LP
fold-kill-buffer, O_PUT_PICK
.IP "" .75c
Places the `pick buffer' before the current line.
.\"}}}
.\"{{{  move-line, O_MOVE
.sp .5v
.LP
move-line, O_MOVE
.IP "" .75c
If the move buffer is empty it removes a line from the file and adds it
to the `move buffer'.  Otherwise the `move buffer' is inserted before
the current line.  Note that the `move buffer' is lost on exiting
Origami, unlike TDS.
.\"}}}
.\"{{{  double-line, O_COPY
.sp .5v
.LP
double-line, O_COPY
.IP "" .75c
Duplicates the current line in the file.  Lines containing filed-folds
cannot be dupplicated.
.\"}}}
.nr PP .75v
.\"}}}
.\"{{{  search & replace
.NH 2
Search and replace
.nr PP .25v
.\"{{{  search-forward
.sp .5v
.LP
search-forward, O_FIND
.IP "" .75c
Prompts for a search string and displays next occurence.  An empty search string
uses the last search string.
.\"}}}
.\"{{{  search-reverse
.sp .5v
.LP
search-reverse, O_FIND_REVERSE
.IP "" .75c
Same as search-forward, but searches backward.
.\"}}}
.\"{{{  replace-string
.sp .5v
.LP
replace-string
.IP "" .75c
Asks for a search and a replace string and replaces all occurences.
.\"}}}
.\"{{{  quere-replace-string
.sp .5v
.LP
query-replace-string
.IP "" .75c
Same as replace-string, but asks before replacing each occurence.
.\"}}}
.\"{{{  incremental-search
.sp .5v
.LP
incremental-search
.IP "" .75c
Searches the text for a string.  String can be entered incremental.
.sp .5v
.LP
incremental-reverse
.IP "" .75c
Same as ``incremental-search'', but it starts with searchdirection
backwards.
.\"}}}
.nr PP .75v
.\"}}}
.\"{{{  files
.NH 2
Files
.nr PP .25v
.\"{{{  read-file
.sp .5v
.LP
read-file, O_OPEN_NEW_FILE
.IP "" .75c
Read another file for editing, saving the current one if required.
.\"}}}
.\"{{{  next-file
.sp .5v
.LP
next-file
.IP "" .75c
Read next file given in commandline, saving the current one if required.
.\"}}}
.\"{{{  previous-file
.sp .5v
.LP
previous-file
.IP "" .75c
Read previous file given in commandline, saving the current one if
required.
.\"}}}
.\"{{{  save-file
.sp .5v
.LP
save-file, O_SAVE_FILE
.IP "" .75c
saves the current file with the current name and language.
.\"}}}
.\"{{{  write-file
.sp .5v
.LP
write-file
.IP "" .75c
Prompts for a filename and `listing-mode'.  The text is written to a new
file with the current language.  If `listing-mode' is set, the generated
file is modified in the following way:
.DS
the `language-definition-line' is removed
in `begin-fold'-comments `{\&{\&{' is removed
the `end-fold'-comments are removed
.DE
While using the language `Tds' it is impossible to set `listing-mode',
because the the generated files have an incompatible structure.
.\"}}}
.\"{{{  insert-file
.sp .5v
.LP
insert-file, O_INSERT_FILE
.IP "" .75c
Insert File.  Prompts for a filename, and imports this file into the
file being edited at the current position.
.\"}}}
.\"{{{  filter-buffer
.sp .5v
.LP
filter-buffer
.IP "" .75c
Filters the current text.  Origami prompts for the name of a filter.
The current text is saved and piped through this filter.  Origami tries
to move the cursor to the line, where the cursor stands before executing this
command.  Environment variables ORIFILE, ORILINE and ORIWORD are set to
current filename, textline and word.
.\"}}}
.\"{{{  pipe-from-command
.sp .5v
.LP
pipe-from-command
.IP "" .75c
Prompts for a command.  This will be executed and the output is inserted
in the text at current position.  Environment variables ORIFILE, ORILINE
and ORIWORD are set to current filename, textline and word.
.\"}}}
.\"{{{  pipe-to-command
.sp .5v
.LP
pipe-to-command
.IP "" .75c
Prompts for a command.  The current file will be piped to this command
as standard input.  Environment variables ORIFILE, ORILINE and ORIWORD
are set to current filename, textline and word.
.\"}}}
.nr PP .75v
.\"}}}
.\"{{{  macros
.NH 2
Macros
.nr PP .25v
.\"{{{  define-macro
.sp .5v
.LP
define-macro, O_DEFINE_MACRO
.IP "" .75c
Records all keys into the keyboard macro buffer until define-macro is
pressed again.  Calling fix-macros is allowed during define-macro.
.\"}}}
.\"{{{  execute-macro
.sp .5v
.LP
execute-macro, O_EXECUTE_MACRO
.IP "" .75c
Call keyboard macro.
.\"}}}
.\"{{{  execute-macro-n
.sp .5v
.LP
execute-macro-??, O_CALL_FIX_?? (1<=??<=32)
.IP "" .75c
execute the fix-macro with number ??.  These macrobuffers are also used
from (defmacro ...  ) in keybind, so that using these operation has the
same effect as calling the keysequence from macro!.  Buffers not used
from keybind are empty.
.\"}}}
.\"{{{  DEFINE_FIX
.sp .5v
.LP
O_DEFINE_FIX
.IP "" .75c
Asks for the number of the fix macro buffer.  The contents of the
keyboard macro buffer is copied to this fix macro buffer.  Using (deffun
\&.. ) in keybind makes it impossible to overwrite
the fix buffer.
.\"}}}
.nr PP .75v
.\"}}}
.\"{{{  shell commands
.NH 2
Shell commands
.nr PP .25v
.\"{{{  i-shell
.sp .5v
.LP
i-shell, O_SHELL
.IP "" .75c
Creates a shell.  Exiting from this shell will return to Origami.
Environment variables ORIFILE, ORILINE and ORIWORD are set to current
filename, textline and word.
.\"}}}
.\"{{{  shell-command
.sp .5v
.LP
shell-command
.IP "" .75c
Prompts for a command and starts a shell which executes it.  Environment
variables ORIFILE, ORILINE and ORIWORD are set to current filename,
textline and word.
.\"}}}
.\"{{{  suspend-origami
.sp .5v
.LP
suspend-origami
.IP "" .75c
If your system has job control, origami will be stopped and the calling
shell is reactivated.
.\"}}}
.nr PP .75v
.\"}}}
.\"{{{  arguments
.NH 2
Arguments
.LP
Like in Emacs, you can repeat each command by using an numerical
argument.  The first digit is given by set-argument-<digit>.  These
functions read more digits of the argument until you type a non-digit.
This non-digit may be a key sequence bound to a command or just a single
character.  It will be executed <argument> times.  Pressing the
abort-key will abort the repeated execution.  These commands can not be
used in macros.  If you are defining a keyboard-macro, the repeated
Operation is stored <argument> times in the buffer.
.LP
set-argument-0 ... set-argument-9
.br
.\"}}}
.\"{{{  special functions
.NH 2
Special functions
.nr PP .25v
.\"{{{  redraw-display
.sp .5v
.LP
redraw-display, O_REFRESH
.IP "" .75c
Refresh and center screen.  This is useful after getting things like
``Message from ...''.  If you are working with a window environment
which does not send a window-changed signal, you may use this command
after resizing a window.
.\"}}}
.\"{{{  help
.sp .5v
.LP
help, O_HELP
.IP "" .75c
Displays the helpfile $HOME/.origamihelp.  Since there is no default file for
this function, you can create your own helpfile.  It may be useful for new
and very complicated macros or other things you always forget.
.\"}}}
.\"{{{  describe-bindings
.sp .5v
.LP
describe-bindings
.IP "" .75c
Displays the keybinding file $HOME/.origamibind created by the keybind
compiler.
.\"}}}
.\"{{{  set-language
.sp .5v
.LP
set-language, O_LANGUAGE
.IP "" .75c
Allows you to select the language for which the code is to be saved \(em
ie the format in which folds are to be saved.  If you choose `User',
origami prompts for the begin- and end-commentstring used for hiding the
foldcomments.  Each string can be up to three characters in length.
.\"}}}
.\"{{{  describe-fold
.sp .5v
.LP
describe-fold, O_FOLD_INFO
.IP "" .75c
Returns information on the fold depth, position and indentation of the
current line, plus information about any fold the cursor is on.
.\"}}}
.\"{{{  modes
.\"{{{  edit/view
.sp .5v
.LP
add-mode-view
.IP "" .75c
Sets view-only mode.
.sp .5v
.LP
delete-mode-view
.IP "" .75c
Sets edit mode.
.\"}}}
.\"{{{  insert/overwrite
.sp .5v
.LP
add-mode-overwrite
.IP "" .75c
Sets overwrite mode.  Text input will overwrite other text.
.sp .5v
.LP
delete-mode-overwrite
.IP  "" .75c
Sets insert mode.  Text input will be inserted.
.\"}}}
.\"{{{  echo on/off
.sp .5v
.LP
add-mode-echo
.IP "" .75c
Incomplete keysequences are displayed under the statusline.
.sp .5v
.LP
delete-mode-echo
.IP "" .75c
Do not show unfinished keysequences.
.\"}}}
.\"{{{  auto-save on/off
.sp .5v
.LP
add-mode-auto-save
.IP "" .75c
Prompts for the auto-save duration.  Origami will save your files
periodically in auto-save mode.  Default time is 8 minutes.
.sp .5v
.LP
delete-mode-auto-save
.IP "" .75c
auto-save is switched of.
.\"}}}
.\"{{{  user-mode on/off
.sp .5v
.LP
set-user-mode
.IP "" .75c
Prompts for a string, which is displayed as additional mode in the
statusline.  Doing this again will overwrite the string.
.sp .5v
.LP
reset-user-mode
.IP "" .75c
No `user-mode'-string is displayed.
.\"}}}
.\"}}}
.\"{{{  abort
.sp .5v
.LP
abort
.IP "" .5i
Aborts any operation.  There are several commands, which can be aborted
by this key.  This operation can only be bound to a
.I single
key.  This key even stops a running macro, which may be useful if contains
an endless loop.  If you are anxious of Origami doing something
wrong, this key brings you back to a safe state.
.\"}}}
.nr PP .75v
.\"}}}
.\"{{{  how to get out
.NH 2
How to get out
.nr PP .25v
.\"{{{  quick-exit
.sp .5v
.LP
quick-exit, O_FINISH
.IP "" .75c
If needed exit entered folds, save the file and exit from Origami.
.\"}}}
.\"{{{  exit-origami
.sp .5v
.LP
exit-origami, O_QUIT
.IP "" .75c
If your file was changed since last save, Origami asks you if you want
to leave anyway.
.\"}}}
.nr PP .75v
.\"}}}
