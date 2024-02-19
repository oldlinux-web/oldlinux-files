CFLAGS= -DAM_KERNEL -I../../fs -I. -F

f=../../fs
i=/usr/include
s=/usr/include/sys
h=/usr/include/minix
l=/usr/lib
a=$f/fs.h $h/config.h $h/const.h $h/type.h $s/types.h $i/limits.h $i/errno.h \
  $f/const.h $f/type.h $f/proto.h $f/glo.h

obj =	main.s open.s read.s write.s pipe.s device.s \
	path.s mount.s link.s super.s inode.s cache.s filedes.s \
	stadir.s protect.s time.s misc.s utility.s table.s \
	putc.s

fs:	Makefile $l/head.s $(obj) $l/libc.a $l/end.s
	asld -i -o fs $l/head.s $(obj) $l/libc.a $l/end.s

clean:
	@rm -f $(obj) fs *.bak

cache.s:	$a
cache.s:	$h/com.h
cache.s:	$h/boot.h
cache.s:	$f/buf.h
cache.s:	$f/file.h
cache.s:	$f/fproc.h
cache.s:	$f/inode.h
cache.s:	$f/super.h
cache.s:	$f/cache.c
	$(CC) $(CFLAGS) -c $f/cache.c

device.s:	$a
device.s:	$h/com.h
device.s:	$f/dev.h
device.s:	$f/file.h
device.s:	$f/fproc.h
device.s:	$f/inode.h
device.s:	$f/param.h
device.s:	$f/device.c
	$(CC) $(CFLAGS) -c $f/device.c

filedes.s:	$a
filedes.s:	$f/file.h
filedes.s:	$f/fproc.h
filedes.s:	$f/inode.h
filedes.s:	$f/filedes.c
	$(CC) $(CFLAGS) -c $f/filedes.c

inode.s:	$a
inode.s:	$s/stat.h
inode.s:	$h/boot.h
inode.s:	$f/buf.h
inode.s:	$f/file.h
inode.s:	$f/fproc.h
inode.s:	$f/inode.h
inode.s:	$f/super.h
inode.s:	$f/inode.c
	$(CC) $(CFLAGS) -c $f/inode.c

link.s:		$a
link.s:		$s/stat.h
link.s:		$f/buf.h
link.s:		$f/file.h
link.s:		$f/fproc.h
link.s:		$f/inode.h
link.s:		$f/param.h
link.s:		$f/link.c
	$(CC) $(CFLAGS) -c $f/link.c

main.s:		$a
main.s:		$h/callnr.h
main.s:		$h/com.h
main.s:		$h/boot.h
main.s:		$f/buf.h
main.s:		$f/file.h
main.s:		$f/fproc.h
main.s:		$f/inode.h
main.s:		$f/param.h
main.s:		$f/super.h
main.s:		$f/main.c
	$(CC) $(CFLAGS) -c $f/main.c

misc.s:		$a
misc.s:		$i/fcntl.h
misc.s:		$h/callnr.h
misc.s:		$h/com.h
misc.s:		$h/boot.h
misc.s:		$f/buf.h
misc.s:		$f/file.h
misc.s:		$f/fproc.h
misc.s:		$f/inode.h
misc.s:		$f/param.h
misc.s:		$f/super.h
misc.s:		$f/misc.c
	$(CC) $(CFLAGS) -c $f/misc.c

mount.s:	$a
mount.s:	$s/stat.h
mount.s:	$f/buf.h
mount.s:	$f/file.h
mount.s:	$f/fproc.h
mount.s:	$f/inode.h
mount.s:	$f/param.h
mount.s:	$f/super.h
mount.s:	$f/mount.c
	$(CC) $(CFLAGS) -c $f/mount.c

open.s:		$a
open.s:		$i/fcntl.h
open.s:		$h/callnr.h
open.s:		$f/buf.h
open.s:		$f/file.h
open.s:		$f/fproc.h
open.s:		$f/inode.h
open.s:		$f/param.h
open.s:		$f/open.c
	$(CC) $(CFLAGS) -c $f/open.c

path.s:		$a
path.s:		$f/buf.h
path.s:		$f/file.h
path.s:		$f/fproc.h
path.s:		$f/inode.h
path.s:		$f/super.h
path.s:		$f/path.c
	$(CC) $(CFLAGS) -c $f/path.c

pipe.s:		$a
pipe.s:		$i/fcntl.h
pipe.s:		$i/signal.h
pipe.s:		$h/callnr.h
pipe.s:		$h/com.h
pipe.s:		$f/dev.h
pipe.s:		$f/file.h
pipe.s:		$f/fproc.h
pipe.s:		$f/inode.h
pipe.s:		$f/param.h
pipe.s:		$f/pipe.c
	$(CC) $(CFLAGS) -c $f/pipe.c

protect.s:	$a
protect.s:	$f/buf.h
protect.s:	$f/file.h
protect.s:	$f/fproc.h
protect.s:	$f/inode.h
protect.s:	$f/param.h
protect.s:	$f/super.h
protect.s:	$f/protect.c
	$(CC) $(CFLAGS) -c $f/protect.c

putc.s:		$a
putc.s:		$h/com.h
putc.s:		$f/putc.c
	$(CC) $(CFLAGS) -c $f/putc.c

read.s:		$a
read.s:		$i/fcntl.h
read.s:		$h/com.h
read.s:		$f/buf.h
read.s:		$f/file.h
read.s:		$f/fproc.h
read.s:		$f/inode.h
read.s:		$f/param.h
read.s:		$f/super.h
read.s:		$f/read.c
	$(CC) $(CFLAGS) -c $f/read.c

stadir.s:	$a
stadir.s:	$s/stat.h
stadir.s:	$f/file.h
stadir.s:	$f/fproc.h
stadir.s:	$f/inode.h
stadir.s:	$f/param.h
stadir.s:	$f/stadir.c
	$(CC) $(CFLAGS) -c $f/stadir.c

super.s:	$a
super.s:	$h/boot.h
super.s:	$f/buf.h
super.s:	$f/inode.h
super.s:	$f/super.h
super.s:	$f/super.c
	$(CC) $(CFLAGS) -c $f/super.c

table.s:	$a
table.s:	$s/stat.h
table.s:	$h/callnr.h
table.s:	$h/com.h
table.s:	$f/buf.h
table.s:	$f/dev.h
table.s:	$f/file.h
table.s:	$f/fproc.h
table.s:	$f/inode.h
table.s:	$f/super.h
table.s:	$f/table.c
	$(CC) $(CFLAGS) -c $f/table.c

time.s:		$a
time.s:		$h/callnr.h
time.s:		$h/com.h
time.s:		$f/file.h
time.s:		$f/fproc.h
time.s:		$f/inode.h
time.s:		$f/param.h
time.s:		$f/time.c
	$(CC) $(CFLAGS) -c $f/time.c

utility.s:	$a
utility.s:	$h/com.h
utility.s:	$h/boot.h
utility.s:	$f/buf.h
utility.s:	$f/file.h
utility.s:	$f/fproc.h
utility.s:	$f/inode.h
utility.s:	$f/param.h
utility.s:	$f/super.h
utility.s:	$f/utility.c
	$(CC) $(CFLAGS) -c $f/utility.c

write.s:	$a
write.s:	$f/buf.h
write.s:	$f/file.h
write.s:	$f/fproc.h
write.s:	$f/inode.h
write.s:	$f/super.h
write.s:	$f/write.c
	$(CC) $(CFLAGS) -c $f/write.c








