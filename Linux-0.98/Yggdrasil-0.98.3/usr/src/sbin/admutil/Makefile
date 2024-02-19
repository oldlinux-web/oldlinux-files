# Makefile for admutils

CC=	gcc
CFLAGS=	-O2 -I. -s
USRBIN=	last chsh passwd newgrp shutdown su ctrlaltdel
PRGS=	${USRBIN} init
OBJS=	last.o chsh.o passwd.o newgrp.o shutdown.o init.o su.o ctrlaltdel.o
LIBS=

all:	${PRGS}

clean:	
	rm -f ${OBJS} ${PRGS} core

${USRBIN}: ${LIBS} ${OBJS}
	${CC} ${CFLAGS} -o $@ $@.o ${LIBS}

init:	init.o
	${CC} -static -s -o init $@.o

install: ${USRBIN}
	cp ${USRBIN} /usr/bin
	(cd /usr/bin; \
	  chown root.wheel passwd newgrp chsh su; \
	  chmod a=x,u+s passwd newgrp chsh su)
	-ln -s /usr/bin/shutdown /usr/bin/reboot
	-ln -s /usr/bin/shutdown /usr/bin/halt
	cp shells /etc

install_init: init
	@echo Now be damn sure you know what you are doing!
	@echo Edit sysvinittab, patch the kernel to use /etc/init
	@echo instead of /bin/init, copy sysvinittab to /etc together with
	@echo init. Try it out by hand before you reboot!

