	.file	"j1.c"
gcc2_compiled.:
.data
	.align 2
_tpi:
	.double 0d6.36619772367581380000e-01
	.align 2
_pio4:
	.double 0d7.85398163397448280000e-01
	.align 2
_p1:
	.double 0d5.81199354001606120000e+20
	.double 0d-6.67210656892491610000e+19
	.double 0d2.31643358063400240000e+18
	.double 0d-3.58881756991010600000e+16
	.double 0d2.90879526383477560000e+14
	.double 0d-1.32298348033212650000e+12
	.double 0d3.41323418230170060000e+09
	.double 0d-4.69575353064299560000e+06
	.double 0d2.70112271089232350000e+03
	.align 2
_q1:
	.double 0d1.16239870800321220000e+21
	.double 0d1.18577071219032100000e+19
	.double 0d6.09206139891752160000e+16
	.double 0d2.08166122130760750000e+14
	.double 0d5.24371026216764950000e+11
	.double 0d1.01386351435867390000e+09
	.double 0d1.50179359499858550000e+06
	.double 0d1.60693157348148770000e+03
	.double 0d1.00000000000000000000e+00
	.align 2
_p2:
	.double 0d-4.43575781679412820000e+06
	.double 0d-9.94224650507764140000e+06
	.double 0d-6.60337324836493940000e+06
	.double 0d-1.52352935118113740000e+06
	.double 0d-1.09824055434593460000e+05
	.double 0d-1.61161664432461020000e+03
	.double 0d0.00000000000000000000e+00
	.align 2
_q2:
	.double 0d-4.43575781679412820000e+06
	.double 0d-9.93412438993458640000e+06
	.double 0d-6.58533947972308750000e+06
	.double 0d-1.51180950663416090000e+06
	.double 0d-1.07263859911038200000e+05
	.double 0d-1.45500944019049620000e+03
	.double 0d1.00000000000000000000e+00
	.align 2
_p3:
	.double 0d3.32209134098572250000e+04
	.double 0d8.51451606753357040000e+04
	.double 0d6.61788365812708360000e+04
	.double 0d1.84942628732238660000e+04
	.double 0d1.70637542902076800000e+03
	.double 0d3.52651338466360330000e+01
	.double 0d0.00000000000000000000e+00
	.align 2
_q3:
	.double 0d7.08712819410287420000e+05
	.double 0d1.81945804224399730000e+06
	.double 0d1.41946066960372100000e+06
	.double 0d4.00294435822669760000e+05
	.double 0d3.78902297457722020000e+04
	.double 0d8.63836776960499150000e+02
	.double 0d1.00000000000000000000e+00
	.align 2
_p4:
	.double 0d-9.96375342430692150000e+22
	.double 0d2.65547383143485450000e+22
	.double 0d-1.21229755541450970000e+21
	.double 0d2.19310733991779780000e+19
	.double 0d-1.96588746272214080000e+17
	.double 0d9.56993023992168380000e+14
	.double 0d-2.58068170219445120000e+12
	.double 0d3.63948854812400200000e+09
	.double 0d-2.10884754013312380000e+06
	.double 0d0.00000000000000000000e+00
	.align 2
_q4:
	.double 0d5.08206736694124320000e+23
	.double 0d5.43531037718885380000e+21
	.double 0d2.95498793589714860000e+19
	.double 0d1.08225825940881950000e+17
	.double 0d2.97663212564727690000e+14
	.double 0d6.46534088126527590000e+11
	.double 0d1.12868683716944220000e+09
	.double 0d1.56328275489958050000e+06
	.double 0d1.61236102967700090000e+03
	.double 0d1.00000000000000000000e+00
.text
	.align 2
LC0:
	.double 0d8.00000000000000000000e+00
	.align 2
LC1:
	.double 0d3.00000000000000000000e+00
	.align 2
.globl _j1
_j1:
	subl $40,%esp
	pushl %ebx
	fldl 48(%esp)
	fstl 36(%esp)
	ftst
	fstp %st(0)
	fnstsw %ax
	sahf
	jae L2
	fldl 48(%esp)
	fchs
	fstpl 36(%esp)
L2:
	fldl LC0
	fldl 36(%esp)
	fcompp
	fnstsw %ax
	sahf
	jbe L3
	pushl 40(%esp)
	pushl 40(%esp)
	call _asympt
	addl $8,%esp
	fldl _pio4
	fmull LC1
	fldl 36(%esp)
	fsubp %st,%st(1)
	subl $8,%esp
	fstl (%esp)
	fstpl 12(%esp)
	call _cos
	addl $8,%esp
	fstpl 28(%esp)
	fldl 4(%esp)
	subl $8,%esp
	fstpl (%esp)
	call _sin
	addl $8,%esp
	fstpl 20(%esp)
	fldl 36(%esp)
	fdivrl _tpi
	fstl 12(%esp)
	ftst
	fstp %st(0)
	fnstsw %ax
	sahf
	ja L4
	pushl 16(%esp)
	pushl 16(%esp)
	call _sqrt
	addl $8,%esp
	jmp L5
	fstp %st(0)
L4:
	fldl 12(%esp)
	fsqrt
L5:
	fldl 28(%esp)
	fmull _pzero
	fldl 20(%esp)
	fmull _qzero
	fsubrp %st,%st(1)
	fmulp %st,%st(1)
	fldl 48(%esp)
	ftst
	fstp %st(0)
	fnstsw %ax
	sahf
	jae L6
	fchs
