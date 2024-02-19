#
# Makefile for getty distribution
#
# To recreate this file, make any changes in config.sh and then
# say "sh Makefile.SH"
#

SHELL=	/bin/sh

# compiler stuff -- check these
CC=	gcc
CFLAGS=	-O -fpcc-struct-return
LFLAGS=	

LIBS=	-ltermcap
LINT=	lint -abchuvx
LLIBS=	

# where things go
BIN=	/bin
UUBIN=	/bin

# what are we making
SH=	Makefile.SH config.h.SH makedep.SH
HDRS=	config.h defaults.h extern.h funcs.h getty.h patchlevel.h \
	table.h tune.h
SRC=	main.c funcs.c defaults.c table.c 
OBJ=	main.o funcs.o defaults.o table.o 
UUSRC=	umain.c funcs.c defaults.c table.c 
UUOBJ=	umain.o funcs.o defaults.o table.o 

# rules

all:	getty uugetty manpages

clist:
	@echo $(SRC) $(UUSRC) | /usr/bin/tr ' ' '\012' | /usr/bin/sort | /usr/bin/uniq > .clist

hlist:
	@echo $(HDRS) | /usr/bin/tr ' ' '\012' > .hlist

shlist:
	@echo $(SH) | /usr/bin/tr ' ' '\012' > .shlist

getty:	$(OBJ)
	$(CC) $(LFLAGS) -o $@ $(OBJ) $(LIBS) 

uugetty: $(UUOBJ)
	$(CC) $(LFLAGS) -o $@ $(UUOBJ) $(LIBS) 

manpages:
	cd man; make

install: getty uugetty
	-mv $(BIN)/getty $(BIN)/getty-
	-mv $(UUBIN)/uugetty $(UUBIN)/uugetty-
	cp getty $(BIN)
	cp uugetty $(UUBIN)
	chmod 700 $(BIN)/getty $(UUBIN)/uugetty
	strip $(BIN)/getty $(UUBIN)/uugetty

manistall: manpages
	cd man; make install

lint:	$(SRC) umain.c
	@echo "linting getty sources..."
	echo "GETTY" >lint.out
	$(LINT) $(SRC) $(LLIBS) >>lint.out
	@echo "linting uugetty sources..."
	@echo '' >>lint.out
	echo 'UUGETTY' >>lint.out
	$(LINT) -DUUGETTY $(UUSRC) $(LLIBS) >>lint.out
	@echo "lint output is in lint.out"

clean:
	rm -f umain.c *.o core *.out .*list *.ln Makefile.old
	cd man; make clean

clobber: clean
	rm -f getty uugetty
	cd man; make clobber

realclean: clobber

depend:	makedep umain.c tune.h
	chmod +x makedep
	./makedep

# special dependancies follow

umain.c: main.c
	-ln main.c umain.c

umain.o:
	$(CC) $(CFLAGS) -DUUGETTY -c umain.c

tune.h:	tune.H
	@echo "------------------------------------------------"
	@echo "Making a tune.h from the tune.H prototype file. "
	@echo "You may wish to edit tune.h before making getty."
	@echo "------------------------------------------------"
	-cp tune.H tune.h

# AUTOMATICALLY GENERATED MAKE DEPENDENCIES--PUT NOTHING BELOW THIS LINE
# If this runs make out of memory, delete /usr/include lines.
defaults.o : defaults.c getty.h config.h tune.h extern.h funcs.h mytermio.h \
  defaults.h 
funcs.o : funcs.c getty.h config.h tune.h extern.h funcs.h mytermio.h table.h 
getutent.o : getutent.c getty.h config.h tune.h extern.h funcs.h mytermio.h 
main.o : main.c getty.h config.h tune.h extern.h funcs.h mytermio.h defaults.h \
  table.h release.h 
putenv.o : putenv.c getty.h config.h tune.h extern.h funcs.h mytermio.h 
strdup.o : strdup.c getty.h config.h tune.h extern.h funcs.h mytermio.h 
table.o : table.c getty.h config.h tune.h extern.h funcs.h mytermio.h table.h 
umain.o : umain.c getty.h config.h tune.h extern.h funcs.h mytermio.h defaults.h \
  table.h release.h 
# WARNING: Put nothing here or make depend will gobble it up!
