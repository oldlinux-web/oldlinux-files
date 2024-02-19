# Configure CFLAGS/LDFLAGS for your machine/OS
# for HP-UX and other Sys V systems:
#CFLAGS= -O -DSYSV
# for VAX 4.[23] BSD, SunOS and other BSD based systems:
# CFLAGS= -O -DBSD
# For Linux
CFLAGS= -O -DLINUX
# Need getopt for pwget, include library that has it.
# System V based system (like HP-UX), 4.3BSD and SunOS have null LDFLAGS.
LDFLAGS= -s -O
# 4.2BSD requires getopt from someplace.
# LDFLAGS= -lgetopt

MODE=755
OWNER=bin
GROUP=bin
DEST=dest
MANMODE=444
MANOWNER=bin
MANGROUP=bin
MANDEST=man

all: grget pwget getpass

install: installcmd installman

installman: pwget.1
# use install for BSD and cpset for SYSV
#	install -c -o $(MANOWNER) -g $(MANGROUP) -m $(MANMODE) pwget.1 $(MANDEST)
	cpset pwget.1 $(MANDEST) $(MANMODE) $(MANOWNER) $(MANGROUP)

grget: pwget
	-rm grget
	ln -s pwget grget

pwget: pwget.o
	cc $(LDFLAGS) -o pwget pwget.o

getpass: getpass.o
	cc $(LDFLAGS) -o getpass getpass.o

pwget.o: pwget.c
	cc $(CFLAGS) -c pwget.c

pwget.cat: pwget.1
	tbl pwget.1 | nroff -man -T > pwget.cat

clean:
	/bin/rm -f pwget.o pwget grget

clobber: clean
