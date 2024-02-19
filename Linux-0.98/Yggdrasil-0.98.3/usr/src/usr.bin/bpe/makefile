#________________ Start customizing here ________________
#
# If your Terminals and your curses lib supports keypad()
# comment out the next line. You probably need it for BSD.
NKEYPAD     = -DNOKEYPAD

# if you want the search operations to show found patterns in context,
# set CLINES to the number of line of data to display before the pattern.
# else comment out the next line
# CLINES      = -DCLINES=1

# if you want the search operations to start the display on a mod 16 
# boundary, leave the next line, else comment out
#ALLIGN	    = -DALLIGN

# libraries to make curses work on your machine. Probably just curses
# for V.2 and later, as is for BSD. You could try termlib instead of 
# termcap if the termcap library is not available.
LIBES       = -lcurses -ltermcap

# local compilation and link options needed, such a 286 model selection, etc
LOCAL       =
#
# ________________ Stop customizing here ________________


CFLAGS = -O $(NKEYPAD) $(CLINES) $(ALLIGN)
OBJS = bpe.o hexsrch.o
SRCS = bpe.c hexsrch.c
EXEC = bpe

# for making a shar file
SHARLIST = $(SRCS) makefile readme bpe.1
SHAR = shar

$(EXEC): $(OBJS)
	$(CC) -o $(EXEC) $(LOCAL) $(OBJS) $(LIBES)

#$(OBJS): $(SRCS)
#	$(CC) -c $(CFLAGS) $(LOCAL) $(SRCS)
# special makerules here, portable
.c.o:
	$(CC) -c $(CFLAGS) $(LOCAL) $?

shar:		bpe.shar
bpe.shar:	$(SHARLIST)
	$(SHAR) $(SHARLIST) > bpe.shar

arc:		bpe.arc
bpe.arc:	bpe.exe bpe.doc
	arc a bpe $?
	rm bpe.doc

bpe.doc:	bpe.1
	nroff -man -Tlp bpe.1 | col > bpe.doc
