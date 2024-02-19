/*
 * pfdisk - Partition a Fixed DISK
 *	by Gordon W. Ross, Jan. 1990
 *
 * See the file "pfdisk.doc" for user instructions.
 *
 * This program uses a simple, line-oriented interpreter,
 * designed for both interactive and non-interactive use.
 * To facilitate non-interactive use, the output from the
 * 'L' (list partitions) command is carefully arranged so it
 * can be used directly as command input.  Neat trick, eh?
 */

char *versionString =
  "# pfdisk version 1.2.1 by Gordon W. Ross  Aug. 1990\nModified by S. Lubkin  Oct. 1991\n";

/* These don't really matter.  The user is asked to set them. */
#define DEFAULT_CYLS 306
#define DEFAULT_HEADS 4
#define DEFAULT_SECTORS 17
#define PROMPT_STRING "pfdisk> "

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "sysdep.h"
#include "syscodes.h"

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;

struct part {	/* An entry in the partition table */
  uchar	active;		/* active flag (0x80 or 0) */
  uchar	b_head;		/* begin head */
  uchar	b_sec;		/* 	 sector */
  uchar	b_cyl;		/*	 cylinder */
  uchar	sysid;		/* system id (see sysid.c) */
  uchar	e_head;		/* end  head */
  uchar	e_sec;		/* end	sector */
  uchar	e_cyl;		/* end	cylinder */
  /* These two are just longs, but this way is machine independent. */
  /* uchar	lsBeg[4];	/* logical sectors, beginning Saul */
  ulong	lsBeg;	/* logical sectors, beginning Saul */
  /* uchar	lsLen[4];	/* logical sectors, length Saul */
  ulong	lsLen;	/* logical sectors, length Saul */
};

#define LOC_PT		0x1BE
#define LOC_NT		0x1AA /* Saul */
/* #define LOC_NT		0x180 Saul */
/* #define LOC_GWR		0x1A0 Saul */
#define LOC_GWR		0x1A9 /* Saul */
#define MAGIC_LOC	0x1FE
#define MAGIC_0		0x55
#define MAGIC_1		0xAA
#define MAX_LINE	80
#define NT_ENTRY_SIZE	5 /* Saul */
/* Note:  Entry in "printf" command, should be manually changed, to
"%-NT_ENTRY_SIZE.NT_ENTRY_SIZEs" Saul */
/* And header printf line should have blanks adjusted Saul */

char s[22];   /* For holding error string */
char	buffer[SECSIZE];	/* The boot block buffer */
int	bufmod=0;		/* buffer modified... */
		/* (zero means buffer is unmodified) */
int	useNTable;		/* boot sector uses name table */

/* device parameters (force someone to set them!) */
unsigned cyls = DEFAULT_CYLS;
unsigned heads = DEFAULT_HEADS;
unsigned sectors = DEFAULT_SECTORS;

char	*devname;		/* device name */
char	cmdline[MAX_LINE];
char	filename[80];		/* used by r/w commands */
char	*prompt;		/* null if no tty input */

/* Some of these strings are used in more than one place.
 * For consistency, I put a newline on all of them.
 */
char h_h[] = "? <enter>             : Help summary\n";
char h_l[] = "L                     : List partition table\n";
char h_1[] = "1 id first last [name]: set partition 1\n";
char h_2[] = "2,3,4 ... (like 1)    : set respective partition\n";
char h_a[] = "A n [m, ...]          : Activate partition(s) n [m, ...]\n";
char h_g[] = "G cyls heads sectors  : set disk Geometry\n";
char h_i[] = "I                     : list known ID numbers\n";
char h_r[] = "R [optional-file]     : Read  device (or specified file)\n";
char h_w[] = "W [optional-file]     : Write device (or specified file)\n";
char h_q[] = "Q[!]                  : Quit (! means force)\n";

char * helpTable[] = {
h_h, h_l, h_1, h_2, h_a, h_g, h_i, h_r, h_w, h_q,
"# (All command letters have lower-case equivalents.)\n",
(char *) 0 }; /* This MUST have a zero as the last element */

char *BadArg="Error: bad argument: %s\n";
char *WarnNotSaved =
	"Warning, modified partition table not saved.\n";

help()
{
  char ** p;
  for (p = helpTable; *p; p++)
    printf(*p);
}

/* forward declarations */
void	checkValidity();
char *	setPartition();
char *	makeActive();
char *	setGeometry();
ulong	chs2long();
char *	nameID();
int	printIDs();

