{{{  Introduction
Introduction
------------
This file contains additional information about the ATARI ST version of
ORIGAMI. Please refer to the ORIGAMI manuals for a general introduction to
ORIGAMI and to the ORIGAMI man page for a description of advanced command line
options.
}}}
 
{{{  Installing ORIGAMI
Installing ORIGAMI
------------------
To install ORIGAMI, simply copy the file ORIGAMI.PRG to your hard disk or to
your working disk. Origami then needs a keybinding-file called ORIGAMI.RC and
- optional - the files ORIGAMI.BIN (key binding description, displayed
through the 'describe-bindings' function) and ORIGAMI.HEL (help file,
displayed through the HELP function). If you are using the desktop, these
files can simply be copied to the same folder which contains ORIGAMI.
 
If you are using a shell like gulam, mupfel etc., you should set up an
environment variable HOME (the mupfel default installation does this
automatically). Then copy the above mentioned files to the folder which the
environment variable HOME points to. example:
 
  SETENV HOME d:\gdist1_2\gemini
  (this command should be placed in your shell startup file, eg the file
  MUPFEL.MUP if you are using MUPFEL. Refer to your shell documentation for
  details)
  In this case, you should copy the files ORIGAMI.RC, ORIGAMI.BIN and
  ORIGAMI.HEL to the folder d:\gdist1_2\gemini
 
Copy the file ORIGAMI.PRG to any place you wish to - but take care that your
shell can find ORIGAMI - see your shell manual for details about PATHs etc.
Although ORIGAMI is a GEM-based program, it will take command line arguments
as described in the docs. As far as I know, there is no possibility to pass
parameters to a GEM program when using the Desktop (who is really using the
Desktop ?)
 
If you do not set the environment variable HOME, ORIGAMI looks for these
files in the current working directory.
 
If you do not have a hard disk, simply copy the files ORIGAMI.PRG,
ORIGAMI.RC, ORIGAMI.BIN and ORIGAMI.HEL to a folder (or, maybe better, to
the root directory) of your working disk.
 
The files ORIGAMI.RC and ORIGAMI.BIN are created automatically by the KEYBIND
compiler which offers a complete redefinition of the keyboard as well as a
powerful macro programming language. For a start, you can simply copy these
files from your distribution disk (if they came with the binaries) and learn
about KEYBIND later. Note that KEYBIND overwrites these files when it is
executed, so be sure to have a copy of them - ORIGAMI will refuse to start
without a proper keybinding!
}}}
 
{{{  Starting ORIGAMI
Starting ORIGAMI
----------------
To start ORIGAMI, double-click on ORIGAMI.PRG from the desktop or type
'origami' from your shell. Origami then prompts for a file to edit. Simply
enter the name of a not-existing file, ORIGAMI will then come up with an
empty editing buffer.
 
You should now take the ORIGAMI manual (perhaps it would be a good idea to
print it out first) and try some functions. When using the default ORIGAMI.RC
and ORIGAMI.BIN from the ST folder of your ORIGAMI distribution, most keys
will behave like they do in many other ST programs. By pressing the HELP key,
a keybind-description is displayed which can be used to find out which keys
activate which functions. The functions not listed in the ORIGAMI manual (eg
towers-of-hanoi) are OCL macros. Refer to the ST keybinding source file ST.KEY
for details about using these macros.
}}}
 
