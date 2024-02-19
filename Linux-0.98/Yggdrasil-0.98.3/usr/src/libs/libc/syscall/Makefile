#
# Makefile for system call functions for Linux
#
#
# Host root directory, maybe /usr for Linux
HOSTROOTDIR=/net/gecko/users/hlu/local
HOSTROOTDIR=/local
# Where we are now.
SRCDIR=$(HOME)/Linux/src/libc-linux
SRCDIR=/usr2/linux/src/libc-linux
# Host root directory, maybe /usr for Linux
TARGETROOTDIR=$(HOME)/Linux/usr
# Host bin direcory
HOSTBINDIR=$(HOSTROOTDIR)/bin
# For Linux, that should be /usr/bin
CROSSBINDIR=/usr2/linux/cross/bin
CROSSBINDIR=$(HOST_LIB_DIR)

HOST_MACHINE=decstation-3100
TARGET_MACHINE=i386-linux

VERSION=`awk '{printf "%s", substr ($$2, 1, length ($$2)); }' $(SRCDIR)/VERSION`

HOST_LIB_DIR =$(HOSTROOTDIR)/lib/gcc-lib/$(TARGET_MACHINE)/$(VERSION)
HOST_SHARED_DIR=$(HOST_LIB_DIR)/shared
TARGE_LIB_TDIR=$(TARGETROOTDIR)/lib/gcc-lib/$(TARGET_MACHINE)/$(VERSION)
TARGET_SHARED_DIR =$(TARGET_LIB_DIR)/shared

MV	=/bin/mv

AR      =$(CROSSBINDIR)/ar
AS      =$(CROSSBINDIR)/as
LD      =$(CROSSBINDIR)/ld
NM      =$(CROSSBINDIR)/nm
RANLIB  =$(CROSSBINDIR)/ranlib

RANLIB  =echo

LDFLAGS =-r -x

LIB=../libc.a

XCFLAGS = -D_GNU_SOURCE -DSTDC_HEADERS -DUSG -DDIRENT -DSYSV \
	-DUSE_BSD_REGEX
CC= $(HOSTBINDIR)/gcc -V $(VERSION) -b i386-linux
CFLAGS = -Wall -O2 -fomit-frame-pointer $(INC) $(XCFLAGS)
INC= -I../soft
CPP=$(CC) -E $(INC) $(XCFLAGS)
RM=/bin/rm

.SUFFIXES:
.SUFFIXES: .S .o .c .s .i

.s.o :
	$(CC) $(CFLAGS) -c $<
	$(LD) $(LDFLAGS) $*.o
	$(MV) a.out $*.o

.S.o :
	$(CC) $(CFLAGS) -c $<
	$(LD) $(LDFLAGS) $*.o
	$(MV) a.out $*.o

.c.o :
	$(CC) $(CFLAGS) -c $<
	$(LD) $(LDFLAGS) $*.o
	$(MV) a.out $*.o

.c.s :
	$(CC) $(CFLAGS) -S $<

.c.i :
	$(CC) $(CFLAGS) -E $< -o $*.i

OBJS = libc_exit.o access.o acct.o alarm.o brk.o chdir.o chmod.o \
	chown.o chroot.o close.o creat.o dup.o dup2.o \
	execve.o fcntl.o fork.o fstat.o getegid.o geteuid.o \
	getgid.o getpid.o getppid.o getuid.o getpgrp.o ioctl.o \
	kill.o link.o lseek.o mkdir.o mknod.o mount.o nice.o \
	open.o pause.o pipe.o read.o rmdir.o setgid.o setpgrp.o \
	setsid.o setuid.o signal.o stat.o stime.o sync.o time.o \
	times.o ulimit.o umask.o umount.o uname.o unlink.o ustat.o \
	utime.o write.o setreuid.o setregid.o sigsusp.o \
	sigpend.o sethostnam.o gethostnam.o setrlimit.o getrlimit.o \
	getrusage.o gettime.o settime.o getgroups.o setgroups.o \
	select.o symlink.o lstat.o readlink.o uselib.o \
	swapoff.o swapon.o reboot.o rename.o ptrace.o \
	fchmod.o fchown.o ftruncate.o truncate.o ioperm.o \
	mmap.o itimer.o fstatfs.o statfs.o setpriority.o iopl.o \
	wait.o wait3.o wait4.o waitpid.o vm86.o

library: $(OBJS)
	$(AR) uvc $(LIB) $(OBJS)
	sync

clean:
	$(RM) -f core *.o *.a tmp_make foo
	-for i in *.c;do \
	  if [ -f $$i ]; then \
	    $(RM) -f `basename $$i .c`.s; \
	  fi; \
	done

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.c;do echo -n `echo $$i | sed 's,\.c,\.s,'`" "; \
	$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile

### Dependencies:
