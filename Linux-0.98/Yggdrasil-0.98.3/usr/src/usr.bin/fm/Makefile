# Makefile for fm, 5/5/92, Greg Lee

DESTDIR = /usr/bin
SOURCES = MANIFEST README README.tex fm.l fm.1 Makefile cmds.tex hyphenate.c \
	trie_link.h trie_char.h trie_op.h

#LIBS = -ltermcap
#CFLAGS = -O
# If the termcap output on SYSV doesn't work, you
#   might try adding -DSYSVR3 to CFLAGS
# Instead of termcap, on an ANSI color display, one can  use -DANSI
LIBS =
CFLAGS = -O -DANSI

LDFLAGS = -s

fm:	fm.o hyphenate.o
	$(CC) $(LDFLAGS) -o fm fm.o hyphenate.o $(LIBS)

hyphenate.o: hyphenate.c trie_link.h trie_char.h trie_op.h
	$(CC) $(CFLAGS) -c hyphenate.c

fm.o:	fm.c
	$(CC) $(CFLAGS) -c fm.c

fm.c:	fm.l
	@rm -f fm.c
	flex -t fm.l >fm.c

doc:	fm.1 fm README.tex
	@rm -f fm.doc README
	fm -jmo fm.1 >fm.doc
	fm -jx README.tex >README

shar:	$(SOURCES)
	@rm -f fm.shar
	makekit -m -nfm.shar.

tar:	$(SOURCES)
	@rm -f fm.tar fm.tar.Z
	tar cf fm.tar $(SOURCES)
	compress fm.tar

freeze:	$(SOURCES)
	@rm -f fm.tar fm.tar.F
	tar cf fm.tar $(SOURCES)
	freeze fm.tar

clean:
	rm -f *.o fm fm.c fm.doc

install:
	cp fm $(DESTDIR)
