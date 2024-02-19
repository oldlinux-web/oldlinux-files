# Makefile for LHarc UNIX Archiver Driver
#		Copyright(C) MCMLXXXIX  Yooichi.Tagawa
# V0.01  Alpha Version				1989.05.28  Y.Tagawa
# V0.02  Alpha Version R2			1989.05.29  Y.Tagawa
# V0.03  Release #3  Beta Version		1989.07.02  Y.Tagawa
# V0.03b Modified				1989.07.13  Y.Tagawa
# V1.00  Fixed					1989.09.19  Y.Tagawa
#
# for OSK
# V1.08						1990.10.09  Sakura Tomozou
# V2.00	+ lzhuf4/5				1990.10.26  
#
# LHa for UNIX
# V0.01  Alpha Version				1991.10.17  Masaru Oki
# V0.02  Alpha Version Rel.2			1991.11.26  Masaru Oki
# V0.03  Beta Version				1991.12.03  Masaru Oki
# V0.04  Beta Version  Rel.2			1992.01.17  Masaru Oki
# V0.05  Beta Version  Final			1992.02.17  Masaru Oki

LHOBJS		= lharc$(O) lhadd$(O) lhlist$(O) lhext$(O) header$(O)
HUFOBJS		= append$(O) crcio$(O) dhuf$(O) extract$(O) huf$(O) \
		  larc$(O) maketbl$(O) maketree$(O) shuf$(O) slide$(O) \
		  util$(O)

OBJS		= $(LHOBJS) $(HUFOBJS) $(LIBRARYOBJS)

CFLAGS		= $(OPTIMIZE) $(SWITCHES)

all : lha

lha	: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(LHOBJS) append$(O) slide$(O) extract$(O) : lharc.h

$(HUFOBJS) : slidehuf.h
crcio$(O) extract$(O) slide$(O) : intrface.h

lhdir$(O) : lhdir.h

$(OBJS) : ../Makefile

clean:
	rm -f core lha $(OBJS)

moreclean:
	rm -f core *.o lha.tar lha.tar.Z *~ \#* *.orig

install:
	$(INSTALL) $(INSTALLBIN) lha $(BINDIR)/lha