main(argc,argv)
int	argc;
char	*argv[];
{
  char	*cmdp;		/* points to command word */
  char	*argp;		/* points to command args */

  /* check command line args (device name) */
  if (argc != 2) {
    usage(argv[0]);	/* See s-sysname.c */
    exit(1);
  }
  devname = argv[1];

  /* Should we prompt? */
  prompt = (isatty(fileno(stdin))) ? PROMPT_STRING : (char *) 0;

  /* Print version name. */
  fputs(versionString, stderr);

  /* get disk parameters */
  getGeometry(devname,&cyls,&heads,&sectors);

  /* Get the boot block. */
  if (getBBlk(devname, buffer) < 0)
    fprintf(stderr,"%s: read failed\n", devname);
  checkValidity();

  if (prompt) fprintf(stderr,"For help, enter: '?'\n");


  /* Read and process commands a line at a time. */
  while (1) {
    if (prompt) fputs(prompt,stdout);
    if (! fgets(cmdline, MAX_LINE, stdin)) break;

    /* Find beginning of command word */
    cmdp = cmdline;
    while (isspace(*cmdp)) cmdp++;

    /* find beginning of args */
    argp = cmdp;
    while (*argp && !isspace(*argp)) argp++;
    while (isspace(*argp) || *argp=='=') argp++;

    switch (*cmdp) {

    case '\0':		/* blank line */
    case '#':		/* line comment */
      break;

    case '?': case 'h': case 'H':
      help();
      break;

    case '1':	/* set partition entry */
    case '2': case '3': case '4':
      argp = setPartition(cmdp, argp);
      if (argp) {	/* arg list error */
	fprintf(stderr,BadArg,argp);
	fprintf(stderr,h_1);
	fprintf(stderr,h_2);
	break;
      }
      bufmod = 1;
      break;

    case 'a': case 'A':	/* activate partition */
      argp = makeActive(argp);
      if (argp) {
	fprintf(stderr,BadArg,argp);
	fprintf(stderr,h_a);
	break;
      }
      bufmod = 1;
      break;

    case 'g': case 'G':	/* set disk parameters (Geometry) */
      argp = setGeometry(argp);
      if (argp) {	/* arg list error */
	fprintf(stderr,BadArg,argp);
	fprintf(stderr,h_g);
      }
      break;

    case 'i': case 'I':	/* List known ID numbers */
      printIDs();
      break;

    case 'l': case 'L':	/* List the partition table */
      listPTable();
      break;

    case 'q': case 'Q':	/* Quit */
      if (bufmod && (cmdp[1]  != '!')) {
	fprintf(stderr,"\007%s%s\n", WarnNotSaved,
		"Use 'wq' or 'q!' (enter ? for help).");
	break;
      }
      exit(0);
      /*NOTREACHED*/

    case 'r': case 'R':	/* read from device or file */
      if (sscanf(argp,"%80s",filename) == 1) {
	/* Arg specified, read from filename */
	if (getFile(filename, buffer, SECSIZE) < 0)
	  fprintf(stderr,"%s: read failed\n", filename);
	bufmod = 1;
      } else {
	/* No arg, use device. */
	if (getBBlk(devname, buffer) < 0)
	  fprintf(stderr,"%s: read failed\n", devname);
	bufmod = 0;
      }
      checkValidity();
      break;

    case 'w': case 'W':	/* Write to file or device */
      if (sscanf(argp,"%80s",filename) == 1) {
	/* Arg specified, write to filename */
	if (putFile(filename, buffer, SECSIZE) < 0)
	  fprintf(stderr, "%s: write failed\n", filename);
      } else {  /* No arg, use device. */
	if (putBBlk(devname, buffer) < 0)
	  fprintf(stderr, "%s: write failed\n", devname);
	bufmod = 0;
      }
      if (cmdp[1] == 'q' || cmdp[1] == 'Q') exit(0);
      break;

    default:
      fprintf(stderr,"'%c': unrecognized.  Enter '?' for help.\n", *cmdp);
      break;

    } /* switch */
  } /* while */
  if (bufmod) fprintf(stderr, WarnNotSaved);
  exit(0);
} /* main */


/* Check for valid boot block (magic number in last two bytes).
 * Also, check for presence of partition name table.
 */
