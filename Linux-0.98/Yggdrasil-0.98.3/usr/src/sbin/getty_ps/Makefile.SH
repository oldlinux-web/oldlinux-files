:
# $Id: Makefile.SH,v 2.0 90/09/19 19:28:46 paul Rel $
#
# Creates Makefile for getty distribution
#
# $Log:	Makefile.SH,v $
# Revision 2.0  90/09/19  19:28:46  paul
# Initial 2.0 release
# 
# 

case $CONFIG in
'')
    if test ! -f config.sh; then
	ln ../config.sh . || \
	ln ../../config.sh . || \
	ln ../../../config.sh . || \
	(echo "Can't find config.sh."; exit 1)
    fi
    . config.sh
    ;;
esac
case "$0" in
*/*) cd `expr X$0 : 'X\(.*\)/'` ;;
esac

: add necessary modules based on config.sh
case "$d_getutent" in
"$define")
    src=""
    obj=""
    ;;
"$undef")
    src="getutent.c"
    obj="getutent.o"
    ;;
esac
case "$d_strdup" in
"$undef")
    src="$src strdup.c"
    obj="$obj strdup.o"
    ;;
esac
case "$d_putenv" in
"$undef")
    src="$src putenv.c"
    obj="$obj putenv.o"
    ;;
esac

echo "Extracting Makefile (with variable substitutions)"
$spitshell >Makefile <<!GROK!THIS!
#
# Makefile for getty distribution
#
# To recreate this file, make any changes in config.sh and then
# say "sh Makefile.SH"
#

SHELL=	/bin/sh

# compiler stuff -- check these
CC=	$cc
CFLAGS=	$optimize $ccflags $small
LFLAGS=	$ldflags $split
LIBS=	$libs
LINT=	lint -abchuvx
LLIBS=	$llib_termlib

# where things go
BIN=	$bin
UUBIN=	$bin

# what are we making
SH=	Makefile.SH config.h.SH makedep.SH
HDRS=	config.h defaults.h extern.h funcs.h getty.h patchlevel.h \\
	table.h tune.h
SRC=	main.c funcs.c defaults.c table.c $src
OBJ=	main.o funcs.o defaults.o table.o $obj
UUSRC=	umain.c funcs.c defaults.c table.c $src
UUOBJ=	umain.o funcs.o defaults.o table.o $obj

# rules

all:	getty uugetty manpages

clist:
	@echo \$(SRC) \$(UUSRC) | $tr ' ' '\012' | $sort | $uniq > .clist

hlist:
	@echo \$(HDRS) | $tr ' ' '\012' > .hlist

shlist:
	@echo \$(SH) | $tr ' ' '\012' > .shlist
!GROK!THIS!
$spitshell >>Makefile <<'!NO!SUBS!'

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
!NO!SUBS!
chmod 644 Makefile
$eunicefix Makefile
