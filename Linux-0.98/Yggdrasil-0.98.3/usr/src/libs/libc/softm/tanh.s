	.file	"tanh.c"
gcc2_compiled.:
.text
	.align 2
LC0:
	.double 0d2.10000000000000000000e+01
	.align 2
.globl _tanh
_tanh:
	subl $24,%esp
	fldl 28(%esp)
	movl $0,16(%esp)
	movl $1072693248,20(%esp)
	ftst
	fnstsw %ax
	sahf
	jae L2
	fchs
	movl $0,16(%esp)
	movl $-1074790400,20(%esp)
L2:
	fldl LC0
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	fstp %st(1)
	ja L5
	subl $8,%esp
	fstl (%esp)
	fstpl 8(%esp)
	call _sinh
	addl $8,%esp
	fstpl 8(%esp)
	fldl (%esp)
	subl $8,%esp
	fstpl (%esp)
	call _cosh
	addl $8,%esp
	fldl 16(%esp)
	fmull 8(%esp)
	fdivp %st,%st(1)
	addl $24,%esp
	ret
	.align 2,0x90
L5:
	fstp %st(0)
	fldl 16(%esp)
	addl $24,%esp
	ret
	.align 2,0x90
