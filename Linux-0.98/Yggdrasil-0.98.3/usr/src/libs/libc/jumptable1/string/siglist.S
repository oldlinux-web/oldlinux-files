	.file	"siglist.c"
.globl _sys_siglist
.text	1
LSIGC0:
	.ascii "Unknown signal\0"
LSIGC1:
	.ascii "Hangup\0"
LSIGC2:
	.ascii "Interrupt\0"
LSIGC3:
	.ascii "Quit\0"
LSIGC4:
	.ascii "Illegal instruction\0"
LSIGC5:
	.ascii "Trace/breakpoint trap\0"
LSIGC6:
	.ascii "IOT trap/Abort\0"
LSIGC7:
	.ascii "Unused signal\0"
LSIGC8:
	.ascii "Floating point exception\0"
LSIGC9:
	.ascii "Killed\0"
LSIGC10:
	.ascii "User defined signal 1\0"
LSIGC11:
	.ascii "Segmentation fault\0"
LSIGC12:
	.ascii "User defined signal 2\0"
LSIGC13:
	.ascii "Broken pipe\0"
LSIGC14:
	.ascii "Alarm clock\0"
LSIGC15:
	.ascii "Terminated\0"
LSIGC16:
	.ascii "SIGSTKFLT\0"
LSIGC17:
	.ascii "Child exited\0"
LSIGC18:
	.ascii "Continued\0"
LSIGC19:
	.ascii "Stopped (signal)\0"
LSIGC20:
	.ascii "Stopped\0"
LSIGC21:
	.ascii "Stopped (tty input)\0"
LSIGC22:
	.ascii "Stopped (tty output)\0"
LSIGC23:
	.ascii "Possible I/O\0"
LSIGC24:
	.ascii "CPU time limit exceeded\0"
LSIGC25:
	.ascii "File size limit exceeded\0"
LSIGC26:
	.ascii "Virtual time alarm\0"
LSIGC27:
	.ascii "Profile signal \0"
LSIGC28:
	.ascii "Window changed\0"
LSIGC29:
	.ascii "File lock lost\0"
.text
	.align 2
_sys_siglist:
	.long LSIGC0
	.long LSIGC1
	.long LSIGC2
	.long LSIGC3
	.long LSIGC4
	.long LSIGC5
	.long LSIGC6
	.long LSIGC7
	.long LSIGC8
	.long LSIGC9
	.long LSIGC10
	.long LSIGC11
	.long LSIGC12
	.long LSIGC13
	.long LSIGC14
	.long LSIGC15
	.long LSIGC16
	.long LSIGC17
	.long LSIGC18
	.long LSIGC19
	.long LSIGC20
	.long LSIGC21
	.long LSIGC22
	.long LSIGC23
	.long LSIGC24
	.long LSIGC25
	.long LSIGC26
	.long LSIGC27
	.long LSIGC28
	.long LSIGC29
	.long LSIGC7
	.long LSIGC7
	.long 0
	.space	0x40,0x90	# for future use
