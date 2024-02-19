	.file	"asin.c"
gcc2_compiled.:
.text
LC0:
	.ascii "asin\0"
	.align 2
LC1:
	.double 0d6.99999999999999960000e-01
	.align 2
.globl _asin
_asin:
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
	fld1
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	jbe L3
	fstp %st(0)
	fstp %st(0)
	movl $33,_errno
	pushl $LC0
	call _perror
	addl $4,%esp
	fldz
	addl $24,%esp
	ret
	.align 2,0x90
L3:
	fld %st(0)
	fmul %st(1),%st
	fsubrp %st,%st(2)
	fxch %st(1)
	fstl 8(%esp)
	ftst
	fstp %st(0)
	fnstsw %ax
	sahf
	ja L4
	pushl 12(%esp)
	pushl 12(%esp)
	fstpl 8(%esp)
	call _sqrt
	addl $8,%esp
	fldl (%esp)
	jmp L5
	fstp %st(1)
L4:
	fldl 8(%esp)
	fsqrt
	fxch %st(1)
L5:
	fldl LC1
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	fstp %st(1)
	jbe L6
	fdivrp %st,%st(1)
	subl $8,%esp
	fstpl (%esp)
	call _atan
	addl $8,%esp
	fsubrl _pio2
	jmp L7
	.align 2,0x90
L6:
	fdivp %st,%st(1)
	subl $8,%esp
	fstpl (%esp)
	call _atan
	addl $8,%esp
L7:
	fldl 16(%esp)
	fmulp %st,%st(1)
	addl $24,%esp
	ret
	.align 2,0x90
LC2:
	.ascii "acos\0"
	.align 2
LC3:
	.double 0d-1.00000000000000000000e+00
	.align 2
.globl _acos
_acos:
	fldl 4(%esp)
	fld1
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	fstp %st(1)
	ja L13
	fldl LC3
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	fstp %st(1)
	jae L10
L13:
	fstp %st(0)
	movl $33,_errno
	pushl $LC2
	call _perror
	addl $4,%esp
	fldz
	ret
	.align 2,0x90
L10:
	subl $8,%esp
	fstpl (%esp)
	call _asin
	addl $8,%esp
	fsubrl _pio2
	ret
	.align 2,0x90
