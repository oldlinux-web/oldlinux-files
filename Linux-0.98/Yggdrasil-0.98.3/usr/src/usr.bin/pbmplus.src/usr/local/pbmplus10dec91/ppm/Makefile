# Makefile for ppm tools.
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
CFLAGS =	-O -DLINUX
#CFLAGS =	-g
#CFLAGS =	-g -O
#RGBDEF =	-DRGB_DB=\"/usr/lib/X11/rgb\"
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
INCLUDE =	-I.. $(INCLUDEPGM) $(INCLUDEPBM)
ALLCFLAGS =	$(CFLAGS) $(RGBDEF) $(INCLUDE)
LIBPPM =	libppm.a

PORTBINARIES =	giftoppm gouldtoppm ilbmtoppm imgtoppm mtvtoppm \
		pcxtoppm pgmtoppm pi1toppm picttoppm \
		pjtoppm ppmdither ppmhist ppmmake \
		ppmquant ppmrelief ppmtoacad ppmtogif ppmtoicr \
		ppmtoilbm ppmtopcx ppmtopgm ppmtopi1 ppmtopict \
		ppmtopj ppmtopuzz ppmtorgb3 ppmtosixel \
		ppmtotga ppmtouil ppmtoxpm ppmtoyuv qrttoppm \
		rawtoppm rgb3toppm sldtoppm spctoppm sputoppm \
		tgatoppm ximtoppm xpmtoppm yuvtoppm
MATHBINARIES =	ppmforge ppmpat
BINARIES =	$(PORTBINARIES) $(MATHBINARIES)
SCRIPTS =	ppmquantall

OBJECTS =	giftoppm.o gouldtoppm.o ilbmtoppm.o imgtoppm.o mtvtoppm.o \
		pcxtoppm.o pgmtoppm.o pi1toppm.o picttoppm.o \
		pjtoppm.o ppmdither.o ppmhist.o ppmmake.o \
		ppmquant.o ppmrelief.o ppmtoacad.o ppmtogif.o ppmtoicr.o \
		ppmtoilbm.o ppmtopcx.o ppmtopgm.o ppmtopi1.o ppmtopict.o \
		ppmtopj.o ppmtopuzz.o ppmtorgb3.o ppmtosixel.o \
		ppmtotga.o ppmtouil.o ppmtoxpm.o ppmtoyuv.o qrttoppm.o \
		rawtoppm.o rgb3toppm.o sldtoppm.o spctoppm.o sputoppm.o \
		tgatoppm.o ximtoppm.o xpmtoppm.o yuvtoppm.o \
		ppmforge.o ppmpat.o

MANUALS1 =	$(BINARIES) $(SCRIPTS)
MANUALS3 =	libppm
MANUALS5 =	ppm


#all:		binaries
all:		merge
#install:	install.bin
install:	install.merge


binaries:	$(BINARIES)

install.bin:	binaries $(SCRIPTS)
	cd $(INSTALLBINARIES) ; rm -f $(BINARIES)
	cp $(BINARIES) $(INSTALLBINARIES)
	cd $(INSTALLSCRIPTS) ; rm -f $(SCRIPTS)
	cp $(SCRIPTS) $(INSTALLSCRIPTS)
	cd $(INSTALLSCRIPTS) ; chmod +x $(SCRIPTS)


merge:		ppmmerge
ppmmerge:	ppmmerge.c $(OBJECTS) $(LIBPPM) $(LIBPGM) $(LIBPBM)
	$(CC) $(ALLCFLAGS) $(LDFLAGS) -o $@ $@.c $(OBJECTS) -lm $(LIBPPM) $(LIBPGM) $(LIBPBM)

install.merge:	install.ppmmerge $(SCRIPTS)
install.ppmmerge:	ppmmerge
	cd $(INSTALLBINARIES) ; rm -f $(BINARIES)
	cp ppmmerge $(INSTALLBINARIES)
	cd $(INSTALLBINARIES) ; for i in $(BINARIES) ; do ln ppmmerge $$i ; done
	rm $(INSTALLBINARIES)/ppmmerge
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


# Rule for plain programs.
$(PORTBINARIES):	ppm.h $(DEFPGM) $(DEFPBM) $(LIBPPM) $(LIBPGM) $(LIBPBM)
	$(CC) $(ALLCFLAGS) $(LDFLAGS) -o $@ $@.c $(LIBPPM) $(LIBPGM) $(LIBPBM)

# Rule for math-dependent programs.
$(MATHBINARIES):	ppm.h $(DEFPGM) $(DEFPBM) $(LIBPPM) $(LIBPGM) $(LIBPBM)
	$(CC) $(ALLCFLAGS) $(LDFLAGS) -o $@ $@.c -lm $(LIBPPM) $(LIBPGM) $(LIBPBM)

# Rule for objects.
$(OBJECTS):	ppm.h $(DEFPGM) $(DEFPBM)
	$(CC) $(ALLCFLAGS) "-Dmain=$*_main" -c $*.c

