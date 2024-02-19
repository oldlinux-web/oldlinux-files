	.file	"errlist.c"
.globl _sys_errlist
.text	1
LERRC0:
	.ascii "Unknown error\0"
LERRC1:
	.ascii "Operation not permitted\0"
LERRC2:
	.ascii "No such file or directory\0"
LERRC3:
	.ascii "No such process\0"
LERRC4:
	.ascii "Interrupted system call\0"
LERRC5:
	.ascii "I/O error\0"
LERRC6:
	.ascii "No such device or address\0"
LERRC7:
	.ascii "Arg list too long\0"
LERRC8:
	.ascii "Exec format error\0"
LERRC9:
	.ascii "Bad file number\0"
LERRC10:
	.ascii "No child processes\0"
LERRC11:
	.ascii "Try again\0"
LERRC12:
	.ascii "Out of memory\0"
LERRC13:
	.ascii "Permission denied\0"
LERRC14:
	.ascii "Bad address\0"
LERRC15:
	.ascii "Block device required\0"
LERRC16:
	.ascii "Device or resource busy\0"
LERRC17:
	.ascii "File exists\0"
LERRC18:
	.ascii "Cross-device link\0"
LERRC19:
	.ascii "No such device\0"
LERRC20:
	.ascii "Not a directory\0"
LERRC21:
	.ascii "Is a directory\0"
LERRC22:
	.ascii "Invalid argument\0"
LERRC23:
	.ascii "File table overflow\0"
LERRC24:
	.ascii "Too many open files\0"
LERRC25:
	.ascii "Not a typewriter\0"
LERRC26:
	.ascii "Text file busy\0"
LERRC27:
	.ascii "File too large\0"
LERRC28:
	.ascii "No space left on device\0"
LERRC29:
	.ascii "Illegal seek\0"
LERRC30:
	.ascii "Read-only file system\0"
LERRC31:
	.ascii "Too many links\0"
LERRC32:
	.ascii "Broken pipe\0"
LERRC33:
	.ascii "Math argument out of domain of func\0"
LERRC34:
	.ascii "Math result not representable\0"
LERRC35:
	.ascii "Resource deadlock would occur\0"
LERRC36:
	.ascii "File name too long\0"
LERRC37:
	.ascii "No record locks available\0"
LERRC38:
	.ascii "Function not implemented\0"
LERRC39:
	.ascii "Directory not empty\0"
LERRC40:
	.ascii "Too many symbolic links encountered\0"
LERRC41:
	.ascii "Operation would block\0"
LERRC42:
	.ascii "No message of desired type\0"
LERRC43:
	.ascii "Identifier removed\0"
LERRC44:
	.ascii "Channel number out of range\0"
LERRC45:
	.ascii "Level 2 not synchronized\0"
LERRC46:
	.ascii "Level 3 halted\0"
LERRC47:
	.ascii "Level 3 reset\0"
LERRC48:
	.ascii "Link number out of range\0"
LERRC49:
	.ascii "Protocol driver not attached\0"
LERRC50:
	.ascii "No CSI structure available\0"
LERRC51:
	.ascii "Level 2 halted\0"
LERRC52:
	.ascii "Invalid exchange\0"
LERRC53:
	.ascii "Invalid request descriptor\0"
LERRC54:
	.ascii "Exchange full\0"
LERRC55:
	.ascii "No anode\0"
LERRC56:
	.ascii "Invalid request code\0"
LERRC57:
	.ascii "Invalid slot\0"
LERRC58:
	.ascii "File locking deadlock error\0"
LERRC59:
	.ascii "Bad font file format\0"
LERRC60:
	.ascii "Device not a stream\0"
LERRC61:
	.ascii "No data available\0"
LERRC62:
	.ascii "Timer expired\0"
LERRC63:
	.ascii "Out of streams resources\0"
LERRC64:
	.ascii "Machine is not on the network\0"
LERRC65:
	.ascii "Package not installed\0"
LERRC66:
	.ascii "Object is remote\0"
LERRC67:
	.ascii "Link has been severed\0"
LERRC68:
	.ascii "Advertise error\0"
LERRC69:
	.ascii "Srmount error\0"
LERRC70:
	.ascii "Communication error on send\0"
LERRC71:
	.ascii "Protocol error\0"
LERRC72:
	.ascii "Multihop attempted\0"
LERRC73:
	.ascii "RFS specific error\0"
LERRC74:
	.ascii "Not a data message\0"
LERRC75:
	.ascii "Value too large for defined data type\0"
LERRC76:
	.ascii "Name not unique on network\0"
LERRC77:
	.ascii "File descriptor in bad state\0"
LERRC78:
	.ascii "Remote address changed\0"
LERRC79:
	.ascii "Can not access a needed shared library\0"
LERRC80:
	.ascii "Accessing a corrupted shared library\0"
LERRC81:
	.ascii ".lib section in a.out corrupted\0"
LERRC82:
	.ascii "Attempting to link in too many shared libraries\0"
LERRC83:
	.ascii "Cannot exec a shared library directly\0"
LERRC84:
	.ascii "Illegal byte sequence\0"
LERRC85:
	.ascii "Interrupted system call should be restarted\0"
