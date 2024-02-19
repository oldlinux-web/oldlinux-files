#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include "global.h"
#include "mbuf.h"
#include "netuser.h"
#include "timer.h"
#include "udp.h"
#include "cmdparse.h"
#include "domain.h"

void drx(),drx_init();
extern int errno;
extern int32 Ip_addr;

struct rr *Rrlist[NRLIST];
struct dserver *Dlist;		/* List of potential servers */
struct dserver *Dserver;	/* Current one being used */
char *Dsuffix;			/* Default suffix for names without periods */
struct socket Dsocket;		/* Socket to use for domain queries */
int Dsignal;
int Drx;			/* Drx started? */
int Ddebug = 0;
char *Dtypes[] = {
	"",
	"A",
	"NS",
	"MD",
	"MF",
	"CNAME",
	"SOA",
	"MB",
	"MG",
	"MR",
	"NULL",
	"WKS",
	"PTR",
	"HINFO",
	"MINFO",
	"MX",
	"TXT"
};
int Ndtypes = 17;
static char delim[] = " \t\r\n";
static struct {
	char *name;
	int32 address;
} cache;

struct cmds Dcmds[] = {
	"addserver",	doadds,		0, 0, NULLCHAR,
	"dropserver",	dodropds,	0, 0, NULLCHAR,
	"suffix",	dosuffix,	0, 0, NULLCHAR,
	"trace",	dodtrace,	0, 0, NULLCHAR,
	NULLCHAR,	NULLFP,		0, 0, "domain subcommands: addserver dropserver suffix trace",
};
int
dodtrace(argc,argv)
int argc;
char *argv[];
{
	if(argc < 2){
		printf("Domain trace: %s\n",Ddebug ? "On" : "Off");
	} else {
		if(strcmp(argv[1],"on") == 0)
			Ddebug = 1;
		else
			Ddebug = 0;
	}
	return 0;
}
int
dodomain(argc,argv,envp)
int argc;
char *argv[];
void *envp;
{
	return subcmd(Dcmds,argc,argv,envp);	
}
int
dosuffix(argc,argv)
int argc;
char *argv[];
{
	if(argc < 2){
		if(Dsuffix != NULLCHAR)
			printf("%s\n",Dsuffix);
		return 0;
	}
	free(Dsuffix);
	Dsuffix = strdup(argv[1]);
	return 0;
}
int
doadds(argc,argv)
int argc;
char *argv[];
{
	struct dserver *dp;
	int32 address;

	if((address = resolve(argv[1])) == 0){
		printf("Resolver %s unknown\n",argv[1]);
		return 1;
	}
	dp = (struct dserver *)calloc(1,sizeof(struct dserver));
	dp->address = address;
	dp->srtt = 5L;
	dp->timeout = dp->srtt * 2;
	dp->mdev = 0;
	dp->next = Dlist;
	if(dp->next != NULLDOM)
		dp->next->prev = dp;
	Dlist = dp;
	Dserver = dp;	/* Make this the first one we try next */
	if(Drx == 0){
		/* Start domain task upon first addserver */
		drx_init();
		Drx = 1;
	}
	return 0;
}
int
dodropds(argc,argv)
int argc;
char *argv[];
{
	struct dserver *dp;
	int32 addr;

	addr = resolve(argv[1]);
	for(dp = Dlist;dp != NULLDOM;dp = dp->next)
		if(addr == dp->address)
			break;

	if(dp == NULLDOM){
		printf("Not found\n");
		return 1;
	}
	if(dp->prev != NULLDOM)
		dp->prev->next = dp->next;
	else
		Dlist = dp->next;
	if(dp->next != NULLDOM)
		dp->next->prev = dp->prev;

	if(Dserver == dp)
		Dserver = Dlist;
	free((char *)dp);
	return 0;
}

/* Search local domain file for resource record of specified type.
 * If a record is found, the domain file pointer is left just after it. If
 * not, the file is rewound.
 */
