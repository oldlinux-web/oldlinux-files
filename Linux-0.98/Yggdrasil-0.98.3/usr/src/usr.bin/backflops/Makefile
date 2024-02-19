# Linux floppy majors and minors:
# 2, 8:		1.2MB floppy in drive A:
# 2, 9:		1.2MB floppy in drive B:
# 2, 16:	760KB 3.5" in drive A:
# 2, 17:	760KB 3.5" in drive B:
# 2, 20:	360KB 5.25" in drive A:
# 2, 21:	360KB 5.25" in drive B:
# 2, 28:	1.44MB floppy in drive A:
# 2, 29:	1.44MB floppy in drive B:
# The full formula is in the Linux FAQ.

DEFAULTS = -DDEFDEVICE=\"/dev/PS0\" -DGNU_TAR=1
SYMLINKS =
# The binary I distribute is static, but there's no reason
# you shouldn't link it shared.
LDFLAGS  = -static

all:: backflops backupall

backflops: backflops.o
	$(CC) $(LDFLAGS) $< -o $@

backflops.o: backflops.c
	$(CC) -Wall -c $(CFLAGS) $(DEFAULTS) $*.c -o $@

install::
	install backflops /usr/local/bin
	( cd /usr/local/bin; \
	  rm -f writemulti readmulti restflops; \
	  ln $(SYMLINKS) backflops writemulti; \
	  ln $(SYMLINKS) backflops readmulti; \
	  ln $(SYMLINKS) backflops restflops; \
	)  
	install backupall /usr/local/bin
	@echo -n ""

clean::
	rm -f *.o backflops backupall
