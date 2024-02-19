# Makefile for pnm tools.
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
CFLAGS =	-O -DLINUX  -DSYSV
#CFLAGS =	-g
#CFLAGS =	-g -O
TIFFDEF =	-DLIBTIFF
TIFFINC =       -I../libtiff
TIFFLIB =       ../libtiff/libtiff.a
TIFFBINARIES =  tifftopnm pnmtotiff
TIFFOBJECTS =   tifftopnm.o pnmtotiff.o
LDFLAGS =	-s
#LDFLAGS =	
INSTALLBINARIES =	/usr/local/pbmplus
INSTALLSCRIPTS =	$(INSTALLBINARIES)
INSTALLMANUALS1 =	/usr/man/man1
SUFFIXMANUALS1 =	1
INSTALLMANUALS3 =	/usr/man/man3
SUFFIXMANUALS3 =	3
INSTALLMANUALS5 =	/usr/man/man5
SUFFIXMANUALS5 =	5
MANCP =			cp 	
PPMDIR =	../ppm
INCLUDEPPM =	-I$(PPMDIR)
LIBPPM =	$(PPMDIR)/libppm.a
DEFPPM =	$(PPMDIR)/ppm.h
DEFLIBPPM =	$(PPMDIR)/libppm.h

PGMDIR =	../pgm
INCLUDEPGM =	-I$(PGMDIR)
LIBPGM =	$(PGMDIR)/libpgm.a
DEFPGM =	$(PGMDIR)/pgm.h
DEFLIBPGM =	$(PGMDIR)/libpgm.h

PBMDIR =	../pbm
INCLUDEPBM =	-I$(PBMDIR)
LIBPBM =	$(PBMDIR)/libpbm.a
DEFPBM =	$(PBMDIR)/pbm.h ../pbmplus.h
DEFLIBPBM =	$(PBMDIR)/libpbm.h

SHELL =		/bin/sh
INCLUDE =	-I.. $(INCLUDEPPM) $(INCLUDEPGM) $(INCLUDEPBM)
ALLCFLAGS =	$(CFLAGS) $(INCLUDE) $(TIFFDEF) $(TIFFINC)
LIBPNM =	libpnm.a

PORTBINARIES =	pnmarith pnmcat pnmconvol pnmcrop pnmcut \
		pnmdepth pnmenlarge pnmfile pnmflip pnminvert \
		pnmnoraw pnmpaste pnmscale pnmtile pnmtops \
		pnmtorast pnmtoxwd rasttopnm xwdtopnm
MATHBINARIES =	pnmgamma pnmrotate pnmshear
BINARIES =      $(PORTBINARIES) $(MATHBINARIES) $(TIFFBINARIES)
SCRIPTS =	anytopnm pnmindex pnmmargin pnmsmooth

PORTOBJECTS =	pnmarith.o pnmcat.o pnmconvol.o pnmcrop.o pnmcut.o \
		pnmdepth.o pnmenlarge.o pnmfile.o pnmflip.o pnminvert.o \
		pnmnoraw.o pnmpaste.o pnmscale.o pnmtile.o pnmtops.o \
		pnmtorast.o pnmtoxwd.o rasttopnm.o xwdtopnm.o \
		pnmgamma.o pnmrotate.o pnmshear.o
OBJECTS =	$(PORTOBJECTS) $(TIFFOBJECTS)

MANUALS1 =	$(BINARIES) $(SCRIPTS)
MANUALS3 =	libpnm
MANUALS5 =	pnm


#all:		binaries
all:		merge
#install:	install.bin
install:	install.merge


binaries:	$(BINARIES)

install.bin:	binaries $(SCRIPTS)
	cd $(INSTALLBINARIES) ; rm -f $(BINARIES)
	cp $(BINARIES) $(INSTALLBINARIES)
	cd $(INSTALLSCRIPTS) ; rm -f $(SCRIPTS)
	cp -v $(SCRIPTS) $(INSTALLSCRIPTS)
	cd $(INSTALLSCRIPTS) ; chmod +x $(SCRIPTS)


merge:		pnmmerge
pnmmerge:	pnmmerge.c $(OBJECTS) $(LIBPNM) $(LIBPPM) $(LIBPGM) $(LIBPBM)
	$(CC) $(ALLCFLAGS) $(LDFLAGS) -o $@ $@.c $(OBJECTS) -lm $(LIBPNM) $(LIBPPM) $(LIBPGM) $(LIBPBM) $(TIFFLIB)

install.merge:	install.pnmmerge $(SCRIPTS)
install.pnmmerge:	pnmmerge
	cd $(INSTALLBINARIES) ; rm -f $(BINARIES)
	cp pnmmerge $(INSTALLBINARIES)
	cd $(INSTALLBINARIES) ; for i in $(BINARIES) ; do ln pnmmerge $$i ; done
	rm $(INSTALLBINARIES)/pnmmerge
	cd $(INSTALLSCRIPTS) ; rm -f $(SCRIPTS)
	cp $(SCRIPTS) $(INSTALLSCRIPTS)
	cd $(INSTALLSCRIPTS) ; chmod +x $(SCRIPTS)


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


