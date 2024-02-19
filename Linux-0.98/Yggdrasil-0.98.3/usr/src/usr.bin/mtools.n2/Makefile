#
#       Makefile for Mtools
#
# check the Configure file for some examples of device-specific setups
# Berkeley flavors of Unix should include -DBSD in the CFLAGS

CC	= cc
CFLAGS	= -O -DMERGED -DLINUX
LDFLAGS	= -s
LD	= cc
LINT	= lint
SHLIB	= 
SHAR	= shar -a
BINDIR	= /usr/bin
MANSECT	= 1
MANDIR	= /usr/man/man$(MANSECT)
#
# for AT&T Unix PC 7300/3b1 style shared libraries.
#SHOBJ	= /lib/crt0s.o /lib/shlib.ifile
#LD	= ld

LINKS=mattrib mcd mcopy mdel mdir mformat mlabel mmd mrd mread mren  mtype \
      mwrite
MTOOLS = mattrib.o mcd.o mcopy.o mdel.o mdir.o mformat.o mlabel.o mmd.o \
	 mrd.o mread.o mren.o mtype.o mwrite.o buf_read.o buf_write.o \
	 devices.o dir_make.o dir_read.o dir_write.o expand.o fat_free.o \
	 fat_read.o fat_write.o file_read.o file_write.o init.o is_dir.o \
	 match.o parse.o subdir.o mtools.o

MKMANIFEST = mkmanifest.o

all:	mtools

mtools:	$(MTOOLS)
	$(LD) $(LDFLAGS) $(MTOOLS) $(SHOBJ) -o mtools $(SHLIB)
	for name in $(LINKS); do if [ -x $$name ]; then rm $$name; ln mtools \
	  $$name; fi; done

mattrib: $(MATTRIB)
	$(LD) $(LDFLAGS) $(MATTRIB) $(SHOBJ) -o mattrib $(SHLIB)

mcd:	$(MCD)
	$(LD) $(LDFLAGS) $(MCD) $(SHOBJ) -o mcd $(SHLIB)

mcopy:	$(MCOPY)
	$(LD) $(LDFLAGS) $(MCOPY) $(SHOBJ) -o mcopy $(SHLIB)

mdel:	$(MDEL)
	$(LD) $(LDFLAGS) $(MDEL) $(SHOBJ) -o mdel $(SHLIB)

mdir:	$(MDIR)
	$(LD) $(LDFLAGS) $(MDIR) $(SHOBJ) -o mdir $(SHLIB)

mformat: $(MFORMAT)
	$(LD) $(LDFLAGS) $(MFORMAT) $(SHOBJ) -o mformat $(SHLIB)

mlabel:	$(MLABEL)
	$(LD) $(LDFLAGS) $(MLABEL) $(SHOBJ) -o mlabel $(SHLIB)

mmd:	$(MMD)
	$(LD) $(LDFLAGS) $(MMD) $(SHOBJ) -o mmd $(SHLIB)

mrd:	$(MRD)
	$(LD) $(LDFLAGS) $(MRD) $(SHOBJ) -o mrd $(SHLIB)

mread:	$(MREAD)
	$(LD) $(LDFLAGS) $(MREAD) $(SHOBJ) -o mread $(SHLIB)

mren:	$(MREN)
	$(LD) $(LDFLAGS) $(MREN) $(SHOBJ) -o mren $(SHLIB)

mtype:	$(MTYPE)
	$(LD) $(LDFLAGS) $(MTYPE) $(SHOBJ) -o mtype $(SHLIB)

mwrite:	$(MWRITE)
	$(LD) $(LDFLAGS) $(MWRITE) $(SHOBJ) -o mwrite $(SHLIB)

mkmanifest:	$(MKMANIFEST)
	$(LD) $(LDFLAGS) $(MKMANIFEST) $(SHOBJ) -o mkmanifest $(SHLIB)

clean:
	-rm *.o 2>/dev/null

spotless:	clean
	-rm mtools $(LINKS) 2>/dev/null

install:	mtools
	cp mtools $(BINDIR)
	cd $(BINDIR); for name in $(LINKS); do if [ -x $$name ]; then \
	  rm $$name; fi; ln mtools $$name; done
	cp *.1 $(MANDIR)

links:	mtools
	for name in $(LINKS); do if [ -x $$name ]; then rm $$name; fi; \
	  ln mtools $$name; done

buf_read.o:	msdos.h
buf_write.o:	msdos.h
devices.o:	msdos.h
dir_make.o:	msdos.h
dir_read.o:	msdos.h
dir_write.o:	msdos.h
fat_free.o:	msdos.h
fat_read.o:	msdos.h
fat_write.o:	msdos.h
file_read.o:	msdos.h
file_write.o:	msdos.h
init.o:		msdos.h
is_dir.o:	msdos.h
mattrib.o:	msdos.h patchlevel.h mattrib.c
		$(CC) -c $(CFLAGS) mattrib.c -Dmain=mattrib
mcd.o:		msdos.h patchlevel.h mcd.c
		$(CC) -c $(CFLAGS) mcd.c -Dmain=mcd
mcopy.o:	patchlevel.h mcopy.c
		$(CC) -c $(CFLAGS) mcopy.c -Dmain=mcopy
mdel.o:		msdos.h patchlevel.h mdel.c
		$(CC) -c $(CFLAGS) mdel.c -Dmain=mdel
mdir.o:		msdos.h patchlevel.h mdir.c
		$(CC) -c $(CFLAGS) mdir.c -Dmain=mdir
mformat.o:	msdos.h patchlevel.h mformat.c
		$(CC) -c $(CFLAGS) mformat.c -Dmain=mformat
mlabel.o:	msdos.h patchlevel.h mlabel.c
		$(CC) -c $(CFLAGS) mlabel.c -Dmain=mlabel
mmd.o:		msdos.h patchlevel.h mmd.c
		$(CC) -c $(CFLAGS) mmd.c -Dmain=mmd
mrd.o:		msdos.h patchlevel.h mrd.c
		$(CC) -c $(CFLAGS) mrd.c -Dmain=mrd
mread.o:	msdos.h patchlevel.h mread.c
		$(CC) -c $(CFLAGS) mread.c -Dmain=mread
mren.o:		msdos.h patchlevel.h mren.c
		$(CC) -c $(CFLAGS) mren.c -Dmain=mren
mtype.o:	msdos.h patchlevel.h mtype.c
		$(CC) -c $(CFLAGS) mtype.c -Dmain=mtype
mwrite.o:	msdos.h patchlevel.h mwrite.c
		$(CC) -c $(CFLAGS) mwrite.c -Dmain=mwrite
parse.o:	msdos.h
subdir.o:	msdos.h
