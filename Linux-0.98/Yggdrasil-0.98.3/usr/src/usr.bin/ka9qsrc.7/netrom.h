/* net/rom support definitions
 * Copyright 1989 by Daniel M. Frank, W9NK.  Permission granted for
 * non-commercial distribution only.
 */

#define NR3HLEN		15	/* length of a net/rom level 3 hdr, */
#define NR3DLEN		241	/* max data size in net/rom l3 packet */
#define NR3NODESIG	0xff	/* signature for nodes broadcast */
#define NR3NODEHL	7	/* nodes bc header length */

#define NRNUMIFACE	10	/* number of interfaces associated */
				/* with net/rom network layer      */
#define NRNUMCHAINS	17	/* number of chains in the */
				/* neighbor and route hash tables */
#define NRRTDESTLEN	21	/* length of destination entry in */
				/* nodes broadcast */
#define NRDESTPERPACK	11	/* maximum number of destinations per */
				/* nodes packet */

/* Internal representation of net/rom network layer header */
struct nr3hdr {
	struct ax25_addr source ;	/* callsign of origin node */
	struct ax25_addr dest ;		/* callsign of destination node */
	unsigned ttl ;			/* time-to-live */
} ;

/* Internal representation of net/rom routing broadcast destination */
/* entry */
struct nr3dest {
	struct ax25_addr dest ;		/* destination callsign */
	char alias[7] ;			/* ident, upper case ASCII, blank-filled */
	struct ax25_addr neighbor ;	/* best-quality neighbor */
	unsigned quality ;		/* quality of route for this neighbor */
} ;


/* net/rom interface table entry */
struct nriface {
	struct interface *interface ;	/* pointer to ax.25 interface */
	char alias[7] ;			/* alias for this interface's node */
					/* broadcasts */
	unsigned quality ;		/* net/rom link quality estimate */
} ;

/* net/rom neighbor table structure */
struct nrnbr_tab {
	struct nrnbr_tab *next ;	/* doubly linked list pointers */
	struct nrnbr_tab *prev ;
	char call[AXALEN*3] ;		/* call of neighbor + 2 digis max */
	unsigned interface ;		/* offset of neighbor's port in */
					/* interface table */
	unsigned refcnt ;		/* how many routes for this neighbor? */
} ;

#define	NULLNTAB	(struct nrnbr_tab *)0


/* A list of these structures is provided for each route table */
/* entry.  They bind a destination to a neighbor node.  If the */
/* list of bindings becomes empty, the route table entry is    */
/* automatically deleted.									   */

struct nr_bind {
	struct nr_bind *next ;		/* doubly linked list */
	struct nr_bind *prev ;
	unsigned quality ;		/* quality estimate */
	unsigned obsocnt ;		/* obsolescence count */
	unsigned flags ;
#define	NRB_PERMANENT	0x01		/* entry never times out */
#define NRB_RECORDED	0x02		/* a "record route" entry */
	struct nrnbr_tab *via ;		/* route goes via this neighbor */
} ;

#define	NULLNRBIND	(struct nr_bind *)0


/* net/rom routing table entry */

struct nrroute_tab {
	struct nrroute_tab *next ;	/* doubly linked list pointers */
	struct nrroute_tab *prev ;
	char alias[7] ;			/* alias of node */
	struct ax25_addr call ;		/* callsign of node */
	unsigned num_routes ;		/* how many routes in bindings list? */
	struct nr_bind *routes ;	/* list of neighbors */

} ;

#define	NULLNRRTAB	(struct nrroute_tab *)0


/* The net/rom nodes broadcast filter structure */
struct nrnf_tab {
	struct nrnf_tab *next ;		/* doubly linked list */
	struct nrnf_tab *prev ;
	struct ax25_addr neighbor ;	/* call of neighbor to filter */
	unsigned interface ;		/* filter on this interface */
} ;

#define	NULLNRNFTAB	(struct nrnf_tab *)0


/* The interface table */
extern struct nriface nrifaces[NRNUMIFACE] ;

/* How many interfaces are in use */
extern unsigned nr_numiface ;

/* The neighbor hash table (hashed on neighbor callsign) */
extern struct nrnbr_tab *nrnbr_tab[NRNUMCHAINS] ;

/* The routes hash table (hashed on destination callsign) */
extern struct nrroute_tab *nrroute_tab[NRNUMCHAINS] ;

/* The nodes broadcast filter table */
extern struct nrnf_tab *nrnf_tab[NRNUMCHAINS] ;

/* filter modes: */
#define	NRNF_NOFILTER	0	/* don't filter */
#define	NRNF_ACCEPT	1	/* accept broadcasts from stations in list */
#define	NRNF_REJECT	2	/* reject broadcasts from stations in list */

/* The filter mode */
extern unsigned nr_nfmode ;

/* The time-to-live for net/rom network layer packets */
extern unsigned nr_ttl ;

/* The obsolescence count initializer */
extern unsigned obso_init ;

/* The threshhold at which routes becoming obsolete are not broadcast */
extern unsigned obso_minbc ;

/* The quality threshhold below which routes in a broadcast will */
/* be ignored */
extern unsigned nr_autofloor ;

/* The maximum number of routes maintained for a destination. */
/* If the list fills up, only the highest quality routes are  */
/* kept.  This limiting is done to avoid possible over-use of */
/* memory for routing tables in closely spaced net/rom networks. */
extern unsigned nr_maxroutes ;

/* Whether we want to broadcast the contents of our routing
 * table, or just our own callsign and alias:
 */
extern unsigned nr_verbose ;

/* The netrom pseudo-interface */
extern struct interface *nr_interface ;

/* Functions */
#if	(UNIX || ATARI_ST)
extern struct nrroute_tab *find_nrroute() ;
extern struct ax25_addr *find_nralias() ;
extern struct nrnbr_tab *find_nrnbr() ;
extern struct nrnf_tab *find_nrnf() ;
extern int nr_routeadd() ;
extern int nr_routedrop() ;
extern int nr_nfadd() ;
extern int nr_nfdrop() ;
extern char *nr_getroute() ;
extern int ntohnr3() ;
extern struct mbuf *htonnr3() ;
extern int ntohnrdest() ;
extern struct mbuf *htonnrdest() ;
#else
extern struct nrroute_tab *find_nrroute(struct ax25_addr *) ;
extern struct ax25_addr *find_nralias(char *) ;
extern struct nrnbr_tab *find_nrnbr(struct ax25_addr *, unsigned) ;
extern struct nrnf_tab *find_nrnf(struct ax25_addr *, unsigned) ;
extern int nr_routeadd(char *, struct ax25_addr *, unsigned,
					   unsigned, char *, unsigned, unsigned) ;
extern int nr_routedrop(struct ax25_addr *, struct ax25_addr *, unsigned) ;
extern int nr_nfadd(struct ax25_addr *, unsigned) ;
extern int nr_nfdrop(struct ax25_addr *, unsigned) ;
extern char *nr_getroute(struct ax25_addr *) ;
extern int ntohnr3(struct nr3hdr *,struct mbuf **) ;
extern struct mbuf *htonnr3(struct nr3hdr *) ;
extern int ntohnrdest(struct nr3dest *,struct mbuf **) ;
extern struct mbuf *htonnrdest(struct nr3dest *) ;
#endif
