#define HEARD_IP		0x01
#define HEARD_ARP		0x02
#define HEARD_NETROM	0x04

struct heard_stuff {
	struct ax25			info;
	unsigned long		htime;
	int16				flags;
	short				next;
};

#define MAX_HEARD 22  /* too many scrolls newest off screen */

struct ax25_heard {
	int16		enabled;
	int16		cnt;
	short		first;
	struct heard_stuff	list[MAX_HEARD];
};
