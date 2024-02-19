/*
 *	HISTORY.C:	Revision History for
 *
 *	MicroEMACS 3.11
 *		written by Daniel M. Lawrence
 *		based on code by Dave G. Conroy.
 *
 *	(C)Copyright 1988,1989,1990,1991 by Daniel M. Lawrence
 *	MicroEMACS 3.11 can be copied and distributed freely for any
 *	non-commercial purposes. MicroEMACS 3.11 can only be incorporated
 *	into commercial software with the permission of the current author.
 *
 * REVISION HISTORY:
 *
 * 1.0	Steve Wilhite, 30-Nov-85
 *	- Removed the old LK201 and VT100 logic. Added code to support the
 *	  DEC Rainbow keyboard (which is a LK201 layout) using the the Level
 *	  1 Console In ROM INT. See "rainbow.h" for the function key defs
 *	Steve Wilhite, 1-Dec-85
 *	- massive cleanup on code in display.c and search.c
 *
 * 2.0	George Jones, 12-Dec-85
 *	- Ported to Amiga.
 *
 * 3.0	Daniel Lawrence, 29-Dec-85
 *	- rebound keys/added new fast buffered I/O for AMIGA
 *	- added META- repeat commands
 *	- added reposition default to center screen (yeah!)
 *	- changed exit with modified buffers message
 *	- made filesave tell us what it is doing
 *	- changed search string entry to terminate with <ESC>
 *	  so we can use <NL> in search/replace strings
 *	- updated version number in mode line to 3.0
 *	12-Jan-86
 *	- Added code to reconize the search/replace functions
 *	- Added code to perform search/replace & query functions
 *	14-Jan-86
 *	- moved search logic to separate function in search.c
 *	- added replace and query replace functions
 *	- separated out control key expansions to be used by others in search.c
 *	15-Jan-86
 *	- changed "visiting" to finding
 *	- changed yes/no responses to not need return
 *	- cleaned up various messages
 *	16-jan-86
 *	- fixed spurious spawn message in MSDOS
 *	- added ^X-S synonym to save command
 *	- moved escape to shell to ^X-C
 *	21-jan-86
 *	- added code to suspend shell under BSD
 *	22-jan-86
 *	- added function key support (SPEC) under MSDOS
 *	- Abort now prints [Aborted] on message line
 *	23-jan-86
 *	- Added modes and commands to set/unset them
 *	24-jan-86
 *	- Added Goto Line command
 *	- added Rename Buffer command
 *	28-jan-86
 *	- added goto beginning and end of paragraph commands (META-P/META-N)
 *	- re-wrote kdelete to use realloc. Gained MUCH speed here when
 *	  doing large wipes on both UNIX and MSDOS. Changed kill buffer
 *	  allocation block size from 256 bytes to 1 k
 *	29-jan-86
 *	- moved extern function declarations to efunc.h
 *	- made name[] name binding table
 *	30-jan-86
 *	- fixed Previous/Next paragraph command not to wrap around EOF
 *	- added Fill Paragraph command (META-Q)
 *	4-feb-86
 *	- added code to properly display long lines, scrolling them right
 *	  to left
 *	5-feb-85
 *	- rewrote code to right/left scroll...much better
 *	- added shifted arrow keys on IBMPC
 *	6-feb-85
 *	- add option to allow forward-word to jump to beginning of
 *	  next word instead of end of current one. This is different from
 *	  other emacs' but can be configured off in estruct.h
 *	- added VIEW mode to allow a buffer to be read only
 *	   (-v switch on command line will activate this)
 *	- changed quick exit to write out ALL changed buffers!!!
 *	  MAKE SURE YOU KNOW THIS WHEN META-Zing
 *	10-feb-86
 *	- added handling of lines longer than allowed on file read in
 *	  (they wrap on additional lines)
 *	- made having space clear the message line and NOT insert itself
 *	  a configuration option in ed.h
 *	11-feb-86
 *	- added Describe-command and Help commands.
 *	13-feb-86
 *	- added View file command (^X ^V) and finished HELP command
 *	14-feb-86
 *	- added option to let main loop skip update if type ahead commands
 *	   are queued up
 *	16-feb-86
 *	- added Insert File command
 *	17-feb-86
 *	- added scroll next window up/down commands
 *	18-feb-86
 *	- added CMODE indentation
 *	- re-arranged header files to standardize extern and global
 *	  definitions
 *	- changed version number to 3.2
 *	- added numeric arguments to search, reverse search and
 *	  search and replace
 *	24-feb-86
 *	- added Bind To Key function (^C for now) to allow the user
 *	  to change his command keys
 *	- added Unbind key function (M-^C for now)
 *	- added execute named command to execute unbound commands (M-X)
 *	- added describe bindings command (not bound)
 *	- changed version number to 3.3
 *	25-feb-86
 *	- scrapped CERROR mode (too many compilers)
 *	- added EXACT mode for case sensitive searchers
 *	26-feb-86
 *	- added command completion on execute named command and
 *	  all routined grabbing a command name
 *	- adding execute-command-line command and its support functions
 *	  (in preparation for sourcing files)
 *	- added Execute Buffer command
 *	27-feb-86
 *	- added execute(source) file command and added code to automatically
 *	  execute emacs.rc (or .emacsrc on UNIX) before initial read in
 *	- changed version number to 3.4
 *	4-mar-86
 *	- changed word delete to be consistant with word move (it gets
 *	  rid of the inter word space now) This is configurable with the
 *	  NFWORD symbol in estruct.h
 *	- added B_ACTIVE entry to the buffer table. Let emacs read multiple
 *	  file names from the command line and only read them in as needed
 *	5-mar-85
 *	- rewrote command line parser to get rid of my patchy code
 *	- changed version number to 3.5
 *	1-apr-86
 *	- added support for Aztec C 3.20e under MSDOS
 *	- fixed bug in mlwrite on ADM3's and their ilk under V7
 *	- added insertion of pounds in column one under CMODE
 *	- changed version number to 3.6
 *	3-apr-86
 *	- added next-buffer command (^X-X)
 *	5-apr-86
 *	- added kill paragraph command (M-^W)
 *	- changed fill-paragraph to leave 2 spaces after a period at the
 *	  end of a word.
 *	- added OVERWRITE mode
 *	7-apr-86
 *	- fixed overwrite mode to handle tabs
 *	8-apr-86
 *	- added add/delete global mode (<ESC>M & <ESC> ^M) commands
 *	9-apr-86
 *	- added insert space command
 *	- moved bindings around 	^C	insert space
 *					M-K	bind-to-key
 *					INSERT	insert space
 *					DELETE	forwdel
 *	- added hunt forward and hunt reverse commands
 *	10-apr-86
 *	- fixed bug in DOBUF with non-terminated command string
 *	15-apr-86
 *	- fixed tab expansion bug in DISPLAY which hung the AMIGA
 *	  (sent in by Dawn Banks)
 *	- fixed curcol problen if forwline/backline during keyboard
 *	  macro execution (sent in by Ernst Christen)
 *	- added AMIGA function/cursor key support
 *	- fixed nonterminating <NL> replacement bug
 *	- fixed word wrapping problems
 *	16-apr-86
 *	- updated documentation and froze development for 3.6 net release
 *	23-apr-86	version 3.6a
 *	- added foreground and background colors. Setable with the
 *	  add mode commands for the moment
 *	24-apr-86
 *	- added command to pipe CLI output to a buffer
 *	25-apr-86
 *	- added Dana Hoggatt's code to replace Lattice's sick system()
 *	  function. Now we no longer care what the switchar is.
 *	- cleaned up the positioning on several of the spawing commands
 *	26-apr-86
 *	- added an output flush in vttidy(). Unix really appreciates this.
 *	- added filter-buffer (^X#) command to send a buffer through
 *	  a DOS filter
 *	- made automatic CMODE on .c and .h file compilation dependant
 *	  in estruct.h
 *	1-may-86
 *	- optimized some code in update(). It certainly needs a lot more.
 *	- added Aztec profiling capabilities. These are conditional on
 *	  the APROF symbol in estruct.h
 *	2-may-86
 *	- added (u)ndo command in query-replace. undoes last repalce.
 *	6-may-86
 *	- re-organized and wrote the update() function in display.c
 *	  Now my color hacks are in the right places and the code can be
 *	  understood.
 *	[Released version 3.6f for BETA test sites]
 *	8-may-86
 *	- fixed bug in new display routine to wrap cursor on extended
 *	  lines at the right time
 *	- modified the buffer-position command to give reasonable info
 *	9-may-86
 *	- improved the word wrap algorithm as not to discard non-space
 *	  delimiters. The backscan now looks for white space rather than
 *	  !inword().
 *	[Released version 3.6g to Krannert]
 *	10-may-86
 *	- Added IBMPC.C an IBM-PC specific display driver. This makes paging
 *	  4-6 times faster. Also made some conditional changes to DISPLAY.C
 *	  to eliminate the pscreen[] if using the PC driver.
 *	[changed version number to 3.6i]
 *	12-may-86
 *	- added delete-window (^X 0) command to dispose of a single window
 *	- fixed problem with multiple prefixes from a command line which
 *	  was reported by John Gamble
 *	14-may-86
 *	- Added Aztec support for the IBMPC display driver. Had to
 *	  readjust some includes and defines for this.
 *	- fixed bug in delete-window.
 *	- fixed some bizarre behavior with the cursor after coming back
 *	  from spawn calls.
 *	[changed version number to 3.7 freezing development for net release]
 *	15-may-86
 *	- (that didn't last long...) Added execute-macro-(1 thru 20) commands
 *	  to execute macro buffers (named "[Macro nn]")
 *	- changed BFTEMP to BFINVS and cleaned up treatment of invisible
 *	  buffers.
 *	16-may-86
 *	- added store-macro (unbound) to store any executed command lines to
 *	  macro buffer.
 *	- added clear-message-line (unbound) command to do just that
 *	- added resize-window command to change a window's size to the
 *	  specified argument
 *	- improved help's logic not to re-read the file if it was already
 *	  in a buffer
 *	- added MAGIC mode to all structures and command tables, but the
 *	  regular expression code that John Gamble is writing is not ready.
 *	18-may-86
 *	- added interactive prompt requests in command line execution (i.e.
 *	  while executing a macro, a parameter starting with an at sign (@)
 *	  causes emacs to prompt with the rest of the parameter and return
 *	  the resulting input as the value of the parameter).
 *	- added arguments to split-current-window to force the cursor into
 *	  the upper or lower window.
 *	20-may-86
 *	- added support for the Microsoft C compiler as per the changes
 *	  sent in by Oliver Sharp
 *	- made some upgrades and fixes for VMS sent in by Guy Streeter
 *	21-may-86
 *	- fixed an Aztec bug in ttgetc by clearing the upper byte
 *	- fixed buf in CMODE with #preprocesser input (bug fix submitted by
 *	  Willis of unknown path)
 *	- added support of alternative startup file ( @<filename> ) in
 *	  the command line
 *	- added ^Q quoting in interactive input (mltreply()).
 *	- added re-binding of meta-prefix and ctlx-prefix
 *	22-may-86
 *	- reorganized getkey routines to make more sense and let prefix
 *	  binding work properly.
 *	23-may-86
 *	- checked new code on BSD4.2 and made a few fixes
 *	- added optional fence matching while in CMODE
 *	- added goto and search command line arguments by Mike Spitzer
 *	26-may-86
 *	- added parameter fetching from buffers
 *	27-may-86
 *	- fixed some HP150 bugs......
 *	31-may-86
 *	- Added Wang PC keyboard support from modifications by
 *	  Sid Shapiro @ Wang Institute
 *	- Fixed some reverse video bugs with code submitted by Peter Chubb
 *	- Fixed bug in nextbuffer reported by Dave Forslund
 *	- added system V support (USG) from Linwood Varney
 *	2-jun-86
 *	- changed defines to just define one Unix define (for example,
 *	  just define BSD for Unix BSD 4.2)
 *	- Added Incremental search functions written by D. R. Banks
 *	  in file ISEARCH.C
 *	- added insert-string (unbound) command to help the macro
 *	  language out.
 *	- added unmark-buffer (M-~) command to turn off the current buffers
 *	  change flag
 *	- fixed nxtarg to truncate strings longer than asked for max length
 *	4-jun-86
 *	- added special characters in command line tokens. Tilde (~) is
 *	  the special lead-in character for "nrtb".
 *	- Fixed bad ifdef in Aztec code so it could look at HOME dir
 *	  for startup, help, and emacs.rc files
 *	6-jun-86
 *	- make delete word commands clear the kill buffer if not after another
 *	  kill command
 *	11-jun-86
 *	- made ~@ in string arguments pass as char(192) to nxtarg() so one can
 *	  quote @ at the beginning of string arguments
 *	- changed buffer size vars in listbuffers() to long (for big files)
 *	- re-wrote buffer-position command to be much faster
 *	12-jun-86
 *	- added count-words (M-^C) command to count the words/chars and
 *	  lines in a region
 *	- changed regions so they could be larger than 65535 (short ->
 *	  long in the REGION structure)
 *	- changed ldelete() and all callers to use a long size. The kill
 *	  buffer will still have a problem >65535 that can not be solved
 *	  until I restructure it.
 *	- grouped paragraph commands and word count together under symbol
 *	  WORDPRO to allow them to be conditionally made (or not)
 *	13-jun-86
 *	- re-wrote kill buffer routines again. Now they support an unlimited
 *	  size kill buffer, and are (in theory) faster.
 *	- changed delete-next-word (M-D) to not eat the newline after a word,
 *	  instead it checks and eats a newline at the cursor.
 *	17-jun-86
 *	- added numeric argument to next/previous-window to access the nth
 *	  window from the top/bottom
 *	- added support for the Data General 10 MSDOS machine
 *	- added save-window (unbound) and restore-window (unbound) commands
 *	  for the use of the menu script. Save-window remembers which window
 *	  is current, and restore-window returns the cursor to that window.
 *	20-jun-86
 *	- fixed a bug with the fence matching locking up PASCAL NEAR the beginning
 *	  of a buffer
 *	- added argument to update to selectively force a complete update
 *	- added update-screen (unbound) command so macros can force a
 *	  screen update
 *	21-jun-86
 *	- rearranged token() and nxtarg() calls so that command names and
 *	  repeat counts could also be prompted and fetched from buffers
 *	  [this broke later with the exec re-write....]
 *	- added write-message (unbound) command to write out a message
 *	  on the message line (for macros)
 *	- changed ifdef's so that color modes are recognized as legal in
 *	  b/w version, and simply do nothing (allowing us to use the same
 *	  script files)
 *	[Released version 3.7 on July 1 to the net and elswhere]
 *	2-jul-86
 *	- Changed search string terminator to always be the meta character
 *	  even if it is rebound.
 *	3-jul-86
 *	- removed extra calls to set color in startup code. This caused the
 *	  original current window to always be the global colors.
 *	7-jul-86
 *	- Fixed bugs in mltreply() to work properly with all terminators
 *	  including control and spec characters
 *	22-jul-86
 *	- fixed replaces() so that it will return FALSE properly on the
 *	  input of the replacement string.
 *	- added a definition for FAILED as a return type.....
 *	- changed version number to 3.7b
 *	23-jul-86
 *	- fixed o -> 0 problem in TERMIO.C
 *	- made ^U universal-argument re-bindable
 *	- wrote asc_int() for systems (like Aztec) where it acts strangely
 *	- changed version number to 3.7c
 *	25-jul-86
 *	- make ^G abort-command rebindable
 *	29-jul-86
 *	- added HP110 Portable Computer support
 *	- changed version number to 3.7d
 *	30-jul-86
 *	- Fixed a couple of errors in the new VMS code as pointer
 *	  out by Ken Shackleford
 *	- split terminal open/close routines into screen and keyboard
 *	  open/close routines
 *	- closed the keyboard during all disk I/O so that OS errors
 *	  can be respoded to correctly (especially on the HP150)
 *	- changed version number to 3.7e
 *	31-jul-86
 *	- added label-function-key (unbound) command under symbol FLABEL
 *	  (primarily for the HP150)
 *	4-aug-86
 *	- added fixes for Microsoft C as suggested by ihnp4!ihuxm!gmd1
 *		<<remember to fix [list] deletion bug as reported
 *		  by craig@hp-pcd>>
 *	8-aug-86
 *	- fixed beginning misspelling error everywhere
 *	- fixed some more MSC errors
 *	- changed version number to 3.7g
 *	?-aug-86 John M. Gamble:
 *	- Made forward and reverse search use the same scan routine.
 *	- Added a limited number of regular expressions - 'any',
 *	  'character class', 'closure', 'beginning of line', and
 *	  'end of line'.
 *	- Replacement metacharacters will have to wait for a re-write of
 *	  the replaces function, and a new variation of ldelete().
 *	- (For those curious as to my references, i made use of
 *	  Kernighan & Plauger's "Software Tools."
 *	  I deliberately did not look at any published grep or editor
 *	  source (aside from this one) for inspiration.  I did make use of
 *	  Allen Hollub's bitmap routines as published in Doctor Dobb's Journal,
 *	  June, 1985 and modified them for the limited needs of character class
 *	  matching.  Any inefficiences, bugs, stupid coding examples, etc.,
 *	  are therefore my own responsibility.)
 *	20-aug-86 Daniel Lawrence
 *	- fixed CMODE .h scanning bug
 *	- changed version number to 3.7h
 *	30-aug-86
 *	- fixed killing renamed [list] buffer (it can't) as submitted
 *	  by James Aldridge
 *	- Added code to allow multiple lines to display during
 *	  vertical retrace
 *	  [total disaster....yanked it back out]
 *	9-sep-86
 *	- added M-A (apropos) command to list commands containing a substring.
 *	- fixed an inefficiency in the display update code submitted
 *	  by William W. Carlson (wwc@pur-ee)
 *	10-sep-86
 *	- added Dana Hoggatt's code for encryption and spliced it into the
 *	  proper commands. CRYPT mode now triggers encryption.
 *	- added -k flag to allow encryption key (no spaces) in command line
 *	14-sep-86
 *	- added missing lastflag/thisflag processing to docmd()
 *	- changed version to 3.7i and froze for partial release via mail
 *	  and BBS
 *	05-oct-86
 *	- changed some strcpys in MAIN.C to bytecopys as suggested by John
 *	  Gamble
 *	- replaces SEARCH.C and ISEARCH.C with versions modified by
 *	  John Gamble
 *	10-oct-86
 *	- removed references to lflick....it just won't work that way.
 *	- removed defines LAT2 and LAT3...the code no longer is Lattice
 *	  version dependant.
 *	14-oct-86
 *	- changed spawn so that it will not not pause if executed from
 *	  a command line
 *	15-oct-86
 *	- added argument concatination (+) to the macro parsing
 *	- added [] as fence pairs
 *	16-oct-86
 *	- rewrote all macro line parsing routines and rearranged the
 *	  mlreply code. Saved 6K!!! Have blazed the path for expanding
 *	  the command language.
 *	17-oct-86
 *	- added new keyboard macro routines (plus a new level to the
 *	  input character function)
 *	22-oct-86
 *	- improved EGA cursor problems
 *	- added -r (restricted) switch to command line for BBS use
 *	06-nov-86
 *	- fixed terminator declarations from char to int in getarg() and
 *	  nxtarg() in EXEC.C as pointed out by John Gamble
 *	07-nov-86
 *	- made wordrap() user callable as wrap-word (M-FNW) and changed
 *	  the getckey() routine so that illegal keystrokes (too many
 *	  prefixes set) could be used for internal bindings. When word
 *	  wrap conditions are met, the keystroke M-FNW is executed. Added
 *	  word wrap check/call to newline().
 *	11-nov-86
 *	- added and checked support for Mark Williams C 86
 *	12-nov-86
 *	- added goto-matching-fence (M-^F) command to jump to a matching
 *	  fence "({[]})" or beep if there is none. This can reframe the
 *	  screen.
 *	- added code and structure elements to support change-screen-size
 *	  command (M-^S) to change the number of lines being used by
 *	  MicroEMACS.
 *	15-nov-86
 *	- finished debugging change-screen-size
 *	17-nov-86
 *	- Incorporated in James Turner's modifications for the Atari ST
 *		23-sep-86
 *		- added support for the Atari ST line of computers (jmt)
 *		  - added a '\r' to the end of each line on output and strip
 *		    it on input for the SHOW function from the desktop
 *		  - added 3 new mode functions (HIREZ, MEDREZ, and LOREZ);
 *		    chgrez routine in TERM structure; and MULTREZ define in
 *		    ESTRUCT.H to handle multiple screen resolutions
 *	[note....ST still not running under Lattice yet...]
 *	25-nov-86
 *	- Made the filter-buffer (^X-#) command not work on VIEW mode
 *	  buffers
 *	- Made the quick-exit (M-Z) command throw out a newline after
 *	  each message so they could be seen.
 *	26-nov-86
 *	- fixed a couple of bugs in change-screen-size (M-^S) command
 *	- changed file read behavior on long lines and last lines
 *	  with no newline (it no longer throws the partial line out)
 *	- [as suggested by Dave Tweten] Made adding a ^Z to the end
 *	  of an output file under MSDOS configurable under the
 *	  CTRL-Z symbol in ESTRUCT.H
 *	- [Dave Tweten] Spawn will look up the "TMP" environment variable
 *	  for use during various pipeing commands.
 *	- [Dave Tweten] changed pipe command under MSDOS to use '>>'
 *	  instead of '>'
 *	04-dec-86
 *	- moved processing of '@' and '#' so that they can be outside
 *	  the quotes in an argument, and added hooks to process '%' for
 *	  environment and user variables.
 *	- modified IBMPC.C to sense the graphics adapter (CGA and MONO)
 *	  at runtime to cut down on the number of versions.
 *	05-dec-86
 *	- changed macro directive character to "!" instead of "$" (see
 *	  below) and fixed the standard .rc file to comply.
 *	- added code to interpret environment variables ($vars). Added
 *	  hooks for built in functions (&func). So, to recap:
 *
 *		@<string>	prompt and return a string from the user
 *		#<buffer name>	get the next line from a buffer and advance
 *		%<var>		get user variable <var>
 *		$<evar> 	get environment variable <evar>
 *		&<func> 	evaluate function <func>
 *
 *	- allowed repeat counts to be any of the above
 *	- added code to allow insert-string (unbound) to use its
 *	  repeat count properly
 *	- added set (^X-A) command to set variables. Only works on
 *	  environmental vars yet.
 *	9-dec-86
 *	- added some code for user defined variables...more to come
 *	- added options for malloc() memory pool tracking
 *	- preliminary user variables (%) working
 *	- changed terminal calls to macro's (to prepare for the new
 *	  terminal drivers)
 *	15-dec-86
 *	- changed previous-line (^P) and next-line (^N) to return a
 *	  FALSE at the end or beginning of the file so repeated
 *	  macros involving them terminate properly!
 *	- added code for $CURCOL and $CURLINE
 *	20-dec-86
 *	- set (^X-A) now works with all vars
 *	- added some new functions
 *		&ADD &SUB &TIMES &DIV &MOD &NEG &CAT
 *	- once again rearranged functions to control macro execution. Did
 *	  away with getarg()
 *	23-dec-86
 *	- added string functions
 *		&LEFt &RIGht &MID
 *	31-dec-86
 *	- added many logical functions
 *		&NOT &EQUal &LESs &GREater
 *	- added string functions
 *		&SEQual &SLEss &SGReater
 *	- added variable indirection with &INDirect
 *	- made fixes to allow recursive macro executions
 *	  (improved speed during macro execution as well)
 *	3-jan-87
 *	- added $FLICKER to control flicker supression
 *	- made spawn commands restricted
 *	- cleaned up lots of unintentional int<->char problems
 *	4-jan-87
 *	- Fixed broken pipe-command (^X-@) command under MSDOS
 *	- added !IF  !ELSE  !ENDIF  directives and changed the
 *	  name of !END to !ENDM....real slick stuff
 *	5-jan-87
 *	- quick-exit (M-Z) aborts on any filewrite errors
 *	8-jan-87
 *	- debugged a lot of the new directive and evaluation code.
 *	  BEWARE of stack space overflows! (increasing stack to
 *	  16K under MSDOS)
 *	- removed non-standard DEC Rainbow keyboard support...let someone
 *	  PLEASE implement this in the standard manner using key bindings
 *	  and send the results to me.
 *	- added change-screen-width () command and $CURWIDTH variable
 *	11-jan-87
 *	- fixed an incredibly deeply buried bug in vtputc and combined
 *	  it with vtpute (saving about 200 bytes!)
 *	16-jan-87
 *	- added code to handle controlling multiple screen resolutions...
 *	  allowed the IBM-PC driver to force Mono or CGA modes.
 *	- added current buffer name and filename variables
 *	  $cbufname and $cfname
 *	18-jan-87
 *	- added $sres variable to control screen resolution
 *	- added $debug variable to control macro debugging code (no longer
 *	  is this activated by GLOBAL spell mode)
 *	- fixed bug in -g command line option
 *	- Released Version 3.8 to BBSNET
 *	21-jan-87
 *	- added $status variable to record return status of last command
 *	2-feb-87
 *	- added ATARI 1040 support...runs in all three modes right now
 *	- added $palette var with palette value in it
 *	- undefined "register" in BIND.C and INPUT.C for ST520 & LATTICE
 *	  to get around a nasty Lattice bug
 *	4-feb-87
 *	- added, debugged code for switching all 1040ST color modes, added
 *	  code for HIGH monochrome mode as well, DENSE still pending
 *	5-feb-87
 *	- with John Gamble, found and corrected the infamous bad matching
 *	  fence problems.
 *	- added error return check in various add/delete mode commands
 *	10-feb-87
 *	- re-arrange code in docmd() so that labels are stored in
 *	  macro buffers
 *	- fixed !RETURN to only return if (execlevel == 0) [If we are
 *	  currently executing]
 *	14-feb-87
 *	- added to outp() calls in the EGA driver to fix a bug in the BIOS
 *	- adding code for 1040ST 40 line DENSE mode (not complete)
 *	25-feb-87
 *	- added auto-save "ASAVE" mode....variables $asave and $acount
 *	  control the frequency of saving and count until next save
 *	- added &and and &or as functions for logical anding and oring
 *	- added string length &LEN, upper and lower case string funtions
 *	  &LOWER and &UPPER
 *	27-feb-87
 *	- added $lastkey   last keystroke struck and
 *		$curchar    character under cursor
 *	28-feb-87
 *	- added code for trim-line (^X^T) command and table entries
 *	  for the entab-line (^X^E) and detab-line (^X^D) commands.
 *	  These are conditional on AEDIT (Advanced editing) in estruct.h
 *	18-mar-87
 *	- finished above three commands
 *	- added $version environment variable to return the current
 *	  MicroEMACS version number
 *	- added $discmd emvironment variable. This is a logical flag that
 *	  indicates if emacs should be echoing commands on the command line.
 *	  real useful in order to stop flashing macros and .rc files
 *	- added $progname environment variable. this always returns the
 *	  string "MicroEMACS". OEM's should change this so that macros can
 *	  tell if they are running on an unmodified emacs or not.
 *	- fixed a minor bug in the CGA/MONO detection routine in IBMPC.C
 *	20-mar-87
 *	- integrated EGAPC.C into IBMPC.C and eliminated the file. Now an
 *	  EGA user can switch from EGA to CGA modes at will
 *	- A LOT of little fixes and corrections sent in by John Ruply
 *	25-mar-87
 *	- Fixed buffer variables so they will work when referencing the
 *	  current buffer
 *	26-mar-87
 *	- Fixed asc_int() to be more reasonable. trailing whitespace ignored,
 *	  only one leading sign, no non-digits allowed after the sign.
 *	- fixed buffer variables to go from the point to the end of
 *	  line.
 *	28-mar-87
 *	- fixed bugs with 8 bit chars as submited by Jari Salminen
 *	- replace AZTEC/MSDOS agetc() with a1getc() which won't strip
 *	  the high order bit
 *	30-mar-87
 *	- changed list-buffers (^X^B) so that with any argument, it will
 *	  also list the normally invisable buffers
 *	- added store-procedure and execute-procedure/run (M-^E)
 *	  commands to store and execute named procedures.
 *	31-mar-87
 *	- Fixed infinite loop in ^X-X command (when [LIST] is the
 *	  only buffer left) as pointed out by John Maline
 *	- made filenames in getfile() always lower case as pointed
 *	  out by John Maline
 *	2-apr-87
 *	- Fixed buffer variables so they would work on non-current displayed
 *	  buffers. They should now work with ALL buffers....
 *	3-apr-87
 *	- Yanked AZTEC profiling code....not very useful
 *	- Modified IBMPC driver so it will not start in EGA mode
 *	- allow the next-buffer (^X-X) command to have a preceding
 *	  non-negative argument.
 *	14-april-87: John M. Gamble
 *	- Deleted the "if (n == 0) n = 1;" statements in front of the
 *	  search/hunt routines.  Since we now use a do loop, these
 *	  checks are unnecessary.  Consolidated common code into the
 *	  function delins().  Renamed global mclen matchlen,
 *	  and added the globals matchline, matchoff, patmatch, and
 *	  mlenold.
 *	  This gave us the ability to unreplace regular expression searches,
 *	  and to put the matched string into an evironment variable.
 *	  SOON TO COME: Meta-replacement characters!
 *	14-apr-87 Daniel Lawrence
 *	- added John Gamble's modified search.c. The code has cut apx
 *	  200-300 bytes off the executable.
 *	- added the &RND function to generate a random integer between
 *	  1 and its arguments value. Also $SEED is availible as the
 *	  random number seed.
 *	- changed the -k command line switch so if there is no argument,
 *	  it will prompt for one when the file is read
 *	15-apr-87
 *	- added 20 bytes of buffer in getval()'s local argument alloc so
 *	  when it returns a value, it has enough stack space to do at least
 *	  one strcpy() before stomping on the returned value. ALWAYS call
 *	  getval() ONLY from within a strcpy() call.
 *	- made $curcol return a 1 based value instead of a zero based one.
 *	  [changed this back later for 3.8o   it was simply wrong.....]
 *	16-apr-87
 *	- re-wrote bytecopy() for AZTEC & MSDOS so it null terminates the
 *	  string.
 *	- changed pipe() to pipecmd() to avoid conflicts with various
 *	  UNIX systems
 *	24-apr-87
 *	- changed open parameters on AMIGA window open to 0/0/640/200
 *	25-apr-87	DML
 *	- cleaned up an unneccessary if/else in forwsearch() and
 *	  backsearch()
 *	- savematch() failed to malloc room for the terminating byte
 *	  of the match string (stomp...stomp...). It does now. Also
 *	  it now returns gracefully if malloc fails
 *	[Froze and released v3.8i via BBS net]
 *	14-may-87
 *	- added nop (M-FNC) that gets called on every command loop
 *	- added $wline, returns and sets # lines in current window
 *	- added $cwline, returns and set current line within window
 *	- added $target, returns/sets target for line moves
 *	- added $search, returns/sets default search string
 *	- added $replace, returns/sets default replace string
 *	- added $match, returns last matched string in magic search
 *	29-may-87
 *	- rewrote word deletes to not kill trailing non-whitespace after
 *	  the last word. Also a zero argument will cause it to just delete
 *	  the word and nothing else.
 *	- more fixes for the search pattern environment variables
 *	30-may-87
 *	- forced all windows to redraw on a width change
 *	2-jun-87
 *	- forced clear-message-line to overide $discmd
 *	- added mlforce() routine and call it in clear-message-line,
 *	  write-message and when $debug is TRUE
 *	- recoded the startup sequence in main()....Much Better...
 *	4-jun-87
 *	- forced interactive arguments ( @"question" ) to ALWAYS be echoed
 *	  regardless of the setting of $discmd.
 *	7-jun-87
 *	- started adding support for Turbo C under MSDOS
 *	11-jun-87
 *	- words now include ONLY upper/lower case alphas and digits
 *	- fixed some more bugs with the startup..(ORed in the global modes)
 *	- took more limits off the self-insert list....
 *	16-jun-87
 *	- macro debugging now displays the name of the current macro.
 *	- fixed a problem in expandp() in search.c that kept high-byte
 *	  characters from working in search strings
 *	18-jun-87
 *	- added &sindex <str1> <str2> function which searches for string 2
 *	  within string 1
 *	[released verion 3.8o internally]
 *	19-jun-87
 *	- added $cmode and $gmode to return and set the mode of the
 *	  current buffer and the global mode
 *	- separated findvar() out from setvar() so it could be used in
 *	  the !LOOP directive (which got canned....read on)
 *	- %No such variable message now types the name of the offending
 *	  variable
 *	22-jun-87
 *	- fixed startup bug when editing the startup file
 *	- added the !LOOP <var> <label> directive
 *	26-jun-87
 *	- dumped !LOOP......added !WHILE. This needed and caused a vaste
 *	  reorginization in exec.c which mainly involved moving all the
 *	  directive handling from docmd() to dobuf(), in the process
 *	  getting rid of a lot of junk and making the result smaller
 *	  than it started.....(yea!)
 *	- added $tpause to control the fence flashing time in CMODE.
 *	  The value is machine dependant, but you can multiply the
 *	  original in macros to stay machine independant. (as
 *	  suggested by Baron O.A. Grey)
 *	- added hook to execute M-FNR (null) during a file read, after
 *	  the name is set and right before the file is read. Took out
 *	  any auto-CMODE code, as this is now handled with a macro.
 *	  (also suggested by Baron O.A. Grey)
 *	- Added Baron O.A. Grey's SYSTEM V typeahead() code...I hope
 *	  this works....if you check this out, drop me a line.
 *	- Added new variable $pending, returns a logical telling if
 *	  a typed ahead character is pending.
 *	29-jun-87
 *	- Made adjustmode() use curbp-> instead of curwp->w_bufp-> which
 *	  fixed some bugs in the startup routines.
 *	- added $lwidth to return the length of the current line
 *	2-jul-87
 *	- Added &env <str> which returns the value of the environment
 *	  variable <str> where possible
 *	- Fixed a NASTY bug in execbuf()..the buffer for the name
 *	  of the buffer to execute was NBUFN long, and got overflowed
 *	  with a long interactive argument.
 *	3-jul-87
 *	- Moved the loop to match a key against its binding out of execute()
 *	  to getbind() so it could be used later elsewhere.
 *	- Added &bind <keyname> which returns the function bound to the
 *	  named key
 *	- changed execute-file to look in the executable path first...
 *	6-jul-87
 *	- changed $curchar to return a newline at the end of a line and
 *	  it no longer advances the cursor
 *	- a lot of minor changes sent by various people....
 *	7-jul-87
 *	- some amiga specific fixes as suggested by Kenn Barry
 *	- added $line [read/write] that contains the current line in the
 *	  current buffer
 *	- changed $curcol so setting it beyond the end of the line will
 *	  move the cursor to the end of the line and then fail.
 *	10-jul-87
 *	- added a number of fixes and optimizations along with the rest
 *	  of the TURBO-C support as submited by John Maline
 *	13-jun-87
 *	- caused dobuf() to copy lastflag to thisflag so the first
 *	  command executed will inherit the lastflag from the command
 *	  before the execute-buffer command. (needed this for smooth
 *	  scrolling to work)
 *	- made flook() look first in the $HOME directory, then in the
 *	  current directory, then down the $PATH, and then in the
 *	  list in epath.h
 *	14-jul-87
 *	- added some fixes for VMS along with support for the SMG
 *	  screen package as submited by Curtis Smith
 *	15-jul-87
 *	- fixed M-^H (delete-previous-word) so it can delete the first
 *	  word in a file....I think there may be more of this kind of thing
 *	  to fix.
 *	16-jul-87
 *	- added code to allow arbitrary sized lines to be read from files..
 *	  speed up file reading in the process.
 *	- made out of memory conditions safer.. especial on file reads
 *	- fixed a bug in bind having to do with uppercasing function
 *	  key names (submitted by Jari Salminen)
 *	- made ST520 exit in the same resolution that EMACS was started in
 *	  (for the 1040ST)
 *	20-jul-87: John M. Gamble
 *	- Set the variables matchlen and matchoff in the 'unreplace'
 *	  section of replaces().  The function savematch() would
 *	  get confused if you replaced, unreplaced, then replaced
 *	  again (serves you right for being so wishy-washy...)
 *	[FROZE development and released version 3.9 to USENET]
 *	{It never made it.....got killed by comp.unix.sources}
 *	12-aug-87: John M. Gamble
 *	- Put new function rmcstr() in SEARCH.C to create the replacement
 *	  meta-character array.  Modified delins() so that it knows
 *	  whether or not to make use of the array.  And, put in the
 *	  appropriate new structures and variables.
 *	15-sep-87
 *	- added support for Mark Williams C on the Atari ST
 *	- made the MALLOC debugging package only conditional on RAMSIZE
 *	- added code for flagging truncated buffers
 *	23-sep-87
 *	- fixed &RIGHT to return the <arg2> rightmost characters
 *	- fixed a buffer in getval() to be static....things are stabler
 *	  now.
 *	- moved all the stack size declarations to after include "estruct.h"
 *	  they work a lot better now....... (rather than not at all)
 *	- made Atari ST spawns all work with MWshell
 *	- fixed spawning on the 1040ST under MWC
 *	27-sep-87
 *	- added &exist function to check to see if a named file exist
 *	- added &find function to find a file along the PATH
 *	- added -A switch to run "error.cmd" from command line
 *	- added $gflags to control startup behavior....
 *	03-oct-87
 *	- changed ":c\" to "c:" in epath.h for the AMIGA as suggested
 *	  by Tom Edds
 *	- added binary and, or, and xor functions as submited by John Maline
 *	  (&band   &bor    and	&bxor)
 *	- added binary not (&bnot) function
 *	- added fixes for gotoline() and nextarg() as submited by David
 *	  Dermott
 *	- fixed return value of $curwidth as submitted by David Dermott
 *	- fixed several calls to ldelete() to have a long first argument
 *	  (pointed out by David Dermott)
 *	- Fixed a bug in stock() which prevented uppercase FN bindings as
 *	  pointed out by many people
 *	- changed dofile() to make sure all executed files don't conflict
 *	  with existing buffer names. Took out cludged code in main.c to
 *	  handle this problem... this solution is better (suggested by
 *	  Don Nash)
 *	05-oct-87
 *	- added in John Gamble's code to allow for a replacement regular
 *	  expresion character in MAGIC mode
 *	[note: under MSDOS  we are still TOO BIG!!!!]
 *	- added overwrite-string as a new user callable function, and
 *	  lowrite(c) as an internal function for overwriting characters
 *	- added &xlate function to translate strings according to a
 *	  translation table
 *	10-oct-87
 *	- added code to allow emacs to clean its own buffers on exit.
 *	  useful if emacs is used as a subprogram. Conditional on
 *	  the CLEAN definition in estruct.h
 *	14-oct-87
 *	- swallowed very hard and switched to LARGE code/LARGE data model
 *	- packaged and released version 3.9c internally
 *	  (MSDOS executables compiled with TURBO C Large model)
 *	16-oct-87
 *	- temporary fix for FUNCTION keys now includes the Meta-O sequence
 *	  if VT100 is definined (submited by Jeff Lomicka)
 *	- an VT100 also triggers input.c to force ESC to always
 *	  be interpeted as META as well as the currently bound key
 *	- fixed a bug in the VMSVT driver as pointed out by Dave Dermott
 *	- added a size parameter to token() to eliminate problems with
 *	  long variable names. (as sugested by Ray Wallace)
 *	18-oct-87
 *	- fixed a bug in the free ram code that did not clear the buffer
 *	  flag causing emacs to ask for more confirnmations on the way out.
 *	19-oct-87
 *	- added ^X-$ execute-program call to directly execute a program
 *	  without going through a shell... not all environments have this
 *	  code completed yet (uses shell-command where not)
 *	[Froze for 3.9d release internally]
 *	28-oct-87
 *	- added code for Atari ST DENSE mode and new bell as submited
 *	  by J. C. Benoist
 *	- made extra CR for ST files conditional on ADDCR. This is only
 *	  needed to make the desktop happy...and is painful for porting
 *	  files (with 2 CR's) elsewhere (Also from J. C. Benoist)
 *	- added optional more intellegent (and larger) close brace
 *	  insertion code (also from J. C. Benoist)
 *	4-nov-87
 *	- fixed AZTEC spawning... all MSDOS spawns should work now
 *	- a META key while debugging will turn $debug to false
 *	  and continue execution.
 *	[Froze for version 3.9e USENET release]
 *	4-nov-87 Geoff Gibbs
 *	- Fast search using simplified version of Boyer and Moore
 *	  Software-Practice and Experience, vol 10, 501-506 (1980).
 *	  Mods to scanner() and readpattern(), and added fbound() and
 *	  setjtable().  Scanner() should be callable as before, provided
 *	  setjtable() has been called first.
 *	13-dec-87
 *	- added a change to the system V termio.c code to solve a keyboard
 *	  polling problem, as submited by Karl Lehenbauer
 *	- removed a "static" from the mod95() function declaration
 *	  in crypt.c to make MSC happy (by Malcolm MacNiven)
 *	- added code so the new exec routines in spawn.c are working
 *	  under MSC (By Malcolm MacNiven)
 *	- fixed the mixed space/tab entabbing bug as suggested
 *	  by Michal Jaegermann
 *	- added the missing brace in the VT100 code in getcmd()
 *	  (This one was submited by many different people)
 *	- added more intelligent spawning for TURBO C as submitted
 *	  by Bob Montante
 *	- added support for the WICAT computers running under the WMCS
 *	  operating system as submitted by Bruce Hunsaker
 *	16-dec-87
 *	- re-wrote fillpara(). Its 4 times Faster!!!
 *	- moved M-FNR binding to $readfile ie to make emacs execute
 *	  a macro when reading files:
 *		set $readhook "nop"	;or whetever
 *	18-dec-1987: John M. Gamble
 *	- Made replaces() beep at you and stop if an empty string is
 *	  replaced.  This is possible in MAGIC mode, if you pick your
 *	  pattern incorrectly.
 *	- Fixed a subtle bug in the new Boyer-Moore routines, which caused
 *	  searches to fail occasionally.
 *	19-dec-87
 *	- new search algorithm from Geoff Gibbs, cleaned up by John Gamble.
 *	  its over 30 TIMES FASTER!!!! using a simplified Boyer and Moore
 *	  pattern search.  This effects all the but MAGIC search commands.
 *	22-dec-87
 *	- narrow-to-region (^X-<) and widen-from-region (^X->) allow you
 *	  to make all but a region invisable, thus all global commands
 *	  can act on a single region.
 *	- a numeric argument on the select-buffer (^X-b) command makes
 *	  that buffer become "invisable"
 *	- M-FNC becomes $cmdhook
 *	  M-FNW becomes $wraphook
 *	23-dec-87
 *	- made unmark-buffer (M-~) update the mode lines of all windows
 *	  pointing into the unmarked buffer
 *	28-dec-87
 *	- added new prefix MOUS (coded as MS-) for dealing with mice
 *	29-dec-87
 *	- vastly reorginized machine dependant code. Put termio.c and
 *	  spawn.c together and then pulled out msdos.c, unix.c, wmcs.c,
 *	  atari.c, amigados.c, and vms.c. These are the operating specific
 *	  source files now.
 *	30-dec-87
 *	- cleaned up msdos.c. Got rid of two unused spawn functions for
 *	  LATTICE. Also removed attempt to lookup SHELL variable to
 *	  find the command processer. This is definatly not applicable
 *	  here as COMSPEC should always be the name of the reloaded
 *	  command processor.
 *	20-jan-87
 *	- added Jeff Lomicka's ST mouse support and added some
 *	  appropriate code to input.c to support this.
 *	- added buffer name completion to the select-buffer (^X-B)
 *	  command as submitted by Martin Neitzel.
 *	- changed some function names:
 *		abs	=>	absv
 *		atoi	=>	asc_int
 *		ltoi	=>	long_int
 *		setmode =>	setmod
 *		strncpy =>	bytecopy
 *	25-jan-88	By Jeff Lomicka
 *	- Added parsing of keypad keys, LK201 function key sequences, mouse
 *	  sequences (VaxStation form and BBN BitGraph form), and the cursor
 *	  position report to VMSVT.C.  This will require a change in the way
 *	  that VMS users bind functions to their function keys or keypad
 *	  keys.  This also defines a defacto-standard for the names of the
 *	  numeric keypad keys, based on the VT100 and compatible terminals.
 *	- Added MOUSE.C to DESCRIP.MMS, and changed it to reference ME.OPT,
 *	  and to generate ME.EXE.
 *	- Renamed MICROEMACS.OPT to ME.OPT so that the filename would fit
 *	  on smaller systems.
 *	- Added capability for copy and paste from the terminal-wide
 *	  copy/paste buffer.  This allows copy and paste of the select region
 *	  from and to non-cooperating sessions in other windows.  (The
 *	  addition of the MOUSE handling routines disables the transparent
 *	  use of the VWS copy/paste mechanism.) There may be a bug in the
 *	  PASTE section of this code.  On some VWS systems, you may get a
 *	  bunch of Hex characters instead of text.  I'm looking in to finding
 *	  out what is really supported in the released versions of VWS.  This
 *	  code is conditional based on MOUSE & VMSVT.  Code was added to
 *	  VMSVT.C.  Added bindings for to ebind.h.  Yank is on ^X^Y.  Copy is
 *	  on ^XC.  If MOUSE & VMSVT is true, this binding overides the use of
 *	  ^XC for spanwcli.  [This won't stay.. alternative binding?]
 *	  Also added definitions for this to efunc.h.
 *	- Added #define of CPM to estruct.h.  It is used in FILE.C.  You
 *	  might want to delete it again. [It was deleted..no CPM support]
 *	- Added noshare attributes to all data in many modules.
 *	- Re-introudced XONDATA condition that was in 3.9eJ's TERMIO.C into
 *	  VMS.C.  (This was overlooked by Dan in making 3.9g.)
 *	- Appended a ":" to the "MICROEMACS$LIB:" path element in the VMS
 *	  portion of EPATH.H.
 *	- Fixed mouse reports from VMSVT.C to correspond to the code in 3.9g.
 *	- In INPUT.C, added VMS to list of systems that generate FN key and
 *	  mouse reports with leading 0 bytes.
 *	- In order to accomodate callable editor support, I had to arrange
 *	  so that some of the CALLED features are active all of the time,
 *	  particularly EXIT HANDLING.  I changed all calls to exit() into
 *	  calls to meexit().  There is a great need to divide up exits
 *	  between error exits that aren't expected to return, and exits that
 *	  just set the exit flag.  Places that didn't expect exit to return
 *	  should be changed to return immediately after calling exit, so that
 *	  the command loop can catch it and return.  This error handling
 *	  business is a real mess right now.
 *	- Re-arranged main() so that most of the guts are in called
 *	  routines, so that there can be more than one entry point into the
 *	  program.  Note, now, the possiblity of adding recursive-edit as a
 *	  function that you can call from a macro.
 *	- Added the universal symbol ME$EDIT to the VMS version as a second
 *	  entry point, for use when called by MAIL and NOTES, which use
 *	  LIB$FIND_IMAGE_SYMBOL and perform run-time linking with their text
 *	  editor.  This stuff works great!
 *	- Changed Atari-ST makefile to link without -VGEM, so that you will
 *	  get argc and argv.
 *	- Fixed ST520.C so that screen would be redrawn when we notice that
 *	  desk accessories exit, and so that the mouse will be turned off
 *	  when exiting into a shell (If text cursor was on, mouse is left
 *	  off).
 *	- Changed ST520.C to have bindings for keypad keys, with PF1 "("
 *	  key as metac, so that it can be used as GOLD prefix.	Uses GOLDKEY
 *	  option in ESTRUCT.H.
 *	26-jan-88	Daniel Lawrence...
 *	- cleaned up coding in main.c
 *	- changed more function names
 *		itoa	=>	int_asc
 *		ltoa	=>	long_asc
 *	27-jan-88
 *	- added terminal initilizer string output to tcap.c as submitted
 *	  by Ge' Wiejers
 *	- switched the -A command line switch to be -E the way MWC demands
 *	  that it be. Also had to move -E to -C (for Change...)
 *	02-feb-88
 *	- changed META and ^X prefixes to use a new mechinism. Hitting
 *	  a prefix stores it as a pending prefix.
 *	05-feb-88
 *	- changed emacs to use ^M for line terminators instead of
 *	  ^J. This required LOTS of changes. Now, however, we can
 *	  search for and manipulate ^M.
 *	- fixed getcmd() to work again so that describe-key and
 *	  interactive binding functions work again.
 *	- added $sterm environmental variable. This hold the current
 *	  value of the keystroke needed to terminate search string
 *	  requests. The value in here is also automatically reset
 *	  any time the meta-prefix is rebound, the the meta-prefix's value
 *	06-feb-88
 *	- fixed bug in stock().. real control characters in command lines
 *	  now bind properly
 *	[BBS release of version 3.9i]
 *	23-feb-88
 *	- added buffer name completion to delete-buffer (^XK) and reorginized
 *	  code their to gain back 1 K
 *	- added $modeflag to allow us to hide the modelines.
 *	- added $sscroll to allow smooth scrolling... (I don't like this
 *	  but there were too many requests for it).
 *	- added $lastmesg to hold last message EMACS wrote out
 *	04-mar-88
 *	- added Alan Phillip's mods to support the VGA adapter on the
 *	  IBM PS/2 machines. Modified it a little.
 *	15-mar-88
 *	- Forever twiddling with the startup code, I moved the initialization
 *	  of exitflag to main() to allow a startup file to exit EMACS
 *	  properly.  Also had to make meexit always return(TRUE) regardless
 *	  of the exitstatus. (Ie it always works and doesn't abort the macro)
 *	17-mar-88
 *	- added code to change hard tab size with $tabsize variable
 *	- added binary search to fncmatch()
 *	28-mar-88
 *	- optimized the IBMPC version in TURBO C by supplying mixed memory
 *	  mode information (NEAR and PASCAL). The executable is down to
 *	  89K..but remember to name the code segment (from the options menu)
 *	1-apr-88
 *	- added remove-mark (^X<space>) to allow you to unset the mark
 *	  in the current window.
 *	8-apr-88
 *	- Lots of twiddling with the mixed memory model TURBOC IBM-PC
 *	  version... its 87K long now and fairly stable
 *	- changed behavior of CMODE when inserting blank lines
 *	- tightened stack usage from 32K to 20K
 *	9-apr-88
 *	- added new cinsert(). blank lines no longer confuse the CMODE
 *	  indentation. Trailing white space is deleted when newline is
 *	  hit in CMODE.
 *	12-apr-88
 *	- fixed long standing bug in query-replace-string. When the line the
 *	  point originally starts on is extended (thus re-malloced) a '.',
 *	  aborting the replace and returning to the original replace had
 *	  locked things up. no more.
 *	07-may-88
 *	- fixed bug in narrow() ... it works everywhere now.
 *	[BBS release of 3.9n]
 *	3-june-88
 *	- removed restriction against deleting invisable buffers
 *	- a lot of little fixes suggested by Dave Tweten
 *	10-jun-88
 *	- added "safe save"ing as coded by Suresh Konda.
 *	  (when writing a file to disk, it writes it to a temporary file,
 *	   deletes the original and renames the temp file)
 *	- added $ssave flag to enable or disable safe file saving
 *	12-jun-88
 *	- added automatic horizontal window scrolling.	$hscroll (default
 *	  is TRUE) controls if it is enabled. $fcol is the current first
 *	  column of window in screen column one.  $hjump determines the
 *	  number of columns scrolled at a time.
 *	- changed version number to 3.9o
 *	22-jun-88
 *	- made binary searches start from 0 instead of one... EMACS can
 *	  find the first of many lists again.
 *	- more debugging things.  Added the dump-variable command which
 *	  creates a buffer (like list-buffers does) with all the environment
 *	  and user variables and their values.	Also the display (^XG)
 *	  command prompts and lists the value of a variable.  These are
 *	  both conditional on the DEBUGM symbol in estruct.h
 *	1-jul-88
 *	- fixed a bug in showcpos() which did not report the proper value
 *	  for the current character when issued at the end of the file
 *	[Released internally as 3.9p]
 *	3-aug-88
 *	- changed keyname in stock() in bind.c to unsigned char for
 *	  compilers that don't default. This allows more arbitrary
 *	  rebinding. (submitor: David Sears)
 *	12-aug-88
 *	- changed wrap-word to force $fcol to 0
 *	- changed fill-paragraph (M-Q) to force $fcol to 0
 *	- added -i command line switch to initialize a variable's value
 *	  for example:	emacs -i$sres VGA
 *	26-aug-88
 *	- changed code in updateline() in display.c to remove reliance
 *	  on well behaved pointer subtraction (submitted by Kenneth Sloan)
 *	- fixed potential bug in fileio.c in ffgetline()
 *	  (submitted by John Owens)
 *	6-sep-88
 *	- &right and &mid now check their arguments so the bytecopy can not
 *	  start past the end of the string.
 *	- added $writehook to execute macroes when writing files
 *	- added $exbhook to execute macro when exiting an old buffer
 *	- added $bufhook to execute macro when entering a new buffer
 *	14-sep-88
 *	- asc_int() now ignores trailing non-digits
 *	15-sep-88
 *	- added &trim function to trim whitespace off a variable
 *	- changed message "Out of memory while inserting string" to
 *	  "Can not insert string" as this error can result when the
 *	  current buffer is in VIEW mode as well.
 *	- made trim-line, entab-line, and detab-line work on the
 *	  current region if they have no arguments.  Also added alias
 *	  trim-region, entab-region and detab-region.  The old names
 *	  will hang about for a version, then dissapear.  I like the
 *	  new functionality much better.
 *	25-sep-88
 *	- Inserted the macros isletter(), isuppper(), islower(), and
 *	  CHCASE() whereever appropriate, replacing explicit character
 *	  comparisons.	When DIACRIT is set to one, they become functions
 *	  and know about the extended ascii character set.  Functions right
 *	  now are in for MSDOS.C - other machines to follow.  From J. Gamble.
 *	26-sep-88
 *	- moved all text constants out to a language specific header file
 *	  and left an english version of the constant in a remark at its
 *	  occurence.
 *	- added $language to return the language of the current version
 *	- added elang.h to call the proper language specific module
 *	2-oct-88
 *	- removed DIACRIT conditional. Its there all the time now. Ripped
 *	  the old code, and replaced it with a upcase[] and lowcase[]
 *	  table.  Functions to manipulate it are now in new source file
 *	  char.c and can be made conditional on system and language.
 *	  Added table for extended MSDOS modeled after John Gamble's
 *	  code.
 *	5-oct-88
 *	- got rid of CHCASE()..[except in the MAGIC mode search code]
 *	  I ALWAYS force to upper or lower, and never simply swap.  They
 *	  are now uppercase() and lowercase() in the new char.c source file.
 *	7-oct-88
 *	- added all the needed code to use TERMCAP on UNIX to interpret
 *	  function and cursor keys.  A rather slimy trick involving
 *	  the key timing is used, and this makes keyboard macroes
 *	  a little dicy....
 *	- added the DIACRIT compilation flag back for people who
 *	  want more speed in MAGIC mode.
 *	10-oct-88
 *	- as suggested by Michael Andrews, when the current buffer is
 *	  in view mode, don't preallocate expansion room in new lines.
 *	  (ie we aren't editing them anyway!)
 *	- added Microsoft Mouse drive... went very well. It all appears
 *	  to work, just like the Atari ST mouse driver.
 *	11-oct-88
 *	- recieved and tested german.h header file from Martin Neitzel
 *	  (unido!sigserv!neitzel@uunet.uu.net)
 *	15-oct-88
 *	- recieved and tested dutch.h header file from Paul De Bra
 *	  (debra@research.att.com)
 *	- fixed a lot of mouse movement problems.  Dragging a mode line
 *	  sideways horizontally scrolls a it's window.  Fixed positioning
 *	  problems with scrolled and extended lines.
 *	- added $msflag to turn the mouse off
 *	- added $diagflag.  When set to FALSE, no diagonal movement is
 *	  possible.  Vertical movement has precidence, but an explicit
 *	  horizontal drag still works.
 *	- grabbing and dragging the last char of the command line allows
 *	  you to resize the editing screen.  This should be tied into
 *	  various windowing systems later.
 *	- write mouse-region-(down/up). See the emacs reference docs on the
 *	  exact behavior of these.
 *	- deleted the old mouse-(kill/yank)-(up/down). They are
 *	  redundant now, and don't work as well as mouse-region-*.
 *	2-nov-88	[changes submitted by Allen Pratt from ATARI]
 *	- created DIRSEPSTR and DIRSEPCHAR definitions for directory seperators
 *	- added TTflush in bindtokey() to force space out to terminal after
 *	  binding name.
 *	- fixed a pointer casting problem in display.c involving pointer subtraction
 *	- removed the cursormove() from the beginning of mlwrite() so
 *	  the cursor need not jump to the command line if the fisplay
 *	  is disabled by setting $discmd to FALSE.
 *	- added &gtc to get a emacs command sequence and return it as
 *	  a bindable string
 *	- changed scrnextup -> nextup, scrnextdn -> nextdown, and
 *	  mlreplyt -> mltreply to make short symbol compilers happy.
 *	- added "print" as a synonim to "write-message".
 *	- added "source" as a synonim to "execute-file".
 *	- added a M-^X binding to the execute-command-line command
 *	4-nov-88
 *	- changed $tabsize to $hardtab. Added $softab as the current
 *	  softtab size (previously only changable by the ^I (handle-tab)
 *	  command.
 *	- fixed various problems in reglines() and made ^X^D (detab-region)
 *	  and ^X^E (entab-region) use the current hard tab setting.
 *	- execute-command-line (M-^X) when executed within a macro line
 *	  (which sounds redundent) will force all of the arguments of
 *	  the executed command to be taken interactivly.  I needed this
 *	  for the new macro to allow mouse functionality on the
 *	  function key window.
 *	- taking Michael Andrew's suggestion to its conclusion, we no longer
 *	  allocate any extra space except when inserting.  This has
 *	  substantially decreased the memory usage.  Note that when
 *	  we must explicitly set l_used to the proper amount since lalloc()
 *	  is not responcible for the rounding up.
 *	- arranged to allow getstring() to return a NULL result.  Aborting
 *	  out of an interactive prompt now will properly terminate a macro.
 *	[Released the 3.10 BETA test version...yes the ATARI version is broke]
 *	24-nov-88
 *	- fixed system V input char HOG problems as submitted by
 *	  Kjartan R. Gudmundsson. Also eliminated some other 7 bit filters
 *	  in unix.c at the same time.
 *	- reading function keys into table has been fixed in TCAP.C.  It will
 *	  no longer attempt to strcpy NULL pointers.
 *	1-dec-88
 *	- multiple marks are now implemented. (What a task...). Set-mark
 *	  (M-<space>), remove-mark (^X<space>) and exchange-point-and-mark
 *	  (^X^X) all take a numeric argument from 0 to NMARKS-1 (9 now).
 *	2-dec-88
 *	- added goto-mark (M-^G) which moves the point to the requested mark.
 *	- fixed some VMS things that Curtis pointed out...
 *	- fixed bug in crypt() algorith!!!! This works properly now.  I had
 *	  to add a $oldcrypt environment variable to trigger the broken
 *	  behavior so people could decrypt their old files.  I will probably
 *	  have to live with this.....
 *	15-dec-88
 *	- added end-of-word command to advance just past the end of a word.
 *	20-dec-88
 *	- added definitions for SUN's OS and SCO XENIX to allow us to use
 *	  specific calls in those OSs for waiting and timing.
 *	- merged in XTCAP.C as submitted by Guy Turcotte into tcap.c.  This
 *	  mirrors the BSD 4.3 changes for function keys for the SCO XENIX
 *	  and SUN OS versions
 *	14-jan-89
 *	- added $region as a read only variable (max 128 chars) of the
 *	  current region (as suggested by Allan Pratt)
 *	- changed KEYTAB structure to allow us to bind user macroes
 *	  (buffers) to keys as well as C functions. Lots of changes
 *	  for this.
 *	18-jan-89
 *	- finished changes for the new KEYTAB structure.
 *	- modified describe-bindings to display the bound macroes.
 *	- added arguments to nullproc() in main.c (BUG FIX!)
 *	- tried (and succeeded) in defining the pascal keyword
 *	  for MSDOS & TURBO. A 2k reduction in the executable was nice.
 *	- added macro-to-key (^X^K) function to bind a macro to a key.
 *	19-jan-89
 *	- made numeric arguments to keys bound to macroes repeat those
 *	  macroes n times.
 *	- modified docmd() to look for a macro name if it could not find
 *	  a function name. Again a numeric leadin implies repitition.
 *	- added machine code execpr() function to the IBMPC version
 *	  because under NOVELL Netware, the DOS 4B exec call does not
 *	  preserve the stack on exit... thus execprog() would die.  This
 *	  requires that you add execpr.obj to the make and link files!
 *	  There is a symbol (NOVELL) in estruct.h to control this code
 *	  being included.
 *	24-jan-89
 *	- various bug fixes and type fixes as suggested by the high C
 *	  compiler on the IBM-RT
 *	1-feb-89
 *	- changed the fgetc() in unix.c to a read() under V7 and BSD so
 *	  the time dependant function key checking works.
 *	- added a support for Hewlet Packard's UNIX system which
 *	  is a System V with BSD 4.3 enhancements.  This is controled
 *	  by the HPUX symbol in estruct.h.
 *	5-feb-89
 *	- modified mouse-region-up so right clicking down on a modeline and
 *	  then releasing closes that window
 *	12-feb-89
 *	- Goaded onward by file completion code submitted by Fuyau Lin, all
 *	  the completion code has been re-written and layered properly.
 *	  Any OS specific driver need to now supply getffile() and
 *	  getnfile() to supply a completion file list, or these can
 *	  return NULL on systems where this is not possible. All this code
 *	  is conditional on the COMPLET symbol in estruct.h.
 *	17-mar-89
 *	- system V code from April 1989 "C Users Journal" installed in
 *	  UNIX.C. Hope this puts that problem to rest.
 *	- Jeff Lomicka fixed a lot of mousing bugs in ST520.c
 *	- Re-wrote AMIGADOS.C.  This should work properly with INTUITION
 *	  and lots of windowing (resizing, mousing, etc.)
 *	- more additions for the DATA GENERAL from Doug Rady. (read
 *	  the history in aosvs.c for details).
 *	- orginized the NOSHARE, VIOD and CONST keywords better.
 *	- A lot of general cleanup.
 *	[Released as version 3.10 to the world]
 *	05-may-89
 *	- installed the BRICKS revsion control system (from DATALIGHT)
 *	  on my PC developmental machine.  This should make diffs possible.
 *	  The installation was convoluted to to brick's inibility to
 *	  use subdirectories, and implemented through a much more
 *	  complex makefile.
 *	- fixed a missing structure element in line 1230 of eval.c
 *	- set attribute to clear screen to 07 in ibmeeop.  This fixes
 *	  a long standing bug involving not seeing the modeline.
 *	16-may-88
 *	- a bad character in UNIX.C's code for the HPUX is fixed
 *	- a couple of header problems for HPUX fixed
 *	- a bug in EVAL.C comparing var names past 10 chars is fixed
 *	- added Dan Corkill's ISEARCH mods (see ISEARCH.C) Had
 *	  to fix them to allow function keys at the end of an isearch.
 *	22-may-89 John Gamble
 *	- added $SEARCHTYPE var to determine where searches terminate.
 *		0 = forward: after string, backward: at beg of string
 *		1 = always at beginning of string
 *		2 = always character past end of string
 *	  added code and vars all around for this
 *	28-may-89
 *	- finished fixing ISEARCH to properly allow function keys and
 *	  mouse movement to terminate the ISEARCH.
 *	28-may-89 John Gamble
 *	- Code finally added to handle the $SEARCHPNT (nee' $SEARCHTYPE)
 *	  variable.  Frankly, i think its pretty sloppy, and should
 *	  be replaced by a separate function to deal with point positions.
 *	  Will change it later.
 *	7-jul-89 John Gamble
 *	- A far more elegant way of dealing with $SEARCHPNT is present,
 *	  more efficient in space also.  Didn't need the above mentioned
 *	  point function.  Altered ISEARCH.C code to use new calling
 *	  parameters in scan routines.
 *	- Got started with group structure in MAGIC mode, with alterations
 *	  present in estruct.h and search.c
 *	15-jul-89 John Gamble
 *	- added $disphigh environment variable to control the escaped
 *	  display of high bit set ascii characters
 *	05-aug-89 Daniel Lawrence
 *	- added to lines to lbound() so a forward search will no longer
 *	  wrap around the end of the buffer if the point was at the end.
 *	- widen-from-region (^X>) preserves the point and marks if they
 *	  were at the end of the fragment, instead of moving them to
 *	  the end of the buffer.
 *	- fixed bug in comp_file() to allow up to 128 char files specs
 *	  instead of 32 chars.
 *	- patched some bugs in the TIPC version.  It runs again, but
 *	  without a mouse and color and reverse video.  The TIPC I had
 *	  to test on was a weird mono one....
 *	- released 3.10a to the BBS for TIPC users
 *	05-aug-89 John Gamble
 *	- refolding and shoving a lot has managed to knock about 300
 *	  bytes of code out of search.c and isearch.c.  Also the some of
 *	  the various commands in an isearch now shadow their bigger
 *	  counterparts.
 *	- fixed the tab bug in entab-line (^X-^E) which munged lines.
 *	15-aug-89 Daniel Lawrnece
 *	- added append-file (^X^A) command to append a buffer on the
 *	  end of a file
 *	[released 3.10b to Krannert.....]
 *	20-aug-89 Daniel Lawrence
 *	- The quote character is now treated as the abort and universal-
 *	  argument characters are: only one key may be bound to it at
 *	  a time.
 *	9-sep-89
 *	- a bug in msdos.c prevented the proper binding of ALT-0 on the
 *	  IBMPC. Fixed this as pointed out by Rik Faith.
 *	17-sep-89 John M. Gamble
 *	- Isearch now understands alternatively bound keys.
 *	  One change had to be made to ebind.h and input.c, to handle
 *	  the delete-previous-character command.  The delete character
 *	  is now represented internally as CNTL|'?', instead of 0x7F.
 *	  Changes made to handle this are in ebind.h and input.c.
 *	16-dec-89
 *	 - Incorporated Kevin Mitchell's VMS improvements.  Using the
 *	   native RMS file I/O, and native memory memory routines
 *	   GREATLY speeds up file and buffer manipulation.
 *	17-dec-89
 *	- updated the HP150.C file to work with the new style I/O layers.
 *	  hp150 version now works properly.
 *	25-jan-90
 *	- dumped a lot of the old choice symbols in estruct.h for some
 *	  things which have either never been used, or have clearly
 *	  superior versions now.
 *	- removed the WORDPRO and AEDIT symbols.... all this code is
 *	  always in now.  (it never made much of a size difference anyway)
 *	- modified sources to allow them to use new ANSI style prototypes.
 *	  (this was a lot of work....)
 *	- in the process, fixed bug where EMACS probably grew confused
 *	  when setting $curchar to a newline.
 *	26-jan-90
 *	- replaced old file completion code with version by Michael J. Arena
 *	  It allows us to partially complete directory names on UNIX.
 *	- modified getffile in MSDOS.C to return directory names as well
 *	  so it can work with the above mod.  I need to do this mod on
 *	  all OSs which will support it.
 *	- added support for the Supermax UNIX system as submitted by
 *	  Michael Hillerstrom.
 *	- fixed a couple of OS2 bugs having to do with key input as submitted
 *	  by Sugih Jamin
 *	- fixed three bugs posted by Kieth Jones (in display.c and eval.c)
 *	- let the TURBO C MSDOS version use the bios instead of DOS for
 *	  keybord input as submitted by David R. Kohr.  This should solve
 *	  problems with TSRs like Sidekick that break rules.
 *	- fixed a bug in MSDOS.C's getffile involving how the filename was
 *	  parsed.  Paths including ".." now can expand properly.
 *	[Released version 3.10e to USENET]
 *	- changed version number to 3.10f
 *	15-feb-90
 *	- added $lterm variable to allow emacs to write non-standard line
 *	  terminators on output.  Set to "" it uses standard terminators.
 *	- forced ffgetline() to trim trailing line terminators on reading
 *	  this allows us to read strange files more robustly.
 *	- removed the ADDCR option as the var above can do it
 *	17-mar-90
 *	- John Gamble found and fixed a bug in the binding code erroneously
 *	  searching for a particular empty slot.
 *	22-mar-90
 *	- changed line buffer allocation scheme during file reading to
 *	  use realloc() and always double the buffer size.  Much faster
 *	  on long line reads. 
 *	01-may-90
 *	- added code to allow SUN verion to work properly
 *	- added .xlk style file locking for unix systems
 *	10-may-90
 *	- added NEC 9801 hardware driver.  There is no background color
 *	  implimented yet as this will involve using the graphic planes
 *	  to simulate a backgound color.
 *	25-may-90
 *	- added DBCS (Double Byte Character Set) symbol to estruct.h and
 *	  also much conditional code to handle display and editing of
 *	  DBCS chars.  This code is active in the NEC version, using
 *	  japanese KANJI characters.
 *	- added REP (replace) mode. Very similar to OVER mode except
 *	  tabs and 2 byte characters are handled differently.
 *	6-jun-90
 *	- modified ansi.c into fmr.c for the Fujitsu FMR-70 series
 *	  msdos machines.  Extensive modifications to msdos.c were
 *	  needed to accomodate this machine. 
 *	16-jun-90
 *	- modified swbuffer() to not check locks on an inactive buffer
 *	  in view mode.  This allows the -v switch to work from the
 *	  command line on a file whose directory the user has no
 *	  write access to.
 *	28-jun-90
 *	- added background colors to the FMR driver utilizing the
 *	  graphics planes via the GDS driver.
 *	30-jun-90
 *	- added $wchars, a list of characters considered "in a word".
 *	  An empty value sets this to its default of alphas, numbers
 *	  and the underscore (_) character.
 *	5-jul-90
 *	- re-arranging code to vector all pop up windows through on
 *	  function for display.
 *	- fixed bug in positioning of file name in buffer list
 *	6-jul-90
 *	- added ability to pop up temporary windows.  Environment variable
 *	  $popval gives the old behavior when FALSE and the new behavior
 *	  when TRUE.
 *	- added pop-buffer function to allow user to pop up the contents
 *	  of a buffer.
 *	27-jul-90
 *	- fixed up the OS2 code back into working.  Wrote getffile()
 *	  and getnfile() to allow it file name completion.
 *	27-jul-90 John M. Gamble
 * 	- Magic mode characters '+' and '?' added, respectively meaning
 *	"one to many" and "zero to one (or, optional)".
 *	12-oct-90
 *	- A BAD bug in vteeol() hung the machine during the redraw.  Vtcol
 *	was not getting incremented when the screen was scrolled horizontally.
 *	26-nov-90
 *	- conditionalized code in MSDOS.C using the BIOS to not be used when
 *	compiled for the HP150.  (It's bios is different, of course)
 *	15-jan-91
 *	- in linstr() made an non-positive number of inserted characters
 *	  illegal, causing the linstr to return FALSE on negative and
 *	  TRUE on zero... but not changing the text
 *	15-jan-91
 *	- rewrote dolock.c to generalize it. it now supports MSDOS and SUN
 *	  the SUN code should work on other UNIXs, but needs confirmation
 *	17-jan-91  John Gamble
 *	- removed references to DIACRIT
 *	- added the $yankpnt variable.
 *	21-jan-91  Daniel Lawrence
 *	- adding OS2 support for file locking
 *	07-mar-91
 *	- added the concepts of SCREENs.  Each screen is a separate list
 *	  of windows which overlay on text based systems.
 *
 *	New commands:
 *	  next-screen		(A-N) switch to the following screen
 *	  previous-screen	(A-P) switch to the previous screen
 *	  find-screen		(A-F) switch or create a named screen
 *	  delete-screen		(A-D) delete the named screen
 *	  list-screens		(A-B) how list of screens
 *
 *	New variables:
 *	  $scrname		name of the current screen
 *	  $scrnum		ordinal number of the current screen
 *
 *	Most of the code for this is in the new source file SCREEN.C
 *
 *	11-mar-91	Code from or suggested by Michael C. Andrews
 *	- getstring() now accepts ^K to return a non-default empty
 *	  result. (to allow for NULL replace strings!)
 *	- added indent-region [M-)] and undent-region [M-(]
 *	- added &ISNum function to test variables for being numeric
 *	- added $curwind and $numwind to give the current ordinal and total
 *	  number of windows on the current screen
 *	12-mar-91	Daniel Lawrence
 *	- checked and fixed code for IBMPC MicroSoft C 5 and 6 compilers
 *	15-mar-91	Curtis Smith
 *	- masked the number generated by ernd in writeout() to limit it
 *	  to 5 digits so that MPE won't barf on a too long file name
 *	  for the temporary file
 *	19-mar-91	Daniel Lawrence
 *	- added code to parse drive letters to dolock()/undolock()
 *	  MSDOS is happier.
 *	- changed search order of files to look in current directory
 *	  before HOME and PATH dirs.
 *	24-mar-91
 *	- command file names default to a .cmd extention from the command
 *	  line and from the source command.
 *	[released BETA version 3.11]
 *	02-apr-91
 *	- neglected to decrement buffer use counts in free_screen()
 *	  These are now fixed, so delete-screen is again safe.
 *	19-apr-91 John M. Gamble
 *	- Made a couple of functions, cclmake() in particular, more efficient.
 *	  Saved a hundred bytes or so, which were immediately used up by:
 *	- Groups!  Groups are now available in MAGIC mode! (hurrah).
 *	- Fixed bug where $match would get set with the wrong characters
 *	  if searching backwards.
 *	- also declared as VOID the VOID functions in search.c.  Should
 *	  get nattered at less often now.
 *	23-apr-91 Daniel Lawrence
 *	- namebuffer() was only updating the current modeline. It needed to
 *	  update all of them in case the current buffer was in multiple
 *	  windows. This bug was pointed out by Martin Shultz.
 *	- added code from Floyd Davidson for new backward kill behavior. This
 *	  is under the NEWKILLB symbol and is for evaluation.
 *	24-apr-91
 *	- John Gamble points out that local variable "ap" in mlwrite() can
 *	  not be register type. Fixed this.
 *	- as reported by Martin Shultz, tabs caused problems in OVERwrite
 *	  mode since the logic for determining to delete or not was based
 *	  on offset into the line rather than column. Preceding tabs
 *	  confused this. I changed the offset reference (in main.c) to a
 *	  reference to the getccol() function and all is well.
 *	- Risto Kankkenun pointed out a problem in the input handling of
 *	  any completion command... it did not handle function keys well,
 *	  and blocked some normal keys from being used. Inputing an
 *	  extended character in complete() and then translating it to a
 *	  standard ascii char let us correct this.
 *	- Risto also points out a one character difference between word
 *	  wrapping and paragraph filling. Changed a couple of >= to > in
 *	  reform() in word.c to correct this.
 *	27-apr-91  John Gamble
 *	- Oops.  In changing the closure code, so that groups and closure
 *	  could be used together, i broke closure.  Argh.  Fixed now.
 *	- Added the &GROUP function, so that macro writers could get at
 *	  the substrings matched by groups in a MAGIC search.
 *	- Since $yankpnt is a Boolean type now, changed its name to
 *	  $yankflag.
 *	- Altered the broadcast message interceptor code in VMS.C.  Instead
 *	  of using a %variable to hold the message, which was not large
 *	  enough, the message(s) are stashed away in a buffer.  Added a
 *	  global boolean variable pending_msg, which Dan will use to notify
 *	  the user of an intercepted message.
 *	1-may-91  Daniel Lawrence
 *	- the backward kill code was very interesting.... but did not update
 *	  the $kill variable (via getkill() in eval.c) and caused all sorts
 *	  of interesting sideffects when the clear kill buffer code was not
 *	  updated either. I refigured the algorithm to use the current kill
 *	  blocks, growing them backwards as needed. Recoding this, it all
 *	  works properly now, allowing negative kills and proper behavior
 *	  when mixing various consective forward and backward kills. This
 *	  new code is smaller and involved less changes.
 *	2-may-91
 *	- changed some WFHARDs to WFMOVEs in basic.c for gotobob() and
 *	  gotoeob() as suggested by Risto Kankkunen
 *	- removed a call to turn break off under MSDOS in ibmkopen()
 *	  in ibmpc.c to answer a observation from John A Steele
 *	- changed some file names. ATARI.C => TOS.C   ST520.C => ST52.C
 *	  to more accurately reflect the OS and screen code for the ATARI
 *	- made a lot of changes in TOS.C as specified by Allan Pratt
 *	- fixed setkey() to not choke on undefined functions
 *	11-jun-91
 *	- added in support for LATIN as submited by Anthony Appleyard
 *	- cause the prompt for search patterns to show the current search
 *	  terminator
 *	23-jun-91
 *	- in BASIC.C in gotobop/eop() changed some offests to zero constants
 *	  speeding things up.
 *	- also added $scribe so I can add in scribe specific formatting
 *	  features, like deciding @commands at the begining of lines delimit
 *	  paragraphs.
 *	26-jun-91	John Gamble
 *	- added check for isearch and risearch binding in the incremental
 *	  search commands, as suggested by ???.
 *	- Put in new text strings from search.c into the <language>.h files.
 *	- Removed the code in BIND.C which checked for the .rc file in the
 *	  current directory before checking HOME.  This was dangerous in a
 *	  multi-user environment.  Change the HOME variable if you want to
 *	  use a different set of .rc and .cmd files.
 *	10-jun-91	Daniel Lawrence
 *	- dumped $scribe. Instead we are adding features to handle any
 *	  formatter of that style. $fmtlead lists all formatter command
 *	  leadin characters. $paralead lists all paragraph start charaters.
 *	- changed a check in setvar() to allow an interactive input value
 *	  to be just the <NL>, allowing you now to interactively set
 *	  a variable to being empty. ^G still aborts the set.
 *	17-jun-91
 *	- a missing parenthesis in a conditional in cinsert() sometimes
 *	  caused a problem when strinking return in CMODE
 *	24-jul-91
 *	- changed a write in ttflush() to go to stdout instead of stdin
 *	18-aug-91
 *	- replaced tipc.c driver and made minor accomodations to many files
 *	  for support of memory mapped TIPC driver under Zortech C as supplied
 *	  by Ron Lepine
 *	- added code to msdos and OS/2 to recognize F11, F12 and application
 *	  keypad as seperate, bindable keys. Submitted by Jon Saxton
 *	2-Sep-91
 *	- made some minor changes for OS/2 submitted by John Ladwig
 *	4-Sep-91
 *	- made a lot of adjustments to the new keyboard.c file for recognizing
 *	  IBMPC extended keys. I think most of its keys are now mapped in all
 *	  modes which the BIOS will recognize under both MSDOS and OS/2.
 *	6-Sep-91 John M. Gamble
 *	- changed the outer while loop of scanner() to a if/do-while
 *	  combination.  This allowed the removal from the inner loop of an if
 *	  statement that was returning a constant value.  Speeds up searches
 *	  noticably.
 *	- reverse searching in fbound() was using matchlen as a jump increment.
 *	  This causes reverse-search to fail if we had just removed MAGIC mode,
 *	  since matchlen may be different from the pattern length, causing the
 *	  search to fail.  Changed to (patlenadd + 1).
 *	- Code which stored away the previous match before saving the current
 *	  match is moved to the replaces() function where it belongs.  Less
 *	  work for simple searches.  Also changed the free()/malloc()
 *	  combinations to simple realloc() calls - much neater.
 *	- Putting the "next character" calls in the correct place in amatch()
 *	  meant that i could get rid of all the special checking for BOL and
 *	  EOL characters.  Saved much code space that way.
 *	- Fixed the fence flashing bug where the cursor failed to flash back
 *	  to a fence which was the first character in a buffer.  Simple matter
 *	  of moving the forward/backward cursor calls in the right place.
 *	- New function regtostr() is what is called now to save matches, group
 *	  matches, set $region, etc.  Space saver, faster.
 *	7-Sep-91 John M. Gamble
 *	- Changed the realloc()s back to free()/malloc() combinations.
 *	  There are still compilers out there who do not have ANSI C
 *	  functions, like certain UNIX systems, which used to be on the
 *	  cutting edge, but are now warts.  Arghhh.
 *	- Fixed a group length problem, when matchlen was getting added
 *	  more than once (closure again).  New group_length array fixes
 *	  that.
 *	12-Sep-91 Daniel Lawrence
 *	- fixed setting of $cbflags, changing mask from BFCHG&BFINVS to
 *	  BFCHG|BFINVS, letting it work as suggested by John Gamble
 *	14-Sep-91
 *	- fixed a bug in the mouse handling in mouseline() which prevented
 *	  us from positioning the mouse on the last line of the window when
 *	  $modeflag was set to FALSE
 *	15-Sep-91
 *	- In frustration from the new help system crashing and leaving me
 *	  unable to comunicate visibly with the command like, emacs now
 *	  sets $discmd and $disinp to TRUE whenever it goes to fetch an
 *	  interactive command. Delete the assignment in execloop() if this
 *	  gets in the way
 *	21-Sep-91
 *	- the new code for decoding the AT keyboard fails real poorly on an
 *	  XT or earlier style machine. When ATKBD is enabled, if the call
 *	  fails the first time, emacs automatically switched to using the
 *	  old style (not that old really) BIOS calls for input.
 *	5-Oct-91
 *	- two new variables, $orgrow and $orgcol, and two new function
 *	  change-screen-row and change-screen-column, all you to change
 *	  the upper left corner of emacs' screen on the physical screen.
 *	  You must resize the screen smaller than default before this
 *	  will work.
 *	- screen resolution changes on the IBMPC force the screen back
 *	  to the upper left corner. This may change in the future.
 *	8-Oct-91
 *	- size and position of screen maintained in the SCREEN structure
 *	  so switching screen resets these.
 *	- Mousing code now switches screens if mouse is used outside of
 *	  current screen.
 *	9-Oct-91
 *	- Switching screen deactivates modeline of outgoing screen.
 *	- Screen list now orginized front->back instead of alphabetically
 *	- dumped $scrnum as it changes dynamically and is not very usefull
 *	- replaces next-screen and previous-screen with cycle-screens (A-C)
 *	  which brings the rearmost screen to front. Mousing to a window
 *	  now operates in front to back order! screen.c is smaller!
 *	11-Oct-91
 *	- added logic to refresh screens when sizing down or moving screen
 *	- added $deskcolor var to hold color desktop should be cleared to
 *	- added TERM structure entry .t_clrdesk as a ptr to function to
 *	  clear the desktop when redrawing windows
 *	15-Oct-91
 *	- added code to ibmpc version to properly track screen resolution
 *	  changes. If a screen is the size of the desktop at resolution change
 *	  time, it is resized to the new desktop size.
 *	16-Oct-91
 *	- noticed the wrong character set when returning from VGA12 mode
 *	  on the ibmpc. Added code to fix this in egaclose() in ibmpc.c
 *	21-oct-91
 *	- added code in IBMPC output routine to interpret char 7 as a beep
 *	25-oct-91
 *	- added ability of macro to suppress screen redraw by setting second
 *	  bit in $gflags... like after a shell-command that doesn't output
 *	30-oct-91
 *	- added ability to set any $...hook variable to a named procedure
 *	  as well as a built in command.
 *	- modified the standard startup file EMACS.RC to use named procedures
 *	  rather than numbered macroes for all its internal functions.
 *	- Modified manual to emphisize the use of named procedures rather
 *	  than the execute-macro-<n> commands which I am trying to phase out.
 *	31-oct-91
 *	[RELEASED version 3.11 for general distribution]
 */
 *	31-Oct-91 Ken Cornetet
 *	- extra parameter in non-proto version of setkey() deleted
 *	- Microport V/AT modifications. An "ndir" package is needed
 *	  (contact Ken at <kenc@ezelmo.UUCP> for this)
 *	  Also, set terminfo string "is2" on closing tty
 *	19-Nov-91 Pete Dunlap
 *	- changed initializers for search path for the AMIGA to more closely
 *	  resemble reality, don't need to put boot disk in on each search
 *	- added XENIX to the list of files to preserve permisions
 *	19-Nov-91 Daniel Lawrence
 *	- added two missing zeroes in the TERM structure for the HP150
 *	21-nov-91
 *	- added $timeflag, when TRUE displays hour/minute of last keystroke
 *	  on bottom modeline
 *	- ? in file/buffer/command name completion brings up a completion
 *	  list as will a partial completion attempt.
 *	- changed all lists/description popups to BFINVS invisible buffers
 *	- changed name of buffer list buffer from [List] to [Buffers]
 *	- added some missing systems to the list for dolock() in dolock.c
 *	  (notable USG, V7 and BSD)
 *	23-nov-91
 *	[RELEASED update 3.11a via BBS]
 *	24-dec-91
 *	- added new commands yank-pop (M-Y), delete-kill-ring (M-^Y),
 *	  and cycle-ring (^X-Y) to handle a ring of kill buffers. More
 *	  info on their use in the reference manual.
 *	- changed version to 3.11b
 *	5-jan-92 John M. Gamble
 *	- pop-buffer, with a numeric argument, now behaves like select-buffer
 *	  by making that buffer INVISIBLE.
 *	- Reverse search groups handled more sensibly, smaller code.
 *	7-jan-92 Walter Warniaha/via BBS
 *	- made some fixes to help MSC under MSDOS work properly. Set default
 *	  stack to 20K, fixed some variable conflicts.
 *	15-jan-92
 *	- added $posflag to show point position on current modeline as
 *	  suggested by Brian Casiello, defaults to on.
 *	19-jan-92 John M. Gamble
 *	- On some otherwise ANSI C compilers, __STDC__ is not 1, because we
 *	  also make use of the PASCAL modifier, which is not ANSI.  So in
 *	  ESTRUCT.H, where there is a check on __STDC__, we also list the
 *	  other ANSI C compilers.  This turns on VOID, CONST, and makes
 *	  ETYPE a union instead of a struct.
 *	25-jan-92 John M. Gamble
 *	- Normal Isearch exit updates the search jump tables too.
 *	31-jan-91
 *	- fill-paragraph on an empty buffer no longer hangs
 *	- changed version number to 3.11c and released via BBS

history()

{
	/* this is here to keep compilers from complaining... it's not needed */
}
