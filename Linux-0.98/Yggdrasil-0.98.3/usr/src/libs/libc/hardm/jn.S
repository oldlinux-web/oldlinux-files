	.file	"jn.c"
gcc2_compiled.:
.text
	.align 2
.globl _jn
_jn:
	subl $32,%esp
	pushl %ebx
	movl 40(%esp),%ebx
	fldl 44(%esp)
	testl %ebx,%ebx
	jge L2
	negl %ebx
	fchs
L2:
	testl %ebx,%ebx
	jne L3
	subl $8,%esp
	fstpl (%esp)
	call _j0
	addl $8,%esp
	popl %ebx
	addl $32,%esp
	ret
	.align 2,0x90
L3:
	cmpl $1,%ebx
	jne L4
	subl $8,%esp
	fstpl (%esp)
	call _j1
	addl $8,%esp
	popl %ebx
	addl $32,%esp
	ret
	.align 2,0x90
L4:
	ftst
	fnstsw %ax
	sahf
	jne L5
	fstp %st(0)
	fldz
	popl %ebx
	addl $32,%esp
	ret
	.align 2,0x90
L5:
	pushl %ebx
	fildl (%esp)
	addl $4,%esp
	fcomp %st(1)
	fnstsw %ax
	sahf
	ja L7
	subl $8,%esp
	fstl (%esp)
	fstpl 36(%esp)
	call _j0
	addl $8,%esp
	fldl 28(%esp)
	subl $8,%esp
	fstl (%esp)
	fstpl 36(%esp)
	fstpl 28(%esp)
	call _j1
	addl $8,%esp
	movl $1,%edx
	fldl 28(%esp)
	fldl 20(%esp)
	cmpl %ebx,%edx
	jge L21
	jmp L11
	.align 2,0x90
L22:
	fxch %st(2)
L11:
	pushl %edx
	fildl (%esp)
	addl $4,%esp
	fadd %st(0),%st
	fdiv %st(2),%st
	fmul %st(3),%st
	fsubp %st,%st(1)
	incl %edx
	cmpl %ebx,%edx
	jl L22
	fstp %st(1)
	fstp %st(1)
	popl %ebx
	addl $32,%esp
	ret
	.align 2,0x90
L21:
	fstp %st(0)
	fstp %st(0)
	popl %ebx
	addl $32,%esp
	ret
	.align 2,0x90
L7:
	fld %st(0)
	fmul %st(1),%st
	fldz
	leal 16(%ebx),%edx
	cmpl %ebx,%edx
	jle L23
	.align 2,0x90
L15:
	pushl %edx
	fildl (%esp)
	addl $4,%esp
	fadd %st(0),%st
	fsubp %st,%st(1)
	fld %st(1)
	fdivp %st,%st(1)
	decl %edx
	cmpl %ebx,%edx
	jg L15
L23:
	fstp %st(1)
	pushl %ebx
	fildl (%esp)
	addl $4,%esp
	fadd %st(0),%st
	fsubp %st,%st(1)
	fld %st(1)
	fdivp %st,%st(1)
	fld %st(0)
	fld1
	leal -1(%ebx),%edx
	testl %edx,%edx
	jle L24
	jmp L19
	.align 2,0x90
L25:
	fxch %st(1)
L19:
	pushl %edx
	fildl (%esp)
	addl $4,%esp
	fadd %st(0),%st
	fdiv %st(4),%st
	fmul %st(1),%st
	fsubp %st,%st(2)
	decl %edx
	testl %edx,%edx
	jg L25
	fstp %st(0)
	jmp L17
	.align 2,0x90
L24:
	fstp %st(1)
L17:
	fxch %st(2)
	subl $8,%esp
	fstpl (%esp)
	fxch %st(1)
	fstpl 20(%esp)
	fstpl 12(%esp)
	call _j0
	addl $8,%esp
	fldl 4(%esp)
	fmulp %st,%st(1)
	fldl 12(%esp)
	fdivrp %st,%st(1)
	popl %ebx
	addl $32,%esp
	ret
	.align 2,0x90
	.align 2
LC0:
	.double 0d-1.79769313486231570000e+308
	.align 2
.globl _yn
_yn:
	subl $12,%esp
	pushl %esi
	pushl %ebx
	movl 24(%esp),%ebx
	fldl 28(%esp)
	ftst
	fstp %st(0)
	fnstsw %ax
	sahf
	ja L27
	fldl LC0
	popl %ebx
	popl %esi
	addl $12,%esp
	ret
	.align 2,0x90
L27:
	movl $1,%esi
	testl %ebx,%ebx
	jge L28
	negl %ebx
	movl %ebx,%edx
	jns L30
	leal 1(%ebx),%edx
L30:
	andb $254,%dl
	movl %edx,8(%esp)
	movl %ebx,%edx
	subl 8(%esp),%edx
	cmpl $1,%edx
	jne L28
	movl $-1,%esi
L28:
	testl %ebx,%ebx
	jne L31
	pushl 32(%esp)
	pushl 32(%esp)
	call _y0
	addl $8,%esp
	popl %ebx
	popl %esi
	addl $12,%esp
	ret
	.align 2,0x90
L31:
	cmpl $1,%ebx
	jne L32
	pushl 32(%esp)
	pushl 32(%esp)
	call _y1
	addl $8,%esp
	pushl %esi
	fimull (%esp)
	addl $4,%esp
	popl %ebx
	popl %esi
	addl $12,%esp
	ret
	.align 2,0x90
L32:
	pushl 32(%esp)
	pushl 32(%esp)
	call _y0
	addl $8,%esp
	pushl 32(%esp)
	pushl 32(%esp)
	fstpl 20(%esp)
	call _y1
	addl $8,%esp
	movl $1,%edx
	fldl 12(%esp)
	cmpl %ebx,%edx
	jge L38
	jmp L36
	.align 2,0x90
L39:
	fxch %st(1)
L36:
	pushl %edx
	fildl (%esp)
	addl $4,%esp
	fadd %st(0),%st
	fdivl 28(%esp)
	fmul %st(2),%st
	fsubp %st,%st(1)
	incl %edx
	cmpl %ebx,%edx
	jl L39
	fstp %st(1)
	jmp L34
	.align 2,0x90
L38:
	fstp %st(0)
L34:
	pushl %esi
	fimull (%esp)
	addl $4,%esp
	popl %ebx
	popl %esi
	addl $12,%esp
	ret
	.align 2,0x90
