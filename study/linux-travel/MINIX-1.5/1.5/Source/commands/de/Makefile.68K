CFLAGS	= -O -DATARI_ST
DEOBJ   = de.o de_stdin.o de_stdout.o de_diskio.o de_recover.o

de:	$(DEOBJ)
	cc $(DEOBJ) -o de
	chmem =20000 de
	chmod  4755  de
	chown  root  de
	chgrp  bin   de

$(DEOBJ): de.h
