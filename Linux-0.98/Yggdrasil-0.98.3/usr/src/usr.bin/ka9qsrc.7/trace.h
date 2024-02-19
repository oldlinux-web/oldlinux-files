/* Definitions for packet dumping */

/* List of tracing functions for each interface. Entries are placed in
 * this table by the driver at attach time
 */

extern int (*tracef[])();

#define	TRACE_AX25	0
#define	TRACE_ETHER	1
#define	TRACE_IP	2
#define TRACE_APPLETALK 3
#define	TRACE_SLFP	4
#define	NTRACE		5

#define	TRACE_SENT	0
#define	TRACE_RECV	1
#define	TRACE_LOOP	2
