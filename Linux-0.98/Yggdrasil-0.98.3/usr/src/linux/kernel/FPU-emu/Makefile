#               Makefile for wm-FPU-emu
#

CC	= gcc
#OPTS	= -O2
OPTS	= -O
#DEBUG	= -DDEBUGGING
DEBUG	=
CFLAGS2	= $(CFLAGS) -DPARANOID $(DEBUG) -Wall -fno-builtin

.c.o:
	$(CC) $(CFLAGS2) $(MATH_EMULATION) -c $<

.S.o:
	$(CC) $(CFLAGS2) -c $<

.s.o:
	$(CC) -c $<

OBJS =	fpu_entry.o\
	div_small.o errors.o\
	fpu_arith.o fpu_aux.o fpu_etc.o fpu_trig.o\
	load_store.o get_address.o\
	poly_atan.o poly_l2.o poly_2xm1.o poly_sin.o poly_tan.o\
	poly_div.o poly_mul64.o polynomial.o\
	reg_add_sub.o reg_compare.o reg_constant.o reg_ld_str.o\
	reg_div.o reg_mul.o reg_norm.o \
	reg_u_add.o reg_u_div.o reg_u_mul.o reg_u_sub.o\
	wm_shrx.o wm_sqrt.o


math.a: $(OBJS)
	rm -f math.a
	$(AR) rcs math.a $(OBJS)
	sync

clean:
	rm -f core *.o *.a tmp_make *~
	for i in *.c;do rm -f `basename $$i .c`.s;done

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	for i in *.c *.S;do $(CPP) -M $$i;done >> tmp_make
	cp tmp_make Makefile

proto:
	cproto -e -DMAKING_PROTO *.c >fpu_proto.h

dummy:

### Dependencies:
errors.o : errors.c /usr/include/linux/signal.h /usr/include/asm/segment.h fpu_system.h \
  /usr/include/linux/math_emu.h /usr/include/linux/sched.h /usr/include/linux/head.h \
  /usr/include/linux/fs.h /usr/include/linux/limits.h /usr/include/linux/wait.h \
  /usr/include/linux/types.h /usr/include/linux/dirent.h /usr/include/linux/vfs.h \
  /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h \
  /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h \
  /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h \
  /usr/include/linux/mm.h /usr/include/linux/kernel.h /usr/include/linux/time.h \
  /usr/include/linux/param.h /usr/include/linux/resource.h /usr/include/linux/vm86.h \
  exception.h fpu_emu.h fpu_proto.h status_w.h control_w.h reg_constant.h version.h 
fpu_arith.o : fpu_arith.c fpu_system.h /usr/include/linux/math_emu.h /usr/include/linux/sched.h \
  /usr/include/linux/head.h /usr/include/linux/fs.h /usr/include/linux/limits.h \
  /usr/include/linux/wait.h /usr/include/linux/types.h /usr/include/linux/dirent.h \
  /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h \
  /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h \
  /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h \
  /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h /usr/include/linux/kernel.h \
  /usr/include/linux/signal.h /usr/include/linux/time.h /usr/include/linux/param.h \
  /usr/include/linux/resource.h /usr/include/linux/vm86.h fpu_emu.h fpu_proto.h 
fpu_aux.o : fpu_aux.c fpu_system.h /usr/include/linux/math_emu.h /usr/include/linux/sched.h \
  /usr/include/linux/head.h /usr/include/linux/fs.h /usr/include/linux/limits.h \
  /usr/include/linux/wait.h /usr/include/linux/types.h /usr/include/linux/dirent.h \
  /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h \
  /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h \
  /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h \
  /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h /usr/include/linux/kernel.h \
  /usr/include/linux/signal.h /usr/include/linux/time.h /usr/include/linux/param.h \
  /usr/include/linux/resource.h /usr/include/linux/vm86.h exception.h fpu_emu.h \
  fpu_proto.h status_w.h 
