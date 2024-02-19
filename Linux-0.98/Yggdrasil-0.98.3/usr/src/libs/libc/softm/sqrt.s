	.file	"sqrt.c"
gcc2_compiled.:
.text
LC0:
	.ascii "sqrt\0"
	.align 2
LC1:
	.double 0d5.00000000000000000000e-01
	.align 2
LC2:
	.double 0d1.00000000000000000000e+00
	.align 2
LC3:
	.double 0d1.07374182400000000000e+09
	.align 2
.globl _sqrt
_sqrt:
	subl $16,%esp
	fldl 20(%esp)
	ftst
	fnstsw %ax
	sahf
	ja L2
	ftst
	fstp %st(0)
	fnstsw %ax
	sahf
	jae L3
	movl $33,_errno
	pushl $LC0
	call _perror
	addl $4,%esp
L3:
	fldz
	addl $16,%esp
	ret
	.align 2,0x90
L2:
	leal 12(%esp),%eax
	pushl %eax
	subl $8,%esp
	fstl (%esp)
	fstpl 16(%esp)
	call _frexp
	addl $12,%esp
	fldl LC1
	fldl 4(%esp)
	fxch %st(2)
	fcom %st(1)
	fnstsw %ax
	sahf
	jae L23
	.align 2,0x90
L6:
	fadd %st(0),%st
	decl 12(%esp)
	fcom %st(1)
	fnstsw %ax
	sahf
	jb L6
L23:
	fstp %st(1)
	movb 12(%esp),%al
	andb $1,%al
	je L7
	fadd %st(0),%st
	decl 12(%esp)
L7:
	faddl LC2
	fmull LC1
	cmpl $60,12(%esp)
	jle L9
	fldl LC3
	.align 2,0x90
L10:
	fmul %st,%st(1)
	addl $-60,12(%esp)
	cmpl $60,12(%esp)
	jg L10
	fstp %st(0)
L9:
	cmpl $-60,12(%esp)
	jge L12
	fldl LC3
	.align 2,0x90
L13:
	fdivr %st,%st(1)
	addl $60,12(%esp)
	cmpl $-60,12(%esp)
	jl L13
	fstp %st(0)
L12:
	cmpl $0,12(%esp)
	jl L14
	movl 12(%esp),%ecx
	testl %ecx,%ecx
	jge L15
	incl %ecx
L15:
	sarl $1,%ecx
	movl $1,%eax
	sall %cl,%eax
	pushl %eax
	fimull (%esp)
	addl $4,%esp
	jmp L16
	.align 2,0x90
L14:
	movl 12(%esp),%ecx
	negl %ecx
	jns L17
	incl %ecx
L17:
	sarl $1,%ecx
	movl $1,%eax
	sall %cl,%eax
	pushl %eax
	fidivl (%esp)
	addl $4,%esp
L16:
	fldl LC1
	movl $4,%edx
	.align 2,0x90
L21:
	fld %st(2)
	fdiv %st(2),%st
	faddp %st,%st(2)
	fmul %st,%st(1)
	decl %edx
	jns L21
	fstp %st(0)
	fstp %st(1)
	addl $16,%esp
	ret
	.align 2,0x90
