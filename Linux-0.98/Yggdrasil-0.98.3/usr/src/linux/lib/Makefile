#
# Makefile for some libs needed in the kernel.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#

.c.s:
	$(CC) $(CFLAGS) -S $<
.s.o:
	$(AS) -c -o $*.o $<
.c.o:
	$(CC) $(CFLAGS) -c $<

OBJS  = ctype.o _exit.o open.o close.o errno.o write.o dup.o setsid.o \
	execve.o wait.o string.o malloc.o

lib.a: $(OBJS)
	$(AR) rcs lib.a $(OBJS)
	sync

clean:
	rm -f core *.o *.a tmp_make
	for i in *.c;do rm -f `basename $$i .c`.s;done

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	$(CPP) -M *.c >> tmp_make
	cp tmp_make Makefile

### Dependencies:
_exit.o : _exit.c /usr/include/linux/unistd.h 
close.o : close.c /usr/include/linux/unistd.h 
ctype.o : ctype.c /usr/include/linux/ctype.h 
dup.o : dup.c /usr/include/linux/unistd.h 
errno.o : errno.c 
execve.o : execve.c /usr/include/linux/unistd.h 
malloc.o : malloc.c /usr/include/linux/kernel.h /usr/include/linux/mm.h /usr/include/linux/fs.h \
  /usr/include/linux/limits.h /usr/include/linux/wait.h /usr/include/linux/types.h \
  /usr/include/linux/dirent.h /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h \
  /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h \
  /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h \
  /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h /usr/include/linux/string.h \
  /usr/include/asm/system.h 
open.o : open.c /usr/include/linux/unistd.h /usr/lib/gcc-lib/i386-linux/2.2.2d/include/stdarg.h 
setsid.o : setsid.c /usr/include/linux/types.h /usr/include/linux/unistd.h 
string.o : string.c /usr/include/linux/types.h /usr/include/linux/string.h 
wait.o : wait.c /usr/include/linux/unistd.h /usr/include/linux/types.h 
write.o : write.c /usr/include/linux/unistd.h /usr/include/linux/types.h 
