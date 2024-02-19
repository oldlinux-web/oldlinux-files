#ifndef	NULLDOM

#define	NRLIST	17

#define	TYPE_A		1	/* Host address */
#define	TYPE_NS		2	/* Name server */
#define	TYPE_MD		3	/* Mail destination (obsolete) */
#define	TYPE_MF		4	/* Mail forwarder (obsolete) */
#define	TYPE_CNAME	5	/* Canonical name */
#define	TYPE_SOA	6	/* Start of Authority */
#define	TYPE_MB		7	/* Mailbox name (experimental) */
#define	TYPE_MG		8	/* Mail group member (experimental) */
#define	TYPE_MR		9	/* Mail rename name (experimental) */
#define	TYPE_NULL	10	/* Null (experimental) */
#define	TYPE_WKS	11	/* Well-known sockets */
#define	TYPE_PTR	12	/* Pointer record */
#define	TYPE_HINFO	13	/* Host information */
#define	TYPE_MINFO	14	/* Mailbox info */
#define	TYPE_MX		15	/* Mail exchanger */
#define	TYPE_TXT	16	/* Text strings */

#define IPPORT_DOMAIN	53

#define	CLASS_IN	1	/* The ARPA Internet */

struct dserver {
	struct dserver *prev;	/* Linked list pointers */
	struct dserver *next;

	int32 address;		/* IP address of server */
	int32 timeout;		/* Current timeout */
	int32 srtt;		/* Smoothed round trip time */
	int32 mdev;		/* Mean deviation */
};
#define	NULLDOM	(struct dserver *)0

/* Header for all domain messages */
struct dhdr {
	int16 id;		/* Identification */
	char qr;		/* Query/Response */
#define	QUERY		0
#define	RESPONSE	1
	char opcode;
#define	IQUERY		1
	char aa;		/* Authoratative answer */
	char tc;		/* Truncation */
	char rd;		/* Recursion desired */
	char ra;		/* Recursion available */
	char rcode;		/* Response code */
#define	NO_ERROR	0
#define	FORMAT_ERROR	1
#define	SERVER_FAIL	2
#define	NAME_ERROR	3
#define	NOT_IMPL	4
#define	REFUSED		5
	int16 qdcount;		/* Question count */
	int16 ancount;		/* Answer count */
	int16 nscount;		/* Authority (name server) count */
	int16 arcount;		/* Additional record count */
	struct quest **qlist;	/* List of questions */
	struct rr **ans;	/* List of answers */
	struct rr **ns;		/* List of name servers */
	struct rr **add;	/* List of additional records */
};
struct quest {
	char *qname;		/* Domain name being sought */
	int16 qtype;		/* Type (A, MX, etc) */
	int16 qclass;		/* Class (IN, etc) */
};

struct mx {
	int16 pref;
	char *exch;
};

struct hinfo {
	char *cpu;
	char *os;
};

struct soa {
	char *mname;
	char *rname;
	int32 serial;
	int32 refresh;
	int32 retry;
	int32 expire;
	int32 minimum;
};

struct rr {
	char *name;		/* Domain name, ascii form */
	int16 type;		/* A, MX, etc */
	int16 class;		/* IN, etc */
	int32 ttl;		/* Time-to-live */
	int16 rdlength;		/* Length of data field */
	union {
		int32 addr;		/* Used for type == A */
		struct soa soa;		/* Used for type == SOA */
		struct mx mx;		/* Used for type == MX */
		struct hinfo hinfo;	/* Used for type == HINFO */
		char *name;		/* for domain names */
		char *data;		/* for anything else */
	} rdata;
};
#define	NULLRR	(struct rr *)0
extern struct rr *Rrlist[NRLIST];

extern char *Dfile;

int doadds(),dodropds(),dosuffix(),dodtrace();

#if	defined(__STDC__) || defined(__TURBOC__)
static struct rr *dfind(FILE *dbase,char *name,int type);
static struct rr *getrr(FILE *fp);
int32 dresolve(char *name);
int32 resolve(char *name);
static int res_mkquery(int op,char *dname,int class,int type,
	char *data,int datalen,int newrr,char *buffer,int buflen);
static int dn_expand(char *msg,char *eom,char *compressed,char *full,int fullen);
void drx();
static void proc_answer(struct dserver *dp,struct mbuf *bp);
static int ntohdomain(struct dhdr *dhdr,struct mbuf **bpp);
static char *getq(struct quest *qp,char *msg,char *cp);
static char *ntohrr(struct rr *rrp,char *msg,char *cp);
static void putrr(FILE *fp,struct rr *rrp);
static void addit(FILE *fp,struct rr *rrp1);
static struct dserver *dslookup(int32 server);
static void free_dhdr(struct dhdr *dp);
static void free_qu(struct quest *qp);
static void free_rr(struct rr *rrp);
static int rrcmp(struct rr *rr1,struct rr *rr2);

#else
void drx();
int32 dresolve(),resolve();
static struct rr *dfind(),*getrr();
static int res_mkquery(),dn_expand(),ntohdomain(),rrcmp();
static char *getq(),*ntohrr();
static void putrr(),addit(),free_dhdr(),free_qu(),free_rr(),proc_answer();
static struct dserver *dslookup();

#endif
#endif	/* NULLDOM */
