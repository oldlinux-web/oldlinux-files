	.file	"lgamma.c"
gcc2_compiled.:
.globl _signgam
#ifdef NO_JUMP_TABLE
.data
	.align 2
#ifndef NO_SHARED_LIBRARY
	.org	0xb8,0x90
#endif
_signgam:
	.long 0
#endif
.text
	.align 2
_goobie:
	.double 0d9.18938533204672780000e-01
	.align 2
_pi:
	.double 0d3.14159265358979310000e+00
	.align 2
_p1:
	.double 0d8.33333333333331070000e-02
	.double 0d-2.77777777735865000000e-03
	.double 0d7.93650576493454010000e-04
	.double 0d-5.95189686119699990000e-04
	.double 0d8.36458789220000000000e-04
	.double 0d-1.63343643100000000000e-03
	.align 2
_p2:
	.double 0d-4.23536895097440860000e+04
	.double 0d-2.08868617892698860000e+04
	.double 0d-8.76271029785214890000e+03
	.double 0d-2.00852740130727920000e+03
	.double 0d-4.39330444060025680000e+02
	.double 0d-5.01086937529709500000e+01
	.double 0d-6.74495072459252930000e+00
	.double 0d0.00000000000000000000e+00
	.align 2
_q2:
	.double 0d-4.23536895097440860000e+04
	.double 0d-2.98038533092566510000e+03
	.double 0d9.94030741508277060000e+03
	.double 0d-1.52860727377952210000e+03
	.double 0d-4.99028526621439030000e+02
	.double 0d1.89498234157028010000e+02
	.double 0d-2.30815515245801240000e+01
	.double 0d1.00000000000000000000e+00
	.align 2
LC0:
	.double 0d8.00000000000000000000e+00
	.align 2
.globl _lgamma
_lgamma:
	fldl 4(%esp)
	movl $1,_signgam
	ftst
	fnstsw %ax
	sahf
	ja L2
	subl $8,%esp
	fstpl (%esp)
	call _neg
	addl $8,%esp
	ret
	.align 2,0x90
L2:
	fldl LC0
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	fstp %st(1)
	ja L3
	subl $8,%esp
	fstpl (%esp)
	call _pos
	addl $8,%esp
	subl $8,%esp
	fstpl (%esp)
	call _log
	addl $8,%esp
	ret
	.align 2,0x90
L3:
	subl $8,%esp
	fstpl (%esp)
	call _asym
	addl $8,%esp
	ret
	.align 2,0x90
	.align 2
LC1:
	.double 0d1.00000000000000000000e+00
	.align 2
LC2:
	.double 0d5.00000000000000000000e-01
	.align 2
_asym:
	subl $16,%esp
	fldl 20(%esp)
	fld %st(0)
	fmul %st(1),%st
	fdivrl LC1
	fldz
	movl $5,%edx
	movl $_p1+40,%eax
	.align 2,0x90
L9:
	fmul %st(1),%st
	faddl (%eax)
	addl $-8,%eax
	decl %edx
	jns L9
	fstp %st(1)
	fxch %st(1)
	subl $8,%esp
	fstl (%esp)
	fxch %st(1)
	fstpl 8(%esp)
	fstpl 16(%esp)
	call _log
	addl $8,%esp
	fldl 8(%esp)
	fld %st(0)
	fsubl LC2
	fmulp %st,%st(2)
	fsubr %st,%st(1)
	fxch %st(1)
	faddl _goobie
	fldl (%esp)
	fdivp %st,%st(2)
	faddp %st,%st(1)
	addl $16,%esp
	ret
	.align 2,0x90
	.align 2
LC3:
	.double 0d5.00000000000000000000e-01
	.align 2
LC4:
	.double 0d1.00000000000000000000e+00
	.align 2
LC5:
	.double 0d2.00000000000000000000e+00
	.align 2
_neg:
	subl $28,%esp
	fldl 32(%esp)
	fchs
	fstpl 20(%esp)
	pushl 24(%esp)
	pushl 24(%esp)
	call _floor
	addl $8,%esp
	fldl 20(%esp)
	fsub %st(1),%st
	fldl LC3
	fxch %st(1)
	fcompp
	fnstsw %ax
	sahf
	jbe L11
	faddl LC4
L11:
	fld %st(0)
	fdivl LC5
	subl $8,%esp
	fstpl (%esp)
	fstpl 12(%esp)
	call _floor
	addl $8,%esp
	fadd %st(0),%st
	fldl 4(%esp)
	fsub %st,%st(1)
	fxch %st(1)
	subl $4,%esp
	fnstcw (%esp)
	movw (%esp),%ax
	orw $3072,%ax
	movw %ax,2(%esp)
	fldcw 2(%esp)
	fistpl _signgam
	fldcw (%esp)
	addl $4,%esp
	movl _signgam,%eax
	decl %eax
	addl %eax,_signgam
	fsubrl 20(%esp)
	ftst
	fnstsw %ax
	sahf
	jae L12
	fchs
	negl _signgam
L12:
	pushl 24(%esp)
	pushl 24(%esp)
	fstpl 12(%esp)
	call _pos
	addl $8,%esp
	fstpl 12(%esp)
	fldl 4(%esp)
	fmull _pi
	subl $8,%esp
	fstpl (%esp)
	call _sin
	addl $8,%esp
	fldl 20(%esp)
	fmull 12(%esp)
	fmulp %st,%st(1)
	fdivl _pi
	subl $8,%esp
	fstpl (%esp)
	call _log
	addl $8,%esp
	fchs
	addl $28,%esp
	ret
	.align 2,0x90
	.align 2
LC6:
	.double 0d2.00000000000000000000e+00
	.align 2
LC7:
	.double 0d1.00000000000000000000e+00
	.align 2
LC8:
	.double 0d3.00000000000000000000e+00
	.align 2
_pos:
	subl $16,%esp
	pushl %ebx
	fldl 24(%esp)
	fldl LC6
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	jae L14
	fstp %st(1)
	fld %st(0)
	faddl LC7
	subl $8,%esp
	fstpl (%esp)
	fstpl 20(%esp)
	call _pos
	addl $8,%esp
	fldl 12(%esp)
	fdivrp %st,%st(1)
	popl %ebx
	addl $16,%esp
	ret
	.align 2,0x90
L14:
	fldl LC8
	fxch %st(1)
	fcom %st(1)
	fnstsw %ax
	sahf
	fstp %st(1)
	jbe L15
	fstp %st(1)
	fsubl LC7
	subl $8,%esp
	fstl (%esp)
	fstpl 12(%esp)
	call _pos
	addl $8,%esp
	fldl 4(%esp)
	fmulp %st,%st(1)
	popl %ebx
	addl $16,%esp
	ret
	.align 2,0x90
L15:
	fsubp %st,%st(1)
	fldz
	fldz
	movl $7,%ecx
	movl $_q2+56,%edx
	movl $_p2+56,%ebx
	.align 2,0x90
L19:
	fxch %st(1)
	fmul %st(2),%st
	faddl (%ebx)
	fxch %st(1)
	fmul %st(2),%st
	faddl (%edx)
	addl $-8,%edx
	addl $-8,%ebx
	decl %ecx
	jns L19
	fstp %st(2)
	fdivp %st,%st(1)
	popl %ebx
	addl $16,%esp
	ret
	.align 2,0x90