# And libraries.
$(LIBPBM):
	cd $(PBMDIR) ; make lib
$(LIBPGM):
	cd $(PGMDIR) ; make lib
lib:		$(LIBPPM)
$(LIBPPM):	libppm1.o libppm2.o libppm3.o libppm4.o libppm5.o
	-rm $(LIBPPM)
	ar rc $(LIBPPM) libppm1.o libppm2.o libppm3.o libppm4.o libppm5.o
	-ranlib $(LIBPPM)

libppm1.o:	ppm.h $(DEFPGM) $(DEFPBM) libppm.h libppm1.c
	$(CC) $(ALLCFLAGS) -c libppm1.c
libppm2.o:	ppm.h $(DEFPGM) $(DEFPBM) libppm.h libppm2.c $(DEFLIBPGM) \
		$(DEFLIBPBM)
	$(CC) $(ALLCFLAGS) -c libppm2.c
libppm3.o:	ppm.h $(DEFPGM) $(DEFPBM) ppmcmap.h libppm.h libppm3.c
	$(CC) $(ALLCFLAGS) -c libppm3.c
libppm4.o:	ppm.h $(DEFPGM) $(DEFPBM) libppm4.c
	$(CC) $(ALLCFLAGS) -c libppm4.c
libppm5.o:	ppm.h $(DEFPGM) $(DEFPBM) ppmdraw.h libppm5.c
	$(CC) $(ALLCFLAGS) -c libppm5.c

# Other dependencies.
giftoppm giftoppm.o:		giftoppm.c
gouldtoppm gouldtoppm.o:	gouldtoppm.c
ilbmtoppm ilbmtoppm.o:		ilbmtoppm.c ilbm.h
imgtoppm imgtoppm.o:		imgtoppm.c
mtvtoppm mtvtoppm.o:		mtvtoppm.c
pcxtoppm pcxtoppm.o:		pcxtoppm.c
pgmtoppm pgmtoppm.o:		pgmtoppm.c
pi1toppm pi1toppm.o:		pi1toppm.c
picttoppm picttoppm.o:		picttoppm.c
pjtoppm pjtoppm.o:		pjtoppm.c
ppmdither ppmdither.o:		ppmdither.c
ppmforge ppmforge.o:		ppmforge.c
ppmhist ppmhist.o:		ppmhist.c ppmcmap.h
ppmmake ppmmake.o:		ppmmake.c
ppmpat ppmpat.o:		ppmpat.c ppmdraw.h
ppmquant ppmquant.o:		ppmquant.c $(PGMDIR)/dithers.h ppmcmap.h
ppmrelief ppmrelief.o:		ppmrelief.c
ppmtoacad ppmtoacad.o:		ppmtoacad.c ppmcmap.h autocad.h
ppmtogif ppmtogif.o:		ppmtogif.c ppmcmap.h
ppmtoicr ppmtoicr.o:		ppmtoicr.c ppmcmap.h
ppmtoilbm ppmtoilbm.o:		ppmtoilbm.c ilbm.h ppmcmap.h
ppmtopcx ppmtopcx.o:		ppmtopcx.c ppmcmap.h
ppmtopgm ppmtopgm.o:		ppmtopgm.c
ppmtopi1 ppmtopi1.o:		ppmtopi1.c ppmcmap.h
ppmtopict ppmtopict.o:		ppmtopict.c ppmcmap.h
ppmtopj ppmtopj.o:		ppmtopj.c
ppmtopuzz ppmtopuzz.o:		ppmtopuzz.c ppmcmap.h
ppmtorgb3 ppmtorgb3.o:		ppmtorgb3.c
ppmtosixel ppmtosixel.o:	ppmtosixel.c ppmcmap.h
ppmtotga ppmtotga.o:		ppmtotga.c ppmcmap.h
ppmtouil ppmtouil.o:		ppmtouil.c ppmcmap.h
ppmtoxpm ppmtoxpm.o:		ppmtoxpm.c ppmcmap.h
ppmtoyuv ppmtoyuv.o:		ppmtoyuv.c
qrttoppm qrttoppm.o:		qrttoppm.c
rawtoppm rawtoppm.o:		rawtoppm.c
rgb3toppm rgb3toppm.o:		rgb3toppm.c
sldtoppm sldtoppm.o:		sldtoppm.c ppmdraw.h autocad.h
spctoppm spctoppm.o:		spctoppm.c
sputoppm sputoppm.o:		sputoppm.c
tgatoppm tgatoppm.o:		tgatoppm.c tga.h
ximtoppm ximtoppm.o:		ximtoppm.c xim.h
xpmtoppm xpmtoppm.o:		xpmtoppm.c
yuvtoppm yuvtoppm.o:		yuvtoppm.c

clean:
	-rm -f *.o *.a *.cat core $(BINARIES) ppmmerge
