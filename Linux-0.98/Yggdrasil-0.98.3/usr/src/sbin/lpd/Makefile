AS	=gas
LD	=gld
CC	=gcc
CFLAGS	=-Wall -O -fstrength-reduce 
CPP	=cpp  -I/usr/include
LPD = lpd.o pcap.o
LP = lp.o lpjob.o char2esc.o pcap.o lptext.o
LPJOB=lpjobtest.o char2esc.o pcap.o 
LPTEXT=lptexttest.o lpjob.o char2esc.o pcap.o strdup.o
TARGETS= lpd lp
.c.o:
	$(CC) $(CFLAGS) \
	 -c -o $*.o $<

all: $(TARGETS)

lpjobtest.o: lpjob.c
	$(CC) $(CFLAGS) -DTEST -c lpjob.c -o lpjobtest.o

lptexttest.o: lptext.c
	$(CC) $(CFLAGS) -DTEST -c lptext.c -o lptexttest.o
	
lpd: $(LPD)
	$(CC) $(LPD) -o lpd

objects: $(LPD)
	sync

lptext :$(LPTEXT)
	$(CC) $(LPTEXT) -o lptext
	
lpjob : $(LPJOB)
	$(CC) $(LPJOB) -o lpjob

lp: $(LP)
	$(CC) $(LP) -o lp

install: lp lpd
	mv -v lp  /usr/bin
	mv -v lpd /usr/bin
	( cd /usr/bin; chmod a+s lp lpd; )
	
clean:
	rm -f *.o tmp_make lpjob lptext

archive: clean
	( tar -T tar.list -czvf lpd.tar.Z)
	sync

depend:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.c;do echo -n "./";$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile

### Dependencies:
