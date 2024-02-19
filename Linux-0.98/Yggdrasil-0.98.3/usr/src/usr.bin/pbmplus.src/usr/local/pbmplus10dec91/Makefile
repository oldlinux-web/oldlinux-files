# Makefile for pbmplus tools.
#
# Copyright (C) 1989, 1991 by Jef Poskanzer.
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted, provided
# that the above copyright notice appear in all copies and that both that
# copyright notice and this permission notice appear in supporting
# documentation.  This software is provided "as is" without express or
# implied warranty.

# CONFIGURE: gcc makes things go faster on some machines, but not everyone
# has it.  Warning: do not use gcc's -finline-functions or -fstrength-reduce
# flags, they can produce incorrect code.  (This is with gcc versions 1.35,
# 1.36, and 1.37, later versions may fix these bugs.)  Also, on some systems
# gcc can't compile pnmconvol - dunno why.  And on some systems you can't
# use the -ansi flag, it gives compilation errors in <math.h>.
#CC =		cc
CC =		gcc
#CC =		gcc -fcombine-regs -fpcc-struct-return
#CC =		gcc -ansi -pedantic -fcombine-regs -fpcc-struct-return

# CONFIGURE: cc flags go here.
CFLAGS =	-O -DSYSV -DLINUX
#CFLAGS =	-g
#CFLAGS =	-g -O

# CONFIGURE: ld flags go here.  Eunice users may want to use -noshare so that
# the binaries can run standalone.
#LDFLAGS =	-s
LDFLAGS =	
#LDFLAGS =	-noshare

# CONFIGURE: If you have an X11-style rgb color names file, define its
# path here.  This is used by PPM to parse color names into rgb values.
# If you don't have such a file, comment this out and use the alternative
# hex and decimal forms to specify colors (see ppm/pgmtoppm.1 for details).
#RGBDEF =	-DRGB_DB=\"/usr/lib/X11/rgb\"

# CONFIGURE: PBMPLUS's support for TIFF files depends on the library from
# Sam Leffler's TIFF Software package - see the OTHER.SYSTEMS file for a
# full description and access information.  To configure PBMPLUS to use the
# library: first, if necessary, fetch the TIFF Software, unpack it in a
# scratch directory somewhere, and move the libtiff subdirectory right here
# into the PBMPLUS top-level directory.  Configure and "make" in the
# libtiff directory.  Yes, you do have to do the TIFF make by hand, the
# general PBMPLUS make will *not* make libtiff.  Finally, uncomment the
# following five definitions.
#
# Libtiff is pretty good about portability, but there are some machines
# it has problems on.  If you run into problems, you may wish to contact
# Sam directly, at the address listed in the OTHER.SYSTEMS file.
#
# By the way, you must have at least version 2.4 of libtiff.  Earlier
# versions will not work.
TIFFDEF =	-DLIBTIFF
TIFFINC =	-I../libtiff
TIFFLIB =	../libtiff/libtiff.a
TIFFBINARIES =	tifftopnm pnmtotiff
TIFFOBJECTS =	tifftopnm.o pnmtotiff.o

# CONFIGURE: Define the directory that you want the binaries copied to.
# If you need scripts and binaries to be in different directories, you
# can set that up too.
INSTALLBINARIES =	/usr/local/pbmplus
INSTALLSCRIPTS =	$(INSTALLBINARIES)

# CONFIGURE: Define the directories that you want the manual sources copied to,
# plus the suffix you want them to have.
INSTALLMANUALS1 =	/usr/man/man1
SUFFIXMANUALS1 =	1
INSTALLMANUALS3 =	/usr/man/man3
SUFFIXMANUALS3 =	3
INSTALLMANUALS5 =	/usr/man/man5
SUFFIXMANUALS5 =	5

# CONFIGURE: Normally the man pages are installed using "cp".  By changing
# this define you can use something else, for example a script that calls
# compress or pack.
MANCP =			cp 

# CONFIGURE: Normally the Makefiles build and install separate binaries for
# each program.  However, on some systems (especially those without shared
# libraries) this can mean a lot of space.  In this case you might try
# building a "merge" instead.  The idea here is to link all the binaries
# together into one huge executable, with a tiny dispatch program as the
# main.  Then the merged binary is installed with file-system links for
# each program it includes.  The dispatch routine can tell which program
# to run by looking at argv[0].  On a Sun3 under SunOS 3.5 the space for
# executables went from 2.9 meg to .36 meg.
#
# Note that if you make a "merge", the executables don't get created
# until you do the install.
all:		binaries
install:	install.bin install.man
#all:		merge
#install:	install.merge install.man

