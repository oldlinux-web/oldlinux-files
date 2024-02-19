i=/usr/include
s=/usr/include/sys
h=/usr/include/minix
l=/usr/lib
m=../../mm

a=$m/mm.h $h/config.h $h/const.h $h/type.h $s/types.h $i/limits.h $i/errno.h \
  $m/const.h $m/type.h $m/proto.h $m/glo.h

CFLAGS = -DAM_KERNEL -I../../mm -I. -F

obj =	main.s forkexit.s break.s exec.s signal.s getset.s \
	alloc.s utility.s table.s putc.s trace.s amoeba.s

mm:	Makefile $l/head.s $(obj) $l/libc.a $l/end.s
	asld -o mm $l/head.s $(obj) $l/libc.a $l/end.s

clean:
	@rm -f $(obj) mm *.bak

alloc.s:	$a
	$(CC) $(CFLAGS)  -c $m/alloc.c

amoeba.s:	$a
amoeba.s:	$h/com.h
amoeba.s:	$i/amoeba.h

break.s:	$a
break.s:	$i/signal.h
break.s:	$m/mproc.h
break.s:	$m/param.h
break.s:	$m/break.c
	$(CC) $(CFLAGS)  -c $m/break.c

exec.s:		$a
exec.s:		$s/stat.h
exec.s:		$h/callnr.h
exec.s:		$m/mproc.h
exec.s:		$m/param.h
exec.s:		$m/exec.c
	$(CC) $(CFLAGS)  -c $m/exec.c

forkexit.s:	$a
forkexit.s:	$h/callnr.h
forkexit.s:	$m/mproc.h
forkexit.s:	$m/param.h
forkexit.s:	$m/forkexit.c
	$(CC) $(CFLAGS)  -c $m/forkexit.c

getset.s:	$a
getset.s:	$h/callnr.h
getset.s:	$m/mproc.h
getset.s:	$m/param.h
getset.s:	$m/getset.c
	$(CC) $(CFLAGS)  -c $m/getset.c

main.s:		$a
main.s:		$h/callnr.h
main.s:		$h/com.h
main.s:		$m/mproc.h
main.s:		$m/param.h
main.s:		$m/main.c
	$(CC) $(CFLAGS)  -c $m/main.c

putc.s:		$a
putc.s:		$h/com.h
putc.s:		$m/putc.c
	$(CC) $(CFLAGS)  -c $m/putc.c	

signal.s:	$a
signal.s:	$s/stat.h
signal.s:	$i/signal.h
signal.s:	$h/callnr.h
signal.s:	$h/com.h
signal.s:	$m/mproc.h
signal.s:	$m/param.h
signal.s:	$m/signal.c
	$(CC) $(CFLAGS)  -c $m/signal.c

table.s:	$a
table.s:	$h/callnr.h
table.s:	$m/mproc.h
table.s:	$m/param.h
table.s:	$m/table.c
	$(CC) $(CFLAGS)  -c $m/table.c

trace.s:	$a
trace.s:	$i/signal.h
trace.s:	$m/mproc.h
trace.s:	$m/param.h
trace.s:	$m/trace.c
	$(CC) $(CFLAGS)  -c $m/trace.c

utility.s:	$a
utility.s:	$s/stat.h
utility.s:	$h/callnr.h
utility.s:	$h/com.h
utility.s:	$m/mproc.h
utility.s:	$m/utility.c
	$(CC) $(CFLAGS)  -c $m/utility.c






