CFLAGS	= -F -D_MINIX -D_POSIX_SOURCE
DEOBJ   = de.s de_stdin.s de_stdout.s de_diskio.s de_recover.s

de:	$(DEOBJ)
	cc -i -T. $(DEOBJ) -o de
	chmem =30000 de
	echo "chmod  755  de; 	chown  bin  de; chgrp  bin   de"

$(DEOBJ): de.h


clean:	
	@rm -f *.bak *.s de
