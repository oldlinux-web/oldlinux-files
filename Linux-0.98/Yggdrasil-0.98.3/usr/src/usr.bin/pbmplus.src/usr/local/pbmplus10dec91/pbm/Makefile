# Makefile for pbm tools.
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
CFLAGS =	-O
#CFLAGS =	-g
#CFLAGS =	-g -O -DLINUX -DSYSV
#RGBDEF =        -DRGB_DB=\"/usr/lib/X11/rgb\"
TIFFDEF =       -DLIBTIFF
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

SHELL =		/bin/sh
INCLUDE =	-I..
ALLCFLAGS =	$(CFLAGS) $(RGBDEF) $(TIFFDEF) $(INCLUDE)
LIBPBM =	libpbm.a
DEFPBM =	pbm.h ../pbmplus.h

BINARIES =	atktopbm brushtopbm cmuwmtopbm g3topbm \
		icontopbm gemtopbm macptopbm mgrtopbm \
		pbmlife pbmmake pbmmask pbmreduce \
		pbmtext pbmto10x pbmtoascii pbmtoatk \
		pbmtobbnbg pbmtocmuwm pbmtoepson pbmtog3 \
		pbmtogem pbmtogo pbmtoicon pbmtolj pbmtomacp \
		pbmtomgr pbmtopi3 pbmtoplot pbmtoptx \
		pbmtox10bm pbmtoxbm pbmtoybm pbmtozinc \
		pbmupc pi3topbm xbmtopbm ybmtopbm

OBJECTS =	atktopbm.o brushtopbm.o cmuwmtopbm.o g3topbm.o \
		icontopbm.o gemtopbm.o macptopbm.o mgrtopbm.o \
		pbmlife.o pbmmake.o pbmmask.o pbmreduce.o \
		pbmtext.o pbmto10x.o pbmtoascii.o pbmtoatk.o \
		pbmtobbnbg.o pbmtocmuwm.o pbmtoepson.o pbmtog3.o \
		pbmtogem.o pbmtogo.o pbmtoicon.o pbmtolj.o pbmtomacp.o \
		pbmtomgr.o pbmtopi3.o pbmtoplot.o pbmtoptx.o \
		pbmtox10bm.o pbmtoxbm.o pbmtoybm.o pbmtozinc.o \
		pbmupc.o pi3topbm.o xbmtopbm.o ybmtopbm.o

MANUALS1 =	$(BINARIES)
MANUALS3 =	libpbm
MANUALS5 =	pbm


#all:		binaries
all:		merge
#install:	install.bin
install:	install.merge


binaries:	$(BINARIES)

install.bin:	binaries
	cd $(INSTALLBINARIES) ; rm -f $(BINARIES)
	cp $(BINARIES) $(INSTALLBINARIES)


merge:		pbmmerge
pbmmerge:	pbmmerge.c $(OBJECTS) $(LIBPBM)
	$(CC) $(ALLCFLAGS) $(LDFLAGS) -o $@ $@.c $(OBJECTS) $(LIBPBM)

install.merge:	install.pbmmerge
install.pbmmerge:	pbmmerge
	cd $(INSTALLBINARIES) ; rm -f $(BINARIES)
	cp pbmmerge $(INSTALLBINARIES)
	cd $(INSTALLBINARIES) ; for i in $(BINARIES) ; do ln pbmmerge $$i ; done
	rm $(INSTALLBINARIES)/pbmmerge


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
$(BINARIES):	$(DEFPBM) $(LIBPBM)
	$(CC) $(ALLCFLAGS) $(LDFLAGS) -o $@ $@.c $(LIBPBM)
$(OBJECTS):	$(DEFPBM)
	$(CC) $(ALLCFLAGS) "-Dmain=$*_main" -c $*.c

# And library.
lib:		$(LIBPBM)
$(LIBPBM):	libpbm1.o libpbm2.o libpbm3.o libpbm4.o libpbm5.o
	-rm $(LIBPBM)
	ar rc $(LIBPBM) libpbm1.o libpbm2.o libpbm3.o libpbm4.o libpbm5.o
	-ranlib $(LIBPBM)

libpbm1.o:	$(DEFPBM) ../version.h libpbm.h libpbm1.c
	$(CC) $(ALLCFLAGS) -c libpbm1.c
libpbm2.o:	$(DEFPBM) libpbm.h libpbm2.c
	$(CC) $(ALLCFLAGS) -c libpbm2.c
libpbm3.o:	$(DEFPBM) libpbm.h libpbm3.c
	$(CC) $(ALLCFLAGS) -c libpbm3.c
libpbm4.o:	$(DEFPBM) libpbm.h libpbm4.c
	$(CC) $(ALLCFLAGS) -c libpbm4.c
libpbm5.o:	$(DEFPBM) pbmfont.h libpbm5.c
	$(CC) $(ALLCFLAGS) -c libpbm5.c

# Other dependencies.
atktopbm atktopbm.o:		atktopbm.c
brushtopbm brushtopbm.o:	brushtopbm.c
cmuwmtopbm cmuwmtopbm.o:	cmuwmtopbm.c cmuwm.h
g3topbm g3topbm.o:		g3topbm.c g3.h
icontopbm icontopbm.o:		icontopbm.c
gemtopbm gemtopbm.o:		gemtopbm.c
macptopbm macptopbm.o:		macptopbm.c macp.h
mgrtopbm mgrtopbm.o:		mgrtopbm.c mgr.h
pbmlife pbmlife.o:		pbmlife.c
pbmmake pbmmake.o:		pbmmake.c
pbmmask pbmmask.o:		pbmmask.c
pbmreduce pbmreduce.o:		pbmreduce.c
pbmtext pbmtext.o:		pbmtext.c pbmfont.h
pbmto10x pbmto10x.o:		pbmto10x.c
pbmtoascii pbmtoascii.o:	pbmtoascii.c
pbmtoatk pbmtoatk.o:		pbmtoatk.c
pbmtobbnbg pbmtobbnbg.o:	pbmtobbnbg.c
pbmtocmuwm pbmtocmuwm.o:	pbmtocmuwm.c cmuwm.h
pbmtoepson pbmtoepson.o:	pbmtoepson.c
pbmtog3 pbmtog3.o:		pbmtog3.c g3.h
pbmtogem pbmtogem.o:		pbmtogem.c
pbmtogo pbmtogo.o:		pbmtogo.c
pbmtoicon pbmtoicon.o:		pbmtoicon.c
pbmtolj pbmtolj.o:		pbmtolj.c
pbmtomacp pbmtomacp.o:		pbmtomacp.c macp.h
pbmtomgr pbmtomgr.o:		pbmtomgr.c mgr.h
pbmtopi3 pbmtopi3.o:		pbmtopi3.c
pbmtoplot pbmtoplot.o:		pbmtoplot.c
pbmtoptx pbmtoptx.o:		pbmtoptx.c
pbmtox10bm pbmtox10bm.o:	pbmtox10bm.c
pbmtoxbm pbmtoxbm.o:		pbmtoxbm.c
pbmtoybm pbmtoybm.o:		pbmtoybm.c
pbmtozinc pbmtozinc.o:		pbmtozinc.c
pbmupc pbmupc.o:		pbmupc.c
pi3topbm pi3topbm.o:		pi3topbm.c
xbmtopbm xbmtopbm.o:		xbmtopbm.c
ybmtopbm ybmtopbm.o:		ybmtopbm.c

clean:
	-rm -f *.o *.a *.cat core $(BINARIES) pbmmerge