void checkValidity()
{
  /* Check the magic number. */
  if ((buffer[MAGIC_LOC] & 0xFF) != MAGIC_0 ||
      (buffer[MAGIC_LOC+1] & 0xFF) != MAGIC_1 ) {
    /* The boot sector is not valid -- Fix it. */
    buffer[MAGIC_LOC] = MAGIC_0;
    buffer[MAGIC_LOC+1] = MAGIC_1;
    bufmod = 1;
    fprintf(stderr,
"\n\tWarning:  The boot sector has an invalid magic number.\n\
\tThe magic number has been fixed, but the other contents\n\
\tare probably garbage.  Initialize using the command:\n\
\t\tR boot-program-file	(i.e. bootmenu.bin)\n\
\tthen set each partition entry if necessary.\n");
  }

  /* Does it use a name table (for a boot menu)?
   * My boot program does, and can be identified by
   * finding my name in a particular (unused) area.
   */
  useNTable = ( buffer[LOC_GWR] == (char)0x3A ); /* Saul */
  /* useNTable = !strcmp(&buffer[LOC_GWR], "Gordon W. Ross"); Saul */

}

char * setPartition(cmdp,argp)	/* return string on error */
char	*cmdp,*argp;
{
  struct part *pp;	/* partition entry */
  char *	np;		/* name table pointer */
  char          tmpname[20];
  char *	newname = tmpname;	/* name field */
  int	index;		/* partition index (0..3) */
  uint	id;		/* ID code (see syscodes.c) */
  uint	first,last;	/* user supplied cylinders */
  uint	c,h,s;		/* working cyl,head,sect, */
  int	i;		/* returned by sscanf */
  ulong	lsbeg, lslen;	/* logical begin, length */

  /* Value check the index */
  index = *cmdp - '1';
  if (index < 0 || index > 3)
    return("index");
  pp = (struct part *) &buffer[LOC_PT + index * 16];
  np = &buffer[LOC_NT + index * NT_ENTRY_SIZE]; /* Saul */
  /* np = &buffer[LOC_NT + index * 8]; Saul */

  /* Read System ID */
  if ((i=sscanf(argp,"%d%d%d%s", &id, &first, &last, newname)) < 1)
    return("id");

  /* If ID==0, just clear out the entry and return. */
  if (id == 0) {
    strncpy( (char *) pp, "", 16);
    if (useNTable) strncpy( np, "", NT_ENTRY_SIZE); /* Saul */
    /* if (useNTable) strncpy( np, "", 8); Saul */
    return((char *)0);
  }

  /* Read first and last cylinder */
  if (i < 3)
    return("first last (missing)");

  /* Reasonable start,end cylinder numbers? */
  if (first > last)	return("first > last");
  if (first > 1023)	return("first > 1023");
  if (last >= cyls)	return("last >= cyls");

  /* Get (optional) system name. */
  if (i == 3) {	/* no name given, use default */
    newname = nameID(id);
  }
    else useNTable = 1;

  /* Set the ID and name. */
  pp->sysid = id;
  if (useNTable) {
    strncpy(np, newname, NT_ENTRY_SIZE); /* Saul */
    /* strncpy(np, newname, 8); Saul */
    /* strcpy(&buffer[LOC_GWR], "Gordon W. Ross"); Saul */
    buffer[LOC_GWR] = (char)0x3A; /* Saul */
  }

  /* set beginning c,h,s */
  c = first;
  /* if c == 0, head == 1 (reserve track 0) */
  h = (first) ? 0 : 1;
  s = 1;
  pp->b_cyl = c & 0xFF;
  pp->b_head = h;
  pp->b_sec = s | ((c >> 2) & 0xC0);
  /* Set the logical sector begin field */
  lsbeg = lslen = chs2long(c,h,s); /* using lslen as temp. */
  /* pp->lsBeg[0] = lslen & 0xff; lslen >>= 8;
  pp->lsBeg[1] = lslen & 0xff; lslen >>= 8;
  pp->lsBeg[2] = lslen & 0xff; lslen >>= 8;
  pp->lsBeg[3] = lslen & 0xff; lslen >>= 8; Saul */
  pp->lsBeg = lslen; /* Saul */

  /* set ending c,h,s (last may be larger than 1023) */
  c = (last>1023) ? 1023 : last; /* limit c to 1023 */
  h = heads - 1; s = sectors;
  pp->e_cyl = c & 0xFF;
  pp->e_head = h;
  pp->e_sec = s | ((c >> 2) & 0xC0);
  /* Set the logical sector length field (using REAL end cylinder) */
  lslen = chs2long(last,h,s) + 1 - lsbeg;
  /* pp->lsLen[0] = lslen & 0xff; lslen >>= 8;
  pp->lsLen[1] = lslen & 0xff; lslen >>= 8;
  pp->lsLen[2] = lslen & 0xff; lslen >>= 8;
  pp->lsLen[3] = lslen & 0xff; lslen >>= 8; Saul */
  pp->lsLen = lslen; /* Saul */

  return((char *)0);	/* success */
} /* setPartition() */