static struct rr *
dfind(dbase,name,type)
FILE *dbase;
char *name;
int type;
{
	struct rr *rrp;
	int nlen;

	/* Search file */
	while((rrp = getrr(dbase)) != NULLRR){
		if((nlen = strlen(name)) == strlen(rrp->name)
		 && strncasecmp(name,rrp->name,nlen) == 0
		 && rrp->class == CLASS_IN
		 && rrp->type == type)
			break;
		free_rr(rrp);
/*		pwait(NULL);	/* Give up CPU for a while, this is slow */
	}
	if(rrp == NULLRR)
		rewind(dbase);
	return rrp;
}
static struct rr *
getrr(fp)
FILE *fp;
{
	char *line,*strtok();
	struct rr *rrp;
	char *name,*ttl,*class,*type,*data;
	int i;

	line = malloc(256);
	/* Search file */
	while(fgets(line,256,fp),!feof(fp)){
		if(line[0] != '#')
			break;
	}
	if(feof(fp) || (rrp = (struct rr *)calloc(1,sizeof(struct rr))) == NULLRR){
		free(line);
		return NULLRR;
	}
	name = strtok(line,delim);
	ttl = strtok(NULLCHAR,delim);
	class = strtok(NULLCHAR,delim);
	type = strtok(NULLCHAR,delim);
	data = strtok(NULLCHAR,delim);
	
	rrp->name = strdup(name);
	if(!isdigit(ttl[0])){
		/* Optional ttl field is missing; slide the other fields over */
		data = type;
		type = class;
		class = ttl;
		ttl = NULLCHAR;
	} else {
		rrp->ttl = atol(ttl);
	}
	for(i=0;i<NRLIST;i++){
		if(strcmp(type,Dtypes[i]) == 0){
			rrp->type = i;
			break;
		}
	}
	if(strcmp(class,"IN") == 0)
		rrp->class = CLASS_IN;

	if(data == NULLCHAR){
		/* Empty record, just return */
		free(line);
		return rrp;
	}
	switch(rrp->type){
	case TYPE_CNAME:
	case TYPE_MB:
	case TYPE_MG:
	case TYPE_MR:
	case TYPE_NS:
	case TYPE_PTR:
	case TYPE_TXT:
		rrp->rdlength = strlen(data);
		rrp->rdata.name = strdup(data);
		break;
	case TYPE_A:
		rrp->rdlength = 4;
		rrp->rdata.addr = aton(data);
		break;
	case TYPE_HINFO:
		rrp->rdlength = strlen(data);
		rrp->rdata.hinfo.cpu = strdup(data);
		if((data = strtok(NULLCHAR,delim)) != NULLCHAR){
			rrp->rdlength += strlen(data);
			rrp->rdata.hinfo.os = strdup(data);
		}
		break;
	case TYPE_MX:
		rrp->rdata.mx.pref = atoi(data);
		rrp->rdlength = 2;

		/* Get domain name of exchanger */
		if((data = strtok(NULLCHAR,delim)) != NULLCHAR){
			rrp->rdlength += strlen(data);
			rrp->rdata.mx.exch = strdup(data);
		}
		break;
	case TYPE_SOA:
		/* Get domain name of master name server */
		rrp->rdlength = strlen(data);
		rrp->rdata.soa.mname = strdup(data);

		/* Get domain name of irresponsible person */
		if((data = strtok(NULLCHAR,delim)) != NULLCHAR){
			rrp->rdata.soa.rname = strdup(data);
			rrp->rdlength += strlen(data);
		}
		data = strtok(NULLCHAR,delim);
		rrp->rdata.soa.serial = atol(data);
		data = strtok(NULLCHAR,delim);
		rrp->rdata.soa.refresh = atol(data);
		data = strtok(NULLCHAR,delim);
		rrp->rdata.soa.retry = atol(data);
		data = strtok(NULLCHAR,delim);
		rrp->rdata.soa.expire = atol(data);
		data = strtok(NULLCHAR,delim);
		rrp->rdata.soa.minimum = atol(data);
		rrp->rdlength += 20;
		break;
	}
	free(line);
	return rrp;
}
/* Search for address record in local database, looking first for PTR
 * and CNAME records. Return values:
 *  0xffffffff	Not found (domain name may exist, but we don't know yet)
 *  0		Domain name definitely doesn't exist (we have a null record)
 */
