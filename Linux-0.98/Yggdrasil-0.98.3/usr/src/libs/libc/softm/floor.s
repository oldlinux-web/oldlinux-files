	.file	"floor.c"
gcc2_compiled.:
.text
	.align 2
LC0:
	.double 0d1.00000000000000000000e+00
	.align 2
.globl _floor
_floor:
	pushl %ebp
	movl %esp,%ebp
	fldl 8(%ebp)
	ftst
	fnstsw %ax
	sahf
	jae L2
	fstp %st(0)
	fldl 8(%ebp)
	fchs
	fstl 8(%ebp)
	leal 8(%ebp),%eax
	pushl %eax
	subl $8,%esp
	fstpl (%esp)
	call _modf
	addl $12,%esp
	ftst
	fstp %st(0)
	fnstsw %ax
	sahf
	je L3
	fldl LC0
	faddl 8(%ebp)
	fstpl 8(%ebp)
L3:
	fldl 8(%ebp)
	fchs
	fstpl 8(%ebp)
	jmp L4
	.align 2,0x90
L2:
	leal 8(%ebp),%eax
	pushl %eax
	subl $8,%esp
	fstpl (%esp)
	call _modf
	fstp %st(0)
	addl $12,%esp
L4:
	fldl 8(%ebp)
	leave
	ret
	.align 2,0x90
	.align 2
.globl _ceil
_ceil:
	fldl 4(%esp)
	fchs
	subl $8,%esp
	fstpl (%esp)
	call _floor
	addl $8,%esp
	fchs
	ret
	.align 2,0x90
