	.file	"herrlist.c"
.globl _h_errlist
.text	1
LHC0:
	.ascii "Error 0\0"
LHC1:
	.ascii "Unknown host\0"
LHC2:
	.ascii "Host name lookup failure\0"
LHC3:
	.ascii "Unknown server error\0"
LHC4:
	.ascii "No address associated with name\0"
.text
	.align 2
_h_errlist:
	.long LHC0
	.long LHC1
	.long LHC2
	.long LHC3
	.long LHC4
	.space  0x80,0x90	# for future use
.globl _h_nerr
	.align 2
_h_nerr:
	.long 5