int32
dresolve(name)
char *name;
{
	register struct rr *rrp;
	char *pname = NULLCHAR;
	char *cname = NULLCHAR;
	int32 result;
	FILE *dbase;

	if(cache.name != NULLCHAR && strcmp(cache.name,name) == 0)
		return cache.address;

	if((dbase = fopen(Dfile,"r")) == NULLFILE)
		return 0xffffffff;

	/* This code can handle a few weird cases. It works when there's
	 * a PTR to a CNAME to an A record, as well as when there's a
	 * a CNAME to a PTR to an A. But it allows only one of each kind
	 * of indirection to prevent infinite loops.
	 */
	while((rrp = dfind(dbase,name,TYPE_A)) == NULLRR){
		/* An address record didn't exist, let's see if it's an alias */
		if(cname == NULLCHAR && (rrp = dfind(dbase,name,TYPE_CNAME)) != NULLRR){
			if((cname = strdup(rrp->rdata.name)) == NULLCHAR)
				break;
			name = cname;
			rewind(dbase);
			free_rr(rrp);
			continue;	/* Try again */
		}
		/* Lacking that, try a pointer entry... */
		if(pname == NULLCHAR && (rrp = dfind(dbase,name,TYPE_PTR)) != NULLRR){
			if((pname = strdup(rrp->rdata.name)) == NULLCHAR)
				break;
			name = pname;
			rewind(dbase);
			free_rr(rrp);
			continue;
		}
		/* Nope, nothing. Give up */
		break;
	}
	fclose(dbase);
	free(pname);
	free(cname);	

	if(rrp == NULLRR){
		result = 0xffffffff;		/* No record in database */
	} else {
		if(rrp->rdlength == 0)
			result = 0;		/* Negative response record */
		else
			result = rrp->rdata.addr;	/* Normal return */
		free(cache.name);
		cache.name = strdup(name);
		cache.address = result;
		free_rr(rrp);
	}
	return result;
}

/* Main entry point for domain name -> address resolution. Returns 0 if
 * name is definitely not valid.
 */