L6:
	popl %ebx
	addl $40,%esp
	ret
	.align 2,0x90
L3:
	fldl 36(%esp)
	fmul %st(0),%st
	fldz
	fldz
	movl $8,%ecx
	movl $_q1+64,%edx
	movl $_p1+64,%ebx
	.align 2,0x90
L10:
	fxch %st(1)
	fmul %st(2),%st
	faddl (%ebx)
	fxch %st(1)
	fmul %st(2),%st
	faddl (%edx)
	addl $-8,%edx
	addl $-8,%ebx
	decl %ecx
	jns L10
	fstp %st(2)
	fldl 48(%esp)
	fmulp %st,%st(1)
	fdivp %st,%st(1)
	popl %ebx
	addl $40,%esp
	ret
	.align 2,0x90
	.align 2
LC2:
	.double 0d-1.79769313486231570000e+308
	.align 2
LC3:
	.double 0d8.00000000000000000000e+00
	.align 2
LC4:
	.double 0d3.00000000000000000000e+00
	.align 2
LC5:
	.double 0d1.00000000000000000000e+00
	.align 2
.globl _y1
_y1:
	subl $56,%esp
	pushl %ebx
	fldl 64(%esp)
	fstl 36(%esp)
	ftst
	fstp %st(0)
	fnstsw %ax
	sahf
	ja L13
	fldl LC2
	popl %ebx
	addl $56,%esp
	ret
	.align 2,0x90
L13:
	fldl LC3
	fldl 36(%esp)
	fcompp
	fnstsw %ax
	sahf
	jbe L14
	pushl 40(%esp)
	pushl 40(%esp)
	call _asympt
	addl $8,%esp
	fldl _pio4
	fmull LC4
	fsubrl 36(%esp)
	fstpl 52(%esp)
	pushl 56(%esp)
	pushl 56(%esp)
	call _sin
	addl $8,%esp
	fstpl 28(%esp)
	pushl 56(%esp)
	pushl 56(%esp)
	call _cos
	addl $8,%esp
	fstpl 20(%esp)
	fldl 36(%esp)
	fdivrl _tpi
	fstl 12(%esp)
	ftst
	fstp %st(0)
	fnstsw %ax
	sahf
	ja L15
	pushl 16(%esp)
	pushl 16(%esp)
	call _sqrt
	addl $8,%esp
	jmp L16
	fstp %st(0)
L15:
	fldl 12(%esp)
	fsqrt
L16:
	fldl 28(%esp)
	fmull _pzero
	fldl 20(%esp)
	fmull _qzero
	faddp %st,%st(1)
	fmulp %st,%st(1)
	popl %ebx
	addl $56,%esp
	ret
	.align 2,0x90
L14:
	fldl 36(%esp)
	fmul %st(0),%st
	movl $0,52(%esp)
	movl $0,56(%esp)
	movl $0,44(%esp)
	movl $0,48(%esp)
	movl $9,%ecx
	movl $_q4+72,%edx
	movl $_p4+72,%ebx
	.align 2,0x90
L20:
	fldl 52(%esp)
	fmul %st(1),%st
	faddl (%ebx)
	fstpl 52(%esp)
	fldl 44(%esp)
	fmul %st(1),%st
	faddl (%edx)
	fstpl 44(%esp)
	addl $-8,%edx
	addl $-8,%ebx
	decl %ecx
	jns L20
	fstp %st(0)
	pushl 40(%esp)
	pushl 40(%esp)
	call _j1
	addl $8,%esp
	pushl 40(%esp)
	pushl 40(%esp)
	fstpl 12(%esp)
	call _log
	addl $8,%esp
	fldl 36(%esp)
	fmull 52(%esp)
	fdivl 44(%esp)
	fldl 4(%esp)
	fmulp %st,%st(2)
	fldl 36(%esp)
	fdivrl LC5
	fsubrp %st,%st(2)
	fxch %st(1)
	fmull _tpi
	faddp %st,%st(1)
	popl %ebx
	addl $56,%esp
	ret
	.align 2,0x90
	.align 2
LC6:
	.double 0d6.40000000000000000000e+01
	.align 2
LC7:
	.double 0d8.00000000000000000000e+00
	.align 2
_asympt:
	fldl 4(%esp)
	fld %st(0)
	fmul %st(1),%st
	fdivrl LC6
	fldz
	fldz
	movl $6,%eax
	movl $_q2+48,%ecx
	movl $_p2+48,%edx
	.align 2,0x90
L26:
	fxch %st(1)
	fmul %st(2),%st
	faddl (%edx)
	fxch %st(1)
	fmul %st(2),%st
	faddl (%ecx)
	addl $-8,%ecx
	addl $-8,%edx
	decl %eax
	jns L26
	fdivrp %st,%st(1)
	fstpl _pzero
	fldz
	fldz
	movl $6,%eax
	movl $_q3+48,%ecx
	movl $_p3+48,%edx
	.align 2,0x90
L30:
	fxch %st(1)
	fmul %st(2),%st
	faddl (%edx)
	fxch %st(1)
	fmul %st(2),%st
	faddl (%ecx)
	addl $-8,%ecx
	addl $-8,%edx
	decl %eax
	jns L30
	fstp %st(2)
	fxch %st(2)
	fdivrl LC7
	fxch %st(2)
	fdivp %st,%st(1)
	fmulp %st,%st(1)
	fstpl _qzero
	ret
	.align 2,0x90
.lcomm _pzero,8
.lcomm _qzero,8
