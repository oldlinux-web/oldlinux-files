	.file	"aux.c"
gcc2_compiled.:
.data
	.align 2
_bdouble_inf:
	.long 0
	.byte 0xff
	.byte 0x7
	.byte 0x0
	.byte 0x0
	.align 2
_bdouble_minf:
	.long 1
	.byte 0xff
	.byte 0x7
	.byte 0x0
	.byte 0x0
	.align 2
_bdouble_NaN:
	.long 0
	.byte 0x1
	.byte 0x0
	.byte 0x0
	.byte 0x80
	.align 2
_bdouble_huge:
	.long 0
	.byte 0xff
	.byte 0x7
	.byte 0xf0
	.byte 0xff
	.align 2
_bdouble_tiny:
	.long 0
	.byte 0x1
	.byte 0x0
	.byte 0x0
	.byte 0x0
	.align 2
_bfloat_huge:
	.byte 0x0
	.byte 0x0
	.byte 0x80
	.byte 0xff
	.align 2
_bfloat_tiny:
	.byte 0x0
	.byte 0x0
	.byte 0x80
	.byte 0x0
.globl _double_inf
	.align 2
_double_inf:
	.long _bdouble_inf
.globl _double_minf
	.align 2
_double_minf:
	.long _bdouble_minf
.globl _double_NaN
	.align 2
_double_NaN:
	.long _bdouble_NaN
.globl _double_huge
	.align 2
_double_huge:
	.long _bdouble_huge
.globl _double_tiny
	.align 2
_double_tiny:
	.long _bdouble_tiny
.globl _float_huge
	.align 2
_float_huge:
	.long _bfloat_huge
.globl _float_tiny
	.align 2
_float_tiny:
	.long _bfloat_tiny
.text
	.align 2
.globl _finite
_finite:
	fldl 4(%esp)
	movl _double_inf,%edx
	fcoml (%edx)
	fnstsw %ax
	sahf
	je L5
	movl _double_minf,%edx
	fcoml (%edx)
	fnstsw %ax
	sahf
	je L6
	movl _double_NaN,%edx
	fcompl (%edx)
	fnstsw %ax
	sahf
	jne L2
	jmp L3
	.align 2,0x90
L5:
L6:
	fstp %st(0)
L3:
	xorl %eax,%eax
	ret
	.align 2,0x90
L2:
	movl $1,%eax
	ret
	.align 2,0x90
	.align 2
.globl _infnan
_infnan:
	movl 4(%esp),%eax
	cmpl $33,%eax
	je L9
	jg L14
	cmpl $-34,%eax
	je L11
	jmp L12
	.align 2,0x90
L14:
	cmpl $34,%eax
	je L10
	jmp L12
	.align 2,0x90
L9:
	movl _double_NaN,%eax
	fldl (%eax)
	ret
	.align 2,0x90
L10:
	movl _double_inf,%eax
	fldl (%eax)
	ret
	.align 2,0x90
L11:
	movl _double_minf,%eax
	fldl (%eax)
	ret
	.align 2,0x90
L12:
	movl _double_NaN,%eax
	fldl (%eax)
	ret
	.align 2,0x90