int32
resolve(name)
char *name;
{
	char *buf;
	int32 addr;
	struct dserver *dp;
	struct mbuf *bp;
	int len;
	struct socket server;
	char *tname = NULLCHAR;
	char *pname = NULLCHAR;
	time_t now, starttime, mainstart;
	if(name == NULLCHAR)
		return 0;

	if(*name == '[')
		return aton(name + 1);

	if(strchr(name,'.') == NULLCHAR && Dsuffix != NULLCHAR){
		/* Append default suffix */
		tname = malloc(strlen(name)+strlen(Dsuffix)+2);
		sprintf(tname,"%s.%s",name,Dsuffix);
		name = tname;
	}
	if(name[strlen(name)-1] != '.'){
		/* Append trailing dot */
		pname = malloc(strlen(name)+2);
		sprintf(pname,"%s.",name);
		name = pname;
	}
	dp = Dserver;
	time(&mainstart);
	while((addr = dresolve(name)) == 0xffffffff){
		if(dp == NULLDOM){
			addr = 0;	/* Unknown, and no servers */
			break;
		}
		/* Not in file, send query */

		if ((bp = alloc_mbuf(512)) == NULL) {
			addr = 0;	/* Unknown, and no servers */
			break;
		}
		len = res_mkquery(0,name,CLASS_IN,TYPE_A,NULLCHAR,
				  0,0,bp->data,512);
		bp->cnt = len;
		server.port = IPPORT_DOMAIN;
		server.address = dp->address;
		Dsignal = 0;
		send_udp(&Dsocket,&server,0,0,bp,len,0,0);

		time(&starttime);
		/* Wait for something to happen */
		for (;;) {
			keep_things_going();
			if (Dsignal)
				break;
			time(&now);
			if (now - starttime >= dp->timeout)
				break;
		}
		if (! Dsignal) {
			if (now - mainstart > 30) {
				addr = 0;
				break;
			} else {
			/* Timeout; back off this one and try another server */
				dp->timeout <<= 1;
				if((dp = dp->next) == NULLDOM)
					dp = Dlist;
			}
		} 
	}
quit:	free(tname);
	free(pname);
	return addr;
}
static int
res_mkquery(op,dname,class,type,data,datalen,newrr,buffer,buflen)
int op;	/* operation */
char *dname;	/* Domain name */
int class;	/* Class of inquiry (IN, etc) */
int type;	/* Type of inquiry (A, MX, etc) */
char *data;
int datalen;
int newrr;
char *buffer;	/* Area for query */
int buflen;	/* Length of same */
{
	char *cp,*cp1;
	int16 parameter;
	int16 dlen,len;
	time_t now;

	cp = buffer;
	time(&now);
	cp = put16(cp,now);	/* Use clock for timestamping */
	parameter = 0x100;	/* Recursion desired */
	cp = put16(cp,parameter);
	cp = put16(cp,1);
	cp = put16(cp,0);
	cp = put16(cp,0);
	cp = put16(cp,0);
	dlen = strlen(dname);
	for(;;){
		/* Look for next dot */
		cp1 = strchr(dname,'.');
		if(cp1 != NULLCHAR)
			len = cp1-dname;	/* More to come */
		else
			len = dlen;	/* Last component */
		*cp++ = len;		/* Write length of component */
		if(len == 0)
			break;
		/* Copy component up to (but not including) dot */
		strncpy(cp,dname,len);
		cp += len;
		if(cp1 == NULLCHAR){
			*cp++ = 0;	/* Last one; write null and finish */
			break;
		}
		dname += len+1;
		dlen -= len+1;
	}
	cp = put16(cp,type);
	cp = put16(cp,class);
	return cp - buffer;
}
/* Convert a compressed domain name to the human-readable form */
static int
dn_expand(msg,eom,compressed,full,fullen)
char *msg;		/* Complete domain message */
char *eom;
char *compressed;	/* Pointer to compressed name */
char *full;		/* Pointer to result buffer */
int fullen;		/* Length of same */
{
	unsigned int slen;	/* Length of current segment */
	register char *cp;
	unsigned int clen = 0;	/* Total length of compressed name */
	int indirect = 0;	/* Set if indirection encountered */
	int nseg = 0;		/* Total number of segments in name */

	cp = compressed;
	for(;;){
		slen = uchar(*cp++);	/* Length of this segment */
		if(!indirect)
			clen++;
		if((slen & 0xc0) == 0xc0){
			if(!indirect)
				clen++;
			indirect = 1;
			/* Follow indirection */
			cp = &msg[((slen & 0x3f)<<8) + uchar(*cp)];
			slen = uchar(*cp++);
		}
		if(slen == 0)	/* zero length == all done */
			break;
		fullen -= slen + 1;
		if(fullen < 0)
			return -1;
		if(!indirect)
			clen += slen;
		while(slen-- != 0)
			*full++ = *cp++;
		*full++ = '.';
		nseg++;
	}
	if(nseg == 0){
		/* Root name; represent as single dot */
		*full++ = '.';
		fullen--;
	}
	*full++ = '\0';
	fullen--;
	return clen;	/* Length of compressed message */
}

void
drx_init()
{
	Dsocket.address = ip_addr;
	Dsocket.port = IPPORT_DOMAIN;
	open_udp(&Dsocket, drx);
}

/* Process to receive all domain server replies */
void
drx(sock,cnt)
struct socket *sock;
int16 cnt;
{
	struct mbuf *bp;
	struct socket fsock;
	struct dserver *dp,*dslookup();
	int foo;

	recv_udp(sock,&fsock,&bp);
	if(Ddebug)
	printf("domain: from %s\n", psocket(&fsock));
	if((dp = dslookup(fsock.address)) == NULLDOM){
		/* Unknown server */
		if(Ddebug)
			printf("Unknown domain server!\n");
		return;
	}
	Dserver = dp;	/* We know this one is good */
	proc_answer(dp,bp);
}

static void
proc_answer(dp,bp)
struct dserver *dp;
struct mbuf *bp;
{
	FILE *fp;
	struct dhdr dhdr;
	int i;
	int16 rtt;
	long ttl = 500;	/* Default TTL for negative records without SOA */
	struct rr *rrp;
	struct quest *qp;
	time_t now;

	ntohdomain(&dhdr,&bp);