fpu_entry.o : fpu_entry.c /usr/include/linux/signal.h /usr/include/linux/sched.h \
  /usr/include/linux/head.h /usr/include/linux/fs.h /usr/include/linux/limits.h \
  /usr/include/linux/wait.h /usr/include/linux/types.h /usr/include/linux/dirent.h \
  /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h \
  /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h \
  /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h \
  /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h /usr/include/linux/kernel.h \
  /usr/include/linux/time.h /usr/include/linux/param.h /usr/include/linux/resource.h \
  /usr/include/linux/vm86.h 
fpu_etc.o : fpu_etc.c fpu_system.h /usr/include/linux/math_emu.h /usr/include/linux/sched.h \
  /usr/include/linux/head.h /usr/include/linux/fs.h /usr/include/linux/limits.h \
  /usr/include/linux/wait.h /usr/include/linux/types.h /usr/include/linux/dirent.h \
  /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h \
  /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h \
  /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h \
  /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h /usr/include/linux/kernel.h \
  /usr/include/linux/signal.h /usr/include/linux/time.h /usr/include/linux/param.h \
  /usr/include/linux/resource.h /usr/include/linux/vm86.h exception.h fpu_emu.h \
  fpu_proto.h status_w.h reg_constant.h 
fpu_trig.o : fpu_trig.c fpu_system.h /usr/include/linux/math_emu.h /usr/include/linux/sched.h \
  /usr/include/linux/head.h /usr/include/linux/fs.h /usr/include/linux/limits.h \
  /usr/include/linux/wait.h /usr/include/linux/types.h /usr/include/linux/dirent.h \
  /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h \
  /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h \
  /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h \
  /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h /usr/include/linux/kernel.h \
  /usr/include/linux/signal.h /usr/include/linux/time.h /usr/include/linux/param.h \
  /usr/include/linux/resource.h /usr/include/linux/vm86.h exception.h fpu_emu.h \
  fpu_proto.h status_w.h control_w.h reg_constant.h 
get_address.o : get_address.c /usr/include/linux/stddef.h /usr/include/linux/math_emu.h \
  /usr/include/linux/sched.h /usr/include/linux/head.h /usr/include/linux/fs.h \
  /usr/include/linux/limits.h /usr/include/linux/wait.h /usr/include/linux/types.h \
  /usr/include/linux/dirent.h /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h \
  /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h \
  /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h \
  /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h \
  /usr/include/linux/kernel.h /usr/include/linux/signal.h /usr/include/linux/time.h \
  /usr/include/linux/param.h /usr/include/linux/resource.h /usr/include/linux/vm86.h \
  /usr/include/asm/segment.h fpu_system.h exception.h fpu_emu.h fpu_proto.h 
load_store.o : load_store.c /usr/include/asm/segment.h fpu_system.h /usr/include/linux/math_emu.h \
  /usr/include/linux/sched.h /usr/include/linux/head.h /usr/include/linux/fs.h \
  /usr/include/linux/limits.h /usr/include/linux/wait.h /usr/include/linux/types.h \
  /usr/include/linux/dirent.h /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h \
  /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h \
  /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h \
  /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h \
  /usr/include/linux/kernel.h /usr/include/linux/signal.h /usr/include/linux/time.h \
  /usr/include/linux/param.h /usr/include/linux/resource.h /usr/include/linux/vm86.h \
  exception.h fpu_emu.h fpu_proto.h status_w.h 
