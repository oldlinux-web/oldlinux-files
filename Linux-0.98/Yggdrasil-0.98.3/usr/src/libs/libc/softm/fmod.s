	.file	"fmod.c"
gcc2_compiled.:
.text
	.align 2
.globl _fmod
_fmod:
	fldl 4(%esp)
	ftst
	fstp %st(0)
	fnstsw %ax
	sahf
	je L3
	fldl 12(%esp)
	ftst
	fstp %st(0)
	fnstsw %ax
	sahf
	jne L2
L3:
	fldl 4(%esp)
	ret
	.align 2,0x90
L2:
	fldl 4(%esp)
	fdivl 12(%esp)
	ftst
	fnstsw %ax
	sahf
	jae L4
	subl $8,%esp
	fstpl (%esp)
	call _ceil
	addl $8,%esp
	fmull 12(%esp)
	fsubrl 4(%esp)
	ret
	.align 2,0x90
L4:
	subl $8,%esp
	fstpl (%esp)
	call _floor
	addl $8,%esp
	fmull 12(%esp)
	fsubrl 4(%esp)
	ret
	.align 2,0x90
