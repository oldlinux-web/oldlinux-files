	.file	"log.c"
gcc2_compiled.:
.data
	.align 2
_log2:
	.double 0d6.93147180559945290000e-01
	.align 2
_ln10:
	.double 0d2.30258509299404550000e+00
	.align 2
_sqrto2:
	.double 0d7.07106781186547570000e-01
	.align 2
_log_p0:
	.double 0d-2.40139179559210520000e+01
	.align 2
_log_p1:
	.double 0d3.09572928215376490000e+01
	.align 2
_log_p2:
	.double 0d-9.63769093368686570000e+00
	.align 2
_log_p3:
	.double 0d4.21087371217979730000e-01
	.align 2
_log_q0:
	.double 0d-1.20069589779605240000e+01
	.align 2
_log_q1:
	.double 0d1.94809660700889720000e+01
	.align 2
_log_q2:
	.double 0d-8.91110902798312310000e+00
.text
LC0:
	.ascii "log\0"
	.align 2
LC1:
	.double 0d-1.79769313486231570000e+308
	.align 2
LC2:
	.double 0d5.00000000000000000000e-01
	.align 2
.globl _log
_log:
	subl $8,%esp
	fldl 12(%esp)
	ftst
	fnstsw %ax
	sahf
	ja L2
	fstp %st(0)
	movl $33,_errno
	pushl $LC0
	call _perror
	addl $4,%esp
	fldl LC1
	addl $8,%esp
	ret
	.align 2,0x90
L2:
	leal 4(%esp),%eax
	pushl %eax
	subl $8,%esp
	fstpl (%esp)
	call _frexp
	addl $12,%esp
	fldl LC2
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	jae L8
	.align 2,0x90
L5:
	fadd %st(0),%st
	decl 4(%esp)
	fcom %st(1)
	fnstsw %ax
	sahf
	jb L5
L8:
	fstp %st(1)
	fcoml _sqrto2
	fnstsw %ax
	sahf
	jae L6
	fadd %st(0),%st
	decl 4(%esp)
L6:
	fld1
	fld %st(1)
	fsub %st(1),%st
	fxch %st(1)
	faddp %st,%st(2)
	fdivp %st,%st(1)
	fld %st(0)
	fmul %st(1),%st
	fld %st(0)
	fmull _log_p3
	faddl _log_p2
	fmul %st(1),%st
	faddl _log_p1
	fmul %st(1),%st
	faddl _log_p0
	fld %st(1)
	faddl _log_q2
	fmul %st(2),%st
	faddl _log_q1
	fmulp %st,%st(2)
	fxch %st(1)
	faddl _log_q0
	fdivrp %st,%st(1)
	fmulp %st,%st(1)
	fldl _log2
	fimull 4(%esp)
	faddp %st,%st(1)
	addl $8,%esp
	ret
	.align 2,0x90
	.align 2
.globl _log10
_log10:
	pushl 8(%esp)
	pushl 8(%esp)
	call _log
	addl $8,%esp
	fdivl _ln10
	ret
	.align 2,0x90