	/* Compute and update the round trip time */
	time(&now);
	rtt = now - dhdr.id;
	dp->srtt = (7 * dp->srtt + rtt) >> 3;
	dp->timeout = 2*dp->srtt;

	if(Ddebug){
		printf("response id %u (rtt %lu sec) qr %u opcode %u aa %u tc %u rd %u ra %u rcode %u\n",
		 dhdr.id,((long)rtt * MSPTICK)/1000,
		 dhdr.qr,dhdr.opcode,dhdr.aa,dhdr.tc,dhdr.rd,
		 dhdr.ra,dhdr.rcode);
		printf("%u questions:\n",dhdr.qdcount);
		for(i=0;i< dhdr.qdcount;i++){
			qp = dhdr.qlist[i];
			printf("%s type %u class %u\n",qp->qname,
			 qp->qtype,qp->qclass);
		}
	}
	if(dhdr.qr == QUERY){
		/* A server will eventually go here. */
		free_dhdr(&dhdr);
		return;
	}
	fp = fopen(Dfile,"r+");
	if (fp == NULLFILE)
		fp = fopen(Dfile,"w+");
	if(fp == NULLFILE){
		printf("Can't write %s!!\n",Dfile);
		free_dhdr(&dhdr);
		return;
	}
	if(Ddebug)
		printf("%u answers:\n",dhdr.ancount);
	for(i=0;i< dhdr.ancount;i++){
		rrp = dhdr.ans[i];
		if(Ddebug)
			putrr(stdout,rrp);
		if(rrp->type == TYPE_SOA)
			ttl = rrp->ttl;
		addit(fp,rrp);
	}
	if(Ddebug)
		printf("%u authority:\n",dhdr.nscount);
	for(i=0;i< dhdr.nscount;i++){
		rrp = dhdr.ns[i];
		if(Ddebug){
			putrr(stdout,rrp);
			fflush(stdout);
		}
		if(rrp->type == TYPE_SOA)
			ttl = rrp->ttl;
		addit(fp,rrp);
	}
	if(Ddebug)
		printf("%u additional:\n",dhdr.arcount);
	for(i=0;i< dhdr.arcount;i++){
		rrp = dhdr.add[i];
		if(Ddebug){
			putrr(stdout,rrp);
			fflush(stdout);
		}
		if(rrp->type == TYPE_SOA)
			ttl = rrp->ttl;
		addit(fp,rrp);
	}
	if(dhdr.aa && (dhdr.rcode == NAME_ERROR || dhdr.ancount == 0)){
		/* Add negative reply to file. This assumes that there was
		 * only one question, which is true for all questions we send.
		 */
		qp = dhdr.qlist[0];
		rrp = (struct rr *)calloc(1,sizeof(struct rr));
		rrp->name = strdup(qp->qname);
		rrp->type = qp->qtype;
		rrp->class = qp->qclass;
		rrp->ttl = ttl;
		rrp->rdlength = 0;	/* no data */
		addit(fp,rrp);
		free_rr(rrp);
	}
	fclose(fp);
	free_dhdr(&dhdr);
	Dsignal = 1;	/* Alert anyone waiting for results */
}
static int
ntohdomain(dhdr,bpp)
struct dhdr *dhdr;
struct mbuf **bpp;
{
	int16 tmp,len,i;
	char *msg,*cp;

	len = len_mbuf(*bpp);
	msg = malloc(len);
	pullup(bpp,msg,len);
	memset((char *)dhdr,0,sizeof(*dhdr));

        dhdr->id = get16(&msg[0]);
	tmp = get16(&msg[2]);
	if(tmp & 0x8000)
		dhdr->qr = 1;
	dhdr->opcode = (tmp >> 11) & 0xf;
	if(tmp & 0x0400)
		dhdr->aa = 1;
	if(tmp & 0x0200)
		dhdr->tc = 1;
	if(tmp & 0x0100)
		dhdr->rd = 1;
	if(tmp & 0x0080)
		dhdr->ra = 1;
	dhdr->rcode = tmp & 0xf;
	dhdr->qdcount = get16(&msg[4]);
	dhdr->ancount = get16(&msg[6]);
	dhdr->nscount = get16(&msg[8]);
	dhdr->arcount = get16(&msg[10]);

