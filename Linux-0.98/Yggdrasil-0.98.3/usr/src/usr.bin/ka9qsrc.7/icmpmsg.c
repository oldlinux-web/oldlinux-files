#include "global.h"
/* ICMP message types */
char *icmptypes[] = {
	"Echo Reply",
	NULLCHAR,
	NULLCHAR,
	"Unreachable",
	"Source Quench",
	"Redirect",
	NULLCHAR,
	NULLCHAR,
	"Echo Request",
	NULLCHAR,
	NULLCHAR,
	"Time Exceeded",
	"Parameter Problem",
	"Timestamp",
	"Timestamp Reply",
	"Information Request",
	"Information Reply"
};

/* ICMP unreachable messages */
char *unreach[] = {
	"Network",
	"Host",
	"Protocol",
	"Port",
	"Fragmentation",
	"Source route"
};
/* ICMP Time exceeded messages */
char *exceed[] = {
	"Time-to-live",
	"Fragment reassembly"
};

/* ICMP redirect messages */
char *redirect[] = {
	"Network",
	"Host",
	"TOS & Network",
	"TOS & Host"
};

