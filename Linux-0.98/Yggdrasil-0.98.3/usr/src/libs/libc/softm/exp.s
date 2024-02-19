	.file	"exp.c"
gcc2_compiled.:
.data
	.align 2
_exp_p0:
	.double 0d2.08038434669466290000e+06
	.align 2
_exp_p1:
	.double 0d3.02869716974403640000e+04
	.align 2
_exp_p2:
	.double 0d6.06148533006108110000e+01
	.align 2
_exp_q0:
	.double 0d6.00272036023883240000e+06
	.align 2
_exp_q1:
	.double 0d3.27725151808291440000e+05
	.align 2
_exp_q2:
	.double 0d1.74928768909307630000e+03
	.align 2
_log2e:
	.double 0d1.44269504088896340000e+00
	.align 2
_log2t:
	.double 0d3.32192809488736220000e+00
	.align 2
_sqrt2:
	.double 0d1.41421356237309490000e+00
	.align 2
_maxf:
	.double 0d1.00000000000000000000e+04
.text
	.align 2
LC0:
	.double 0d5.00000000000000000000e-01
	.align 2
LC1:
	.double 0d1.79769313486231570000e+308
	.align 2
.globl _pow2
_pow2:
	subl $12,%esp
	fldl 16(%esp)
	ftst
	fnstsw %ax
	sahf
	jne L2
	fstp %st(0)
	fld1
	addl $12,%esp
	ret
	.align 2,0x90
L2:
	fldl _maxf
	fchs
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	fstp %st(1)
	jae L3
	fstp %st(0)
	fldz
	addl $12,%esp
	ret
	.align 2,0x90
L3:
	fcoml _maxf
	fnstsw %ax
	sahf
	ja L6
	subl $8,%esp
	fstl (%esp)
	fstpl 12(%esp)
	call _floor
	addl $8,%esp
	subl $4,%esp
	fnstcw (%esp)
	movw (%esp),%ax
	orw $3072,%ax
	movw %ax,2(%esp)
	fldcw 2(%esp)
	fistpl 4(%esp)
	fldcw (%esp)
	addl $4,%esp
	fldl 4(%esp)
	fisubl (%esp)
	fsubl LC0
	fld %st(0)
	fmul %st(1),%st
	fld %st(0)
	fmull _exp_p2
	faddl _exp_p1
	fmul %st(1),%st
	faddl _exp_p0
	fmulp %st,%st(2)
	fld %st(0)
	faddl _exp_q2
	fmul %st(1),%st
	faddl _exp_q1
	fmulp %st,%st(1)
	faddl _exp_q0
	pushl (%esp)
	fld %st(0)
	fadd %st(2),%st
	fmull _sqrt2
	fxch %st(1)
	fsubp %st,%st(2)
	fdivp %st,%st(1)
	subl $8,%esp
	fstpl (%esp)
	call _ldexp
	addl $12,%esp
	addl $12,%esp
	ret
	.align 2,0x90
L6:
	fstp %st(0)
	fldl LC1
	addl $12,%esp
	ret
	.align 2,0x90
	.align 2
.globl _pow10
_pow10:
	fldl 4(%esp)
	fmull _log2t
	subl $8,%esp
	fstpl (%esp)
	call _pow2
	addl $8,%esp
	ret
	.align 2,0x90
	.align 2
.globl _exp
_exp:
	fldl 4(%esp)
	fmull _log2e
	subl $8,%esp
	fstpl (%esp)
	call _pow2
	addl $8,%esp
	ret
	.align 2,0x90
LC2:
	.ascii "pow\0"
	.align 2
.globl _pow
_pow:
	pushl %ebx
	fldl 8(%esp)
	ftst
	fnstsw %ax
	sahf
	ja L10
	ftst
	fnstsw %ax
	sahf
	jne L11
	fstp %st(0)
	fldl 16(%esp)
	ftst
	fstp %st(0)
	fnstsw %ax
	sahf
	jbe L13
	fldz
	popl %ebx
	ret
	.align 2,0x90
L11:
	fldl 16(%esp)
	subl $4,%esp
	fnstcw (%esp)
	movw (%esp),%ax
	orw $3072,%ax
	movw %ax,2(%esp)
	fldcw 2(%esp)
	subl $4,%esp
	fistl (%esp)
	popl %ebx
	fldcw (%esp)
	addl $4,%esp
	pushl %ebx
	fildl (%esp)
	addl $4,%esp
	fcompp
	fnstsw %ax
	sahf
	jne L17
	fchs
	subl $8,%esp
	fstpl (%esp)
	call _log
	addl $8,%esp
	fmull 16(%esp)
	fmull _log2e
	subl $8,%esp
	fstpl (%esp)
	call _pow2
	addl $8,%esp
	movb %bl,%al
	andb $1,%al
	je L15
	fchs
L15:
	popl %ebx
	ret
	.align 2,0x90
L10:
	subl $8,%esp
	fstpl (%esp)
	call _log
	addl $8,%esp
	fmull 16(%esp)
	fmull _log2e
	subl $8,%esp
	fstpl (%esp)
	call _pow2
	addl $8,%esp
	popl %ebx
	ret
	.align 2,0x90
L17:
	fstp %st(0)
L13:
	movl $33,_errno
	pushl $LC2
	call _perror
	addl $4,%esp
	fldz
	popl %ebx
	ret
	.align 2,0x90
