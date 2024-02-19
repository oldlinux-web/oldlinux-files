	.file	"stdstrbufs.cc"
gcc2_compiled.:
.globl __cin_sbuf
.data
	.align 2
__cin_sbuf:
	.long -72499064
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.word 0
	.space 2
	.long __vt$filebuf
	.word 0
	.space 10
	.space	0x100		# For future use
.globl __cout_sbuf
	.align 2
__cout_sbuf:
	.long -72499068
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long __cin_sbuf
	.long 0
	.long 0
	.long 0
	.long 0
	.word 0
	.space 2
	.long __vt$filebuf
	.word 1
	.space 10
	.space	0x100		# For future use
.globl __cerr_sbuf
	.align 2
__cerr_sbuf:
	.long -72499068
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long __cout_sbuf
	.long 0
	.long 0
	.long 0
	.long 0
	.word 0
	.space 2
	.long __vt$filebuf
	.word 2
	.space 10
	.space	0x100		# For future use