	/* Now parse the variable length sections */
	cp = &msg[12];

	/* Question section */
	if(dhdr->qdcount != 0)
		dhdr->qlist = (struct quest **)malloc(dhdr->qdcount *
		 sizeof(struct quest *));
	for(i=0;i<dhdr->qdcount;i++){
		dhdr->qlist[i] = (struct quest *)malloc(sizeof(struct quest));
		if((cp = getq(dhdr->qlist[i],msg,cp)) == NULLCHAR){
			free(msg);
			return -1;
		}
	}
	/* Answer section */
	if(dhdr->ancount != 0)
		dhdr->ans = (struct rr **)malloc(dhdr->ancount *
		 sizeof(struct rr *));
	for(i=0;i<dhdr->ancount;i++){
		dhdr->ans[i] = (struct rr *)malloc(sizeof(struct rr));
		if((cp = ntohrr(dhdr->ans[i],msg,cp)) == NULLCHAR){
			free(msg);
			return -1;
		}
	}		
	/* Name server (authority) section */
	if(dhdr->nscount != 0)
		dhdr->ns = (struct rr **)malloc(dhdr->nscount *
		 sizeof(struct rr *));
	for(i=0;i<dhdr->nscount;i++){
		dhdr->ns[i] = (struct rr *)malloc(sizeof(struct rr));
		if((cp = ntohrr(dhdr->ns[i],msg,cp)) == NULLCHAR){
			free(msg);
			return -1;
		}
	}
	/* Additional section */
	if(dhdr->arcount != 0)
		dhdr->add = (struct rr **)malloc(dhdr->arcount *
		 sizeof(struct rr *));
	for(i=0;i<dhdr->arcount;i++){
		dhdr->add[i] = (struct rr *)malloc(sizeof(struct rr));
		if((cp = ntohrr(dhdr->add[i],msg,cp)) == NULLCHAR){
			free(msg);
			return -1;
		}
	}
	free(msg);
}
static char *
getq(qp,msg,cp)
struct quest *qp;
char *msg;
char *cp;
{
	int len;
	char *name;

	name = malloc(512);
	len = dn_expand(msg,NULLCHAR,cp,name,512);
	if(len == -1){
		free(name);
		return NULLCHAR;
	}
	cp += len;
	qp->qname = strdup(name);
	qp->qtype = get16(cp);
	cp += 2;
	qp->qclass = get16(cp);
	cp += 2;
	free(name);
	return cp;
}
/* Read a resource record from a domain message into a host structure */
static char *
ntohrr(rrp,msg,cp)
struct rr *rrp;	/* Pointer to allocated resource record structure */
char *msg;	/* Pointer to beginning of domain message */
char *cp;	/* Pointer to start of encoded RR record */
{
	int len;
	char *name;

	if((name = malloc(512)) == NULLCHAR)
		return NULLCHAR;
	if((len = dn_expand(msg,NULLCHAR,cp,name,512)) == -1){
		free(name);
		return NULLCHAR;
	}
	cp += len;
	rrp->name = strdup(name);
	rrp->type = get16(cp);
	cp += 2;
	rrp->class = get16(cp);
	cp+= 2;
	rrp->ttl = get32(cp);
	cp += 4;
	rrp->rdlength = get16(cp);
	cp += 2;
	switch(rrp->type){
	case TYPE_CNAME:
	case TYPE_MB:
	case TYPE_MG:
	case TYPE_MR:
	case TYPE_NS:
	case TYPE_PTR:
		/* These types all consist of a single domain name;
		 * convert it to ascii format
		 */
		len = dn_expand(msg,NULLCHAR,cp,name,512);
		if(len == -1){
			free(name);
			return NULLCHAR;
		}
		rrp->rdata.name = strdup(name);
		cp += len;
		break;
	case TYPE_A:
		/* Just read the address directly into the structure */
		rrp->rdata.addr = get32(cp);
		cp += 4;
		break;
	case TYPE_HINFO:
		rrp->rdata.hinfo.cpu = strdup(cp);
		cp += strlen(cp) + 1;

		rrp->rdata.hinfo.os = strdup(cp);
		cp += strlen(cp) + 1;
		break;
	case TYPE_MX:
		rrp->rdata.mx.pref = get16(cp);
		cp += 2;
		/* Get domain name of exchanger */
		len = dn_expand(msg,NULLCHAR,cp,name,512);
		if(len == -1){
			free(name);
			return NULLCHAR;
		}
		rrp->rdata.mx.exch = strdup(name);
		cp += len;
		break;
	case TYPE_SOA:
		/* Get domain name of name server */
		len = dn_expand(msg,NULLCHAR,cp,name,512);
		if(len == -1){
			free(name);
			return NULLCHAR;
		}
		rrp->rdata.soa.mname = strdup(name);
		cp += len;

		/* Get domain name of responsible person */
		len = dn_expand(msg,NULLCHAR,cp,name,512);
		if(len == -1){
			free(name);
			return NULLCHAR;
		}
		rrp->rdata.soa.rname = strdup(name);
		cp += len;

		rrp->rdata.soa.serial = get32(cp);
		cp += 4;
		rrp->rdata.soa.refresh = get32(cp);
		cp += 4;
		rrp->rdata.soa.retry = get32(cp);
		cp += 4;
		rrp->rdata.soa.expire = get32(cp);
		cp += 4;
		rrp->rdata.soa.minimum = get32(cp);
		cp += 4;
		break;
	case TYPE_TXT:
		/* Just stash */
		rrp->rdata.data = malloc(rrp->rdlength);
		memcpy(rrp->rdata.data,cp,rrp->rdlength);
		cp += rrp->rdlength;
		break;
	default:
		/* Ignore */
		cp += rrp->rdlength;
		break;
	}
	free(name);
	return cp;
}
/* Print a resource record */
static void
putrr(fp,rrp)
FILE *fp;
struct rr *rrp;
{
	if(fp == NULLFILE || rrp == NULLRR)
		return;

