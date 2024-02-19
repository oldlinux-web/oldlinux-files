/* derived from: zoo.h 2.16 88/01/27 23:21:36 */

/*
The contents of this file are hereby released to the public domain.

                           -- Rahul Dhesi 1986/11/14
*/


/* Global data structures and also some information about archive structure.

Among other things, the archive header contains:  

(a) A text message.  In the MS-DOS version this message is terminated by
control Z.  This allows naive users to type the archive to the screen
and see a brief but meaningful message instead of garbage.  The contents of
the text message are however not used by Zoo and they may be anything.  
In particular, the text message may identify the type or archive or the
particular computer system it was created on.  When an archive is packed
by any version of Zoo, the text message is changed to the text message
used by that version.  For example, if Zoo 1.10 packs an archive created
by Zoo 1.31, the text message changes to "Zoo 1.10 archive.".  This
was once considered a shortcoming, but it is now an essential feature,
because packing will also update an old archiver header structure
into a new one.

(b) A four-byte tag that identifies all Zoo archives.  This helps prevent
arbitrary binary files from being treated as Zoo archives.  The tag value is
arbitrary, but seemed to be unlikely to occur in an executable file.  The
same tag value is used to identify each directory entry.  

(c) A long pointer to where in the file the archive starts.  This pointer
is stored along with its negation for consistency checking.  It is hoped
that if the archive is damaged, both the pointer and its negation won't
be damaged and at least one would still be usable to tell us where the 
data begins.

(d) A two-byte value giving the major and minor version number of the
minimum version of Zoo that is needed to fully manipulate the archive.  
As the archive structure is modified, this version number may increase.
Currently version 1.71 of Zoo creates archives that may be fully manipulated
by version 1.40 onwards.

(e) With zoo 2.00 addtional fields have been added in the archive
header to store information about the archive comment and generation
limit.

Version numbering:  
The directory entry of each file will contain the minimum version number of
Zoo needed to extract that file.  As far as possible, version 1.00 of Zoo
will be able to extract files from future version archives.
*/

#define H_TYPE	1				/* archive header type */

/* Define major and minor version numbers */
#define MAJOR_VER 2        /* needed to manipulate archive */
#define MINOR_VER 0

/* version needed to extract packing method 1 */
#define MAJOR_EXT_VER 1
#define MINOR_EXT_VER 0

/* version needed to extract packing method 2 */
#define MAJOR_LZH_VER	2
#define MINOR_LZH_VER	1

#define CTRL_Z 26

/* should be 0xFDC4A7DCUL but many c compilers don't recognize UL at end */
#define ZOO_TAG ((unsigned long) 0xFDC4A7DCL) /* A random choice */
#define TEXT "ZOO 2.10 Archive.\032"   /* Header text for archive. */
#define SIZ_TEXT  20                   /* Size of header text */

#define PATHSIZE 256                   /* Max length of pathname */
#define FNAMESIZE 13                   /* Size of DOS filename */
#define LFNAMESIZE 256                 /* Size of long filename */
#define ROOTSIZE 8                     /* Size of fname without extension */
#define EXTLEN 3                       /* Size of extension */
#define FILE_LEADER  "@)#("            /* Allowing location of file data */
#define SIZ_FLDR  5                    /* 4 chars plus null */
#define MAX_PACK 2                     /* max packing method we can handle */
#define BACKUP_EXT ".bak"              /* extension of backup file */

#ifdef OOZ
#define FIRST_ARG 2
#endif

#ifdef ZOO
#define FIRST_ARG 3        /* argument position of filename list */
#endif

typedef unsigned char uchar;

/* WARNING:  Static initialization in zooadd.c or zooext.c depends on the 
   order of fields in struct zoo_header */
struct zoo_header {
   char text[SIZ_TEXT];       /* archive header text */
   unsigned long zoo_tag;     /* identifies archives */
   long zoo_start;            /* where the archive's data starts */
   long zoo_minus;      		/* for consistency checking of zoo_start */
   uchar major_ver;
   uchar minor_ver;           /* minimum version to extract all files   */
	uchar type;						/* type of archive header */
	long acmt_pos;					/* position of archive comment */
	unsigned int acmt_len;		/* length of archive comment */
	unsigned int vdata;			/* byte in archive;  data about versions */
};

struct direntry {
   unsigned long zoo_tag;     /* tag -- redundancy check */
   uchar type;                 /* type of directory entry.  always 1 for now */
   uchar packing_method;       /* 0 = no packing, 1 = normal LZW */
   long next;                 /* pos'n of next directory entry */
   long offset;               /* position of this file */
   unsigned int date;         /* DOS format date */
   unsigned int time;         /* DOS format time */
   unsigned int file_crc;     /* CRC of this file */
   long org_size;
   long size_now;
   uchar major_ver;
   uchar minor_ver;            /* minimum version needed to extract */
   uchar deleted;              /* will be 1 if deleted, 0 if not */
   uchar struc;                /* file structure if any */
   long comment;              /* points to comment;  zero if none */
   unsigned int cmt_size; 		/* length of comment, 0 if none */
   char fname[FNAMESIZE]; 		/* filename */

