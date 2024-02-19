AR	=ar
CPP	=gcc
CFLAGS	=-O6

.c.o:
	$(CPP) $(CFLAGS) -c -o $*.o $<

all:	libvga.a vgatest

libvga.a: vga.o
	$(AR) rcs libvga.a vga.o
	cp libvga.a /usr/lib
	cp vga.h /usr/include

vgatest: libvga.a vgatest.o 
	$(CPP) $(CFLAGS) -o vgatest vgatest.o -lvga

clean:
	rm -f *.o libvga.a vgatest

vga.o: vga.c vga.h
vgatest.o: vgatest.c vga.h