# End of configurable definitions.

SHELL =		/bin/sh
MAKE =		make
SUBDIRS =	pbm pgm ppm pnm

binaries:
	for i in $(SUBDIRS) ; do \
	    ( echo $$i ; cd $$i ; $(MAKE) $(MFLAGS) 'CC=$(CC)' 'CFLAGS=$(CFLAGS)' 'RGBDEF=$(RGBDEF)' 'TIFFDEF=$(TIFFDEF)' 'TIFFINC=$(TIFFINC)' 'TIFFLIB=$(TIFFLIB)' 'TIFFBINARIES=$(TIFFBINARIES)' 'TIFFOBJECTS=$(TIFFOBJECTS)' 'LDFLAGS=$(LDFLAGS)' binaries ); \
	done

merge:
	for i in $(SUBDIRS) ; do \
	    ( echo $$i ; cd $$i ; $(MAKE) $(MFLAGS) 'CC=$(CC)' 'CFLAGS=$(CFLAGS)' 'RGBDEF=$(RGBDEF)' 'TIFFDEF=$(TIFFDEF)' 'TIFFINC=$(TIFFINC)' 'TIFFLIB=$(TIFFLIB)' 'TIFFBINARIES=$(TIFFBINARIES)' 'TIFFOBJECTS=$(TIFFOBJECTS)' 'LDFLAGS=$(LDFLAGS)' merge ); \
	done

install.bin:
	if ! test -d /usr/local ; then mkdir /usr/local ; fi
	if ! test -d /usr/local/pbmplus ; then mkdir /usr/local/pbmplus ; fi
	if ! test -d /usr/man ; then mkdir /usr/man ; fi
	if ! test -d /usr/man/man1 ; then mkdir /usr/man/man1 ; fi
	if ! test -d /usr/man/man3 ; then mkdir /usr/man/man3 ; fi
	if ! test -d /usr/man/man5 ; then mkdir /usr/man/man5 ; fi
	for i in $(SUBDIRS) ; do \
	    ( echo $$i ; cd $$i ; $(MAKE) $(MFLAGS) 'CC=$(CC)' 'CFLAGS=$(CFLAGS)' 'RGBDEF=$(RGBDEF)' 'TIFFDEF=$(TIFFDEF)' 'TIFFINC=$(TIFFINC)' 'TIFFLIB=$(TIFFLIB)' 'TIFFBINARIES=$(TIFFBINARIES)' 'TIFFOBJECTS=$(TIFFOBJECTS)' 'LDFLAGS=$(LDFLAGS)' 'INSTALLBINARIES=$(INSTALLBINARIES)' 'INSTALLSCRIPTS=$(INSTALLSCRIPTS)' install.bin ); \
	done

install.merge:
	for i in $(SUBDIRS) ; do \
	    ( echo $$i ; cd $$i ; $(MAKE) $(MFLAGS) 'CC=$(CC)' 'CFLAGS=$(CFLAGS)' 'RGBDEF=$(RGBDEF)' 'TIFFDEF=$(TIFFDEF)' 'TIFFINC=$(TIFFINC)' 'TIFFLIB=$(TIFFLIB)' 'TIFFBINARIES=$(TIFFBINARIES)' 'TIFFOBJECTS=$(TIFFOBJECTS)' 'LDFLAGS=$(LDFLAGS)' 'INSTALLBINARIES=$(INSTALLBINARIES)' 'INSTALLSCRIPTS=$(INSTALLSCRIPTS)' install.merge ); \
	done

install.man:
	for i in $(SUBDIRS) ; do \
	    ( echo $$i ; cd $$i ; $(MAKE) $(MFLAGS) 'TIFFBINARIES=$(TIFFBINARIES)' 'INSTALLMANUALS1=$(INSTALLMANUALS1)' 'SUFFIXMANUALS1=$(SUFFIXMANUALS1)' 'INSTALLMANUALS3=$(INSTALLMANUALS3)' 'SUFFIXMANUALS3=$(SUFFIXMANUALS3)' 'INSTALLMANUALS5=$(INSTALLMANUALS5)' 'SUFFIXMANUALS5=$(SUFFIXMANUALS5)' 'MANCP=$(MANCP)' install.man ); \
	done

clean:
	-rm -f *.shar *.shar? art.*
	for i in $(SUBDIRS) ; do \
	    ( echo $$i ; cd $$i ; $(MAKE) $(MFLAGS) clean ); \
	done