{{{  The ST keyboard and GEM menu bar
The ST keyboard
---------------
When generating own keybindings using the KEYBIND compiler, you will need some
additional knowledge about the ST specific keyboard routines used in ORIGAMI.
Nearly every ST key can be defined differently in combination with the Shift,
CTRL and ALT keys. Combinations of SHIFT, CTRL and ALT are not allowed, the
keys are checked in the order first ALT, then CTRL, then SHIFT. Pressing
CTRL-ALT-'A' achieves the same result as ALT-'A' as ALT is checked first.
 
The ST keys generate two types of keyboard codes:
 - normal ASCII and Control-Codes , e.g. the letters a..z and most of the
   combinations of SHIFT / CTRL and a letter.
 
 - some keys generate 'meta'-codesequences. These consist of a meta-code and
   a keycode. There are four meta-codes depending on the state of the Shift/
   CTRL/ALT keys. These meta codes are called 'meta' (none of the keys Shift/
   CTRL/ALT pressed), 'smeta' (Shift pressed) ,'cmeta' (CTRL pressed) and
   'ameta' (ALT pressed). They can be accessed under these names in the
   keybinding (see the ST sample keybinding file ST.KEY for examples).
   This method is a bit unusual, but it enables you to define nearly every
   keyboard layout you might want.
   The GEM menu bar creates sequences of 'mmeta' and a character. Although
   the menu bar itself is fixed, this method enables it to redefine the
   menu functions - in the sample ST keybinding, the file functions are
   bound to macros which activate the GEM fileselector (works only if the
   current file was not changed). Keybindings which contain 'mmeta'-codes
   are not listed in the describe-bindings list.
   The GEM screen redraw message creates the sequence 'mmeta','1' and should
   be bound to the ORIGAMI function 'redraw-display'. Note that ORIGAMI always
   redraws the whole screen and so it will not work together with accessories
   which open own windows (e.g. the CONTROL.ACC). It may be possible to use
   such accessories if you do not try to move their windows - otherwise the
   window will disappear instantly and you will have to click around to find
   the window closer (the window is still active - just not visible).
 
On the ST, the 'GENERAL ABORT KEY' can not be freely defined as described
in the ORIGAMI manuals. GENERAL ABORT is activated by pressing both SHIFT
keys together (you may have to press them for about 0.5 seconds). You will
most probably not see any effect until you release the keys.

The following aliases are pre-defined on the ST (this are all aliases which
MAY be available on UN*X via the termcap library if the terminal supports
them):

"backspace"
"rubout"    (DEL key)
"home"
"left"
"right"
"up"
"down"
"next_page" (CTRL-down)
"prev_page" (CTRL-up)
"f1" - "f10"  (function keys)

Here's a list of ST keys which generate CTRL-codes or meta-sequences. Listed
are the name of the ST key (on a german ST keyboard) and the codesequence
which is generated by this key. The codesequences are given in keybind-syntax
- look at the file ST.KEY for examples on using these special ST keys.
 
If something in the translation tables is wrong, changes can be made in the
file ST_KEYB.C. If you have problems because of a non-german keyboard layout,
please try to remap the keys by changing the keybinding before making changes
to the ORIGAMI C-code - I want to avoid several versions of ORIGAMI. I know
that some of the meta-sequences are not very logical - but please keep them
as they are in order to avoid confuzion with different ORIGAMI versions. It
should not bother too much as you get to see these sequences only once when
defining aliases for your the special keys used in your keybinding.

{{{  ST Keyboard code table
ST Keyboard code table
----------------------
{{{  normal:
normal:
-------
ESC ( = M-xxx in keybind)               $esc
Backspace                               C-H
Delete                                  $meta C-G
Return                                  C-M
TAB                                     C-I
HELP                                    $meta C-V
UNDO                                    $meta C-U
INSERT                                  $meta C-F
Up                                      $meta C-B
Clr/Home                                $meta C-A
arrow left                              $meta C-C
arrow down                              $meta C-E
arrow right                             $meta C-D
num. keypad (                           $meta  "(
num. keypad )                           $meta  ")
num. keypad /                           $meta  "/
num. keypad *                           $meta  "*
num. keypad 0                           $meta  "0
num. keypad 1                           $meta  "1
(...)
num. keypad 9                           $meta  "9
num. keypad -                           $meta  "-
num. keypad +                           $meta  "+
num. keypad ENTER                       $meta C-M
num. keypad .                           $meta  ".
F1                                      $meta C-J
F2                                      $meta C-K
F3                                      $meta C-L
F4                                      $meta C-N
F5                                      $meta C-O
F6                                      $meta C-P
F7                                      $meta C-Q
F8                                      $meta C-R
F9                                      $meta C-S
F10                                     $meta C-T
}}}
 
{{{  with SHIFT pressed:
with SHIFT pressed:
-------------------
ESC                                     $smeta $esc
Backspace                               $smeta C-H
TAB                                     $smeta C-I
Delete                                  $smeta C-G
Return                                  C-M
(same as RETURN without Shift)
HELP                                    $smeta C-V
UNDO                                    $smeta C-U
INSERT                                  $smeta C-F
arrow up                                $smeta C-B
Clr/Home                                $smeta C-A
arrow left                              $smeta C-C
arrow down                              $smeta C-E
arrow right                             $smeta C-D
num. keypad (                           $smeta "(
num. keypad )                           $smeta ")
num. keypad /                           $smeta "/
num. keypad *                           $smeta "*
num. keypad 0                           $smeta "0
(...)
num. keypad 9                           $smeta "9
num. keypad -                           $smeta "-
num. keypad +                           $smeta "+
num. keypad ENTER                       $smeta C-M
num. keypad .                           $smeta ".
F1                                      $smeta C-J
F2                                      $smeta C-K
F3                                      $smeta C-L
F4                                      $smeta C-N
F5                                      $smeta C-O
F6                                      $smeta C-P
F7                                      $smeta C-Q
F8                                      $smeta C-R
F9                                      $smeta C-S
F10                                     $smeta C-T
}}}
 
{{{  with CTRL pressed:
with CTRL pressed:
------------------
ESC                                     $cmeta $esc
1                                       $cmeta "1
2                                       $cmeta "2
(...)
9                                       $cmeta "9
0                                       $cmeta "0
ž                                       $cmeta "?
'                                       $cmeta "`
#                                       $cmeta "#
Backspace                               $cmeta C-H
TAB                                     $cmeta C-I
Q                                       C-Q
W                                       C-W
E                                       C-E
R                                       C-R
T                                       C-T
Z                                       C-Z
U                                       C-U
I                                       $cmeta "I
O                                       C-O
P                                       C-P
š                                       $cmeta "@
+                                       $cmeta "+
RETURN                                  $cmeta C-M
DELETE                                  $cmeta "q
A                                       C-A
S                                       C-S
D                                       C-D
F                                       C-F
G                                       C-G
H                                       $cmeta "H
J                                       C-J
K                                       C-K
L                                       C-L
™                                       $cmeta "[
Ž                                       $cmeta "]
~                                       $cmeta "~
<                                       $cmeta "<
Y                                       C-Y
X                                       C-X
C                                       C-C
V                                       C-V
B                                       C-B
N                                       C-N
M                                       $cmeta "M
,                                       $cmeta ",
.                                       $cmeta ".
-                                       $cmeta "-
HELP                                    $cmeta "s
UNDO                                    $cmeta "r
INSERT                                  $cmeta "p
arrow up                                $cmeta "l
Clr/Home                                $cmeta "k
arrow left                              $cmeta "m
arrow down                              $cmeta "o
arrow right                             $cmeta "n
num. keypad (                           $cmeta "(
num. keypad )                           $cmeta ")
num. keypad /                           $cmeta "/
num. keypad *                           $cmeta "*
num. keypad 0                           $cmeta "A
num. keypad 1                           $cmeta "B
num. keypad 2                           $cmeta "C
num. keypad 3                           $cmeta "D
num. keypad 4                           $cmeta "E
num. keypad 5                           $cmeta "F
num. keypad 6                           $cmeta "G
num. keypad 7                           $cmeta "H
num. keypad 8                           $cmeta "I
num. keypad 9                           $cmeta "J
num. keypad -                           $cmeta "_
num. keypad +                           $cmeta "=
num. keypad ENTER                       $cmeta ";
num. keypad .                           $cmeta ":
F1                                      $cmeta "a
F2                                      $cmeta "b
F3                                      $cmeta "c
F4                                      $cmeta "d
F5                                      $cmeta "e
F6                                      $cmeta "f
F7                                      $cmeta "g
F8                                      $cmeta "h
F9                                      $cmeta "i
F10                                     $cmeta "j
}}}
 
{{{  with ALT pressed:
with ALT pressed:
-----------------
ESC                                     $ameta $esc
1                                       $ameta "1
2                                       $ameta "2
(...)
9                                       $ameta "9
0                                       $ameta "0
ž                                       $ameta "?
'                                       $ameta "`
#                                       $ameta "#
Backspace                               $ameta C-H
TAB                                     $ameta C-I
A                                       $ameta "A
B                                       $ameta "B
C                                       $ameta "C
(...)
Z                                       $ameta "Z
+                                       $ameta "+
RETURN                                  $ameta C-M
DELETE                                  $ameta "q
~                                       $ameta "~
<                                       $ameta "<
,                                       $ameta ",
.                                       $ameta ".
-                                       $ameta "-
HELP/Undo/cursor/Insert/ClrHome cannot be used together with ALT
num. keypad (                           $ameta "(
num. keypad )                           $ameta ")
num. keypad /                           $ameta "/
num. keypad *                           $ameta "*
num. keypad -                           $ameta "_
num. keypad +                           $ameta "=
num. keypad ENTER                       $ameta ";
num. keypad 0                           $ameta C-A
num. keypad 1                           $ameta C-B
num. keypad 2                           $ameta C-C
num. keypad 3                           $ameta C-D
num. keypad 4                           $ameta C-E
num. keypad 5                           $ameta C-F
num. keypad 6                           $ameta C-G
num. keypad 7                           $ameta C-J
num. keypad 8                           $ameta C-K
num. keypad 9                           $ameta C-L
F1                                      $ameta "a
F2                                      $ameta "b
F3                                      $ameta "c
F4                                      $ameta "d
F5                                      $ameta "e
F6                                      $ameta "f
F7                                      $ameta "g
F8                                      $ameta "h
F9                                      $ameta "i
F10                                     $ameta "j
}}}
 
{{{  GEM menu codes:
GEM menu codes:
---------------
  FILE-Quit:                    $mmeta,'b'
  FILE-Save&Quit:               $mmeta,'c'
  FILE-Read new file:           $mmeta,'d'
  FILE-Write to file:           $mmeta,'e'
  FILE-Save:                    $mmeta,'f'
  FILE-Insert file:             $mmeta,'g'
  FILE-TOS shell:               $mmeta,'h'
  FILE-Shell command:           $mmeta,'i'
  EDIT-double line:             $mmeta,'j'
  EDIT-move line:               $mmeta,'k'
  EDIT-Pick line:               $mmeta,'l'
  EDIT-Copy-pick line           $mmeta,'m'
  EDIT-Fold pick buffer:        $mmeta,'n'
  EDIT-Delete line:             $mmeta,'o'
  EDIT-Undelete line:           $mmeta,'p'
  GOTO-Line:                    $mmeta,'q'
  GOTO-Top of fold:             $mmeta,'r'
  GOTO-Bottom of fold:          $mmeta,'s'
  GOTO-Search:                  $mmeta,'t'
  GOTO-Search reverse:          $mmeta,'v'
  GOTO-Replace:                 $mmeta,'w'
  GOTO-Query-replace:           $mmeta,'x'
  FOLDING-Open fold:            $mmeta,'y'
  FOLDING-Close fold:           $mmeta,'z'
  FOLDING-Enter fold:           $mmeta,'A'
  FOLDING-Exit fold:            $mmeta,'B'
  FOLDING-Unfold fold:          $mmeta,'C'
  FOLDING-Create fold:          $mmeta,'D'
  FOLDING-Create auto fold:     $mmeta,'E'
  FOLDING-Auto header:          $mmeta,'F'
  FOLDING-File fold:            $mmeta,'G'
  FOLDING-Attach file:          $mmeta,'H'
  FOLDING-Describe fold:        $mmeta,'I'
  MACROS-Define macro:          $mmeta,'J'
  MACROS-Execute macro:         $mmeta,'K'
  MACROS-Define fix mac:        $mmeta,'a'
  MACROS-Execute fix 1:         $mmeta,'P'
  MACROS-Execute fix 2:         $mmeta,'Q'
  MACROS-Execute fix 3:         $mmeta,'R'
  MACROS-Execute fix 4:         $mmeta,'S'
  MACROS-Execute fix 5:         $mmeta,'T'
  SPECIAL-Help:                 $mmeta,'L'
  SPECIAL-Describe bindings:    $mmeta,'M'
  SPECIAL-Set language:         $mmeta,'N'
 
  GEM-Redraw-message: (not a menu function, created by screen manager)
                      $mmeta,'1'
}}}
}}}
 
}}}
 
