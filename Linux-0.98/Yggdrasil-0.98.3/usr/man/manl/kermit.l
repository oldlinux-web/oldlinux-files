.TH KERMIT 1C LOCAL
.SH NAME
kermit \- file transfer, management, and terminal connection
.SH SYNOPSIS
.B kermit
[ option ...] [file ...]
.SH DESCRIPTION
.I Kermit
is a family of file transfer, management, and telecommunication programs
available for many different computer operating systems and architectures.
Kermit programs transfer both text and binary files completely and correctly,
and most Kermit programs include dialout and terminal connection capability.
This man page describes version 5A of C-Kermit, the Kermit program written in
C for UNIX, VAX/VMS, and several other computer systems.  See the C-Kermit
documentation (/usr/doc/kermit.doc, distributed as ckuker.doc in plain text
format and ckuker.ps for Postscript printers) or the Kermit book for further
information.  (During testing, this version of kermit may be called wermit.
In that case, you should replace kermit with wermit in file names mentioned in
this man page in order to use the newer version.)
.PP
Kermit is designed to help you communicate between two computers over dialups
and other RS-232 lines, and in some cases also over networks.  If you run
C-Kermit on a desktop computer, it will dial for you, and even log in
automatically over the dialup connection.  Once the connection is established,
you can have a terminal session and you can transfer files.  To transfer files,
you need to run Kermit on both computers.
.PP
Kermit is an "integrated" communications program.  It does the same thing as
separate dialout programs like CU or Tip, and file transfer programs such as
Xmodem.  It also has a script language, which you can use to automate
communications tasks.  PC implementations of Kermit normally have a terminal
emulator built in.  The version of Kermit described in this man page
doesn't include a terminal emulator because most systems on which it is used
don't need one.
.PP
Kermit has one major philosophical difference from most file transfer
programs:  Normally the copy of Kermit on the remote machine runs as a
"server".  With programs such as Xmodem, in order to transfer a file you have
to tell the program on the remote machine to send the file, then return to
your home computer and tell it to receive the file.  For every file to be sent
or received, you must give commands to both computers.  In contrast, Kermit
allows you to control all operations from your home computer.  The copy of
Kermit on the remote machine acts as a slave.  Your home computer coordinates
operations, sending the necessary commands to the remote Kermit.  In addition
to transferring files, you can also tell Kermit to execute commands on the
server, such as doing directories, deleting files, and even sending and
reading mail.
.PP
Normally you give Kermit commands interactively.  That is, to start Kermit,
simply type the command "kermit" with no arguments.  Once Kermit has started
up, it will prompt you for commands.  Kermit's command interpreter has good
built-in help facilities.  If you use the same commands a lot, you can put
them into files, and get Kermit to execute them automatically.  It is also
possible to give Kermit commands by using options on the command line.  This
method is normally used in shell scripts.  Those options will be described at
the end of this man page.
.PP
When Kermit starts, it reads commands from a file called .kermrc in your home
directory.  It executes those commands before prompting you for interactive
commands.  Typically .kermrc will define parameters of the terminal line such
as line speed and parity, or it may contain macro definitions.  It will also
give instructions to Kermit on such matters as transfer block size and use of
sliding windows.
.PP
Kermit's command language is too large to describe it completely in a man
page.  Thus the rest of this man page will give some examples, and then
summarize the facilities available.  See /usr/doc/kermit.doc for complete
documentation.
.SH "TYPICAL COMMANDS FOR A SERVER SYSTEM"
.PP
Here is a reasonable .kermrc file for use on a server system.  By "server", we
mean the "big" computer you are logged into remotely, as opposed to the PC on
your desktop.  On the remote computer, you typically want .kermrc to set up
some default parameters and then put Kermit into server mode.  The idea is
that you log into the big system, start the Kermit server, and then return to
Kermit on your PC and issue commands from there.  (To return to Kermit on your
PC, type Control-] and then "c", or whatever its escape sequence is.)  When
you are finished doing Kermit commands, you type the "finish" and "connect"
commands on your PC.  That tells Kermit on the big system to go out of server
mode, and returns you to an interactive connection.  Or, if you have no
further need to use the remote computer, type "bye" to log out your remote
session and hang up your connection.
.PP
Here is a typical .kermrc file.  It assumes you are using a connection with 8
bits, no parity, which is the default for Kermit and for most direct, modem,
and terminal server PC-to-host connections.
.PP
The following commands set the most efficient communications mode,
with big packet sizes and sliding windows.
.nf

  set receive packet 740
  set window 4