char * makeActive(argp)	/* return error string or zero */
char	*argp;
{
  struct part *pp;	/* partition entry */
  int	i,act1,act2,act3,act4,act5;		/* which one becomes active */

  act1=0;
  act2=0;
  act3=0;
  act4=0;
  if ((i=sscanf(argp,"%d%d%d%d%d", &act1, &act2, &act3, &act4, &act5)) < 1)
    return("missing partition number");
  if ( i > 4)
    return("at most four partition numbers");
  act1--;			/* make it zero-origin */
  act2--;			/* make it zero-origin */
  act3--;			/* make it zero-origin */
  act4--;			/* make it zero-origin */

  i=0; pp = (struct part *) &buffer[LOC_PT];
  while (i<4) {
    if (pp->sysid == 0 && (i == act1|| i == act2 || i == act3 || i == act4)) {
         sprintf(s, "partition %d empty", i+1);
         return(s);
         }
    i++; pp++;
  }
  i=0; pp -= 4;
  while (i<4) {
    if (i == act1|| i == act2 || i == act3 || i == act4)
      pp->active = 0x80;
    else
      pp->active = 0;
    i++; pp++;
  }
  return((char *)0);
}

char * setGeometry(argp)	/* return string on error */
char	*argp;
{
  int	c,h,s;

  if (sscanf(argp,"%d%d%d", &c, &h, &s) < 3)
    return("(missing)");
  if (c<1) return("cyls");
  if (h<1) return("heads");
  if (s<1) return("sectors");
  cyls=c; heads=h; sectors=s;
  return((char *)0);
}

