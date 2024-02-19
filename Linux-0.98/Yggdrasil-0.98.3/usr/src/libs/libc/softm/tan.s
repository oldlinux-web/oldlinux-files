	.file	"tan.c"
gcc2_compiled.:
.data
	.align 2
_invpi:
	.double 0d1.27323954473516250000e+00
	.align 2
_tan_p0:
	.double 0d-1.30682026475482560000e+04
	.align 2
_tan_p1:
	.double 0d1.05597090171495300000e+03
	.align 2
_tan_p2:
	.double 0d-1.55068565348326630000e+01
	.align 2
_tan_p3:
	.double 0d3.42255438724100370000e-02
	.align 2
_tan_p4:
	.double 0d3.38663864267717230000e-05
	.align 2
_tan_q0:
	.double 0d-1.66389523894711890000e+04
	.align 2
_tan_q1:
	.double 0d4.76575136291648320000e+03
	.align 2
_tan_q2:
	.double 0d-1.55503316403171000000e+02
.text
	.align 2
LC0:
	.double 0d-1.00000000000000000000e+00
	.align 2
LC1:
	.double 0d1.00000000000000000000e+00
	.align 2
LC2:
	.double 0d1.79769313486231570000e+308
	.align 2
LC3:
	.double 0d-1.79769313486231570000e+308
	.align 2
.globl _tan
_tan:
	subl $20,%esp
	pushl %ebx
	fldl 28(%esp)
	xorl %ebx,%ebx
	fld1
	fxch %st(1)
	ftst
	fnstsw %ax
	sahf
	jae L2
	fstp %st(1)
	fchs
	fldl LC0
	fxch %st(1)
L2:
	fmull _invpi
	leal 16(%esp),%eax
	pushl %eax
	subl $8,%esp
	fstpl (%esp)
	fstpl 20(%esp)
	call _modf
	addl $12,%esp
	fldl 16(%esp)
	subl $4,%esp
	fnstcw (%esp)
	movw (%esp),%ax
	orw $3072,%ax
	movw %ax,2(%esp)
	fldcw 2(%esp)
	subl $4,%esp
	fistpl (%esp)
	popl %edx
	fldcw (%esp)
	addl $4,%esp
	movl %edx,%ecx
	fldl 8(%esp)
	testl %ecx,%ecx
	jge L10
	addl $3,%ecx
L10:
	movl %ecx,%eax
	andb $252,%al
	movl %edx,%ecx
	subl %eax,%ecx
	cmpl $1,%ecx
	je L4
	jle L3
	cmpl $2,%ecx
	je L5
	cmpl $3,%ecx
	je L6
	jmp L3
	.align 2,0x90
L4:
	fxch %st(1)
	fsubrl LC1
	movl $1,%ebx
	jmp L16
	.align 2,0x90
L5:
	fchs
	movl $1,%ebx
	jmp L3
	.align 2,0x90
L6:
	fxch %st(1)
	fsubrl LC1
	fxch %st(1)
	fchs
	jmp L3
	.align 2,0x90
L16:
	fxch %st(1)
L3:
	fld %st(1)
	fmul %st(2),%st
	fld %st(0)
	fmull _tan_p4
	faddl _tan_p3
	fmul %st(1),%st
	faddl _tan_p2
	fmul %st(1),%st
	faddl _tan_p1
	fmul %st(1),%st
	faddl _tan_p0
	fmulp %st,%st(3)
	fld %st(0)
	faddl _tan_q2
	fmul %st(1),%st
	faddl _tan_q1
	fmulp %st,%st(1)
	faddl _tan_q0
	fdivrp %st,%st(2)
	cmpl $1,%ebx
	jne L12
	fxch %st(1)
	ftst
	fnstsw %ax
	sahf
	jne L13
	fstp %st(0)
	ftst
	fstp %st(0)
	fnstsw %ax
	sahf
	jbe L14
	fldl LC2
	popl %ebx
	addl $20,%esp
	ret
	.align 2,0x90
L14:
	fldl LC3
	popl %ebx
	addl $20,%esp
	ret
	.align 2,0x90
L13:
	fdivrl LC1
	fxch %st(1)
L12:
	fmulp %st,%st(1)
	popl %ebx
	addl $20,%esp
	ret
	.align 2,0x90