{{{  GEM Fileselector
GEM Fileselector
----------------
You can invoke the GEM fileselector by pressing RETURN when ORIGAMI asks for a
filename. This enables macros to enter a fixed filename which would not be
possible if the fileselector would appear automatically.
}}}
 
{{{  The ST Screen implementation
The ST Screen implementation
----------------------------
All output of ORIGAMI is printed via the VDI. This should work on nearly every
type of monitor.
The VDI output is not as fast as TEMPUS etc., but it should be compatible to
the TT and some special ST graphic cards. It was tested on a normal 1040ST and it
also works well with Autoswitch-Overscan.
  For speed reasons, the mouse and text cursor are only displayed when in edit
mode, not during the execution of a command. This is not ideal, but it was
a major speedup against the previous version which turned the mouse off before
every text output and on afterwards. If someone has a better method, tell me
(no Line-A or similar things please!)
  The screen routines always do a full-screen redraw, no matter what the GEM
screen manager requests. This is a problem if you use accessories which have
own windows which stay open while ORIGAMI is running - ORIGAMI will overwrite
these windows and you will have to search the closer (invisible, but still
working).
}}}
 
{{{  The ST version of KEYBIND
The ST version of KEYBIND
-------------------------
The ST version behaves much like the SUN/UN*X version described in the ORIGAMI
docs. However, as the ST OS (can we really call this thing an OS ?) does not
support pipes, a separate program called SORT.TTP is called automatically if
you start keybind with the '-s' option (which is highly recommended). SORT is
executed via a system()-call - this means it only works if a shell like MUPFEL
or GULAM is available.
 
If the environment variable HOME is not set, KEYBIND.TTP creates the files
ORIGAMI.BIN and ORIGAMI.RC in the current working directory.

Keybindings containing 'mmeta'-codes are not listed in the ORIGAMI.BIN-file,
as they only contain internal codes which would not be of any use.
}}}
 
