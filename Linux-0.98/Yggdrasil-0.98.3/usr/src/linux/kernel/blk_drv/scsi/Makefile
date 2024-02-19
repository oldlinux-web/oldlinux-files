#
# Makefile for the linux kernel block device drivers.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#

#DEBUG = -DDEBUG=0xffffffff -DDEBUG_NO_CMD

.c.s:
	$(CC) $(CFLAGS) $(DEBUG) -S $<

.s.o:
	$(AS) -c -o $*.o $<

.c.o:
	$(CC) $(CFLAGS) $(DEBUG) -c $<


LOWLEVELCSRC = aha1542.c fdomain.c seagate.c ultrastor.c 7000fasst.c
LOWLEVELHSRC = aha1542.h fdomain.h seagate.h ultrastor.h 7000fasst.h
 
CSRC = hosts.c sd.c sd_ioctl.c st.c st_ioctl.c sr.c sr_ioctl.c scsi.c scsi_ioctl.c $(LOWLEVELCSRC)
HSRC = hosts.h sd.h st.h sr.h sr_ioctl.h scsi.h scsi_ioctl.h $(LOWLEVELHSRC)

OBJS =	scsi.o hosts.o scsi_ioctl.o sd.o sd_ioctl.o st.o st_ioctl.o \
	sr.o sr_ioctl.o \
	aha1542.o fdomain.o seagate.o ultrastor.o 7000fasst.o

all: scsi.a

figure : hosts.h $(KERNELHDRS)/linux/config.h hosts.c 
	$(HOSTCC) -N -DFIGURE_MAX_SCSI_HOSTS hosts.c -o figure

max_hosts.h : figure
	(echo "#ifndef _MAX_HOSTS_H"; \
	 echo "#define _MAX_HOSTS_H"; \
	 echo "#define MAX_SCSI_HOSTS `./figure`";\
	 echo "#endif") > tmp_max
	cp tmp_max max_hosts.h

scsi.a: $(OBJS)
	$(AR) rcs scsi.a $(OBJS)
	sync

clean:
	rm -f core *.o *.a tmp_make tmp_max figure max_hosts.h

seagate.o: seagate.c
	$(CC) -Wall -c seagate.c 

dep:
	touch max_hosts.h
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	$(CPP) -M *.c >> tmp_make
	rm max_hosts.h
	cp tmp_make Makefile

### Dependencies:
7000fasst.o : 7000fasst.c /usr/include/linux/config.h /usr/include/linux/config.dist.h \
  /usr/include/linux/kernel.h /usr/include/linux/head.h /usr/include/linux/types.h \
  /usr/include/linux/string.h /usr/include/asm/system.h /usr/include/asm/io.h \
  scsi.h hosts.h max_hosts.h 7000fasst.h 
aha1542.o : aha1542.c /usr/include/linux/config.h /usr/include/linux/config.dist.h \
  /usr/include/linux/kernel.h /usr/include/linux/head.h /usr/include/linux/types.h \
  /usr/include/linux/string.h /usr/include/asm/system.h /usr/include/asm/io.h \
  scsi.h hosts.h max_hosts.h aha1542.h 
fdomain.o : fdomain.c /usr/include/linux/config.h /usr/include/linux/config.dist.h \
  /usr/include/linux/sched.h /usr/include/linux/head.h /usr/include/linux/fs.h \
  /usr/include/linux/limits.h /usr/include/linux/wait.h /usr/include/linux/types.h \
  /usr/include/linux/dirent.h /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h \
  /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h \
  /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h \
  /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h \
  /usr/include/linux/kernel.h /usr/include/linux/signal.h /usr/include/linux/time.h \
  /usr/include/linux/param.h /usr/include/linux/resource.h /usr/include/linux/vm86.h \
  /usr/include/asm/io.h fdomain.h scsi.h hosts.h max_hosts.h /usr/include/asm/system.h \
  /usr/include/linux/errno.h 
hosts.o : hosts.c /usr/include/linux/config.h /usr/include/linux/config.dist.h \
  /usr/include/linux/kernel.h scsi.h hosts.h max_hosts.h aha1542.h /usr/include/linux/types.h \
  fdomain.h seagate.h ultrastor.h 7000fasst.h 