	fprintf(fp,"%s\t%lu",rrp->name,rrp->ttl);
	if(rrp->class == CLASS_IN)
		fprintf(fp,"\tIN");
	else
		fprintf(fp,"\t%u",rrp->class);
	if(rrp->type < Ndtypes)
		fprintf(fp,"\t%s",Dtypes[rrp->type]);
	else
		fprintf(fp,"\t%u",rrp->type);
	if(rrp->rdlength == 0){
		/* Null data portion, indicates nonexistent record */
		fprintf(fp,"\n");
		return;
	}
	switch(rrp->type){
	case TYPE_CNAME:
	case TYPE_MB:
	case TYPE_MG:
	case TYPE_MR:
	case TYPE_NS:
	case TYPE_PTR:
	case TYPE_TXT:
		/* These are all printable text strings */
		fprintf(fp,"\t%s\n",rrp->rdata.data);
		break;
	case TYPE_A:
		fprintf(fp,"\t%s\n",inet_ntoa(rrp->rdata.addr));
		break;
	case TYPE_MX:
		fprintf(fp,"\t%u\t%s\n",rrp->rdata.mx.pref,
		 rrp->rdata.mx.exch);
		break;
	case TYPE_SOA:
		fprintf(fp,"\t%s\t%s\t%lu\t%lu\t%lu\t%lu\t%lu\n",
		 rrp->rdata.soa.mname,rrp->rdata.soa.rname,
		 rrp->rdata.soa.serial,rrp->rdata.soa.refresh,
		 rrp->rdata.soa.retry,rrp->rdata.soa.expire,
		 rrp->rdata.soa.minimum);
		break;
	default:
		fprintf(fp,"\n");
		break;
	}
}
/* Add a record to the database only if it doesn't already exist */
void
addit(fp,rrp1)
FILE *fp;
struct rr *rrp1;
{
	register struct rr *rrp;

