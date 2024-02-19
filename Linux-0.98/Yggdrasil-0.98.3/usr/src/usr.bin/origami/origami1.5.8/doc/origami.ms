.\"{{{  Notes
.\" This manual should be improved in many points, but it is ok for a first
.\" impression of Origami.  You should have seen the original documentation,
.\" and you would be happy about this document...  On the other side, have you
.\" ever seen the GNU Emacs manual?  It is for people who are able to read the
.\" whole bible while waiting for the bus.
.\" I wrote this manual using Origami and the -ms macros of MROFF, but it
.\" should be formattable with every other member of the roff family.  It does
.\" not need tbl or pic.
.\"
.\" Michael Haardt
.\"}}}
.\"{{{  Title and abstract
.TL
Origami User Guide
.AU
Michael Haardt
.AI
.if "\(oe"" Auf der Hoern 40
.if !"\(oe"" Auf der H\(oern 40
5100 Aachen
Germany
.AB
This document explains the principles of folding editors in general,
because most people have never used such an editor.  It gives an example of
an Origami editing session to show the advantages of folding program sources.
It then explains how to compile and install Origami.  All functions of
Origami are listed and described.  It never refers to concrete keys,
because the Emacs-like and the TDS keybindings are completely different.
This document explains the Origami control language (OCL) detail,
because very complicated editor commands can be created.
.AE
.\"}}}
.\"{{{  Introduction
.NH 1
Introduction
.\"{{{  Origami
.NH 2
Origami
.LP
Origami is a `folding editor', similar to, and inspired by, the editor
included in the Inmos TDS (Transputer Development System).  Those of us
who have become familiar with the TDS editor have long bemoaned the lack
of a similar editor for use with other languages, for example Pascal and
C.
.PP
Origami is not a word processor (although many of its features would be
useful in a word processor), but it comes into its own as a program
editor for structured languages.  The structuring of the code is obvious
from the screen display (if Origami is sensibly used!), although no
actual code may be visible.
.PP
Existing files, not produced using Origami, may be imported into
Origami, and then folded up for future use, and ease of development.
These files may be further edited either using Origami or some other
editor (if you can bear to use one after Origami!), and compiled in the
normal way.
.\"}}}
.\"{{{  The Folding Editor
.NH 2
The Folding Editor
.LP
A `folding editor' extends the principle of tree structured directories
to editing text files.  This allows the simultaneous display of large amounts
of text by `folding' sections of text away behind a descriptive
heading.  This results in a tree structure very similar to a
sub-directory structure of, for example, UNIX.
.PP
By suitable structuring of a text it should be possible, in most
circumstances, to ensure that no display exceeds a single screen at any
time.  To access text which is folded away you can either ENTER the
containing fold, in which case the contents of the fold and its header
are the only displayed text, or you can OPEN the fold, in which case the
contents are displayed in the context of the surrounding text.
.PP
The advantage of this system is that it eliminates the need for
seemingly endless paging through long files to find the section of
interest, allowing you to move down the tree structure, following the
(hopefully) descriptive headers to locate the text you require.
.\"}}}
.\"{{{  Examples of Origami folding
.NH 2
Examples of Origami Folding
.LP
As an example, a Pascal file might look like this at
the top level (eg immediately after starting Origami):
.DS L
program test;
\&...  CONSTs
\&...  TYPEs
\&...  VARs
\&...  procedures
\&...  main body of code
.DE
OPENing the VARs fold might lead to the following display:
.DS L
program test;
\&...  CONSTs
\&...  TYPEs
{\&{\&{  VARs
VAR  i : integer;
     x : real;
}\&}\&}
\&...  procedures
\&...  main body of code
.DE
ENTERing the procedures fold might give:
.DS L
{\&{\&{  procedures
\&...   statistical procedures
\&...   string handling procedures
\&...   procedure user_input
}\&}\&}
.DE
Each of these folds can be ENTERed further until actual code
is reached - for example if we ENTER the
\&`procedure user_input' fold we might find:
.DS L
{\&{\&{  procedure user_input
procedure user_input(prompt : str_type; var reply : str_type);
\&...  local declarations
\&...  local procedures
  begin
    repeat
      write(prompt);
      readln(reply);
    until verify_reply(reply);
  end;
}\&}\&}
.DE
.\"}}}
.\"}}}
.\"{{{  Installing Origami
.NH 1
Installing Origami
.\"{{{  Compiling Origami
.NH 2
Compiling Origami
.LP
Before compiling Origami, you have to generate a makefile for your
machine.  There is a cpp-makefile with explanations on how to do this at
its head.  If you port Origami to a new machine, please change the
cpp-makefile and send me cdiffs for everything.  After generating the
makefile, type make and wait.  Then copy Origami and keybind to
/usr/local/bin.  I suggest to copy the keybind-files and the learn
directory to /usr/pub/installing/origami, where every user can get them
to modify them for his personal use.  At last, you have to make your
choice, which keybindings you want to use.  Read the next section to see
how to install them.
.\"}}}
.\"{{{  Keybindings
.NH 2
Keybindings
.\"{{{  synopsis of keybind
.NH 3
Synopsis of Keybind
.LP
Origami allows to build your own keybinding table.  Therefore Origami
reads a startup file .origamirc in your home directory.  This file binds
all the operations which one wishes to use, to key sequences.  The
operation ``describe-bindings'' displays only the bindings in the file
\&.origamibind in your home directory.  The keybind program compiles a
keybinding-file into these .origamixyz files.  Its synopsis is:
.DS
keybind [-hlsv?] [\-o<namebase>] \fIkeybinding-file\fP
keybind \-k<number>
.DE
Options:
.IP \-s
the entries in bind are sorted
.IP "\-o<name>"
the outputfiles are named $HOME/<name>rc and $HOME/<name>bind instead
of $HOME/.origamirc etc.
.IP \-k<number>
display the names of the origami function with token-value number.  This
is useful, if keybind complains about your keybinding (different
functions bound to the same keysequence etc.).
.IP \-?
display usage
.IP \-h
same
.IP \-l
list all keywords known in origami and keybind.
.IP \-v
during compilation of the sourcefile print information about needed
resources.
.LP
Be careful with keybind, it overwrites existing files!
.\"}}}
.\"{{{  the keybinding file
.NH 3
The keybinding file
.nr PP .25v
.\"{{{  introduction to keybindings and OCL
.sp .5v
.LP
This file contains a list of ASCII rules to bind keysequences to Origami
functions.  It looks like LISP, but it isn't LISP.  The language is
named OCL (Origami control language).  This section describes all
functions used for binding functions to keys.  OCL is described in a
later chapter, because it is not neccessary for keybindings and very
powerful, perhaps to powerful for learning it while installing Origami.
.\"}}}
.\"{{{  defbinding for name of keybinding
.sp .5v
.LP
(defbinding <name-of-binding>)
.IP "" .75c
Defines a name for the binding, which is displayed in the statusline of
Origami.  Multiple usage of this command is not allowed.  The name is
limited in length to 16 characters.
.\"}}}
.\"{{{  keybind
.sp .5v
.LP
(keybind <operation> <keysequence>)
.IP "" .75c
The Origami-operation named <operation> is bound to the keyboard input
<keysequence>.  The file .origamibind contains an entry ``<operation>
<keysequence>''
.\"}}}
.\"{{{  alias
.sp .5v
.LP
(alias <name> <keysequence>)
.IP "" .75c
The keyboard input <keysequence> can be named $<name> in the following
definitions.  Using $<name> puts name in the file .origamibind instead
of the sequence of keys.
.\"}}}
.\"{{{  description of operands
.\"{{{  name
.sp .5v
.LP
\fI<name>\fP is a normal identifier
.\"}}}
.\"{{{  keysequence
.sp .5v
.LP
A \fI<keysequence>\fP is a sequence of <key> enclosed in parentheses.
.\"}}}
.\"{{{  key
.sp .5v
.LP
\fI<key>\fP specifies a single key (or two keys, because M-"a requires two keystrokes):
.IP \fI"x\fP 1i
is a single character
.IP \fIC-X\fP 1i
is control-x
.IP \fIC-?\fP 1i
is the delete-key (127)
.IP \fIM-"x\fP 1i
means Meta-key followed by x (case-sensitive)
.IP \fIM-C-X\fP 1i
META-control-x
.IP \fI$name\fP 1i
represents the keys defined in (alias name ..)
.\"}}}
.\"}}}
.nr PP .75v
.\"{{{  comments and folds
.sp .5v
.LP
Newlines can be used anywhere in the text between syntactical
structures.  Comments can be put in the same way.  A comment starts with
``;'' and ends at the end of the line.  The braces ``{'' and ``}'' have
the same effect as ``;'', so you can use origami with language None.  In
this case filed-folds are included by keybind.
.\"}}}
.\"{{{  default aliases
.\"{{{  terminal independent capabilities
.LP
These aliases can be used without declaration:
.DS
$esc = escape-key = 27
$tab = tab-key = 9
$return = return-key = 13 (yes!)
.DE
.\"}}}
.\"{{{  terminal depedent capabilities
The following default aliases are available, if your terminal has the
termcap capabilities in parentheses: $backspace (kb), $backtab (kB),
$rubout (kD), $clreol (kE), $delline (kL), $home (kh), $end (kH), $up
(ku), $down (kd), $left (kl), $right (kr), $prev_page (kN), $next_page
(kP) $f0 (k0), $f1 (k1), $f2 (k2), $f3 (k3), $f4 (k4), $f5 (k5), $f6
(k6), $f7 (k7), $f8 (k8), $f9 (k9), $f10 (k;)
.PP
Your termcap(5) manual explains all capabilities in detail.  The above
keys mentioned do not represent all capabilities, but those which are
frequently used.
.\"}}}
.\"}}}
.\"}}}
.\"{{{  example
.NH 3
Example
.LP
The following examples are only written to explain the syntax.  They
will not really work, because you will not be able to move the cursor or
even quit Origami.
.\"{{{  first example with return
.DS
; This is a simple keybind-file
(keybind newline-and-indent C-M)
.DE
The operation `newline-and-indent' is activated through C-M.  The
keybindinghelp contains `newline-and-indent C-M'.
.\"}}}
.\"{{{  same example with alias
You can achieve the same keybinding by using an alias:
.DS
; This keybinding uses alias
(alias return C-M)
(keybind newline-and-indent $return)
.DE
The operation `newline-and-indent' is also activated through C-M, but
the keybinding help contains now `newline-and-indent return'.
.\"}}}
.\"}}}
.\"}}}
.\"{{{  Termcap
.NH 2
Required termcap capabilities
.LP
Origami uses the termcap database.  It requires a few capabilities to
work.  It does not need intelligent terminals.  The following
capabilities are required:
.\"{{{  required capabilities
.IP cm
cursor motion
.IP ce
clear to end of line
.IP cd
clear to end of display
.IP co
number of columns
.IP li
number of lines
.\"}}}
.LP
The following capabilities are optional:
.\"{{{  optional capabilities
.IP ti
termcap usage initialization
.IP te
termcap usage end
.IP al
add line
.IP dl
delete line
.IP so
stand out
.IP se
standout end
.IP sg
standout glitch
.\"}}}
.\"}}}
.\"{{{  Bugs and limitations of Origami
.NH 2
Limitations of Origami
.LP
Origami stores the complete file in memory while editing which limits
the size of files to 24 kByte on PC MINIX 1.5.10.  The length of a line
is limited to 256 characters, and the depth of folding is limited to
256.  When reading files, longer lines are splitted, but indenting folds
may generate long lines.  This case isn't handled correct.  Origami
expands tab characters to blanks.  You should never edit Makefiles with
Origami, because make needs tab characters.  Otherwise, it is unable to
work.  In my opinion, this is a (documented!) bug in make.
.\" I know of only one serious problem: If one tries
.\" to undelete a filed fold which contains another filed fold, origami
.\" becomes unhappy in a hurry.  This has to do with the dispose chain
.\" somehow.
If your file contains `{\&{\&{', `:\&:\&:\&', `.\&.\&.' or
\&`}\&}\&}', Origami will think of them as folding marks!  Origami may
die on such things.  There is a problem editing ``C''-files, if you
use the preprocessor-mark ``#'' within the fold-structure.  Origami only
allows input, when the current position is behind current indentation.
Use option \-p to change the position of leading ``#'' while writing or
reading files.
.\"}}}
.\"}}}
.\"{{{  Using Origami
.NH 1
Using Origami
.\"{{{  Getting started
.NH 2
Getting Started
.LP
At this point, you have the choice either to read the next chapters
about using Origami or to use the tutorial to learn by doing.  This
tutorial is similar to the well-known teach-emacs tutorial.  You need a
copy of the learn directory.  Change to this directory and type start-me
<return>.  This tutorial must be used in Emacs-mode.
.PP
It is assumed, that Origami is installed and ready to use on your
system.  It is invoked by typing `origami' after the system prompt.
Filenames me may be typed after `origami' \(em if this is not done a
prompt for a filename will be issued.
.PP
If the file being read contains tab characters, Origami will ask for the
tab size in order to preserve your formatting.  If you have
no read permissions for the specified file, Origami will switch to
view-only mode.  Look at the manual page for advanced command line
options.
.\"}}}
.\"{{{  The screen format
.NH 2
The Screen Format
.LP
On starting an Origami session, a command bar appears near the bottom of
the screen.  This line will remain throughout your edit, and any prompts
or warnings issued by the editor will appear below this line.  If you
started typing a key sequence bound to a command, this sequence will be
displayed under the status line, too.  If the second character of the
line is a star, your file contains unsaved changes.  The
current language and the active modes are indicated in parentheses:
.IP DefMac/D 1i
is displayed during defining a keyboardmacro.
.IP ExMac/E 1i
is displayed during the execution of a macro (keyboard or fix).
.IP View/V 1i
is displayed, if origami is in view-only mode.
.IP Over/O 1i
Origami is in overwrite-mode.
.IP ArgRep/A 1i
Origami is in argument-repeat-mode.
.IP AutoSave/S 1i
Origami is in auto-save-mode.  Origami will save your
files periodically.
.LP
The short forms are used, if the statusline becomes too long.  If
there is a name defined in your keybinding file, it is displayed
to the left of the language indicator.
.\"}}}
.\"{{{  The editor buffers
.NH 2
The Editor Buffers
.LP
There are three buffers in Origami.  The `move buffer' that contains at
most one textline, the `pick buffer' in which several lines can be
accumulated before inserting them back into the file, and the `delete
buffer', which holds the last deleted line (in each case a line can be a
closed fold, with all its contents).  The operations of these buffers in
Origami are slightly different from TDS \(em move, delete and pick
buffers are lost on exit from Origami.
.\"}}}
.\"{{{  Processing Origami files
.NH 2
Processing Origami Files
.LP
If a saved file should pass a particular compiler, the fold
structure must appear to the compiler as comments.  As comments differ
from one language to another, this means choosing the language before
saving a new file for the first time, although on reading a file back
into Origami the language type is retained (provided the file contained
at least one fold).  As a consequence of this you should take care not
to include strings which the compiler would recognise as comments within
the fold header, as this will lead to nested comments \(em not allowed
in some languages.
.PP
Origami knows the languages None, Occam, Pascal, C, Lisp, Roff, Fortran,
User and Tds.  You can specify your own comment strings using language
User.  Tds allows to edit files generated by Tds and MultiTool without
converting them.
.\"}}}
.\"}}}
.\"{{{  All Origami functions
.\" needed because of file fold
.so functions.ms
.\"{{{F functions.ms
.\":::F functions.ms
.\"}}}
.\"}}}
.\"{{{  Programming in OCL
.\" needed because of file fold
.so ocl.ms
.\"{{{F Programming in OCL
.\":::F ocl.ms
.\"}}}
.\"}}}
.\"{{{  Miscellanous things
.NH 1
Miscellanous things
.LP
Sometimes, it is useful to know if a document contains ``dangerous''
strings for Origami.  The following simple shell script searches these
strings.
.DS
fgrep '{\&{\&{' $1
fgrep '}\&}\&}' $1
fgrep ':\&:\&:' $1
.DE
Another shellscript takes all files from the current directory and
produces one Origami file consisting of filefolds for all these files:
.DS
for i in *.c *.h *.p *.tex *.ms *.1
do
  if [ -f $i ]
  then
    echo '{\&{\&{F ' $i
    echo ':\&:\&:F ' $i
    echo '}\&}\&}'
  fi
done
.DE
The resulting document is send to stdout.
.\"}}}
.\"{{{  Acknowledgements
.NH 1
Acknowledgements
.LP
Most of this manual was written by me, but the original was written by
someone else, who didn't leave his name in it.  The concept of the
folding editor was originally implemented as part of the Transputer
development system at INMOS Ltd., Bristol, UK.  The editor was a part of
the TDS D700, and was written in Occam.  It first ran on a 68000 based
Sage/Stride development system, and later was Transputer resident on the
PC development system.
.PP
Origami was rewritten from scratch in Turbo Pascal by Martin Green while
at Ferranti Ltd, Cheadle Heath, Stockport, England, and used the same
key mappings and editor functions as the TDS editor.  It was then
converted from Turbo Pascal by p2c and ported by Charlie Lawrence, while
at Eastman Kodak Co.  He added lex/yacc based keyfiles, and did the
considerable work needed to get it running in C.  I fixed some bugs in
memory managment and otherwhere, ported it to MINIX, changed its user
interface to look more Emacs-like and added new features.  A friend of
mine, Wolfgang Stumvoll, wrote the OCL code and the keybind compiler.
He also made beta tests of all my new versions by using them for his
daily work, which helped a lot to find existing and new bugs.  Last but
not least, he wrote the OCL fill-paragraph, goto-matching-fence and
towers-of-hanoi macros.  Both of us made a general code clean-up, which
was highly overdue.
.\"}}}
