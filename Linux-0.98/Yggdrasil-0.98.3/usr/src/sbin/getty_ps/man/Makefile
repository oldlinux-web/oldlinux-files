#
# Makefile for getty man pages
#

SHELL=	/bin/sh
# no nroff?  get Henry Spencer's AWF!
ROFF=	nroff -man

SRC=	..
TUNE=	$(SRC)/tune.h
RELEASE=$(SRC)/release.h

# what to make
all:	getty.1m gettydefs.4 issue.4

man:	getty.man gettydefs.man issue.man

install:
	@echo "Manpage installation must be done manually, Sorry."

# how to make it
.SUFFIXES: .m4 .3 .man

.m4.3:
	m4 $*.m4 > $*.3

.3.man:
	$(ROFF) $*.3 > $*.man

# dependencies
config.m4:	$(SRC)/config.sh $(TUNE) $(RELEASE)
	chmod +x makeconfig
	./makeconfig

getty.1m:	getty.m4 config.m4
	m4 getty.m4 > getty.1m

getty.man:	getty.1m
	$(ROFF) getty.1m > getty.man

gettydefs.4:	gettytab.m4 config.m4
	m4 gettytab.m4 > gettydefs.4

gettydefs.man:	gettydefs.4
	$(ROFF) gettydefs.4 > gettydefs.man

issue.4:	issue.m4 config.m4
	m4 issue.m4 > issue.4

issue.man:	issue.4
	$(ROFF) issue.4 > issue.man

getutent.3:	getutent.m4 config.m4

getutent.man:	getutent.3

strdup.3:	strdup.m4 config.m4

strdup.man:	strdup.3

putenv.3:	putenv.m4 config.m4

putenv.man:	putenv.3

clean:
	rm -f *.out config.m4 config.sh core

clobber: clean
	rm -f *.1m *.[34] *.man