.PP
.fi
The following enables 12-bit checksums.  This is recommended when using big
block sizes.
.nf

  set block 2
.PP
.fi
Remove the semicolons from the following lines if your PC uses Unix or some
Unix-like system such as Minix.  Normally Kermit assumes the two computers are
different and performs conversion of file names and file contents.  This
disables these conversions.

  ;;set fil type binary
  ; Go into server mode.
  server
.fi
.PP
The "server" command causes Kermit to set itself up to accept commands from
Kermit on your PC.  If you have this command in your .kermrc, Kermit will not
prompt you for commands, since it will go into server mode instead.  If you
have put Kermit into server mode by mistake, you can get its prompt back by
typing several Control-C's.
.SH "USE OF C-KERMIT FOR DIALING OUT"
.PP
This man page is for the Unix version of Kermit.  Normally Unix is the 'big'
or remote computer system.  Your desktop computer will normally be using a PC
version of Kermit that has its own documentation, which you should use instead
of this.  This section gives instructions for when your local desktop computer
is running Unix, and you are using C-Kermit to dial out to another computer.
If the other computer is not Unix-based, or not running C-Kermit, you'll need
to refer to the other computer's Kermit documentation for details on commands
and file syntax.
.PP
Here is a .kermrc for use by Kermit on your desktop Unix computer that is
compatible with the setup shown above.  You'll only be able to use this
example directly if you're running Unix on your PC.  But there are equivalent
commands for other versions of Kermit.
.PP
.nf
  ; Tell Kermit which kind of modem your PC has.
  ; Try 'set modem ?' for a list of supported modems.
  set modem hayes

  ; Some computers need the following and some don't.
  ; Remove the semicolon if yours does.
  ;set dial hangup off

  ; The device name for the communication port that the
  ; modem is connected to.  Substitute the name used on 
  ; your computer, if necessary.
  set line /dev/tty0

  ; Speed to use for dialing.  Change this if necessary.
  set speed 2400

  ; Set up efficient communications parameters.
  set receive packet-length 740
  set window 4
  set block-check 2

  ; Uncomment the next line if both systems are Unix:
  ;set file type binary

  ; Define a macro for connecting to a terminal server.
  ; You'll need to change this to match the phone number
  ; you use. 'prompt>' should be changed to match the 
  ; prompt issued by the terminal server, if any.
  ; 'vt100' should be changed to match the terminal type
  ; your console driver or Kermit emulates.  Note the
  ; use dashes to continue the long command line.
  ;
  define dodial dial 9999999,-
  script ~w1prompt>--prompt>--prompt>--prompt>--prompt> -
  term~sterm~svt100, connect

  ; Here's a macro that connects to a terminal server and
  ; logs you in.  Use the name of the computer you log
  ; into for 'host'.  Use your own name for 'username'
  ; and password, don't store your password in a file!
  ; If the system normally prompts you for an account use
  ; your account for 'account', otherwise eliminate
  ; ~raccount.  Note the use of dash for line continuation.
  ;
  define dologin askq \\%p Password:,-
  dial 9999999,-
  script ~w1prompt>--prompt>--prompt>--prompt>--prompt> -
  term~sterm~svt100~r~w15host ogin: username word: \\%p~r-
  account, connect
