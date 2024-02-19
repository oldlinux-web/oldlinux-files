# Lander Makefile
 
# System 5.3 #
# This is for Suns, to get the 'nifty line drawing graphics' mentioned in
# the README file.  -bcs
#CC=/usr/5bin/cc
#OSV=SYS5_3
#LIBS= -lcurses -lm
#RAND_L=RAND_SYS5
#KLUDGES=

# Linux 0.96a (this is a mix of the BSD stuff and SysV stuff - took me half an hour to find though ;-)
CC=gcc
OSV=BSD
LIBS= -lcurses -ltermcap -lm
RAND_L=RAND_SYS5
KLUDGES=-DLINUX_HACK

# BSD Unix
#
#OSV=BSD
#LIBS= -lcurses -ltermcap -lm
#RAND_L=RAND_BSD
#KLUDGES=

# System 5.2 or earlier (untested but should work)
#
# OSV=BSD
# LIBS= -lcurses -lm
# RAND_L=RAND_SYS5
# KLUDGES=
 
# high score file name - change for your system
HSFILE= /usr/games/lib/lander.hs
 
OBJS= land.o screen.o move.o score.o
SRC= land.c screen.c move.c score.c
INC= consts.h funcs.h
BIN= $(HOME)/lander
OPT= -O
HSSTRING='"$(HSFILE)"'
CFLAGS= $(OPT) -D$(RAND_L) $(KLUDGES) -DHS_FILE=$(HSSTRING) -D$(OSV)
 
lander: $(OBJS)
	$(CC) -o lander $(CFLAGS) $(OBJS) $(LIBS)
 
$(OBJS): consts.h
 
shar:
	xshar README lander.6 Makefile $(SRC) $(INC) > lander.shar

 
lint:
	lint $(CFLAGS) $(SRC)

install: lander
	rm -f $(BIN)/lander
	cp lander $(BIN)

clean:
	rm *.o lander