{{{  Known bugs
Known bugs
----------
When you start TurboC from GEMINI, then use the 'TOS SHELL' function to create
a MUPFEL sub shell and then start ORIGAMI from that shell, the mouse will not
appear. This seems to be a bug in TurboC or in Mupfel, as other programs have
the same problems when started that way.
}}}
 
{{{  Compiling Origami
Compiling Origami
-----------------
If you want to compile ORIGAMI yourself, you will need TURBO-C 2.0. In order
to compile ORIGAMI you have to do the following:
 - unpack the source archive
 - convert LF at end of line to CR/LF (using UNX2ST or some other utility, some
   text editors will work too - simply load and save the files). Maybe you
   do not have to convert anything because it was done when downloading the
   files in ASCII mode ...
 - create a LOCAL subdirectory inside your TC 'include' directory and copy
   the files from ORIGAMI\SRC\LOCAL there.
 - start the program UNIXNAME.PRG. This does a translation of unix-line '/'
   to a tos-line '\' in filenames. If you haven't got UNIXNAME, you will have
   to change all #include statements manually. UNIXNAME is a resident utility
   that only needs to be executed once after booting your computer (maybe in
   the AUTO folder). UNIXNAME should come with the ST specific parts of
   ORIGAMI - but I can not guarantee because the posted version of ORIGAMI
   is put together by someone else ...
 - start TC, select the project file SRC\ORIGAMI\ORIGAMI.PRJ. Disable ALL
   compiler and linker options (except debug info if required) - they are set
   in the project file.
 - select 'MAKE'. If TC complains about non-existent include files - that is
   OK. Simply create an empty file with the required name. I remember the
   files UNISTD.H, TERMCAP.H, SGTTY.H, PWD.H and SYS\TYPES.H (create SYS
   subdirectory in INCLUDE directory). Maybe there are more files which I
   forgot. TC should now compile ORIGAMI - this takes some time. Some warnings
   are OK, but there should be no errors.
 - The GEM ressource data is compiled directly into the program (module
   ORI_RSC.C) in order to avoid loading extra RSC-files (bad when ORIGAMI
   is called from a shell). This C-file is created by a special utility which
   I will post soon.

WARNING: in the file SRC\ORIGAMI\OS.H, there is a recursive #define for
         strlen(). If you want to use a different compiler than TC, this might
         cause problems.
 
Now, you have to compile KEYBIND:
 - again, make sure UNIXNAME is activated
 - convert CR/LF as mentioned above
 - copy the file SRC\TOS\KEYBIND.PRJ to SRC\KEYBIND\
 - select project file SRC\KEYBIND\KEYBIND.PRJ
 - disable all compiler and linker options
 - select MAKE. There may be warnings, but no errors.
 
Now you are ready to compile the sample ST.KEY keybinding (or simply use the
provided, pre-compiled ORIGAMI.RC and ORIGAMI.BIN if they came with the
program) and make changes to the code.
AS MICHAEL HAARDT WHO MAINTAINS THE UN*X VERSION OF ORIGAMI DOES NOT
LIKE NON-POSIX STUFF IN THE SOURCES (EVEN IF I USE #IFDEF), YOU SHOULD
RESTRICT CHANGES TO THE ST-SPECIFIC FILES IN THE TOS FOLDER IF IT IS POSSIBLE
 - THIS INCREASES THE CHANCE THAT THESE CHANGES CAN BE SUPPORTED IN THE NEXT
VERSION OF ORIGAMI.
I do not want to apply the ST-specific changes in the ORIGAMI main code every
time a new ORIGAMI/UNIX version comes out (most improvements are first made
in the UN*X version and then ported to other systems)
}}}

{{{  Bugfixes
Bugfixes
--------
If you have discovered new bugs in ORIGAMI, please send me a note, if possible
with a clear description of a method by which the problem can be reproduced.
If you have a fix for that bug - even better, use cdiff to create a file that
contains the differences between the original ORIGAMI code and your code, then
send me this file. DO NOT SEND THE COMPLETE SOURCE, ONLY DIFFS!
}}}

If I forgot to document some special ST feature (which is not unlikely, because
I wrote this text after finishing ORIGAMI) - please tell me.

(email: send any comments to Michael Haardt, he will forward them to me)
Michael Schwingen
Ahornstr. 36
W-5100 Aachen
Germany