listPTable()		/* print out partition table */
{
  struct part * pp;	/* partition table entry */
  char	*name;
  int	i;		/* partition number */
  /* int	numActive=0;	/* active partition [1-4], 0==none */
  char Active[20]; /* active partitions [1-4], 0==none */
  uint	pbc,pbh,pbs;	/* physical beginning  c,h,s */
  uint	pec,peh,pes;	/* physical ending     c,h,s */
  uint	lbc,lbh,lbs;	/* logical beginning   c,h,s */
  uint	lec,leh,les;	/* logical ending      c,h,s */
  ulong	lsbeg,lslen;	/* logical sectors: begin, length */

  strcpy(Active, "active:");
  printf("# Partition table on device: %s\n", devname);
  printf("geometry %d %d %d (cyls heads sectors)\n",
	 cyls, heads, sectors);
  /* printf("#  ID  First(cyl)  Last(cyl)  Name     "); Saul */
  printf("#  ID  First(cyl)  Last(cyl)  Name  "); /* Saul */
  printf("# start, length (sectors)\n");

  for (i=0; i<4; i++) {
    pp = (struct part *) &buffer[LOC_PT + i * 16];

    if (pp->active) {
      char s[3];
      sprintf(s, " %d", i+1);
      strcat(Active,s);
	  if (pp->active != 0x80)
	  	fprintf(stderr, "Warning:  Partition %d is active, with the illegal activity byte %d.\nCorrect with the \"A\" command.\n", i+1, pp->active);
   /* if(numActive)
	fprintf(stderr,"Error: multiple active partitions.\n");
      else numActive = i+1; */
    }

    /* physical beginning c,h,s */
    pbc = pp->b_cyl & 0xff | (pp->b_sec << 2) & 0x300;
    pbh = pp->b_head;
    pbs = pp->b_sec & 0x3F;

    /* physical ending c,h,s */
    pec = pp->e_cyl & 0xff | (pp->e_sec << 2) & 0x300;
    peh = pp->e_head;
    pes = pp->e_sec & 0x3F;

    /* compute logical beginning (c,h,s) */
    /* lsbeg = ((((((pp->lsBeg[3] ) << 8 )
		| pp->lsBeg[2] ) << 8 )
		| pp->lsBeg[1] ) << 8 )
		| pp->lsBeg[0] ; Saul */
	lsbeg = pp->lsBeg; /* Saul */
    long2chs(lsbeg, &lbc, &lbh, &lbs);
    /* compute logical ending (c,h,s) */
    /* lslen = ((((((pp->lsLen[3]) << 8 )
		| pp->lsLen[2]) << 8 )
		| pp->lsLen[1]) << 8 )
		| pp->lsLen[0] ; Saul */
	lslen = pp->lsLen; /* Saul*/
    /* keep beginning <= end ... */
    if (lslen > 0) long2chs(lsbeg+lslen-1, &lec, &leh, &les);
    else	   long2chs(lsbeg,	   &lec, &leh, &les);

    if (useNTable)
      name = &buffer[LOC_NT + i * NT_ENTRY_SIZE ]; /* Saul */
      /* name = &buffer[LOC_NT + i * 8]; Saul */
    else
      name = nameID((uint) pp->sysid);

    /* show physical begin, logical end (works for cyl>1023) */
    /*      #  ID  First(cyl)  Last(cyl)  Name... # ... */
    /* printf("%d %3d   %4d       %4d       %-8.8s # %ld, %ld\n",  Saul */
    printf("%d %3d   %4d       %4d       %-5.5s # %ld, %ld\n", /* Saul */
	   i+1, pp->sysid, pbc, lec, name, lsbeg, lslen );

    /* That's all, for an empty partition. */
    if (pp->sysid == 0) continue;

    /*
     * Now do some consistency checks...
     */

    /* Same physical / logical beginning? */
    if (pbc != lbc || pbh != lbh || pbs != lbs ) {
      printf("# note: first(%d): ", i+1);
      printf("phys=(%d,%d,%d) ",    pbc, pbh, pbs);
      printf("logical=(%d,%d,%d)\n",lbc, lbh, lbs);
    }
    /* Same physical / logical ending? */
    if (pec != lec || peh != leh || pes != les ) {
      printf("# note:  last(%d): ", i+1);
      printf("phys=(%d,%d,%d) ",    pec, peh, pes);
      printf("logical=(%d,%d,%d)\n",lec, leh, les);
    }

    /* Beginning on cylinder boundary? */
    if (pbc == 0) { /* exception: start on head 1 */
      if (pbh != 1 || pbs != 1) {
	printf("# note: first(%i): ", i+1);
	printf("phys=(%d,%d,%d) ", pbc, pbh, pbs);
	printf("should be (%d,1,1)\n", pbc);
      }
    } else { /* not on cyl 0 */
      if (pbh != 0 || pbs != 1) {
	printf("# note: first(%i): ", i+1);
	printf("phys=(%d,%d,%d) ", pbc, pbh, pbs);
	printf("should be (%d,0,1)\n", pbc);
      }
    }

    /* Ending on cylinder boundary? */
    if (peh != (heads-1) || pes != sectors) {
      printf("# note: last(%i): ", i+1);
      printf("phys=(%d,%d,%d) ", pec, peh, pes);
      printf("should be (%d,%d,%d)\n",
	     pec, heads-1, sectors);
    }

  } /* for */
  if ( !Active[7] ) /* No active partitions */
	strcat(Active, " 0 (none)");
  strcat(Active, "\n");
  printf(Active);
/* printf("active: %d  %s\n", numActive,
	 (numActive) ? "" : "(none)"); */
} /* listPTable() */

ulong chs2long(c,h,s)
uint c,h,s;
{
  ulong	l;
  if (s<1) s=1;
  l  = c; l *= heads;
  l += h; l *= sectors;
  l += (s - 1);
  return(l);
}

long2chs(ls, c, h, s)	/* convert logical sec-num to c,h,s */
ulong	ls;		/* Logical Sector number */
uint	*c,*h,*s;	/* cyl, head, sector */
{
  int	spc = heads * sectors;
  *c = ls / spc;
  ls = ls % spc;
  *h = ls / sectors;
  *s = ls % sectors + 1;	/* sectors count from 1 */
}

char * nameID(n)
unsigned int n;
{
  struct intString *is;

  is = sysCodes;
  while (is->i) {
    if (is->i == n) return(is->s);
    is++;
  }
  if (!n) return(is->s);
  return("unknown");
}

int printIDs()		/* print the known system IDs */
{
  struct intString * is = sysCodes;

  /* This might need to do more processing eventually, i.e.
   * if (prompt) { ... do more processing ... }
   */
  printf("_ID_\t__Name__ ____Description____\n");
  while (is->i) {
    printf("%3d\t%s\n", is->i, is->s);
    is++;
  }
}
