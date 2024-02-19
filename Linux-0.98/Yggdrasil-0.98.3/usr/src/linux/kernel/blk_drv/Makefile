#
# Makefile for the kernel block device drivers.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now inherited from the
# parent makefile.
#

.c.s:
	$(CC) $(CFLAGS) $(RAMDISK) -S $<
.s.o:
	$(AS) -c -o $*.o $<
.c.o:
	$(CC) $(CFLAGS) $(RAMDISK) -c $<

SUBDIRS	= scsi

OBJS = hd.o ll_rw_blk.o floppy.o ramdisk.o genhd.o

all: blk_drv.a scsisubdirs

blk_drv.a: $(OBJS)
	rm -f blk_drv.a
	$(AR) rcs blk_drv.a $(OBJS)
	sync

scsisubdirs: dummy
	@for i in $(SUBDIRS); do (cd $$i && echo $$i && $(MAKE)) || exit; done

clean:
	rm -f core *.o *.a tmp_make
	for i in *.c;do rm -f `basename $$i .c`.s;done
	for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean); done

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	$(CPP) -M *.c >> tmp_make
	cp tmp_make Makefile
	for i in $(SUBDIRS); do (cd $$i && $(MAKE) dep); done

dummy:

### Dependencies:
floppy.o : floppy.c /usr/include/linux/sched.h /usr/include/linux/head.h /usr/include/linux/fs.h \
  /usr/include/linux/limits.h /usr/include/linux/wait.h /usr/include/linux/types.h \
  /usr/include/linux/dirent.h /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h \
  /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h \
  /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h \
  /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h \
  /usr/include/linux/kernel.h /usr/include/linux/signal.h /usr/include/linux/time.h \
  /usr/include/linux/param.h /usr/include/linux/resource.h /usr/include/linux/vm86.h \
  /usr/include/linux/timer.h /usr/include/linux/fdreg.h /usr/include/linux/fd.h \
  /usr/include/linux/errno.h /usr/include/asm/dma.h /usr/include/asm/io.h /usr/include/asm/system.h \
  /usr/include/asm/segment.h blk.h 
genhd.o : genhd.c /usr/include/linux/config.h /usr/include/linux/config.dist.h \
  /usr/include/linux/fs.h /usr/include/linux/limits.h /usr/include/linux/wait.h \
  /usr/include/linux/types.h /usr/include/linux/dirent.h /usr/include/linux/vfs.h \
  /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h \
  /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h \
  /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h \
  /usr/include/linux/genhd.h /usr/include/linux/kernel.h 
hd.o : hd.c /usr/include/linux/config.h /usr/include/linux/config.dist.h /usr/include/linux/errno.h \
  /usr/include/linux/signal.h /usr/include/linux/sched.h /usr/include/linux/head.h \
  /usr/include/linux/fs.h /usr/include/linux/limits.h /usr/include/linux/wait.h \
  /usr/include/linux/types.h /usr/include/linux/dirent.h /usr/include/linux/vfs.h \
  /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h \
  /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h \
  /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h \
  /usr/include/linux/mm.h /usr/include/linux/kernel.h /usr/include/linux/time.h \
  /usr/include/linux/param.h /usr/include/linux/resource.h /usr/include/linux/vm86.h \
  /usr/include/linux/timer.h /usr/include/linux/hdreg.h /usr/include/linux/genhd.h \
  /usr/include/asm/system.h /usr/include/asm/io.h /usr/include/asm/segment.h blk.h 
ll_rw_blk.o : ll_rw_blk.c /usr/include/linux/sched.h /usr/include/linux/head.h \
  /usr/include/linux/fs.h /usr/include/linux/limits.h /usr/include/linux/wait.h \
  /usr/include/linux/types.h /usr/include/linux/dirent.h /usr/include/linux/vfs.h \
  /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h \
  /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h \
  /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h \
  /usr/include/linux/mm.h /usr/include/linux/kernel.h /usr/include/linux/signal.h \
  /usr/include/linux/time.h /usr/include/linux/param.h /usr/include/linux/resource.h \
  /usr/include/linux/vm86.h /usr/include/linux/errno.h /usr/include/linux/string.h \
  /usr/include/linux/config.h /usr/include/linux/config.dist.h /usr/include/linux/locks.h \
  /usr/include/asm/system.h blk.h 
ramdisk.o : ramdisk.c /usr/include/linux/config.h /usr/include/linux/config.dist.h 
