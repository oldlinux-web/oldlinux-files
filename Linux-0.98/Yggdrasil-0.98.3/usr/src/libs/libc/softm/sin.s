	.file	"sin.c"
gcc2_compiled.:
.data
	.align 2
_twoopi:
	.double 0d6.36619772367581380000e-01
	.align 2
_cos_p0:
	.double 0d1.35788409787737560000e+07
	.align 2
_cos_p1:
	.double 0d-4.94290810090284420000e+06
	.align 2
_cos_p2:
	.double 0d4.40103053537526630000e+05
	.align 2
_cos_p3:
	.double 0d-1.38472724998245280000e+04
	.align 2
_cos_p4:
	.double 0d1.45968840666576880000e+02
	.align 2
_cos_q0:
	.double 0d8.64455865292253530000e+06
	.align 2
_cos_q1:
	.double 0d4.08179225234329990000e+05
	.align 2
_cos_q2:
	.double 0d9.46309610153820720000e+03
	.align 2
_cos_q3:
	.double 0d1.32653490878613640000e+02
.text
	.align 2
LC0:
	.double 0d3.27640000000000000000e+04
	.align 2
LC1:
	.double 0d2.50000000000000000000e-01
	.align 2
LC2:
	.double 0d4.00000000000000000000e+00
	.align 2
LC3:
	.double 0d1.00000000000000000000e+00
	.align 2
_sinus:
	subl $28,%esp
	pushl %ebx
	movl 44(%esp),%ebx
	fldl 36(%esp)
	ftst
	fnstsw %ax
	sahf
	jae L2
	fchs
	addl $2,%ebx
L2:
	fmull _twoopi
	fldl LC0
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	fstp %st(1)
	jbe L3
	leal 24(%esp),%eax
	pushl %eax
	subl $8,%esp
	fstpl (%esp)
	call _modf
	addl $12,%esp
	fldl 24(%esp)
	pushl %ebx
	fiaddl (%esp)
	addl $4,%esp
	fstpl 24(%esp)
	leal 16(%esp),%eax
	pushl %eax
	fldl LC1
	fmull 28(%esp)
	subl $8,%esp
	fstpl (%esp)
	fstpl 20(%esp)
	call _modf
	fstp %st(0)
	addl $12,%esp
	fldl LC2
	fmull 16(%esp)
	fsubrl 24(%esp)
	subl $4,%esp
	fnstcw (%esp)
	movw (%esp),%ax
	orw $3072,%ax
	movw %ax,2(%esp)
	fldcw 2(%esp)
	subl $4,%esp
	fistpl (%esp)
	popl %ebx
	fldcw (%esp)
	addl $4,%esp
	fldl 8(%esp)
	jmp L4
	.align 2,0x90
L3:
	subl $4,%esp
	fnstcw (%esp)
	movw (%esp),%ax
	orw $3072,%ax
	movw %ax,2(%esp)
	fldcw 2(%esp)
	fistl 8(%esp)
	fldcw (%esp)
	addl $4,%esp
	fisubl 4(%esp)
	addl 4(%esp),%ebx
	andl $3,%ebx
L4:
	movb %bl,%al
	andb $1,%al
	je L5
	fsubrl LC3
L5:
	cmpl $1,%ebx
	jle L6
	fchs
L6:
	fld %st(0)
	fmul %st(1),%st
	fld %st(0)
	fmull _cos_p4
	faddl _cos_p3
	fmul %st(1),%st
	faddl _cos_p2
	fmul %st(1),%st
	faddl _cos_p1
	fmul %st(1),%st
	faddl _cos_p0
	fmulp %st,%st(2)
	fld %st(0)
	faddl _cos_q3
	fmul %st(1),%st
	faddl _cos_q2
	fmul %st(1),%st
	faddl _cos_q1
	fmulp %st,%st(1)
	faddl _cos_q0
	fdivrp %st,%st(1)
	popl %ebx
	addl $28,%esp
	ret
	.align 2,0x90
	.align 2
.globl _cos
_cos:
	fldl 4(%esp)
	ftst
	fnstsw %ax
	sahf
	jae L8
	fchs
L8:
	pushl $1
	subl $8,%esp
	fstpl (%esp)
	call _sinus
	addl $12,%esp
	ret
	.align 2,0x90
	.align 2
.globl _sin
_sin:
	pushl $0
	pushl 12(%esp)
	pushl 12(%esp)
	call _sinus
	addl $12,%esp
	ret
	.align 2,0x90
