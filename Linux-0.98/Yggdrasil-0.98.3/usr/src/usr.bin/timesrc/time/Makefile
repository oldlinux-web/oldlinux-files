#
# Makefile for some odd library functions
#

CC	= gcc
LDFLAGS	= -static

TZFILES = africa antarctica asia australasia europe northamerica \
	   southamerica pacificnew etcetera factory systemv
VPATH	= datfiles:.

all: zic zdump clock zones

tar:
	tar -cz -f timebin.tar.Z -X bin.excludes -C /usr doc/time.doc bin/date bin/clock lib/zoneinfo
	tar -cz -f timesrc.tar.Z -X src.excludes -C /usr/src time

zic: scheck.o ialloc.o emkdir.o zic.o
	$(CC) $(CFLAGS) -o zic -static zic.o scheck.o ialloc.o emkdir.o

# following will compile binary zone files with no leap seconds.
# This appears to be the POSIX way.  If you want leap seconds, use
#	zic -L leapseconds -d ./zoneinfo $(TZFILES)
zones:  zic $(TZFILES)
	(cd datfiles; ../zic -d ../zoneinfo $(TZFILES) )
	touch zones

# installation:
# clock should be setuid root
# zoneinfo should be /usr/lib/zoneinfo

clean:
	$(RM) -f core *.o *.a tmp_make zic zdump clock
	rm -rf zoneinfo zones
	for i in *.c;do $(RM) -f `basename $$i .c`.s;done

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.c;do echo -n `echo $$i | sed 's,\.c,\.s,'`" "; \
		$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile

### Dependencies:
