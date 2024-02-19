AS = as
LD = ld
CC = gcc
CFLAGS = -Wall -O2 -fstrength-reduce -ofomit-frame-pointer \
	-finline-functions -nostdinc -I../include

OBJS = vsprintf.o printk.o i8259.o system.o syscall.o traps.o sched.o fork.o 

all: kernel.o

kernel.o:	$(OBJS)
	$(LD) -r -o $@ $(OBJS)

vsprintf.o:	vsprintf.c
	$(CC) $(CFLAGS) -c $< -o $@

printk.o:	printk.c
	$(CC) $(CFLAGS) -c $< -o $@

i8259.o:	i8259.c
	$(CC) $(CFLAGS) -c $< -o $@

traps.o:	traps.c
	$(CC) $(CFLAGS) -c $< -o $@

system.o:	system.S
	$(CC) $(CFLAGS) -c $< -o $@

syscall.o:	syscall.c
	$(CC) $(CFLAGS) -c $< -o $@

sched.o:	sched.c
	$(CC) $(CFLAGS) -c $< -o $@

fork.o:	fork.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.s
	rm -f *~
	rm -f .*.swp
