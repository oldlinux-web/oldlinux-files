#define BUFFERSIZE	20480
#define MAXENV		  100
#define NFD		    3

/* initial command from rsh to sherver */
#define EXEC_COMMAND	1

/* flags in h_extra for EXEC_COMMAND */
#define BFLG		0x01
#define EFLG		0x02
#define IFLG		0x04

/* commands from sherver to rsh (parameter (fd or status) in h_extra) */
#define READ_FD		1
#define WRITE_FD	2
#define EXIT_STATUS	4

/* replies */
#define COMM_DONE	0
#define COMM_ERROR	-1
