# Makefile for ps, w, free, top and tload
# for Linux version 0.97 patch 2
#

CFLAGS= -O

# uncomment next line if you are using a compiler older than 2.2.2d
#CFLAGS= -O -Iinclude -I/usr/src/linux/include -Dhack

#LDFLAGS= -s
#LDFLAGS= -static

# BINDIR: if you type "make install" the programs will be copied to this
# directory. This will also set the right mode and owner of these files
# and the devices /dev/kmem and /dev/swap.
BINDIR= /bin

CC= gcc

TLIB= -ltermcap

PROG= ps free w tload top

W_OBJ=	w.o attime.o no_update.o cmdline.o w_linux.o
PS_OBJ=	ps.o psdata.o update_db.o cmdline.o devname.o pwcache.o
TOP_OBJ= top.o no_update.o cmdline.o pwcache.o setpriority.o devname.o


all: $(PROG)


ps: $(PS_OBJ)
	$(CC) -o $@ $(PS_OBJ) $(LDFLAGS)

memmap: memmap.o no_update.o
	$(CC) -o $@ memmap.o no_update.o $(LDFLAGS)

free: free.o no_update.o
	$(CC) -o $@ free.o no_update.o $(LDFLAGS)

tload: tload.o no_update.o
	$(CC) -o $@ tload.o no_update.o $(LDFLAGS)

w: $(W_OBJ)
	$(CC) -o $@ $(W_OBJ) $(LDFLAGS)

top: $(TOP_OBJ)
	$(CC) -o $@ $(TOP_OBJ) $(TLIB) $(LDFLAGS)


tload.o: tload.c
	$(CC) -o $@ $(OPT) -c tload.c

no_update.o: psdata.c
	$(CC) -o $@ $(CFLAGS) -DNO_UPDATE -c psdata.c


install: $(PROG)
	-chown root.kmem /dev/kmem || chown root.mem /dev/kmem
	-chown root.kmem /dev/swap || chown root.mem /dev/swap
	-chmod 640 /dev/kmem /dev/swap
	-chown root.kmem $(PROG) || chown root.mem $(PROG)
	chmod 2755 $(PROG)
	cp -p $(PROG) $(BINDIR)
	rm -f $(BINDIR)/uptime
	ln $(BINDIR)/w $(BINDIR)/uptime

clean:
	rm -f *.o *.a .depend core a.out $(PROG)

.depend:
	$(CC) -M $(CFLAGS) *.c >.depend

.SUFFIXES: .i

.c.i:
	$(CC) -E $(CFLAGS) -o $@ $<

include .depend