poly_2xm1.o : poly_2xm1.c exception.h fpu_emu.h fpu_proto.h reg_constant.h 
poly_atan.o : poly_atan.c exception.h fpu_emu.h fpu_proto.h reg_constant.h 
poly_l2.o : poly_l2.c exception.h fpu_emu.h fpu_proto.h reg_constant.h 
poly_sin.o : poly_sin.c exception.h fpu_emu.h fpu_proto.h reg_constant.h 
poly_tan.o : poly_tan.c exception.h fpu_emu.h fpu_proto.h reg_constant.h 
reg_add_sub.o : reg_add_sub.c exception.h fpu_emu.h fpu_proto.h reg_constant.h 
reg_compare.o : reg_compare.c fpu_system.h /usr/include/linux/math_emu.h /usr/include/linux/sched.h \
  /usr/include/linux/head.h /usr/include/linux/fs.h /usr/include/linux/limits.h \
  /usr/include/linux/wait.h /usr/include/linux/types.h /usr/include/linux/dirent.h \
  /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h \
  /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h \
  /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h \
  /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h /usr/include/linux/kernel.h \
  /usr/include/linux/signal.h /usr/include/linux/time.h /usr/include/linux/param.h \
  /usr/include/linux/resource.h /usr/include/linux/vm86.h exception.h fpu_emu.h \
  fpu_proto.h status_w.h 
reg_constant.o : reg_constant.c fpu_system.h /usr/include/linux/math_emu.h /usr/include/linux/sched.h \
  /usr/include/linux/head.h /usr/include/linux/fs.h /usr/include/linux/limits.h \
  /usr/include/linux/wait.h /usr/include/linux/types.h /usr/include/linux/dirent.h \
  /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h /usr/include/linux/minix_fs_i.h \
  /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h /usr/include/linux/iso_fs_i.h \
  /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h /usr/include/linux/msdos_fs_sb.h \
  /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h /usr/include/linux/kernel.h \
  /usr/include/linux/signal.h /usr/include/linux/time.h /usr/include/linux/param.h \
  /usr/include/linux/resource.h /usr/include/linux/vm86.h fpu_emu.h fpu_proto.h \
  status_w.h reg_constant.h 
reg_ld_str.o : reg_ld_str.c /usr/include/asm/segment.h fpu_system.h /usr/include/linux/math_emu.h \
  /usr/include/linux/sched.h /usr/include/linux/head.h /usr/include/linux/fs.h \
  /usr/include/linux/limits.h /usr/include/linux/wait.h /usr/include/linux/types.h \
  /usr/include/linux/dirent.h /usr/include/linux/vfs.h /usr/include/linux/pipe_fs_i.h \
  /usr/include/linux/minix_fs_i.h /usr/include/linux/ext_fs_i.h /usr/include/linux/msdos_fs_i.h \
  /usr/include/linux/iso_fs_i.h /usr/include/linux/minix_fs_sb.h /usr/include/linux/ext_fs_sb.h \
  /usr/include/linux/msdos_fs_sb.h /usr/include/linux/iso_fs_sb.h /usr/include/linux/mm.h \
  /usr/include/linux/kernel.h /usr/include/linux/signal.h /usr/include/linux/time.h \
  /usr/include/linux/param.h /usr/include/linux/resource.h /usr/include/linux/vm86.h \
  exception.h fpu_emu.h fpu_proto.h reg_constant.h control_w.h 
reg_mul.o : reg_mul.c exception.h fpu_emu.h fpu_proto.h reg_constant.h 
div_small.o : div_small.S fpu_asm.h fpu_emu.h 
poly_div.o : poly_div.S fpu_asm.h fpu_emu.h 
poly_mul64.o : poly_mul64.S fpu_asm.h fpu_emu.h 
polynomial.o : polynomial.S fpu_asm.h fpu_emu.h 
reg_div.o : reg_div.S exception.h fpu_emu.h fpu_asm.h 
reg_norm.o : reg_norm.S fpu_asm.h fpu_emu.h 
reg_u_add.o : reg_u_add.S exception.h fpu_emu.h fpu_asm.h 
reg_u_div.o : reg_u_div.S exception.h fpu_emu.h fpu_asm.h 
reg_u_mul.o : reg_u_mul.S exception.h fpu_emu.h fpu_asm.h 
reg_u_sub.o : reg_u_sub.S exception.h fpu_emu.h fpu_asm.h 
wm_shrx.o : wm_shrx.S fpu_asm.h fpu_emu.h 
wm_sqrt.o : wm_sqrt.S exception.h fpu_emu.h fpu_asm.h 
