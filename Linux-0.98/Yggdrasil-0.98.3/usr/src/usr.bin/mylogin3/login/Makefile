# Makefile for login program and other related utils.
# By poe@daimi.aau.dk
# All this code is freely distributable.

CC = gcc
CFLAGS = -O

loginobj = login.o utmp2.o putenv.o setenv.o
whoobj = who.o utmp2.o
encryptobj = encrypt.o
lastobj = last.o

all:	login who encrypt last

login:	$(loginobj)
	$(CC) -o login $(loginobj) libufc.a

who:	$(whoobj)
	$(CC) -o who $(whoobj)

encrypt:	$(encryptobj)
	$(CC) -o encrypt $(encryptobj) libufc.a

last:	$(lastobj)
	$(CC) -o last $(lastobj)

hostname:	hostname.o
	$(CC) -o hostname hostname.o

clean:
	rm -f *.o
