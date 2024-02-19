SHELL	= /bin/sh
CFLAGS	=
CC = cc 

all: from to rsh sherver master
	
from: from.s
	$(CC) -o $@ from.s

to: to.s
	$(CC) -o $@ to.s

rsh: rsh.s prot.s
	$(CC) -o $@ rsh.s prot.s
	chmem =500 $@

sherver: sherver.s prot.s
	$(CC) -o $@ sherver.s prot.s
	chmem =500 $@

master: master.s
	$(CC) -o $@ master.s
	chmem =500 $@

sherver.s: sherver.h
rsh.s: sherver.h

clean:
	rm -f sherver.s from.s to.s master.s rsh.s prot.s
	rm -f sherver from to master rsh