scsi.o : scsi.c /usr/include/linux/config.h /usr/include/linux/config.dist.h \
  /usr/include/asm/system.h /usr/include/linux/sched.h /usr/include/linux/head.h \
  /usr/include/linux/fs.h /usr/include/linux/limits.h /usr/include/linux/wait.h \
  /usr/include/linux/types.h /usr/include/linux/dirent.h /usr/include/linux/vfs.h \
  /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h \
  /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h \
  /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h \
  /usr/include/linux/mm.h /usr/include/linux/kernel.h /usr/include/linux/signal.h \
  /usr/include/linux/time.h /usr/include/linux/param.h /usr/include/linux/resource.h \
  /usr/include/linux/vm86.h /usr/include/linux/timer.h /usr/include/linux/string.h \
  scsi.h hosts.h max_hosts.h sd.h /usr/include/linux/genhd.h st.h sr.h 
scsi_ioctl.o : scsi_ioctl.c /usr/include/linux/config.h /usr/include/linux/config.dist.h \
  /usr/include/asm/io.h /usr/include/asm/segment.h /usr/include/asm/system.h /usr/include/linux/errno.h \
  /usr/include/linux/kernel.h /usr/include/linux/sched.h /usr/include/linux/head.h \
  /usr/include/linux/fs.h /usr/include/linux/limits.h /usr/include/linux/wait.h \
  /usr/include/linux/types.h /usr/include/linux/dirent.h /usr/include/linux/vfs.h \
  /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h \
  /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h \
  /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h \
  /usr/include/linux/mm.h /usr/include/linux/signal.h /usr/include/linux/time.h \
  /usr/include/linux/param.h /usr/include/linux/resource.h /usr/include/linux/vm86.h \
  /usr/include/linux/string.h scsi.h hosts.h max_hosts.h scsi_ioctl.h 
sd.o : sd.c /usr/include/linux/config.h /usr/include/linux/config.dist.h /usr/include/linux/fs.h \
  /usr/include/linux/limits.h /usr/include/linux/wait.h /usr/include/linux/types.h \
  /usr/include/linux/dirent.h /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h \
  /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h \
  /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h \
  /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h /usr/include/linux/kernel.h \
  /usr/include/linux/sched.h /usr/include/linux/head.h /usr/include/linux/mm.h \
  /usr/include/linux/signal.h /usr/include/linux/time.h /usr/include/linux/param.h \
  /usr/include/linux/resource.h /usr/include/linux/vm86.h /usr/include/linux/string.h \
  /usr/include/linux/errno.h /usr/include/asm/system.h scsi.h hosts.h max_hosts.h \
  sd.h /usr/include/linux/genhd.h scsi_ioctl.h ../blk.h 
sd_ioctl.o : sd_ioctl.c /usr/include/linux/config.h /usr/include/linux/config.dist.h \
  /usr/include/linux/kernel.h /usr/include/linux/sched.h /usr/include/linux/head.h \
  /usr/include/linux/fs.h /usr/include/linux/limits.h /usr/include/linux/wait.h \
  /usr/include/linux/types.h /usr/include/linux/dirent.h /usr/include/linux/vfs.h \
  /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h \
  /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h \
  /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h \
  /usr/include/linux/mm.h /usr/include/linux/signal.h /usr/include/linux/time.h \
  /usr/include/linux/param.h /usr/include/linux/resource.h /usr/include/linux/vm86.h \
  scsi.h sd.h /usr/include/linux/genhd.h 
seagate.o : seagate.c /usr/include/linux/config.h /usr/include/linux/config.dist.h \
  /usr/include/asm/io.h /usr/include/asm/system.h /usr/include/linux/signal.h \
  /usr/include/linux/sched.h /usr/include/linux/head.h /usr/include/linux/fs.h \
  /usr/include/linux/limits.h /usr/include/linux/wait.h /usr/include/linux/types.h \
  /usr/include/linux/dirent.h /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h \
  /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h \
  /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h \
  /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h \
  /usr/include/linux/kernel.h /usr/include/linux/time.h /usr/include/linux/param.h \
  /usr/include/linux/resource.h /usr/include/linux/vm86.h seagate.h scsi.h hosts.h \
  max_hosts.h 