.fi
.PP
When you start Kermit on your local Unix computer, it will read this .kermrc
and then wait for you to type commands.  If you just want to talk directly to
the modem port, type the CONNECT command.  At that point you'll have to use
"atdt" to dial, or whatever command your modem uses.  If nothing appears on
your screen when you give commands to the modem, escape back to Kermit, give
the command SET CARRIER OFF, CONNECT, and try again.
.PP
The .kermrc file defines two commands that you can use instead of CONNECT to
make things more automatic.  dodial will dial up a terminal server, set the
terminal type, and leave you at the terminal server's prompt.  dologin will do
this and then actually log you into a Unix host.  NOTE: This example of
.kermrc will probably need to be changed to match the syntax of the computer
you are actually logging in to.
.PP
.SH "USING C-KERMIT TO TRANSFER FILES"
Here's how to use Kermit to transfer files.  First run Kermit on local
computer.   Either use the CONNECT command and login to the remote
computer yourself, or use something like the dologin script shown above.  
To transfer files, do the following:
.PP
.in +0.5i
.il -0.5i
.ta +0.2i
.ti -0.2i
\(bu	Run Kermit on your local (desktop) computer.

.ti -0.2i
\(bu	Establish connection to the remote computer.  If you local computer is
running C-Kermit, then use the sequence SET MODEM <modem-name>, SET SPEED,
DIAL <number> (if you are dialing with a modem).  If you are using MS-DOS or
other Kermit that lacks a DIAL command, proceed to the next step.

.ti -0.2i
\(bu	Set any other necessary communication parameters, such as PARITY,
DUPLEX, and FLOW-CONTROL.  If you will be communicating with an IBM mainframe
in line-mode (not full-screen mode), give the command DO IBM.

.ti -0.2i
\(bu	Give the CONNECT command.  If you have a modem, but have not yet
dialed the remote computer, type the modem dialing commands now.

.ti -0.2i
\(bu	Log in to the remote computer.

.ti -0.2i
\(bu	Start Kermit on the remote computer, give it any desired SET commands
for file, communication, or protocol-related parameters.  If you will be
transferring binary files, give the command SET FILE TYPE BINARY to the remote
Kermit.

.ti -0.2i
\(bu	Give the remote Kermit the SERVER command.

.ti -0.2i
\(bu	Escape back to the Kermit program on your local (desktop) computer.
If your local computer is running C-Kermit, type Ctrl-\\ c (Control-backslash
followed by the letter 'c').  If MS-DOS Kermit, use Alt-X or Ctrl-] c.  You
should now see your local Kermit program's prompt.

.ti -0.2i
\(bu	If you will be transferring binary files, give the command SET FILE
TYPE BINARY.

.ti -0.2i
\(bu	Use GET followed by the filename to get files from the remote computer
to your local computer.  Use SEND followed by the filename to put files from
your local computer to the big system.  You may use "wildcard" characters such
as * and ? in the filenames to specify groups of files.

.ti -0.2i
\(bu	When you finish transferring files, and you need to continue your work
on the remote remote computer, type FINISH and then CONNECT.  You're now
talking Kermit on the remote computer again.  Type EXIT to get back to the
command parser on the remote system.  At the end, logout normally on the big
system.  Then escape back to Kermit on your local computer, and then EXIT out
of the local Kermit program.

.ti -0.2i
\(bu	If you don't need to go back to the remote computer again, type the
BYE command to the local Kermit program.  This will shut down the remote
Kermit server and terminate (log out) your remote session.
.il +0.5i
.in -0.5i
.fi
.PP
.SH HELP
.PP
Kermit has extensive built-in help.  You can find out what commands exist by
typing ? at the beginning of a line.  You can type HELP followed by the name
of a command for information on it.  For commands that take multiple
arguments, you can type ? in the middle of the command for more information,
and you can also use help with several arguments.  E.g. SET has many different
things you can set, like SET RECEIVE PACKET-LENGTH 1000.  You can type 'SET
?', or 'SET RECEIVE ?', etc.  'HELP SET' will talk about the 'SET' command in
general,
'HELP SET RECEIVE will talk about the RECEIVE options.
.PP
Kermit also accepts traditional Unix-style command-line arguments.
To see what these are, type 'kermit -h'.
.SH BACKSLASH NOTATION
Within an interactive command, \\ is a special character to help you enter
special quantities, or ordinary characters that would otherwise be illegal or
hard to type.  At the end of a line, \\ (backslash) (or - dash) makes the next
line a continuation of the current line.  Other than that, the character
following the \\ identifies what the thing is:
.nf

  % A user-defined simple (scalar) variable
  & an array reference
  $ an environment variable
  v (or V) a built-in variable
  f (or F) a function
  d (or D) a decimal (base 10) number
  o (or O) an octal (base 8) number
  x (or X) a hexadecimal (base 16) number
  \\ the backslash character itself
  b (or B) the BREAK signal (OUTPUT command only)
  a decimal digit (a 1-3 digit decimal number)
  anything else: following character taken literally.

.fi
Note that numbers turn into the character with that binary code, so
you can use \\7 for a bell, or \\13 for a carriage return. 

.SH COMMANDS
.PP
You can use upper or lower case for commands.  Either one will work (but
remember that Unix filenames are case-sensitive).  Also, you can abbreviate
commands as long as the abbreviation matches only one possibility.  This is a
summary of C-Kermit's commands.  For more information on each, look in the
user manual, or online in /usr/doc/kermit.doc, or type HELP and then the
command name.
.nf
.in 0
.ll 80
.ta 15

;	Introduce a full-line or trailing comment (also #).
!	Run a system command or enter system command interpreter.
ASK	Prompt the user, store user's reply in a variable.
ASKQ	Like ASK, but, but doesn't echo (useful for passwords).
ASSIGN	Assign an evaluated string to a variable or macro.
BYE	Terminate and log out a remote Kermit server.
CD	Change Working Directory (also, CWD).
CLEAR	Clear communication device input buffer.
CLOSE	Close a log or other local file.
COMMENT	Introduce a full-line comment.
CONNECT	Establish a terminal connection to a remote computer.
DECLARE	Declare an array.
DECREMENT	Subtract one (or other number) from a variable.
DEFINE	Define a variable or macro.
DELETE	Delete a file or files.
DIAL	Dial a telephone number.
DIRECTORY	Display a directory listing.
DISABLE	Disallow access to selected features during server operation.
DO	Execute a macro.
ECHO	Display text on the screen.
ENABLE	Allow access to selected features during server operation.
END	A command file or macro.
EXIT	Exit from the program, closing all open files and devices.
FINISH	Instruct a remote Kermit server to exit, but not log out.
FOR	Execute commands repeatedly in a counted loop.
GET	Get files from a remote Kermit server.
GOTO	Go to a labeled command in a command file or macro.
HANGUP	Hang up the phone or network connection.
HELP	Display a help message for a given command.
IF	Conditionally execute the following command.
INCREMENT	Add one (or other number) to a variable.
INPUT	Match characters from another computer against a given text.
LOG	Open a log file -- debugging, packet, session, transaction.
MAIL	Send a file as electronic mail to a specified address.
OPEN	Open a local file for reading or writing.
OUTPUT	Send text to another computer.
PAUSE	Do nothing for a given number of seconds.
PUSH	Invoke host system interactive command interpreter.
PWD	Display current working device/directory.
QUIT	Same as EXIT.
READ	Read a line from a local file.
RECEIVE	Passively wait for files to arrive.
REINPUT	Reexamine text previously received from another computer.
REMOTE	Issue file management commands to a remote Kermit server.
RENAME	Change the name of a file.
RETURN	Return from a user-defined function.
RUN	Run a program or system command.
SCRIPT	Execute a UUCP-style login script.
SEND	Send files.
SERVER	Begin server operation.
SET	Set various parameters.
SHOW	Display values of SET parameters.
SPACE	Display current disk space usage.
STATISTICS	Display statistics about most recent transaction.
STOP	Stop executing macro or command file, return to prompt.
TAKE	Execute commands from a file.
TRANLATE	Translate a file's character set.
TRANSMIT	Upload a file with no error checking.
TYPE	Display a file on the screen.
VERSION	Display the program version number on the screen.
WAIT	Wait for the specified modem signals.
WHILE	Execute commands repeatedly while a condition is true.
WRITE	Write text to a local file.
XIF	Extended IF command.
.ll
.in
.fi
.SH PARAMETERS YOU CAN SET
Here are the parameters you can change with the SET command.  Note
that some of these parameters require further specification.  E.g.
there are several RECEIVE parameters.  So commands using SET RECEIVE
look like "SET RECEIVE PACKET-LENGTH 1000".  For parameters like that,
the possible second words are listed below them. 
.nf
.in 0
.ll 80
.ta 15 

ATTRIBUTES	Turn Attribute packet processing on or off.
     BLOCKSIZE, CHARACTER-SET, DATE, DISPOSITION,
     LENGTH, OS-SPECIFIC, SYSTEM-ID, ALL 
BLOCK-CHECK	Level of packet error detection.  
BUFFERS	Size of send and receive packet buffers.
CARRIER	Treatment of carrier on terminal connections.
COUNT	For counted loops.
DEBUG	Log or display debugging information.
DELAY	How long to wait before sending first packet.
DUPLEX	Specify which side echoes during CONNECT.
ESCAPE	Prefix for "escape commands" during CONNECT.
FILE	Set various file parameters.
     BLOCKSIZE, CARRIAGE-CONTROL, CHARACTER-SET,
     COLLISION, DISPLAY, FORMAT, NAMES, ORGANIZATION,
     RECORD-LENGTH, TYPE, WARNING
FLOW-CONTROL	Communication line full-duplex flow control.
HANDSHAKE	Communication line half-duplex turnaround character.
HOST	Specify network host name.
INCOMPLETE	Disposition for incompletely received files.
INPUT	Control behavior of INPUT command.  
LANGUAGE	Enable language-specific character-set translations.
LINE	Communication line device name.
MACRO	Control aspects of macro execution.
MODEM-DIALER	Type of modem-dialer on communication line.
PARITY	Communication line character parity.  
PROMPT	The C-Kermit program's interactive command prompt.
RECEIVE	Parameters for inbound packets.
     END-OF-PACKET, PACKET-LENGTH, PAD-CHARACTER, PADDING
     START-OF-PACKET, TIMEOUT 
RETRY	Packet retransmission limit.
SEND	Parameters for outbound packets.
	See RECEIVE for subparameters.  Normally you set only
	RECEIVE parameters.  SEND parameters come automatically
	from the Kermit on the other end.  
SERVER	Parameters for server operation.
     TIMEOUT 
SESSION-LOG	File type for session log, text or binary.
SPEED	Communication line speed, e.g. 2400, 9600.
TAKE	Control aspects of TAKE file execution.
TERMINAL	Terminal parameters.
     BYTESIZE 
TRANSFER	File transfer parameters.
     CHARACTER-SET
TRANSMIT	Control aspects of TRANSMIT command execution.
     FILL, LINEFEED, PROMPT 
UNKNOWN	Specify handling of unknown character sets.
WINDOW	File transfer packet window size. 
.in
.ll
.SH SCRIPT SYNTAX
.PP
SCRIPT is a brief method for specifying a dialog between your system and
another one.  There is also a general purpose script programming language
described below.  The SCRIPT command can be used for dialing, logging in, etc.
The syntax consists of strings separated by spaces.  The syntax is 
.nf

  SCRIPT expect send [expect send] . . .  

.fi
Kermit waits until it sees the expect string, and then sends the send string.
If there are several pairs, it waits for each expect string, and then sends
the send string.  (To specify a null expect string, i.e.  to tell Kermit not
to wait, use ~0 as the expect string.)  It normally waits up to 15 seconds for
each expect string.  However ~w can be used to redefine this time.  If the
expect string does not arrive, the script fails at that point.  If you want to
be able to do something about failure, you can use conditional strings, of the
form
.nf

  -send-expect[-send-expect[...]]

.fi
You can think of this as giving a list of things to try.  The first time one
succeeds (i.e. an expected string arrives), it is finished.  At that point
Kermit skips to the end of the dashed sequence.  If an expect fails, Kermit
simply proceeds with the next send in the dashed sequence.
.PP
Within the strings, some special sequences can be used:
.nf

  ~b  backspace
  ~s  space
  ~q  `?'(trapped by Kermit's command interpreter)
  ~n  linefeed
  ~r  carriage return
  ~t  tab
  ~'  single quote
  ~~  tilde
  ~"  double quote
  ~x  XON (Control-Q)
  ~c  don't append a carriage return
  ~o[o[o]]  octal representation of an ASCII character code
  ~d  delay approx 1/3 second during send
  ~w[d[d]]  set wait for next expect to DD seconds

.fi
Kermit sends a carriage return after each of the send-strings
unless the string has a ~c.
.SH "MACROS AND VARIABLES"
C-Kermit allows macros and variables.  A macro is a command
that you define, composed of one or more other C-Kermit commands.
A typical macro is a list of Kermit commands, separated
by commas.  For example:
.nf

  define sun set speed 9600, set parity none, set duplex full,-
  set flow xon/xoff

.fi
You call a macro by using its name, just like normal commands.  You can also
call them using the DO command.  If you have given the DEFINE command above
(or have it in your .kermrc file), then you can type SUN or DO SUN to execute
all the commands in the definition.
.PP
C-Kermit also lets you define variables.  Normal variables look like \\%i,
where i is a single letter.  The alphabetic case doesn't matter.  \\%a and
\\%A are the same.  Like macros, they are defined by DEFINE or ASSIGN.  All
values are strings.  DEFINE gives a variable a fixed value.  ASSIGN computes a
value and assigns it to the variable.  To see the difference, look at
.nf

  def \\%a Monday
  def \\%b Today is \\%a
  assign \\%c Today is \\%a
  def \\%a Tuesday
  echo \\%b
  echo \\%c

.fi
This will print "Today is Tuesday" then "Today is Monday".  The difference is
that when defining \\%c, the \\%a is evaluated at the time of the definition,
whereas when defining \\%b, the variable name \\%a itself is put in the
definition.  It isn't evaluated until the echo \\%b.
.PP
There are also arrays, which use \\& instead of \\%.  They are declared by
DECLARE, e.g. DECLARE \\&A[100].  Elements are referenced with subscripts,
which may themselves be variables, and act like simple variables, e.g.
.nf

  DEFINE \\&A[3] Tuesday

.fi
You can destroy the array by making it zero size, DECLARE \\&A[0].
The first element of an array is [1].
.PP
Macros have normal names.  No \\.  You call them by using the name like a
command.  If you put additional words on the same line as the macro
invocation, they become arguments.  Inside the macro, you can refer to the
arguments as \\%1, \\%2, etc.  For example:
.nf

  C-Kermit>define bsend set file type binary, send \\%1
  C-Kermit>define tsend set file type text, send \\%1
  C-Kermit>bsend kermit
  C-Kermit>tsend kermit.doc

.fi
The number of arguments supplied can be referred to as \\v(argc).  If you call
another macro from a macro, the new one gets its own set of arguments, which
do not interfere with the previous set.
.PP
There are a number of built-in variables, which are referred to by \\v(name).
They cannot be changed.  Type SHOW VARIABLES for a complete list.
.nf

  \\v(argc)      number of arguments in current macro
  \\v(args)      number of program command-line arguments
  \\v(count)     current COUNT value
  \\v(date)      date as 8 Feb 1991
  \\v(directory) current/default directory
  \\v(filespec)  filespec given in most recent SEND/REC/GET command
  \\v(home)      home directory
  \\v(host)      computer host name
  \\v(input)     current INPUT buffer contents
  \\v(line)      current communications device, set by LINE or HOST
  \\v(ndate)     date as 19910208 (yyyymmdd)
  \\v(platform)  Specific machine and/or operating system
  \\v(program)   Name of this program ("C-Kermit")
  \\v(return)    Most recent RETURN value
  \\v(speed)     Current speed, if known, or "unknown"
  \\v(status)    0 or 1 (SUCCESS or FAILURE of previous command)
  \\v(system)    UNIX or VMS
  \\v(time)      time as 13:45:23 (hh:mm:ss)
  \\v(version)   numeric version of Kermit
.fi
.PP
There are builtin functions, invoked as \\Fname.  Type SHOW FUNCTIONS for
a complete list.
.nf

   \\Fliteral(arg)     copy argument literally, no evaluation
   \\Fcharacter(arg)   convert numeric arg to character
   \\Fsubstr(a1,a2,a3) substring of a1, starts at a2, length a3
   \\Flower(arg)       convert to lower case
   \\Fupper(arg)       convert to upper case
   \\Freverse(arg)     reverse character in arg
   \\Frepeat(a1,a2)    repeat a1 a2 times
   \\Flpad(text,n,c)   left pad text to length n with char c
   \\Frpad(text,n,c)   right pad text to length n with char c
   \\Fvalue()          value of most recent RETURN
   \\Fexecute(m a)     execute macro "m" with parameters "a"
   \\Fcontents(v)      return current definition of variable
   \\Fdefinition(m)    return current definition of macro    
   \\Flength(arg)      return the length of the string "arg"
   \\Findex(a1,a2,a3)  position of a2 in string a1, starting at pos a3
   \\Ffiles(f)         number of files matching file spec
   \\Fnextfile()       next file name from list in last \\Ffiles
   \\Fmax(a1,a2)       max of two numbers
   \\Fmin(a1,a2)       min of two numbers
   \\Fmod(a1,a2)       a1 mod a2
   \\Feval(expr)       evaluate arithmetic expression

.fi
Eval allows the following operators in the expression.  The expression
can use variables with \\%.  Precedences are shown
.nf

  ()      1 		
  n !     2    factorial       
  ~ n     3    logical NOT
  - n     4    negative
  n ^ n   4    power
  n * n   5    times
  n / n   5    division
  n % n   5    modulus
  n & n   5    logical AND
  n + n   6    plus
  n - n   6    minus
  n | n   6    logical OR
  n # n   6    exclusion OR
  n @ n   6    greatest common divisor

.fi
.SH "OPTIONS AND COMMAND LINE ARGUMENTS"
.PP
Typically you run Kermit without any arguments, and use a combination of
.kermrc and interactive commands.  However it is possible to put options on
the command line.  This is normally used for scripts.  In this case, Kermit is
invoked as follows:
.nf
.ll 80

  kermit [-x arg [-x arg]...[-yyy]..]]

  -x is an option requiring an argument, 
  -y an option with no argument.
.ta 15

actions:
  -s files	send files
  -s -	send files from stdin
  -r	receive files
  -k	receive files to stdout
  -x	enter server mode
  -f	finish remote server
  -g files	get remote files from server (quote wildcards)
  -a name	alternate file name, used with -s, -r, -g
  -c	connect (before file transfer), used with -l and -b
  -n	connect (after file transfer), used with -l and -b

settings:
  -l line	communication line device
  -j host	network host name
  -q	quiet during file transfer
  -i	binary file transfer
  -b bps	line speed, e.g. 1200
  -m name	modem type
  -p x	parity, x = e,o,m,s, or n
  -t	half duplex, xon handshake
  -e n	receive packet length
  -v n  window size
  -y name	alternate init file name
  -w	don't write over files
  -d	log debug info to debug.log

If no action command is included, enter interactive dialog.
.ll
.in
.fi
.SH FILES
$HOME/.kermrc	\fIKermit\fR initialization commands
.br
\&./.kermrc	more \fIKermit\fR initialization commands
.PD
.SH AUTHORS
Frank da Cruz, Columbia University, with contributions from hundreds of
other volunteer programmers all over the world; "man page" courtesy of Charles
Hedrick, Rutgers University.
.SH SEE ALSO
cu(1C), uucp(1C)
.TP
Christine Gianone and Frank da Cruz,
.IR "C-Kermit User's Guide" ,
version 5A, Columbia University (January 1991)
.TP
The file /usr/doc/kermit.doc
.TP
Frank da Cruz,
.IR "Kermit, A File Transfer Protocol" ,
Digital Press (1987)
.SH DIAGNOSTICS
The diagnostics produced by
.I Kermit
itself are intended to be self-explanatory.
.SH BUGS
See recent issues of the Info-Kermit digest (on BITNET/EARN, the Internet, or
the comp.protocols.kermit newsgroup on Usenet) for discussion, or the file
/usr/doc/kermit.bwr (distributed as ckuker.bwr), for a list of bugs.  Report
bugs via e-mail to Info-Kermit-Request@columbia.edu or KERMIT@CUVMA.BITNET.
