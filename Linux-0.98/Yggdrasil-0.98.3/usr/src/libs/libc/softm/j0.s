	.file	"j0.c"
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
	.double 0d4.93378725179413360000e+20
	.double 0d-1.17915762910761040000e+20
	.double 0d6.38205934107235640000e+18
	.double 0d-1.36762035308817140000e+17
	.double 0d1.43435493914034400000e+15
	.double 0d-8.08522203485379390000e+12
	.double 0d2.50715828553688200000e+10
	.double 0d-4.05041237183313300000e+07
	.double 0d2.68578685698001500000e+04
	.align 2
_q1:
	.double 0d4.93378725179413360000e+20
	.double 0d5.42891838409228490000e+18
	.double 0d3.02463561670946240000e+16
	.double 0d1.12775673967979840000e+14
	.double 0d3.12304311494121340000e+11
	.double 0d6.69998767298223970000e+08
	.double 0d1.11463609846298540000e+06
	.double 0d1.36306365232897060000e+03
	.double 0d1.00000000000000000000e+00
	.align 2
_p2:
	.double 0d5.39348508386943860000e+06
	.double 0d1.23323847681763810000e+07
	.double 0d8.41304145655043980000e+06
	.double 0d2.01613528304998370000e+06
	.double 0d1.53982653262391130000e+05
	.double 0d2.48527192895740380000e+03
	.double 0d0.00000000000000000000e+00
	.align 2
_q2:
	.double 0d5.39348508386943860000e+06
	.double 0d1.23383102278632480000e+07
	.double 0d8.42644905062979650000e+06
	.double 0d2.02506680157013400000e+06
	.double 0d1.56001727694003090000e+05
	.double 0d2.61570073692083950000e+03
	.double 0d1.00000000000000000000e+00
	.align 2
_p3:
	.double 0d-3.98461735759522250000e+03
	.double 0d-1.03814169874846390000e+04
	.double 0d-8.23906631348560590000e+03
	.double 0d-2.36595617077910810000e+03
	.double 0d-2.26263064193370410000e+02
	.double 0d-4.88719939584126180000e+00
	.double 0d0.00000000000000000000e+00
	.align 2
_q3:
	.double 0d2.55015510886094240000e+05
	.double 0d6.66745423931982720000e+05
	.double 0d5.33291363421689720000e+05
	.double 0d1.56021320667929160000e+05
	.double 0d1.57048919151539560000e+04
	.double 0d4.08771467398349900000e+02
	.double 0d1.00000000000000000000e+00
	.align 2
_p4:
	.double 0d-2.75028667862910940000e+19
	.double 0d6.58747327571955470000e+19
	.double 0d-5.24706558111276440000e+18
	.double 0d1.37562431639934400000e+17
	.double 0d-1.64860581718572950000e+15
	.double 0d1.02552085968639410000e+13
	.double 0d-3.43637122297904050000e+10
	.double 0d5.91521346568688970000e+07
	.double 0d-4.13703549793314810000e+04
	.align 2
_q4:
	.double 0d3.72645883898616610000e+20
	.double 0d4.19241704341084010000e+18
	.double 0d2.39288304349978160000e+16
	.double 0d9.16203803407518590000e+13
	.double 0d2.61306575504108120000e+11
	.double 0d5.79512264070073010000e+08
	.double 0d1.00170264128890620000e+06
	.double 0d1.28245277247899390000e+03
	.double 0d1.00000000000000000000e+00
.text
	.align 2
LC0:
	.double 0d8.00000000000000000000e+00
	.align 2
.globl _j0
_j0:
	subl $32,%esp
	pushl %ebx
	fldl 40(%esp)
	ftst
	fnstsw %ax
	sahf
	jae L11
	fchs
	fstpl 40(%esp)
	jmp L2
	.align 2,0x90
L11:
	fstp %st(0)
