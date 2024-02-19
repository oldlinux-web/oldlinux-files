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
.globl _not_open_filebuf
	.align 2
_not_open_filebuf:
	.long -72499060
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
	.word -1
	.space 10
	.space	0x100		# For future use
.globl __9streambuf$_list_all
	.align 2
__9streambuf$_list_all:
	.long __cerr_sbuf
