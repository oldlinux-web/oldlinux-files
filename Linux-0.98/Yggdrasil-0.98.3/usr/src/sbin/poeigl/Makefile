# Makefile for login program and other related utils.
# By poe@daimi.aau.dk
# All this code is freely distributable.

CC = cc
CFLAGS = -O3 -I.

loginobj = login.o vhangup.o getpass.o
whoobj = who.o
encryptobj = encrypt.o
gettyobj = agetty.o
initobj = simpleinit.o
writeobj = write.o

.SUFFIXES: .c.o
.c.o:
	${CC} ${CFLAGS} -c $*.c

all:	login who getty hostname init write

install:	all
	# strip login who getty hostname init write
	cp init getty /etc
	chmod 544 /etc/init /etc/getty
	cp login /bin
	chmod u+s /bin/login
	cp who hostname write users mesg /usr/bin
	-ln -s /usr/bin/write /usr/bin/wall
	cp inittab securetty issue motd /etc
	touch /etc/utmp /etc/wtmp /etc/lastlog
	-ln -s /dev/tty1 /dev/console

init:	${initobj}
	${CC} -static -s -o init ${initobj}

getty:	${gettyobj}
	${CC} -static -s -o getty ${gettyobj}

login:	$(loginobj)
	$(CC) -static -s -o login $(loginobj)

who:	$(whoobj)
	$(CC) -s -o who $(whoobj)

hostname:	hostname.o
	$(CC) -s -o hostname hostname.o

write:	$(writeobj)
	$(CC) -s -o write $(writeobj)


clean:
	rm -f *.o

Clean:
	rm -f *.o login who getty hostname init libufc.a write
