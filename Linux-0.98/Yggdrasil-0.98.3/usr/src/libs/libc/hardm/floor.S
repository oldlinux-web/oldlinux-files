	.file	"floor.c"
gcc2_compiled.:
.text
#ifndef NO_SHARED_LIBRARY
	.org	0x10,0x90
#endif
	.align 2,0x90
.globl _floor
_floor:
	subl $4,%esp
	fldl 8(%esp)
/APP
	fnstcw 2(%esp)
/NO_APP
	movw 2(%esp),%ax
	andb $243,%ah
	orb $4,%ah
LC1:
	movw %ax,(%esp)
/APP
	fldcw (%esp)
	frndint
	fldcw 2(%esp)
/NO_APP
	addl $4,%esp
	ret
	.align 2,0x90
	.org	0x74,0x90
	.align 2
.globl _ceil
_ceil:
	subl $4,%esp
	fldl 8(%esp)
/APP
	fnstcw 2(%esp)
/NO_APP
	movw 2(%esp),%ax
	andb $243,%ah
	orb $8,%ah
	jmp	LC1
	.align 2,0x90
