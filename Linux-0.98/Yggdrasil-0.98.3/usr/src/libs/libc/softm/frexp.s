	.file	"frexp.c"
gcc2_compiled.:
.text
	.align 2
.globl _frexp
_frexp:
	movl 12(%esp),%edx
	testw $32752,10(%esp)
	jne L2
	movl $0,(%edx)
	jmp L3
	.align 2,0x90
L2:
	movw 10(%esp),%ax
	shrw $4,%ax
	andl $2047,%eax
	addl $-1022,%eax
	movl %eax,(%edx)
	andw $-32753,10(%esp)
	orw $16352,10(%esp)
L3:
	fldl 4(%esp)
	ret
	.align 2,0x90
