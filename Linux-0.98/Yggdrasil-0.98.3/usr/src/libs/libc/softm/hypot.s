	.file	"hypot.c"
gcc2_compiled.:
.text
	.align 2
LC0:
	.double 0d1.00000000000000000000e+00
	.align 2
.globl _hypot
_hypot:
	subl $16,%esp
	fldl 20(%esp)
	fldl 28(%esp)
	fxch %st(1)
	ftst
	fnstsw %ax
	sahf
	jae L2
	fchs
L2:
	fxch %st(1)
	ftst
	fnstsw %ax
	sahf
	jae L3
	fchs
L3:
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	jbe L4
	fxch %st(1)
L4:
	fxch %st(1)
	ftst
	fnstsw %ax
	sahf
	jne L5
	fstp %st(0)
	fstp %st(0)
	fldz
	addl $16,%esp
	ret
	.align 2,0x90
L5:
	fdivr %st,%st(1)
	fld %st(1)
	fmulp %st,%st(2)
	fxch %st(1)
	faddl LC0
	fstl 8(%esp)
	ftst
	fstp %st(0)
	fnstsw %ax
	sahf
	ja L6
	pushl 12(%esp)
	pushl 12(%esp)
	fstpl 8(%esp)
	call _sqrt
	addl $8,%esp
	fldl (%esp)
	jmp L7
	fstp %st(1)
L6:
	fldl 8(%esp)
	fsqrt
	fxch %st(1)
L7:
	fmulp %st,%st(1)
	addl $16,%esp
	ret
	.align 2,0x90
