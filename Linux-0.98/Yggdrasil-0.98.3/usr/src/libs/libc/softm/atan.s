	.file	"atan.c"
gcc2_compiled.:
.globl _pio2
.data
	.align 2
_pio2:
	.double 0d1.57079632679489660000e+00
	.align 2
_sq2p1:
	.double 0d2.41421356237309490000e+00
	.align 2
_sq2m1:
	.double 0d4.14213562373095030000e-01
	.align 2
_pio4:
	.double 0d7.85398163397448280000e-01
	.align 2
_p4:
	.double 0d1.61536412982230220000e+01
	.align 2
_p3:
	.double 0d2.68425481955039740000e+02
	.align 2
_p2:
	.double 0d1.15302935154048510000e+03
	.align 2
_p1:
	.double 0d1.78040631643319690000e+03
	.align 2
_p0:
	.double 0d8.96785974036638660000e+02
	.align 2
_q4:
	.double 0d5.89569705084446200000e+01
	.align 2
_q3:
	.double 0d5.36265374031215290000e+02
	.align 2
_q2:
	.double 0d1.66678381488163360000e+03
	.align 2
_q1:
	.double 0d2.07933497444540990000e+03
	.align 2
_q0:
	.double 0d8.96785974036638660000e+02
.text
	.align 2
.globl _atan
_atan:
	fldl 4(%esp)
	ftst
	fnstsw %ax
	sahf
	ja L2
	fchs
	subl $8,%esp
	fstpl (%esp)
	call _satan
	addl $8,%esp
	fchs
	ret
	.align 2,0x90
L2:
	subl $8,%esp
	fstpl (%esp)
	call _satan
	addl $8,%esp
	ret
	.align 2,0x90
	.align 2
.globl _atan2
_atan2:
	fldl 4(%esp)
	fldl 12(%esp)
	fld %st(1)
	fadd %st(1),%st
	fcomp %st(2)
	fnstsw %ax
	sahf
	jne L6
	fstp %st(0)
	ftst
	fstp %st(0)
	fnstsw %ax
	sahf
	jb L7
	fldl _pio2
	ret
	.align 2,0x90
L7:
	fldl _pio2
	fchs
	ret
	.align 2,0x90
L6:
	ftst
	fnstsw %ax
	sahf
	jae L10
	fxch %st(1)
	ftst
	fnstsw %ax
	sahf
	jb L11
	fchs
	fdivp %st,%st(1)
	subl $8,%esp
	fstpl (%esp)
	call _satan
	addl $8,%esp
	fldl _pio2
	fadd %st(0),%st
	fsubp %st,%st(1)
	ret
	.align 2,0x90
L11:
	fdivp %st,%st(1)
	subl $8,%esp
	fstpl (%esp)
	call _satan
	addl $8,%esp
	fldl _pio2
	fchs
	fsubl _pio2
	faddp %st,%st(1)
	ret
	.align 2,0x90
L10:
	fxch %st(1)
	ftst
	fnstsw %ax
	sahf
	ja L9
	fchs
	fdivp %st,%st(1)
	subl $8,%esp
	fstpl (%esp)
	call _satan
	addl $8,%esp
	fchs
	ret
	.align 2,0x90
L9:
	fdivp %st,%st(1)
	subl $8,%esp
	fstpl (%esp)
	call _satan
	addl $8,%esp
	ret
	.align 2,0x90
	.align 2
LC0:
	.double 0d1.00000000000000000000e+00
	.align 2
_satan:
	fldl 4(%esp)
	fcoml _sq2m1
	fnstsw %ax
	sahf
	jae L17
	subl $8,%esp
	fstpl (%esp)
	call _xatan
	addl $8,%esp
	ret
	.align 2,0x90
L17:
	fcoml _sq2p1
	fnstsw %ax
	sahf
	ja L18
	fld1
	fld %st(1)
	fsub %st(1),%st
	fxch %st(1)
	faddp %st,%st(2)
	fdivp %st,%st(1)
	subl $8,%esp
	fstpl (%esp)
	call _xatan
	addl $8,%esp
	faddl _pio4
	ret
	.align 2,0x90
L18:
	fdivrl LC0
	subl $8,%esp
	fstpl (%esp)
	call _xatan
	addl $8,%esp
	fsubrl _pio2
	ret
	.align 2,0x90
	.align 2
_xatan:
	fldl 4(%esp)
	fld %st(0)
	fmul %st(1),%st
	fld %st(0)
	fmull _p4
	faddl _p3
	fmul %st(1),%st
	faddl _p2
	fmul %st(1),%st
	faddl _p1
	fmul %st(1),%st
	faddl _p0
	fld %st(1)
	faddl _q4
	fmul %st(2),%st
	faddl _q3
	fmul %st(2),%st
	faddl _q2
	fmul %st(2),%st
	faddl _q1
	fmulp %st,%st(2)
	fxch %st(1)
	faddl _q0
	fdivrp %st,%st(1)
	fmulp %st,%st(1)
	ret
	.align 2,0x90
