# makefile for the dvi-to-(3812 || lj + || lj II || lj IIp ) filter.
#
CFLAGS=-O2 -Dunix -ansi -pedantic
BINDIR=/usr/TeX/bin
MANDIR=/usr/TeX/man
MANSEC=1
CC=gcc

OBJS=findlj.o find3812.o
HFILES=commands.h config.h
PROGRAMS=dvilj2p dvilj2 dvi3812 dvilj dviljp 

all: $(PROGRAMS) dvi2xx.txt

dvi3812: dvi2xx.c find3812.o $(HFILES)
	$(CC) ${CFLAGS} -DIBM3812 -o $@ dvi2xx.c find3812.o ${LFLAGS}

dvilj2p: dvi2xx.c findlj.o  $(HFILES)
	$(CC) ${CFLAGS} -DLJ2P -o $@ dvi2xx.c findlj.o  ${LFLAGS}

dvilj2: dvi2xx.c findlj.o  $(HFILES)
	$(CC) ${CFLAGS} -DLJ2 -o $@ dvi2xx.c findlj.o ${LFLAGS}

dvilj: dvi2xx.c findlj.o  $(HFILES)
	$(CC) ${CFLAGS} -DLJ -DSEVENBIT -o $@ dvi2xx.c findlj.o  ${LFLAGS}

dviljp: dvi2xx.c findlj.o  $(HFILES)
	$(CC) ${CFLAGS} -DLJ2P -DSEVENBIT -o $@ dvi2xx.c findlj.o  ${LFLAGS}

findlj.o: findfile.c config.h
	$(CC) ${CFLAGS} -DLJ -c findfile.c
	mv findfile.o $@

find3812.o: findfile.c config.h
	$(CC) ${CFLAGS} -DIBM3812 -c findfile.c
	mv findfile.o $@

clean:
	rm -f core $(OBJS) $(PROGRAMS)

DISTFILES = \
	./dvi2xx.c ./commands.h ./config.h ./findfile.c ./Makefile \
	./testpage.tex ./dvi2xx.history ./dvi2xx.man ./dvi2xx.txt ./graybox.sty \
	./dvi2xx.051 ./README ./sample2.tex ./2up.sty ./README.pcl


install:
	@for f in $(PROGRAMS); do \
		echo installing $$f in $(BINDIR); \
		install -c $$f $(BINDIR); \
	done
	cp dvi2xx.man dvi2xx.$(MANSEC)
	install dvi2xx.$(MANSEC) $(MANDIR)/man$(MANSEC)

dvi2xx.txt: dvi2xx.man
	-nroff -man dvi2xx.man | col -b > dvi2xx.txt