	rewind(fp);
	while((rrp = dfind(fp,rrp1->name,rrp1->type)) != NULLRR){
		if(rrcmp(rrp,rrp1) == 0){
			free_rr(rrp);
			return;
		}
		free_rr(rrp);
	}
	/*
	 * following rewind should not be needed, since it's immediately
	 * followed by fseek.  The fseek sometimes goes the wrong place
	 * without it.  Buggy Gnu iostream.
	 */
	rewind(fp);
	fseek(fp,0L,2);
	putrr(fp,rrp1);
}
static struct dserver *
dslookup(server)
int32 server;
{
	struct dserver *dp;

	for(dp = Dlist;dp != NULLDOM;dp = dp->next)
		if(dp->address == server)
			break;
	return dp;
}
/* Free a domain message */
static void
free_dhdr(dp)
struct dhdr *dp;
{
	int i;

	if(dp->qdcount != 0){
		for(i=0;i<dp->qdcount;i++)
			free_qu(dp->qlist[i]);
		free((char *)dp->qlist);
	}
	if(dp->ancount != 0){
		for(i=0;i<dp->ancount;i++)
			free_rr(dp->ans[i]);
		free((char *)dp->ans);
	}
	if(dp->nscount != 0){
		for(i=0;i<dp->nscount;i++)
			free_rr(dp->ns[i]);
		free((char *)dp->ns);
	}
	if(dp->arcount != 0){
		for(i=0;i<dp->arcount;i++)
			free_rr(dp->add[i]);
		free((char *)dp->add);
	}
}

/* Free a question record */
static void
free_qu(qp)
struct quest *qp;
{
	free(qp->qname);
	free((char *)qp);
}

/* Free a resource record */
static void
free_rr(rrp)
struct rr *rrp;
{
	if(rrp == NULLRR)
		return;
	free(rrp->name);
	if(rrp->rdlength != 0){
		switch(rrp->type){
		case TYPE_CNAME:
		case TYPE_MB:
		case TYPE_MG:
		case TYPE_MR:
		case TYPE_NS:
		case TYPE_PTR:
			free(rrp->rdata.name);
			break;
		case TYPE_A:
			break;	/* Nothing allocated in rdata section */
		case TYPE_HINFO:
			free(rrp->rdata.hinfo.cpu);
			free(rrp->rdata.hinfo.os);
			break;
		case TYPE_MX:
			free(rrp->rdata.mx.exch);
			break;
		case TYPE_SOA:
			free(rrp->rdata.soa.mname);
			free(rrp->rdata.soa.rname);
			break;
		case TYPE_TXT:
			free(rrp->rdata.data);
			break;
		}
	}
	free((char *)rrp);
}
/* Compare two resource records, returning 0 if equal, nonzero otherwise */
static int
rrcmp(rr1,rr2)
register struct rr *rr1,*rr2;
{
	int i;

	if(rr1 == NULLRR || rr2 == NULLRR)
		return -1;
	if((i = strlen(rr1->name)) != strlen(rr2->name))
		return 1;
	if((i = strncasecmp(rr1->name,rr2->name,i)) != 0)
		return i;
	if(rr1->type != rr2->type)
		return 2;
	if(rr1->class != rr2->class)
		return 3;
	/* Note: rdlengths are not compared because they vary depending
	 * on the representation (ASCII or encoded) this record was
	 * generated from.
	 */
	switch(rr1->type){
	case TYPE_A:
		i = rr1->rdata.addr != rr2->rdata.addr;
		break;
	case TYPE_SOA:
		i = rr1->rdata.soa.serial != rr2->rdata.soa.serial;
		break;
	case TYPE_HINFO:
		i = strcmp(rr1->rdata.hinfo.cpu,rr2->rdata.hinfo.cpu) ||
			strcmp(rr1->rdata.hinfo.os,rr2->rdata.hinfo.os);
		break;
	case TYPE_MX:
		i = strcmp(rr1->rdata.mx.exch,rr2->rdata.mx.exch);
		break;
	case TYPE_MB:
	case TYPE_MG:
	case TYPE_MR:
	case TYPE_NULL:
	case TYPE_WKS:
	case TYPE_PTR:
	case TYPE_MINFO:
	case TYPE_TXT:
	case TYPE_NS:
		i = strcmp(rr1->rdata.data,rr2->rdata.data);
		break;
	case TYPE_MD:
	case TYPE_MF:
	case TYPE_CNAME:
		i = strcmp(rr1->rdata.data,rr2->rdata.data);
		break;
	}
	return i;
}