LERRC86:
	.ascii "Streams pipe error\0"
LERRC87:
	.ascii "Too many users\0"
LERRC88:
	.ascii "Socket operation on non-socket\0"
LERRC89:
	.ascii "Destination address required\0"
LERRC90:
	.ascii "Message too long\0"
LERRC91:
	.ascii "Protocol wrong type for socket\0"
LERRC92:
	.ascii "Protocol not available\0"
LERRC93:
	.ascii "Protocol not supported\0"
LERRC94:
	.ascii "Socket type not supported\0"
LERRC95:
	.ascii "Operation not supported on transport endpoint\0"
LERRC96:
	.ascii "Protocol family not supported\0"
LERRC97:
	.ascii "Address family not supported by protocol\0"
LERRC98:
	.ascii "Address already in use\0"
LERRC99:
	.ascii "Cannot assign requested address\0"
LERRC100:
	.ascii "Network is down\0"
LERRC101:
	.ascii "Network is unreachable\0"
LERRC102:
	.ascii "Network dropped connection because of reset\0"
LERRC103:
	.ascii "Software caused connection abort\0"
LERRC104:
	.ascii "Connection reset by peer\0"
LERRC105:
	.ascii "No buffer space available\0"
LERRC106:
	.ascii "Transport endpoint is already connected\0"
LERRC107:
	.ascii "Transport endpoint is not connected\0"
LERRC108:
	.ascii "Cannot send after transport endpoint shutdown\0"
LERRC109:
	.ascii "Too many references: cannot splice\0"
LERRC110:
	.ascii "Connection timed out\0"
LERRC111:
	.ascii "Connection refused\0"
LERRC112:
	.ascii "Host is down\0"
LERRC113:
	.ascii "No route to host\0"
LERRC114:
	.ascii "Operation already in progress\0"
LERRC115:
	.ascii "Operation now in progress\0"
LERRC116:
	.ascii "Stale NFS file handle\0"
LERRC117:
	.ascii "Structure needs cleaning\0"
LERRC118:
	.ascii "Not a XENIX named type file\0"
LERRC119:
	.ascii "No XENIX semaphores available\0"
LERRC120:
	.ascii "Is a named type file\0"
LERRC121:
	.ascii "Remote I/O error\0"
	.text
	.align 2
_sys_errlist:
	.long LERRC0
	.long LERRC1
	.long LERRC2
	.long LERRC3
	.long LERRC4
	.long LERRC5
	.long LERRC6
	.long LERRC7
	.long LERRC8
	.long LERRC9
	.long LERRC10
	.long LERRC11
	.long LERRC12
	.long LERRC13
	.long LERRC14
	.long LERRC15
	.long LERRC16
	.long LERRC17
	.long LERRC18
	.long LERRC19
	.long LERRC20
	.long LERRC21
	.long LERRC22
	.long LERRC23
	.long LERRC24
	.long LERRC25
	.long LERRC26
	.long LERRC27
	.long LERRC28
	.long LERRC29
	.long LERRC30
	.long LERRC31
	.long LERRC32
	.long LERRC33
	.long LERRC34
	.long LERRC35
	.long LERRC36
	.long LERRC37
	.long LERRC38
	.long LERRC39
	.long LERRC40
	.long LERRC41
	.long LERRC42
	.long LERRC43
	.long LERRC44
	.long LERRC45
	.long LERRC46
	.long LERRC47
	.long LERRC48
	.long LERRC49
	.long LERRC50
	.long LERRC51
	.long LERRC52
	.long LERRC53
	.long LERRC54
	.long LERRC55
	.long LERRC56
	.long LERRC57
	.long LERRC58
	.long LERRC59
	.long LERRC60
	.long LERRC61
	.long LERRC62
	.long LERRC63
	.long LERRC64
	.long LERRC65
	.long LERRC66
	.long LERRC67
	.long LERRC68
	.long LERRC69
	.long LERRC70
	.long LERRC71
	.long LERRC72
	.long LERRC73
	.long LERRC74
	.long LERRC75
	.long LERRC76
	.long LERRC77
	.long LERRC78
	.long LERRC79
	.long LERRC80
	.long LERRC81
	.long LERRC82
	.long LERRC83
	.long LERRC84
	.long LERRC85
	.long LERRC86
	.long LERRC87
	.long LERRC88
	.long LERRC89
	.long LERRC90
	.long LERRC91
	.long LERRC92
	.long LERRC93
	.long LERRC94
	.long LERRC95
	.long LERRC96
	.long LERRC97
	.long LERRC98
	.long LERRC99
	.long LERRC100
	.long LERRC101
	.long LERRC102
	.long LERRC103
	.long LERRC104
	.long LERRC105
	.long LERRC106
	.long LERRC107
	.long LERRC108
	.long LERRC109
	.long LERRC110
	.long LERRC111
	.long LERRC112
	.long LERRC113
	.long LERRC114
	.long LERRC115
	.long LERRC116
	.long LERRC117
	.long LERRC118
	.long LERRC119
	.long LERRC120
	.long LERRC121
	.long 0
	.space  0x50,0x90	# for future use
.globl _sys_nerr
	.align 2
_sys_nerr:
	.long 122
