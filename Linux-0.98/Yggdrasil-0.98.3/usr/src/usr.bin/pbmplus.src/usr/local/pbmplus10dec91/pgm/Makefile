# Makefile for pgm tools.
#
# Copyright (C) 1989, 1991 by Jef Poskanzer.
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted, provided
# that the above copyright notice appear in all copies and that both that
# copyright notice and this permission notice appear in supporting
# documentation.  This software is provided "as is" without express or
# implied warranty.

# Default values, usually overridden by top-level Makefile.
CC =		gcc
#CC =		gcc -ansi -pedantic -fcombine-regs -fpcc-struct-return
CFLAGS =	-O -DSYSV -DLINUX
#CFLAGS =	-g
#CFLAGS =	-g -O
LDFLAGS =	-s
#LDFLAGS =	
INSTALLBINARIES =	/usr/local/pbmplus
INSTALLMANUALS1 =	/usr/man/man1
SUFFIXMANUALS1 =	1
INSTALLMANUALS3 =	/usr/man/man3
SUFFIXMANUALS3 =	3
INSTALLMANUALS5 =	/usr/man/man5
SUFFIXMANUALS5 =	5
MANCP =			cp 	

PBMDIR =	../pbm
INCLUDEPBM =	-I$(PBMDIR)
LIBPBM =	$(PBMDIR)/libpbm.a
DEFPBM =	$(PBMDIR)/pbm.h ../pbmplus.h
DEFLIBPBM =	$(PBMDIR)/libpbm.h

SHELL =		/bin/sh
INCLUDE =	-I.. $(INCLUDEPBM)
ALLCFLAGS =	$(CFLAGS) $(INCLUDE)
LIBPGM =	libpgm.a

PORTBINARIES =	fitstopgm fstopgm hipstopgm lispmtopgm pgmbentley \
		pgmenhance pgmhist pgmnorm pgmoil pgmramp \
		pgmtofits pgmtofs pgmtolispm pgmtopbm \
		psidtopgm rawtopgm
MATHBINARIES =	pgmcrater pgmedge pgmtexture
BINARIES =	$(PORTBINARIES) $(MATHBINARIES)

OBJECTS =	fitstopgm.o fstopgm.o hipstopgm.o lispmtopgm.o pgmbentley.o \
		pgmenhance.o pgmhist.o pgmnorm.o pgmoil.o pgmramp.o \
		pgmtofits.o pgmtofs.o pgmtolispm.o pgmtopbm.o \
		psidtopgm.o rawtopgm.o pgmcrater.o pgmedge.o pgmtexture.o

MANUALS1 =	$(BINARIES)
MANUALS3 =	libpgm
MANUALS5 =	pgm


#all:		binaries
all:		merge
#install:	install.bin
install:	install.merge


binaries:	$(BINARIES)

install.bin:	binaries
	cd $(INSTALLBINARIES) ; rm -f $(BINARIES)
	cp $(BINARIES) $(INSTALLBINARIES)


merge:		pgmmerge
pgmmerge:	pgmmerge.c $(OBJECTS) $(LIBPGM) $(LIBPBM)
	$(CC) $(ALLCFLAGS) $(LDFLAGS) -o $@ $@.c $(OBJECTS) -lm $(LIBPGM) $(LIBPBM)

install.merge:	install.pgmmerge
install.pgmmerge:	pgmmerge
	cd $(INSTALLBINARIES) ; rm -f $(BINARIES)
	cp pgmmerge $(INSTALLBINARIES)
	cd $(INSTALLBINARIES) ; for i in $(BINARIES) ; do ln pgmmerge $$i ; done
	rm $(INSTALLBINARIES)/pgmmerge


install.man:
	for i in $(MANUALS1) ; do \
	    rm -f $(INSTALLMANUALS1)/$$i.$(SUFFIXMANUALS1) ; \
	    $(MANCP) $$i.1 $(INSTALLMANUALS1)/$$i.$(SUFFIXMANUALS1) ; \
	done
	for i in $(MANUALS3) ; do \
	    rm -f $(INSTALLMANUALS3)/$$i.$(SUFFIXMANUALS3) ; \
	    $(MANCP) $$i.3 $(INSTALLMANUALS3)/$$i.$(SUFFIXMANUALS3) ; \
	done
	for i in $(MANUALS5) ; do \
	    rm -f $(INSTALLMANUALS5)/$$i.$(SUFFIXMANUALS5) ; \
	    $(MANCP) $$i.5 $(INSTALLMANUALS5)/$$i.$(SUFFIXMANUALS5) ; \
	done


# Rule for plain programs.
$(PORTBINARIES):	pgm.h $(DEFPBM) $(LIBPGM) $(LIBPBM)
	$(CC) $(ALLCFLAGS) $(LDFLAGS) -o $@ $@.c $(LIBPGM) $(LIBPBM)

# Rule for math-dependent programs.
$(MATHBINARIES):	pgm.h $(DEFPBM) $(LIBPGM) $(LIBPBM)
	$(CC) $(ALLCFLAGS) $(LDFLAGS) -o $@ $@.c -lm $(LIBPGM) $(LIBPBM)

# Rule for objects.
$(OBJECTS):	pgm.h $(DEFPBM)
	$(CC) $(ALLCFLAGS) "-Dmain=$*_main" -c $*.c

# And libraries.
$(LIBPBM):
	cd $(PBMDIR) ; make lib
lib:		$(LIBPGM)
$(LIBPGM):	libpgm1.o libpgm2.o
	-rm $(LIBPGM)
	ar rc $(LIBPGM) libpgm1.o libpgm2.o
	-ranlib $(LIBPGM)

libpgm1.o:	pgm.h $(DEFPBM) libpgm.h libpgm1.c
	$(CC) $(ALLCFLAGS) -c libpgm1.c
libpgm2.o:	pgm.h $(DEFPBM) libpgm.h libpgm2.c $(DEFLIBPBM)
	$(CC) $(ALLCFLAGS) -c libpgm2.c

# Other dependencies.
fitstopgm fitstopgm.o:		fitstopgm.c
fstopgm fstopgm.o:		fstopgm.c
hipstopgm hipstopgm.o:		hipstopgm.c
lispmtopgm lispmtopgm.o:	lispmtopgm.c
pgmbentley pgmbentley.o:	pgmbentley.c
pgmcrater pgmcrater.o:		pgmcrater.c
pgmedge pgmedge.o:		pgmedge.c
pgmenhance pgmenhance.o:	pgmenhance.c
pgmhist pgmhist.o:		pgmhist.c
pgmnorm pgmnorm.o:		pgmnorm.c
pgmoil pgmoil.o:		pgmoil.c
pgmramp pgmramp.o:		pgmramp.c
pgmtexture pgmtexture.o:	pgmtexture.c
pgmtopbm pgmtopbm.o:		pgmtopbm.c dithers.h $(DEFPBM)
pgmtofits pgmtofits.o:		pgmtofits.c
pgmtofs pgmtofs.o:		pgmtofs.c
pgmtolispm pgmtolispm.o:	pgmtolispm.c
psidtopgm psidtopgm.o:		psidtopgm.c
rawtopgm rawtopgm.o:		rawtopgm.c

clean:
	-rm -f *.o *.a *.cat core $(BINARIES) pgmmerge
