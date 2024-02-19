	.file	"fabs.c"
gcc2_compiled.:
.text
	.align 2
.globl _fabs
_fabs:
	fldl 4(%esp)
	ftst
	fnstsw %ax
	sahf
	jae L2
	fchs
L2:
	ret
	.align 2,0x90