sr.o : sr.c /usr/include/linux/config.h /usr/include/linux/config.dist.h /usr/include/linux/fs.h \
  /usr/include/linux/limits.h /usr/include/linux/wait.h /usr/include/linux/types.h \
  /usr/include/linux/dirent.h /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h \
  /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h \
  /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h \
  /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h /usr/include/linux/kernel.h \
  /usr/include/linux/sched.h /usr/include/linux/head.h /usr/include/linux/mm.h \
  /usr/include/linux/signal.h /usr/include/linux/time.h /usr/include/linux/param.h \
  /usr/include/linux/resource.h /usr/include/linux/vm86.h /usr/include/linux/string.h \
  /usr/include/linux/errno.h scsi.h sr.h scsi_ioctl.h ../blk.h 
sr_ioctl.o : sr_ioctl.c /usr/include/linux/config.h /usr/include/linux/config.dist.h \
  /usr/include/linux/kernel.h /usr/include/linux/sched.h /usr/include/linux/head.h \
  /usr/include/linux/fs.h /usr/include/linux/limits.h /usr/include/linux/wait.h \
  /usr/include/linux/types.h /usr/include/linux/dirent.h /usr/include/linux/vfs.h \
  /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h \
  /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h \
  /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h \
  /usr/include/linux/mm.h /usr/include/linux/signal.h /usr/include/linux/time.h \
  /usr/include/linux/param.h /usr/include/linux/resource.h /usr/include/linux/vm86.h \
  /usr/include/asm/segment.h /usr/include/linux/errno.h ../blk.h scsi.h sr.h scsi_ioctl.h \
  /usr/include/linux/cdrom.h 
st.o : st.c /usr/include/linux/config.h /usr/include/linux/config.dist.h scsi.h \
  st.h /usr/include/linux/fs.h /usr/include/linux/limits.h /usr/include/linux/wait.h \
  /usr/include/linux/types.h /usr/include/linux/dirent.h /usr/include/linux/vfs.h \
  /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h \
  /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h \
  /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h \
  /usr/include/linux/kernel.h /usr/include/linux/sched.h /usr/include/linux/head.h \
  /usr/include/linux/mm.h /usr/include/linux/signal.h /usr/include/linux/time.h \
  /usr/include/linux/param.h /usr/include/linux/resource.h /usr/include/linux/vm86.h \
  ../blk.h 
st_ioctl.o : st_ioctl.c /usr/include/linux/config.h /usr/include/linux/config.dist.h \
  /usr/include/linux/kernel.h /usr/include/linux/sched.h /usr/include/linux/head.h \
  /usr/include/linux/fs.h /usr/include/linux/limits.h /usr/include/linux/wait.h \
  /usr/include/linux/types.h /usr/include/linux/dirent.h /usr/include/linux/vfs.h \
  /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h \
  /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h \
  /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h \
  /usr/include/linux/mm.h /usr/include/linux/signal.h /usr/include/linux/time.h \
  /usr/include/linux/param.h /usr/include/linux/resource.h /usr/include/linux/vm86.h \
  st.h scsi.h 
ultrastor.o : ultrastor.c /usr/include/linux/config.h /usr/include/linux/config.dist.h \
  /usr/include/linux/stddef.h /usr/include/linux/string.h /usr/include/linux/types.h \
  /usr/include/linux/sched.h /usr/include/linux/head.h /usr/include/linux/fs.h \
  /usr/include/linux/limits.h /usr/include/linux/wait.h /usr/include/linux/dirent.h \
  /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h \
  /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h \
  /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h \
  /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h /usr/include/linux/kernel.h \
  /usr/include/linux/signal.h /usr/include/linux/time.h /usr/include/linux/param.h \
  /usr/include/linux/resource.h /usr/include/linux/vm86.h /usr/include/asm/io.h \
  /usr/include/asm/system.h ultrastor.h scsi.h hosts.h max_hosts.h 