L2:
	fldl LC0
	fldl 40(%esp)
	fcompp
	fnstsw %ax
	sahf
	jbe L3
	pushl 44(%esp)
	pushl 44(%esp)
	call _asympt
	addl $8,%esp
	fldl 40(%esp)
	fsubl _pio4
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
	fldl 40(%esp)
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
	popl %ebx
	addl $32,%esp
	ret
	.align 2,0x90
L3:
	fldl 40(%esp)
	fmul %st(0),%st
	fldz
	fldz
	movl $8,%ecx
	movl $_q1+64,%edx
	movl $_p1+64,%ebx
	.align 2,0x90
L9:
	fxch %st(1)
	fmul %st(2),%st
	faddl (%ebx)
	fxch %st(1)
	fmul %st(2),%st
	faddl (%edx)
	addl $-8,%edx
	addl $-8,%ebx
	decl %ecx
	jns L9
	fstp %st(2)
	fdivp %st,%st(1)
	popl %ebx
	addl $32,%esp
	ret
	.align 2,0x90
	.align 2
LC1:
	.double 0d-1.79769313486231570000e+308
	.align 2
LC2:
	.double 0d8.00000000000000000000e+00
	.align 2
.globl _y0
_y0:
	subl $48,%esp
	pushl %ebx
	fldl 56(%esp)
	ftst
	fstp %st(0)
	fnstsw %ax
	sahf
	ja L13
	fldl LC1
	popl %ebx
	addl $48,%esp
	ret
	.align 2,0x90
L13:
	fldl LC2
	fldl 56(%esp)
	fcompp
	fnstsw %ax
	sahf
	jbe L14
	pushl 60(%esp)
	pushl 60(%esp)
	call _asympt
	addl $8,%esp
	fldl 56(%esp)
	fsubl _pio4
	fstpl 44(%esp)
	pushl 48(%esp)
	pushl 48(%esp)
	call _sin
	addl $8,%esp
	fstpl 28(%esp)
	pushl 48(%esp)
	pushl 48(%esp)
	call _cos
	addl $8,%esp
	fstpl 20(%esp)
	fldl 56(%esp)
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
	addl $48,%esp
	ret
	.align 2,0x90
L14:
	fldl 56(%esp)
	fmul %st(0),%st
	movl $0,44(%esp)
	movl $0,48(%esp)
	movl $0,36(%esp)
	movl $0,40(%esp)
	movl $8,%ecx
	movl $_q4+64,%edx
	movl $_p4+64,%ebx
	.align 2,0x90
L20:
	fldl 44(%esp)
	fmul %st(1),%st
	faddl (%ebx)
	fstpl 44(%esp)
	fldl 36(%esp)
	fmul %st(1),%st
	faddl (%edx)
	fstpl 36(%esp)
	addl $-8,%edx
	addl $-8,%ebx
	decl %ecx
	jns L20
	fstp %st(0)
	pushl 60(%esp)
	pushl 60(%esp)
	call _j0
	addl $8,%esp
	pushl 60(%esp)
	pushl 60(%esp)
	fstpl 12(%esp)
	call _log
	addl $8,%esp
	fldl 44(%esp)
	fdivl 36(%esp)
	fldl 4(%esp)
	fmull _tpi
	fmulp %st,%st(2)
	faddp %st,%st(1)
	popl %ebx
	addl $48,%esp
	ret
	.align 2,0x90
	.align 2
LC3:
	.double 0d6.40000000000000000000e+01
	.align 2
LC4:
	.double 0d8.00000000000000000000e+00
	.align 2
_asympt:
	fldl 4(%esp)
	fld %st(0)
	fmul %st(1),%st
	fdivrl LC3
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
	fdivrl LC4
	fxch %st(2)
	fdivp %st,%st(1)
	fmulp %st,%st(1)
	fstpl _qzero
	ret
	.align 2,0x90
.lcomm _pzero,8
.lcomm _qzero,8
