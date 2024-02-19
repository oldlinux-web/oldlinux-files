#
# Makefile for the linux networking.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now in the main makefile...

# only these two lines should need to be changed to remove inet sockets.
# (and the tcp/tcpip.o in net.o)

SUBDIRS		= tcp
SOCK_FLAGS	= -DINET_SOCKETS

.c.o:
	$(CC) $(CFLAGS) $(SOCK_FLAGS) -c $<
.s.o:
	$(AS) -o $*.o $<
.c.s:
	$(CC) $(CFLAGS) -S $<

OBJS	=  socket.o unix.o 

net.o: $(OBJS) subdirs
	$(LD) -r -o net.o $(OBJS) tcp/tcpip.o


subdirs: dummy
	for i in $(SUBDIRS); do (cd $$i && echo $$i && $(MAKE)) || exit; done

clean:
	rm -f core *.o *.a tmp_make
	for i in *.c;do rm -f `basename $$i .c`.s;done
	@for i in $(SUBDIRS); do (cd $$i && echo $$i && $(MAKE) clean) || exit; done

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	$(CPP) -M *.c >> tmp_make
	cp tmp_make Makefile
	@for i in $(SUBDIRS); do (cd $$i && echo $$i && $(MAKE) dep) || exit; done

dummy:

### Dependencies:
socket.o : socket.c /usr/include/linux/signal.h /usr/include/linux/errno.h /usr/include/linux/sched.h \
  /usr/include/linux/head.h /usr/include/linux/fs.h /usr/include/linux/limits.h \
  /usr/include/linux/wait.h /usr/include/linux/types.h /usr/include/linux/dirent.h \
  /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h \
  /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h \
  /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h \
  /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h /usr/include/linux/kernel.h \
  /usr/include/linux/time.h /usr/include/linux/param.h /usr/include/linux/resource.h \
  /usr/include/linux/vm86.h /usr/include/linux/stat.h /usr/include/linux/socket.h \
  /usr/include/linux/fcntl.h /usr/include/linux/termios.h /usr/include/asm/system.h \
  /usr/include/asm/segment.h kern_sock.h socketcall.h 
unix.o : unix.c /usr/include/linux/signal.h /usr/include/linux/sched.h /usr/include/linux/head.h \
  /usr/include/linux/fs.h /usr/include/linux/limits.h /usr/include/linux/wait.h \
  /usr/include/linux/types.h /usr/include/linux/dirent.h /usr/include/linux/vfs.h \
  /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h \
  /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h \
  /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h \
  /usr/include/linux/mm.h /usr/include/linux/kernel.h /usr/include/linux/time.h \
  /usr/include/linux/param.h /usr/include/linux/resource.h /usr/include/linux/vm86.h \
  /usr/include/linux/errno.h /usr/include/linux/string.h /usr/include/linux/stat.h \
  /usr/include/linux/socket.h /usr/include/linux/un.h /usr/include/linux/fcntl.h \
  /usr/include/linux/termios.h /usr/include/asm/system.h /usr/include/asm/segment.h \
  kern_sock.h 
