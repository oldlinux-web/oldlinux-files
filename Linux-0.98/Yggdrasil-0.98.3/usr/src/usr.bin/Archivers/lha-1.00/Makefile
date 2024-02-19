################################################################
# Makefile for LHa topdir
#		Mar. 2, 1992, written by Masaru Oki.
################################################################

#-----------------------------------------------------------------------
# CONFIGURATION
#	SWITCHES macro definitions ... see config.eng or config.jpn.
#-----------------------------------------------------------------------

SHELL=/bin/sh
MAKE = make
CC = gcc

# Switches addapted to Linux 

SWITCHES	= -DEUC -DSYSV_SYSTEM_DIR -DSYSTIME_HAS_NO_TM -DMKTIME

OPTIMIZE	= -g -O # -fstrength-reduce -fomit-frame-pointer

BINDIR = /usr/local/bin
MANDIR = /usr/local/man
MANSECT = n

INSTALL = install
INSTALLBIN = -s -m 755
INSTALLMAN = -m 644

SUBDIRS = src man

O		= .o

# missing library objects.
#	memset$(O) lhdir$(O)
LIBRARYOBJS	= patmatch$(O)

MYMAKEFLAGS = 'BINDIR=$(BINDIR)' 'MANDIR=$(MANDIR)' 'MANSECT=$(MANSECT)'\
 'INSTALL=$(INSTALL)' 'INSTALLBIN=$(INSTALLBIN)' 'INSTALLMAN=$(INSTALLMAN)'\
 'CC=$(CC)' 'SWITCHES=$(SWITCHES)' 'O=$(O)' 'LIBRARYOBJS=$(LIBRARYOBJS)' \
 'OPTIMIZE=$(OPTIMIZE)'

all:
	@for i in $(SUBDIRS); do \
	echo "make all in $$i..."; \
	(cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) all); done

install:
	@for i in $(SUBDIRS); do \
	echo "Installing in $$i..."; \
	(cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) install); done

clean:
	@for i in $(SUBDIRS); do \
	echo "Clearing in $$i..."; \
	(cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) clean); done

moreclean:
	@for i in $(SUBDIRS); do \
	echo "Clearing in $$i..."; \
	(cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) moreclean); done









