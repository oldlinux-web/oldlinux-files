#
# Makefile for the linux proc-filesystem routines.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...

.c.s:
	$(CC) $(CFLAGS) -S $<
.c.o:
	$(CC) $(CFLAGS) -c $<
.s.o:
	$(AS) -o $*.o $<

OBJS=	inode.o root.o base.o mem.o link.o fd.o

proc.o: $(OBJS)
	$(LD) -r -o proc.o $(OBJS)

clean:
	rm -f core *.o *.a tmp_make
	for i in *.c;do rm -f `basename $$i .c`.s;done

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	$(CPP) -M *.c >> tmp_make
	cp tmp_make Makefile

### Dependencies:
base.o : base.c /usr/include/asm/segment.h /usr/include/linux/errno.h /usr/include/linux/sched.h \
  /usr/include/linux/head.h /usr/include/linux/fs.h /usr/include/linux/limits.h \
  /usr/include/linux/wait.h /usr/include/linux/types.h /usr/include/linux/dirent.h \
  /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h \
  /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h \
  /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h \
  /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h /usr/include/linux/kernel.h \
  /usr/include/linux/signal.h /usr/include/linux/time.h /usr/include/linux/param.h \
  /usr/include/linux/resource.h /usr/include/linux/vm86.h /usr/include/linux/proc_fs.h \
  /usr/include/linux/stat.h 
fd.o : fd.c /usr/include/asm/segment.h /usr/include/linux/errno.h /usr/include/linux/sched.h \
  /usr/include/linux/head.h /usr/include/linux/fs.h /usr/include/linux/limits.h \
  /usr/include/linux/wait.h /usr/include/linux/types.h /usr/include/linux/dirent.h \
  /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h \
  /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h \
  /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h \
  /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h /usr/include/linux/kernel.h \
  /usr/include/linux/signal.h /usr/include/linux/time.h /usr/include/linux/param.h \
  /usr/include/linux/resource.h /usr/include/linux/vm86.h /usr/include/linux/proc_fs.h \
  /usr/include/linux/stat.h 
inode.o : inode.c /usr/include/linux/sched.h /usr/include/linux/head.h /usr/include/linux/fs.h \
  /usr/include/linux/limits.h /usr/include/linux/wait.h /usr/include/linux/types.h \
  /usr/include/linux/dirent.h /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h \
  /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h \
  /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h \
  /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h \
  /usr/include/linux/kernel.h /usr/include/linux/signal.h /usr/include/linux/time.h \
  /usr/include/linux/param.h /usr/include/linux/resource.h /usr/include/linux/vm86.h \
  /usr/include/linux/proc_fs.h /usr/include/linux/string.h /usr/include/linux/stat.h \
  /usr/include/linux/locks.h /usr/include/asm/system.h /usr/include/asm/segment.h 
link.o : link.c /usr/include/asm/segment.h /usr/include/linux/errno.h /usr/include/linux/sched.h \
  /usr/include/linux/head.h /usr/include/linux/fs.h /usr/include/linux/limits.h \
  /usr/include/linux/wait.h /usr/include/linux/types.h /usr/include/linux/dirent.h \
  /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h \
  /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h \
  /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h \
  /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h /usr/include/linux/kernel.h \
  /usr/include/linux/signal.h /usr/include/linux/time.h /usr/include/linux/param.h \
  /usr/include/linux/resource.h /usr/include/linux/vm86.h /usr/include/linux/minix_fs.h \
  /usr/include/linux/stat.h 
mem.o : mem.c /usr/include/linux/types.h /usr/include/linux/errno.h /usr/include/linux/sched.h \
  /usr/include/linux/head.h /usr/include/linux/fs.h /usr/include/linux/limits.h \
  /usr/include/linux/wait.h /usr/include/linux/dirent.h /usr/include/linux/vfs.h \
  /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h \
  /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h \
  /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h \
  /usr/include/linux/mm.h /usr/include/linux/kernel.h /usr/include/linux/signal.h \
  /usr/include/linux/time.h /usr/include/linux/param.h /usr/include/linux/resource.h \
  /usr/include/linux/vm86.h /usr/include/asm/segment.h /usr/include/asm/io.h 
root.o : root.c /usr/include/asm/segment.h /usr/include/linux/errno.h /usr/include/linux/sched.h \
  /usr/include/linux/head.h /usr/include/linux/fs.h /usr/include/linux/limits.h \
  /usr/include/linux/wait.h /usr/include/linux/types.h /usr/include/linux/dirent.h \
  /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h \
  /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h \
  /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h \
  /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h /usr/include/linux/kernel.h \
  /usr/include/linux/signal.h /usr/include/linux/time.h /usr/include/linux/param.h \
  /usr/include/linux/resource.h /usr/include/linux/vm86.h /usr/include/linux/proc_fs.h \
  /usr/include/linux/stat.h 