   int var_dir_len;           /* length of variable part of dir entry */
   uchar tz;                   /* timezone where file was archived */
   unsigned int dir_crc;      /* CRC of directory entry */

   /* fields for variable part of directory entry follow */
   uchar namlen;               /* length of long filename */
   uchar dirlen;               /* length of directory name */
   char lfname[LFNAMESIZE];   /* long filename */
   char dirname[PATHSIZE];    /* directory name */
   unsigned int system_id;    /* Filesystem ID */
	unsigned long fattr;			/* File attributes -- 24 bits */
	unsigned int vflag;			/* version flag bits -- one byte in archive */
	unsigned int version_no;	/* file version number if any */
};

/* Values for direntry.system_id */
#define SYSID_NIX       0     /* UNIX and similar filesystems */
#define SYSID_MS        1     /* MS-DOS filesystem */
#define SYSID_PORTABLE  2     /* Portable syntax */

/* Structure of header of small archive containing just one file */

#define  TINYTAG     0x07FE   /* magic number */

#ifndef PORTABLE
struct tiny_header {          /* one-file small archive */
   int tinytag;               /* magic number */
   char type;                 /* always 1 for now */
   char packing_method;
   unsigned int date;
   unsigned int time;
   unsigned int file_crc;
   long org_size;
   long size_now;
   char major_ver;
   char minor_ver;
   unsigned int cmt_size; /* length of comment, 0 if none */
   char fname[FNAMESIZE];     /* filename */
};
#endif /* ifndef PORTABLE */

#define	FIXED_OFFSET 34		/* zoo_start in old archives */
#define	MINZOOHSIZ 34			/* minimum size of archive header */
#define  SIZ_ZOOH  42			/* length of current archive header */

/* offsets of items within the canonical zoo archive header */
#define  TEXT_I    0       	/* text in header */
#define  ZTAG_I    20      	/* zoo tag */
#define  ZST_I     24      	/* start offset */
#define  ZSTM_I    28      	/* negative of start offset */
#define  MAJV_I    32      	/* major version */
#define  MINV_I    33      	/* minor version */
#define	HTYPE_I	 34			/* archive header type */
#define	ACMTPOS_I 35			/* position of archive comment */
#define	ACMTLEN_I 39			/* length of archive comment */
#define	HVDATA_I	 41			/* version data */

/* offsets of items within the canonical directory entry structure */
#define  SIZ_DIR  51          /* length of type 1 directory entry */
#define  SIZ_DIRL 56          /* length of type 2 directory entry */
#define  DTAG_I   0           /* tag within directory entry */
#define  DTYP_I   4           /* type of directory entry */
#define  PKM_I    5           /* packing method */
#define  NXT_I    6           /* pos'n of next directory entry */
#define  OFS_I    10          /* position (offset) of this file */
#define  DAT_I    14          /* DOS format date */
#define  TIM_I    16          /* DOS format time */
#define  CRC_I    18          /* CRC of this file */
#define  ORGS_I   20          /* original size */
#define  SIZNOW_I 24          /* size now */
#define  DMAJ_I   28          /* major version number */
#define  DMIN_I   29          /* minor version number */
#define  DEL_I    30          /* deleted or not */
#define  STRUC_I  31          /* file structure */
#define  CMT_I    32          /* comment [offset] */
#define  CMTSIZ_I 36          /* comment size */
#define  FNAME_I  38          /* filename */
#define  VARDIRLEN_I  51      /* length of var. direntry */
#define  TZ_I     53          /* timezone */
#define  DCRC_I   54          /* CRC of directory entry */

#define  FNM_SIZ  13          /* size of stored filename */

/* Offsets within variable part of directory entry */
#define  NAMLEN_I   (SIZ_DIRL + 0)
#define  DIRLEN_I   (SIZ_DIRL + 1)
#define  LFNAME_I   (SIZ_DIRL + 2)
#define  DIRNAME_I  LFNAME_I  /* plus length of filename */

/*
Total size of fixed plus variable directory recognized currently:
One byte each for dirlen and namlen, 256 each for long filename and
directory name, 2 for system id, 3 for file attributes, 1 for 
version flag, 2 for version number, plus a fudge factor of 5.
*/
#define  MAXDIRSIZE  (SIZ_DIRL+1+1+256+256+2+3+1+2+5)

/* Value used to stuff into timezone field if it is not known */
#define  NO_TZ    127

/* Value for no file attributes */
#define	NO_FATTR		0L

/* version flag bits */
#define	VFL_ON	0x80		/* enable version numbering */
#define	VFL_GEN	0x0f		/* generation count */
#define	VFL_LAST 0x40		/* last generation of this file */

/* default generation value for archive */
#define	GEN_DEFAULT			3
/* max generation count, file or archive */
#define	MAXGEN				0x0f
/* version mask to prune down to correct size on large-word machines */
#define VER_MASK				0xffff
