.NH 1
Programming in OCL
.\"{{{  introduction to this chapter
.LP
This chapter describes OCL (Origami control language) and gives some
examples to show how powerful it is.  OCL is a programming language
offering capabilitie sto use
conditional statements, subroutines, loops, variables and
recursion.  The keybind program compiles human readable OCL source code
to a binary which will be interpreted by Origami.  The functions
described in the chapter explaining keybindings are very simple OCL
statements.
.\"}}}
.\"{{{  OCL syntax and semantic
.NH 2
OCL syntax and semantic
.LP
This sections describes all other OCL elements.  It is assumed, that
you are familiar with other programming languages.  You will find
examples in a later section.
.\"{{{  marks
.NH 3
Other marks for folds
.nr PP .25v
.sp .5v
.LP
(defmark <name> ( <mark1> <mark2> <mark3> <mark4> ))
.IP "" .75c
This option put data in the rcfile, so that origami can switch on
commandlineoption `-mname' to other marks for folds. The format for
<mark> is:
.DS
( <single-char> <single-char> <single-char> )
.DE
where <single-char> is a ``"x'' definition like in (keybind ... ).
The meanings of <mark1> ... <mark4> are:
.DS
<mark1> begin of fold, standard is `{\&{\&{'
<mark2> closed fold, standard is `.\&.\&.'
<mark3> name of filed/attached fold, standard is `:\&:\&:'
<mark4> end of fold, standard is `}\&}\&}'
.DE
.\"}}}
.\"{{{  Subroutines and macros
.NH 3
Subroutines and macros
.nr PP .25v
.\"{{{  defmac
.sp .5v
.LP
(defmac <name> (<statement> ... ) )
.IP "" .75c
The Origami operations defined in (<statement> ... ) are furthermore
reproducable
as <name> too, so you can use them to define other macros.  Remark
that these are really macros, they are only copied to the calling
macrodefinition.  Inside of (<statement> ... ) you are allowed to use
<name>.  The effect is, that (<statement> ... ) is executed and
afterwards the following statements are executed.
.\"}}}
.\"{{{  deffun
.sp .5v
.LP
(deffun <name> (<statement> ... ) )
.IP "" .75c
The operations in (<statement> ... ) are written to the .origamirc-file
so that origami can initialiaze a fix-macro-buffer.  Recursion is
allowed. You can use <name> in the following OCL-text as subroutine-calls
or for keybinds.  You cannot redefine the fix-macro-buffer.
.\"}}}
.\"{{{  initmac
.sp .5v
.LP
(initfun <name> (<statement> ... ) )
.IP "" .75c
Same as above.  But the macro can be redefined with ``O_DEFINE_FIX''.
Using the commands `defmac' and `initmac' uses the fix-macro-buffers
32, 31, etc.
.\"}}}
.\"{{{  forward
.sp .5v
.LP
(forward <name>)
.IP "" .75c
<name> is declared as a name for a macro defined with ``initfun'' or
``deffun''.  <name> can be used from now on as normal macro-call.  The
definition with ``(initfun <name> .. '' or ``(deffun <name> ...  '' can
be put somewhere in the following script.
.\"}}}
.\"{{{  automacro
.sp .5v
.LP
(automacro <name>)
.IP "" .75c
The macro named <name> is called, when you are exiting a filed fold,
entering it or opening a new file.  Therefore you must enable this with
commandline option ``\-A''.
.\"}}}
.nr PP .75v
.LP
Warning:
Using ``O_DEFINE_FIX x'' has no effect on macros calling x by its name.
If you want to use such an effect you must use ``O_CALL_FIX x''.
.\"}}}
.\"{{{  control structures
.NH 3
Control structures
.LP
A <statement> can be one of the following control structures or functions:
.nr PP .25v
.\"{{{  while
.sp .5v
.LP
while <condition> (<statement> ... )
.IP "" .75c
Prechecked loop, the macro is repeated while the condition is true.
.\"}}}
.\"{{{  do while
.sp .5v
.LP
do (<stament> ... ) while <condition>
.IP "" .75c
Postchecked loop, the macro is repeated until the condition is false.
.\"}}}
.\"{{{  if then else fi
.sp .5v
.LP
if <condition> then (<statement> ... ) fi
.br
if <condition> then (<statement> ... ) else (<statement> ... ) fi
.IP "" .75c
Conditional statement.  If conditon is true, the first macro is
executed.  If condition is false, the second macro, if any, is executed.
.\"}}}
.\"{{{  case esac
.sp .5v
.LP
case (<condition> (<statement> ... )) ... esac
.br
case (<condition> (<statement> ... )) ... default (<statement> ... ) esac
.IP "" .75c
Conditional statement.  The block of statements following the first
conditional expression evaluating to TRUE is executed.  If no condition
is true, the given default-statements are executed.
.\"}}}
.\"{{{  repeat
.sp .5v
.LP
repeat <number> <statement>
.IP "" .75c
<macro> is repeated <number> times. ATTENTION: The tokens are stored
<number> times!.  This cannot be done in another way, because origami
knows no local variables!.  If you want to repeat big macros, think
about an implementation with counters.
.\"}}}
.\"{{{  prompt
.sp .5v
.LP
prompt <prompt>
.IP "" .75c
Displays <prompt> below the statusline.  <prompt> can be:
.DS
<string>
(<prompt-item> ...  )
.DE
where <promp-item> can be:
.DS
counter <name>
<string>
.DE
Origami displays the ``prompt'' and waits till you type a ``O_RETURN''
on your keyboard.
.\"}}}
.\"{{{  exits
.sp .5v
.LP
exit
.IP "" .75c
The macroexecution is aborted.  Origami waits for keyboardinput.
.sp .5v
.LP
message-exit <prompt>
.IP "" .75c
Same as exit.  <prompt> is displayed under the statusline.
.\"}}}
.\"{{{  origami-commands
.sp .5v
.LP
<origami-command>
.IP "" .75c
All Origami commands you can use from keyboard can be used by their names.
.\"}}}
.\"{{{  strings
.sp .5v
.LP
<strings>
.IP "" .75c
Normal text can be used. The syntax is:
.DS
"text
where text can be a any string not containing: <space> " )
"<space> for <space>
") for )
"" for "
.DE
.sp .5v
.LP
Warning: if you use origami to edit a OCL-file: "<space> should not
be the last statement of a line, because origami deletes all spaces at
end of line. If you put a comment after the "<space> command you have
no problems.
.\"}}}
.nr PP .75v
.\"}}}
.\"{{{  variables
.NH 3
Variables
.LP
OCL allows to use variables.  OCL variables are global integer
variables.  They are declared by using them.  They are initialized
to zero when Origami starts up.  The following functions deal with
variables:
.nr PP .25v
.\"{{{  set-counter
.sp .5v
.LP
set-counter <name> <number>
.IP "" .75c
The counter-variable <name> is set to the value of <number>.
.\"}}}
.\"{{{  prompt-counter
.sp .5v
.LP
prompt-counter <name> <string>
.IP "" .75c
Prompts <string> below the statusline and reads the new value for <name>
from keyboard.  Input ``y'' sets it to 1, ``n'' sets it to 0.  All other
inputs have to be decimals or a simple O_RETURN.
.\"}}}
.\"{{{  read-repeat
.sp .5v
.LP
read-repeat <name>
.IP "" .75c
The counter-variable <name> takes its value of the
``repeat-argument-counter''.  This counter is set to 0, so that the
macro-call is not repeated.  The value of ``repeat-macro-counter'' is
the number of remaining loops when using ``set-argument-x'', otherwise
0.
.\"}}}
.\"{{{  add-counter
.sp .5v
.LP
add-counter <name> <number>
.IP "" .75c
The counter-variable <name> is set to <name>+<number>.
.\"}}}
.\"{{{  sum-counter
.sp .5v
.LP
sum-counter <name1> <name2>
.IP "" .75c
The counter-variable <name1> is set to <name1>+<name2>.
.\"}}}
.\"{{{  inv-counter
.sp .5v
.LP
inv-counter <name>
.IP "" .75c
The counter-variable <name> is set to -<name>.
.\"}}}
.\"{{{  goto
.sp .5v
.LP
goto <number>
.IP "" .75c
Move the cursor on x-position <number>.  1 is the first displayed
character.  If you enter a indented fold, position 1 is not the
first character on that line.
.\"}}}
.\"{{{  goto-counter
.sp .5v
.LP
goto-counter <name>
.IP "" .75c
Same as above, except that the position is the value of variable <name>.
.\"}}}
.\"{{{  store-pos
.sp .5v
.LP
store-pos <name>
.IP "" .75c
The x-position of the cursor is stored in the variable <name>.
.\"}}}
.\"{{{  goto-line-counter
.sp .5v
.LP
goto-line-counter <name>
.IP "" .75c
Like O_GOTO_LINE. The linenumber is the value of variable name.
.\"}}}
.\"{{{  store-line
.sp .5v
.LP
store-line <name>
.IP "" .75c
The current line-number is stored in the variable <name>.
.\"}}}
.nr PP .75v
.\"}}}
.\"{{{  boolean expressions
.NH 3
Boolean expressions
.LP
The above mentioned boolean expressions <condition> can be:
.nr PP .25v
.\"{{{  boolean operators
.\"{{{  not
.sp .5v
.LP
not(<condition>)
.IP "" .75c
The value of <condition> is inverted.
.\"}}}
.\"{{{  and
.sp .5v
.LP
and(<condition>,<condition>)
.IP "" .75c
is true, if both <condition>'s are true.
.\"}}}
.\"{{{  or
.sp .5v
.LP
or(<condition>,<condition>)
.IP "" .75c
is true, if one of the <condition>'s is true.
.\"}}}
.\"}}}
.\"{{{  boolean basicfunctions
.\"{{{  test-filed
.sp .5v
.LP
test-filed
.IP "" .75v
is true, if the current line is a file-folded-line.
.\"}}}
.\"{{{  test-fold-line
.sp .5v
.LP
test-fold-line
.IP "" .75c
is true, if the current line is a closed-fold-line.
.\"}}}
.\"{{{  test-begin-fold
.sp .5v
.LP
test-begin-fold
.IP "" .75c
is true, if the current line is the {\&{\&{ line of a open fold.
.\"}}}
.\"{{{  test-end-fold
.sp .5v
.LP
test-end-fold
.IP "" .75c
is true, if the current line is the }\&}\&} line of a open fold
.\"}}}
.\"{{{  test-text
.sp .5v
.LP
test-text
.IP "" .75c
is true, if the current line is a normal text-line.
.\"}}}
.\"{{{  test-top
.sp .5v
.LP
test-top
.IP "" .75c
is true, if the current line is the top of the entered fold.
.\"}}}
.\"{{{  test-bottom
.sp .5v
.LP
test-bottom
.IP "" .75c
is true, if the current line is the bottom of the entered fold.
.\"}}}
.\"{{{  test-char
.sp .5v
.LP
test-char "x
.IP "" .75c
is true, if the character under the cursor is equal to x.
.\"}}}
.\"{{{  test-language
.sp .5v
.LP
test-language "x
.IP "" .75c
is true, if the language is set to the one, which is activated by
``O_LANGUAGE x''.
.\"}}}
.\"{{{  counter-0
.sp .5v
.LP
counter-0 <name>
.IP "" .75c
is true, if the variable <name> is 0.
.\"}}}
.\"{{{  counter>0
.sp .5v
.LP
counter>0 <name>
.IP "" .75c
is true, if the variable <name> is greater than 0.
.\"}}}
.\"{{{  test-begin-line
.sp .5v
.LP
test-begin-line
.IP "" .75c
is true, if the cursor stands on or before the first non-space-character
of the current line.
.\"}}}
.\"{{{  test-end-line
.sp .5v
.LP
test-end-line
.IP "" .75c
is true, if the cursor stand behind the last non-space-character of the
current line.
.\"}}}
.\"{{{  test-behind-counter
.sp .5v
.LP
test-behind-counter
.IP "" .75c
is true, if the position in line is after the value of the counter-
variable.
.\"}}}
.\"}}}
.nr PP .75v
.\"}}}
.\"}}}
.\"{{{  OCL examples
.NH 2
OCL examples
.LP
This section explains some useful OCL applications.  They are useful for
understanding OCL.  Perhaps you missed some functions while reading the
previous chapters.  Some of them are written in OCL and you will find
them here.  It is assumed that you are familiar with Origami, before you
start writing in OCL.  Quiche eaters should skip this chapter.
.\"{{{  byebye
.NH 3
A first example
.LP
The first OCL example does not use any control structures.  It defines a
little macro named byebye.  You should play a little bit with such macros
for understanding how to compile them and for getting some syntax errors.
.DS
; this is an example for a macro-definition:
(deffun byebye
(
  O_BOT_OF_FOLD
  O_START_OF_LINE
  O_RETURN
  O_UP
  "end "  "of " "text
))
.sp
(keybind byebye (M-"x "bye))
.DE
byebye creates a new line containing `end of text' at the end of the
entered fold.
.\"}}}
.\"{{{  shell version control
.NH 3
Shell version control
.LP
Two more little macros deal with SVC.  Note that the check-out macro is
only for personal use of SVC.  If you are working in a group, someone
else may have edited the file, which will not change your own read-only
copy.  The advantage is that you can visit a file and decide if you want
to change it.
.DS
(deffun check-out (
  delete-mode-view
  shell-command "co "  "-l "  "$ORIFILE
))

(deffun check-in (
  save-file
  add-mode-view
  shell-command "ci "  "-u "  "$ORIFILE
))
.DE
.\"}}}
.\"{{{  mtool-top
.NH 3
MTool-version of O_TOP_OF_FOLD
.LP
This a macro-version of the command O_TOP_OF_FOLD as its used in the
mtool-system.  The cursor moves to the begin of the currently opened
fold.
.DS
; example for a more complex macro
(deffun mtool-top (
  while and(not(test-begin-fold),not(test-top))
  (
    O_UP
    if test-end-fold
    (
      mtool-top
      O_UP
    ) fi
  )
))
.DE
While the cursor is not being located on top of a fold (entered or
opened) the cursor is moved up. If the macro finds the end-of-fold line
from another opened fold, this fold is skipped by a recursive call of
mtool-top.
.\"}}}
.\"{{{  fill-paragraph
.NH 3
Fill paragraph
.LP
The following example is an advanced macro.  It is too long for printing
it here, but it is included in the file fill-paragraph.
.PP
fill-paragraph fills all lines from cursor position to the next line
with a blank or a dot in the first column.  An end of a fold stops
filling too.  First column of a line is here the first character in
currently entered fold, not opended fold!  Filling a line means to add
words from the next line if it is too short or to put words to the next
line if it is too long.  The default right margin is set to column 72.
If there are unfillable lines, fill-paragraph stops filling and
complains.
.PP
The macro set-fill-column allows to change to right margin.  To do so,
you can either use a numeric argument or put the cursor to the position of
the new right margin or put it to the left and wait for Origami asking
you about the new column.
.\"}}}
.\"{{{  goto-matching-fence
.NH 3
Goto matching fence
.LP
If you are writing programs, you often want to know which closing paren
in a complicated expression fits to another opening paren.
goto-matching-fence performs this function.  It uses the character
under the cursor to determine if are looking for a paren, a bracket or
a { and if you want to jump to the opening or the closing one.  If the
cursor stands somewhere in usual text, nothing will happen.  Hereafter
the macro searches the matching fence.  If it can't find it, it will
jump back to your old position.  If neccessary, this macro opens and
closes folds.  Do not worry if it does not seem to stop searching.  If
your text is very long and there is not fitting fence, you will be
amazed how many folds you are using and how fast they can be opened and
closed, but usually goto-matching-fence goes back to your old position.
.\"}}}
.\"{{{  towers-of-hanoi
.NH 3
The towers of hanoi
.LP
This is a funny macro to demonstrate the power of OCL.  You should type a
string of a few characters without blanks, then start this macro and
have fun.  Warning: If you type more than eight or ten characters, the
game will require a long time.
.\"}}}
.PP
I am sure that you will invent more useful OCL macros.  If you wrote a
really new and good one, please post it to the USENET newsgroup
comp.editors.
.\"}}}
