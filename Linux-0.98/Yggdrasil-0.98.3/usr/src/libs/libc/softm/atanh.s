	.file	"atanh.c"
gcc2_compiled.:
.text
	.align 2
LC0:
	.double 0d1.00000000000000000000e+00
	.align 2
.globl _asinh
_asinh:
	subl $8,%esp
	fldl 12(%esp)
	fmul %st(0),%st
	faddl LC0
	fstl (%esp)
	ftst
	fstp %st(0)
	fnstsw %ax
	sahf
	ja L2
	pushl 4(%esp)
	pushl 4(%esp)
	call _sqrt
	addl $8,%esp
	jmp L3
	fstp %st(0)
L2:
	fldl (%esp)
	fsqrt
L3:
	faddl 12(%esp)
	subl $8,%esp
	fstpl (%esp)
	call _log
	addl $8,%esp
	addl $8,%esp
	ret
	.align 2,0x90
LC1:
	.ascii "acosh\0"
	.align 2
.globl _acosh
_acosh:
	subl $16,%esp
	fldl 20(%esp)
	fld1
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	ja L5
	fcompp
	fnstsw %ax
	sahf
	jae L6
	movl $33,_errno
	pushl $LC1
	call _perror
	addl $4,%esp
L6:
	fldz
	addl $16,%esp
	ret
	.align 2,0x90
L5:
	fld %st(0)
	fmul %st(1),%st
	fsubp %st,%st(2)
	fxch %st(1)
	fstl 8(%esp)
	ftst
	fstp %st(0)
	fnstsw %ax
	sahf
	ja L7
	pushl 12(%esp)
	pushl 12(%esp)
	fstpl 8(%esp)
	call _sqrt
	addl $8,%esp
	fldl (%esp)
	jmp L8
	fstp %st(1)
L7:
	fldl 8(%esp)
	fsqrt
	fxch %st(1)
L8:
	faddp %st,%st(1)
	subl $8,%esp
	fstpl (%esp)
	call _log
	addl $8,%esp
	addl $16,%esp
	ret
	.align 2,0x90
LC2:
	.ascii "atanh\0"
	.align 2
LC3:
	.double 0d-1.00000000000000000000e+00
	.align 2
LC4:
	.double 0d1.79769313486231570000e+308
	.align 2
LC5:
	.double 0d-1.79769313486231570000e+308
	.align 2
LC6:
	.double 0d5.00000000000000000000e-01
	.align 2
.globl _atanh
_atanh:
	subl $16,%esp
	fldl 20(%esp)
	fld1
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	jae L12
	fldl LC3
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	fstp %st(1)
	ja L11
L12:
	movl $33,_errno
	pushl $LC2
	fstpl 12(%esp)
	fstpl 4(%esp)
	call _perror
	addl $4,%esp
	fldl (%esp)
	fldl 8(%esp)
	fcom %st(1)
	fnstsw %ax
	sahf
	ja L18
	fldl LC3
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	fstp %st(1)
	jae L13
L18:
	fstp %st(0)
	fstp %st(0)
	fldz
	addl $16,%esp
	ret
	.align 2,0x90
L13:
	fcompp
	fnstsw %ax
	sahf
	jne L15
	fldl LC4
	addl $16,%esp
	ret
	.align 2,0x90
L15:
	fldl LC5
	addl $16,%esp
	ret
	.align 2,0x90
L11:
	fld %st(0)
	fadd %st(2),%st
	fxch %st(1)
	fsubrp %st,%st(2)
	fdivp %st,%st(1)
	subl $8,%esp
	fstpl (%esp)
	call _log
	addl $8,%esp
	fmull LC6
	addl $16,%esp
	ret
	.align 2,0x90
