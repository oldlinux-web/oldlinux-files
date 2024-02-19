	.file	"sinh.c"
gcc2_compiled.:
.data
	.align 2
_sinh_p0:
	.double 0d-6.30767364049771680000e+05
	.align 2
_sinh_p1:
	.double 0d-8.99127202203950870000e+04
	.align 2
_sinh_p2:
	.double 0d-2.89421135598956380000e+03
	.align 2
_sinh_p3:
	.double 0d-2.63056321339749710000e+01
	.align 2
_sinh_q0:
	.double 0d-6.30767364049771680000e+05
	.align 2
_sinh_q1:
	.double 0d1.52151737879001900000e+04
	.align 2
_sinh_q2:
	.double 0d-1.73678953558233700000e+02
.text
	.align 2
LC0:
	.double 0d2.10000000000000000000e+01
	.align 2
LC1:
	.double 0d2.00000000000000000000e+00
	.align 2
LC2:
	.double 0d5.00000000000000000000e-01
	.align 2
.globl _sinh
_sinh:
	subl $16,%esp
	pushl %ebx
	fldl 24(%esp)
	movl $1,%ebx
	ftst
	fnstsw %ax
	sahf
	jae L2
	fchs
	movl $-1,%ebx
L2:
	fldl LC0
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	fstp %st(1)
	jbe L3
	subl $8,%esp
	fstpl (%esp)
	call _exp
	addl $8,%esp
	fdivl LC1
	testl %ebx,%ebx
	jle L4
	popl %ebx
	addl $16,%esp
	ret
	.align 2,0x90
L4:
	fchs
	popl %ebx
	addl $16,%esp
	ret
	.align 2,0x90
L3:
	fldl LC2
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	fstp %st(1)
	ja L6
	fld %st(0)
	fmul %st(1),%st
	fld %st(0)
	fmull _sinh_p3
	faddl _sinh_p2
	fmul %st(1),%st
	faddl _sinh_p1
	fmul %st(1),%st
	faddl _sinh_p0
	fmulp %st,%st(2)
	fld %st(0)
	faddl _sinh_q2
	fmul %st(1),%st
	faddl _sinh_q1
	fmulp %st,%st(1)
	faddl _sinh_q0
	fdivrp %st,%st(1)
	pushl %ebx
	fimull (%esp)
	addl $4,%esp
	popl %ebx
	addl $16,%esp
	ret
	.align 2,0x90
L6:
	subl $8,%esp
	fstl (%esp)
	fstpl 12(%esp)
	call _exp
	addl $8,%esp
	fstpl 12(%esp)
	fldl 4(%esp)
	fchs
	subl $8,%esp
	fstpl (%esp)
	call _exp
	addl $8,%esp
	fsubrl 12(%esp)
	pushl %ebx
	fimull (%esp)
	addl $4,%esp
	fdivl LC1
	popl %ebx
	addl $16,%esp
	ret
	.align 2,0x90
	.align 2
LC3:
	.double 0d2.10000000000000000000e+01
	.align 2
LC4:
	.double 0d2.00000000000000000000e+00
	.align 2
.globl _cosh
_cosh:
	subl $16,%esp
	fldl 20(%esp)
	ftst
	fnstsw %ax
	sahf
	jae L9
	fchs
L9:
	fldl LC3
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	fstp %st(1)
	ja L10
	subl $8,%esp
	fstl (%esp)
	fstpl 16(%esp)
	call _exp
	addl $8,%esp
	fldl 8(%esp)
	fchs
	subl $8,%esp
	fstpl (%esp)
	fstpl 8(%esp)
	call _exp
	addl $8,%esp
	fldl (%esp)
	faddp %st,%st(1)
	fdivl LC4
	addl $16,%esp
	ret
	.align 2,0x90
L10:
	subl $8,%esp
	fstpl (%esp)
	call _exp
	addl $8,%esp
	fdivl LC4
	addl $16,%esp
	ret
	.align 2,0x90