# Rules for plain programs.
$(PORTBINARIES):	pnm.h $(DEFPPM) $(DEFPGM) $(DEFPBM) $(LIBPNM) $(LIBPPM) $(LIBPGM) $(LIBPBM) $(TIFFLIB)
	$(CC) $(ALLCFLAGS) $(LDFLAGS) -o $@ $@.c $(LIBPNM) $(LIBPPM) $(LIBPGM) $(LIBPBM) $(TIFFLIB)

# Rules for tiff binaries.
$(TIFFBINARIES):	pnm.h $(DEFPPM) $(DEFPGM) $(DEFPBM) $(LIBPNM) $(LIBPPM) $(LIBPGM) $(LIBPBM) $(TIFFLIB)
	$(CC) $(ALLCFLAGS) -DUSE_VARARGS=0 -DUSE_PROTOTYPES=1 -DBSDTYPES $(LDFLAGS) -o $@ $@.c $(LIBPNM) \
					$(LIBPPM) $(LIBPGM) $(LIBPBM) $(TIFFLIB)

# Rule for math-dependent programs.
  $(MATHBINARIES):        pnm.h $(DEFPPM) $(DEFPGM) $(DEFPBM) $(LIBPNM) \
			$(LIBPPM) $(LIBPGM) $(LIBPBM)
	$(CC) $(ALLCFLAGS) $(LDFLAGS) -o $@ $@.c -lm $(LIBPNM) $(LIBPPM) $(LIBPGM) $(LIBPBM)

# Rule for objects.
$(OBJECTS):	pnm.h $(DEFPPM) $(DEFPGM) $(DEFPBM)
	$(CC) $(ALLCFLAGS) "-Dmain=$*_main" -c $*.c

# And libraries.
$(LIBPBM):
	cd $(PBMDIR) ; make lib
$(LIBPGM) FOO:
	cd $(PGMDIR) ; make lib
$(LIBPPM) BAR:
	cd $(PPMDIR) ; make lib
lib:		$(LIBPNM)
$(LIBPNM):	libpnm1.o libpnm2.o libpnm3.o libpnm4.o
	-rm $(LIBPNM)
	ar rc $(LIBPNM) libpnm1.o libpnm2.o libpnm3.o libpnm4.o
	-ranlib $(LIBPNM)

libpnm1.o:	pnm.h $(DEFPPM) $(DEFPGM) $(DEFPBM) libpnm1.c
	$(CC) $(ALLCFLAGS) -c libpnm1.c
libpnm2.o:	pnm.h $(DEFPPM) $(DEFPGM) $(DEFPBM) libpnm2.c $(DEFLIBPPM) \
		$(DEFLIBPGM) $(DEFLIBPBM)
	$(CC) $(ALLCFLAGS) -c libpnm2.c
libpnm3.o:	pnm.h $(DEFPPM) $(DEFPGM) $(DEFPBM) libpnm3.c $(DEFLIBPPM) \
		$(DEFLIBPGM) $(DEFLIBPBM)
	$(CC) $(ALLCFLAGS) -c libpnm3.c
libpnm4.o:	pnm.h $(DEFPPM) $(DEFPGM) $(DEFPBM) rast.h libpnm4.c
	$(CC) $(ALLCFLAGS) -c libpnm4.c

# Other dependencies.
pnmarith pnmarith.o:		pnmarith.c
pnmcat pnmcat.o:		pnmcat.c
pnmconvol pnmconvol.o:		pnmconvol.c
pnmcrop pnmcrop.o:		pnmcrop.c
pnmcut pnmcut.o:		pnmcut.c
pnmdepth pnmdepth.o:		pnmdepth.c
pnmenlarge pnmenlarge.o:	pnmenlarge.c
pnmfile pnmfile.o:		pnmfile.c
pnmflip pnmflip.o:		pnmflip.c
pnmgamma pnmgamma.o:		pnmgamma.c
pnminvert pnminvert.o:		pnminvert.c
pnmnoraw pnmnoraw.o:		pnmnoraw.c
pnmpaste pnmpaste.o:		pnmpaste.c
pnmrotate pnmrotate.o:		pnmrotate.c
pnmscale pnmscale.o:		pnmscale.c
pnmshear pnmshear.o:		pnmshear.c
pnmtile pnmtile.o:		pnmtile.c
pnmtops pnmtops.o:		pnmtops.c
pnmtorast pnmtorast.o:		pnmtorast.c rast.h
pnmtotiff pnmtotiff.o:		pnmtotiff.c
pnmtoxwd pnmtoxwd.o:		pnmtoxwd.c x11wd.h
rasttopnm rasttopnm.o:		rasttopnm.c rast.h
tifftopnm tifftopnm.o:		tifftopnm.c
xwdtopnm xwdtopnm.o:		xwdtopnm.c x10wd.h x11wd.h

clean:
	-rm -f *.o *.a *.cat core $(BINARIES) pnmmerge
