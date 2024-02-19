	.file	"erf.c"
gcc2_compiled.:
.data
	.align 2
_torp:
	.double 0d1.12837916709551260000e+00
	.align 2
_p1:
	.double 0d8.04373630960840240000e+04
	.double 0d7.40407142710151490000e+03
	.double 0d3.01782788536507590000e+03
	.double 0d3.80140318123903000000e+01
	.double 0d1.43383842191748200000e+01
	.double 0d-2.88805137207594100000e-01
	.double 0d7.54772803341863160000e-03
	.align 2
_q1:
	.double 0d8.04373630960840240000e+04
	.double 0d3.42165257924628530000e+04
	.double 0d6.37960017324428280000e+03
	.double 0d6.58070155459240480000e+02
	.double 0d3.80190713951939400000e+01
	.double 0d1.00000000000000000000e+00
	.double 0d0.00000000000000000000e+00
	.align 2
_p2:
	.double 0d1.82633488422951130000e+03
	.double 0d2.89802932921676570000e+03
	.double 0d2.32043959025163530000e+03
	.double 0d1.14326207070388610000e+03
	.double 0d3.68519615471001090000e+02
	.double 0d7.70816173036842830000e+01
	.double 0d9.67580788298726620000e+00
	.double 0d5.64187782550739760000e-01
	.double 0d0.00000000000000000000e+00
	.align 2
_q2:
	.double 0d1.82633488422951130000e+03
	.double 0d4.95882756472114030000e+03
	.double 0d6.08954242327244360000e+03
	.double 0d4.42961280388368230000e+03
	.double 0d2.09438436778953970000e+03
	.double 0d6.61736120710765360000e+02
	.double 0d1.37125596050062230000e+02
	.double 0d1.71498094362760800000e+01
	.double 0d1.00000000000000000000e+00
.text
	.align 2
LC0:
	.double 0d5.00000000000000000000e-01
	.align 2
LC1:
	.double 0d1.00000000000000000000e+01
	.align 2
LC2:
	.double 0d1.00000000000000000000e+00
#ifndef NO_SHARED_LIBRARY
	.org	0x64,0x90
#endif
	.align 2
.globl _erf
_erf:
	pushl %esi
	pushl %ebx
	fldl 12(%esp)
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
	jae L3
	fld %st(0)
	fmul %st(1),%st
	fldz
	fldz
	movl $6,%ecx
	movl $_q1+48,%edx
	movl $_p1+48,%esi
	.align 2,0x90
L7:
	fxch %st(1)
	fmul %st(2),%st
	faddl (%esi)
	fxch %st(1)
	fmul %st(2),%st
	faddl (%edx)
	addl $-8,%edx
	addl $-8,%esi
	decl %ecx
	jns L7
	fstp %st(2)
	fldl _torp
	pushl %ebx
	fimull (%esp)
	addl $4,%esp
	fmulp %st,%st(3)
	fmulp %st,%st(2)
	fdivrp %st,%st(1)
	popl %ebx
	popl %esi
	ret
	.align 2,0x90
L3:
	fldl LC1
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	fstp %st(1)
	jae L10
	subl $8,%esp
	fstpl (%esp)
	call _erfc
	addl $8,%esp
	fsubrl LC2
	pushl %ebx
	fimull (%esp)
	addl $4,%esp
	popl %ebx
	popl %esi
	ret
	.align 2,0x90
L10:
	fstp %st(0)
	pushl %ebx
	fildl (%esp)
	addl $4,%esp
	popl %ebx
	popl %esi
	ret
	.align 2,0x90
	.align 2
LC3:
	.double 0d2.00000000000000000000e+00
	.align 2
LC4:
	.double 0d1.00000000000000000000e+01
	.align 2
.globl _erfc
_erfc:
	subl $16,%esp
	pushl %ebx
	fldl 24(%esp)
	ftst
	fnstsw %ax
	sahf
	jae L12
	fchs
	subl $8,%esp
	fstpl (%esp)
	call _erfc
	addl $8,%esp
	fsubrl LC3
	popl %ebx
	addl $16,%esp
	ret
	.align 2,0x90
L12:
	fldl LC4
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	fstp %st(1)
	jb L13
	fstp %st(0)
	fldz
	popl %ebx
	addl $16,%esp
	ret
	.align 2,0x90
L13:
	fldz
	fldz
	movl $8,%ecx
	movl $_q2+64,%edx
	movl $_p2+64,%ebx
	.align 2,0x90
L17:
	fxch %st(1)
	fmul %st(2),%st
	faddl (%ebx)
	fxch %st(1)
	fmul %st(2),%st
	faddl (%edx)
	addl $-8,%edx
	addl $-8,%ebx
	decl %ecx
	jns L17
	fld %st(2)
	fchs
	fmulp %st,%st(3)
	fxch %st(2)
	subl $8,%esp
	fstpl (%esp)
	fstpl 20(%esp)
	fstpl 12(%esp)
	call _exp
	addl $8,%esp
	fldl 12(%esp)
	fmulp %st,%st(1)
	fldl 4(%esp)
	fdivrp %st,%st(1)
	popl %ebx
	addl $16,%esp
	ret
	.align 2,0x90
