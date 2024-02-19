To: Linux-Development-System@senator-bedfellow.MIT.EDU
From: rjc@snoopy.apana.org.au
Subject: hpfs.h replacement
Date: 4 Mar 1995 09:52:26 GMT
Reply-To: rjc@snoopy.apana.org.au

A friend of mine who was writing some software that needed to access HPFS
partitions used the hpfs.h file from Linux for the start of his development.
After getting hiss software working he sent me a modified version of the file
hpfs.h which apparently contains some information missed in the version that
is in Linux.  Here it is for anyone who wishes to work on it:

/*
        ZDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBD?
        CDEDEDADADADADADADADADADADADADADADADADADADADADADADADADADADADADEDED4
        CDED4   PROJECT      : <generic module>                       CDED4
        CDED4   FILE         : HPFSINFO.H - HPFS and DISK STRUCTURES  CDED4
        CDED4   Last modified: 27 Jun 94                              CDED4
        CDEDEDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDBDEDED4
        @DADADADADADADADADADADADADADADADADADADADADADADADADADADADADADADADADY

    HPFSINFO.H - Include File
*/

/* ------------------------------------------------------------------------ */
/* History                                                                  */
/* ------------------------------------------------------------------------ */
/*
    1.00    27/06/94    Initial Version
*/
/* ------------------------------------------------------------------------ */
/* Include files                                                            */
/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */
/* Definitions                                                              */
/* ------------------------------------------------------------------------ */

#pragma pack(1)

#define SUPER_BLOCK 0x10L       /* The Superblock sector */
#define SPARE_BLOCK 0x11L       /* The Spareblock sector */

#define HPFS_PART   7           /* The .PartitionType reserver for HPFS */
                                /* And HPFS386.                         */

#define DOWN        (-1)
#define SAME        0
#define UP          1

/* ------------------------------------------------------------------------ */
/* Structures                                                               */
/* ------------------------------------------------------------------------ */

typedef ULONG   LSN ;       /* Relative Sector Number */
typedef USHORT  WORD ;

typedef struct _PARTITIONENTRY
    {
    BYTE    PartitionStatus ;
    BYTE    BeginHead ;
    WORD    BeginCylSect ;
    BYTE    PartitionType ;
    BYTE    EndHead ;
    WORD    EndCylSect ;
    LSN     PartitionOffset ;
    ULONG   PartitionSize ;
    } PARTITIONENTRY, FAR *PPARTITIONENTRY ;

/*
    The *CylSect are decoded as follows:

    |  Byte N       |  Byte N + 1   |
    ---------------------------------
    |C|C|S|S|S|S|S|S|C|C|C|C|C|C|C|C|
    ---------------------------------
     MSB MSB     LSB             LSB

    When this is mapped as a WORD it is:

    ---------------------------------
    |C|C|C|C|C|C|C|C|C|C|S|S|S|S|S|S|
    ---------------------------------
                 LSB MSB MSB     LSB
*/
#define EXTRACTSECTOR( W )      ( W & 0x3F )
#define EXTRACTCYLINDER( W )    ( ( ( W & 0xC0 ) << 2 ) | ( W >> 8 ) )

typedef struct _VOLUMEBOOTSECTOR
    {
    BYTE            BootCode[0x1BE];
    PARTITIONENTRY  Partitions[4];
    USHORT          Signature;
    } VOLUMEBOOTSECTOR, FAR *PVOLUMEBOOTSECTOR ;

typedef struct _PARTITIONBOOTSECTOR
    {
    BYTE    JMP[3];
    BYTE    OEM[8];
    USHORT  BytesPerSector;
    BYTE    SectorsPerCluster;
    USHORT  ReservedSectors;
    BYTE    NumOfFATs;
    USHORT  NumRootDirEntries;
    USHORT  SectorsInMedia;
    BYTE    MediaDescriptor;
    USHORT  SectorsIn1FAT;
    USHORT  SectorsInTrack;
    USHORT  NumOfHeads;
    ULONG   NumOfHiddenSectors;
    ULONG   LongSectorsInMedia;
    BYTE    BootCode[0x1de];
    } PARTITIONBOOTSECTOR, FAR *PPARTITIONBOOTSECTOR ;


/* ------------------------------------------------------------------------ */
/* Most of these are from the LINUX sources                                 */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/* Super Block Sector 16 (0x10)                                             */
/* ------------------------------------------------------------------------ */

#define SB_MAGIC1   0xF995E849L
#define SB_MAGIC2   0xFA53E9C5L

typedef struct _SUPERBLOCK
    {
    ULONG   ulMagic1 ;              /* 49, e8, 95, f9                       */
    ULONG   ulMagic2 ;              /* c5, e9, 53, fa                       */
    ULONG   ulUnknown ;             /* 02, 02, 00, 00 Version # ?           */
    LSN     ulRootDirFnode;         /* sector number of root dir FNODE      */
    ULONG   ulSectors ;             /* size of file system                  */
    ULONG   ulBadBlocks ;           /* number of bad blocks                 */
    LSN     ulBitmaps ;             /* sector number of free space bitmap lists */
    ULONG   ulZero1 ;               /*                                      */
    LSN     ulBadBlockList ;        /* sector number of bad block list      */
    ULONG   ulZero2 ;               /*                                      */
    ULONG   ulLastChkDsk ;          /* DATE/TIME of the last CHKDSK /F (GMT)*/
    ULONG   ulZero3 ;               /*                                      */
    ULONG   ulDirBandSize ;         /* no of sectors in dir band            */
    LSN     ulDirBandStart ;        /* first sector in dir band             */
    LSN     ulDirBandEnd ;          /* last sector in dir band              */
    LSN     ulDirBandBitmap ;       /* sector no of free space bitmap, 1 bit per DIRCLUSTER (DNODE) */
    ULONG   ulZero4[8] ;            /*                                      */
    LSN     ulScratchNodes ;        /* ?? 8 preallocated sectors near dir band */
    BYTE    byZero5[412] ;          /*                                      */
    } SUPERBLOCK, FAR *PSUPERBLOCK ;

/* ------------------------------------------------------------------------ */
/* Spare Block Sector 17 (0x11)                                             */
/* ------------------------------------------------------------------------ */

#define SP_MAGIC1   0xF9911849L
#define SP_MAGIC2   0xFA5229C5L

typedef struct _SPAREBLOCK
    {
    ULONG   ulMagic1 ;              /* 49, 18, 91, f9                       */
    ULONG   ulMagic2 ;              /* c5, 29, 52, fa                       */
    ULONG   ulDirtyFlag ;           /*                                      */
    LSN     ulHotFixMap ;           /* Info about remapped bad sectors      */
    ULONG   ulSpareUsed ;           /* number of hot fixes                  */
    ULONG   ulSpare ;               /* number of spares in hotfix map       */
    ULONG   ulSpareDirClusterFree ; /* spare DIRECTORYCLUSTERs unused       */
    ULONG   ulSpareDirCluster ;     /* number of SpareDirClusters[] list    */
    LSN     ulCodePage ;            /* code page directory block            */
    ULONG   ulCodePageNumber ;      /* the number of code pages             */
    ULONG   ulUnknown1 ;            /* ??                                   */
    ULONG   ulUnknown2 ;            /* ??                                   */
    ULONG   ulZero1[15] ;           /* all zeroes                           */
    LSN     ulSpareDirClusters[20] ;/* list of sectors that form the spare DIRCLUSTERS areas */
    ULONG   ulZero2[81] ;           /* all zerors                           */
    } SPAREBLOCK, FAR *PSPAREBLOCK ;

#define PARTITION_DIRTY (1L)
#define PARTITION_CLEAN (0L)

/* ------------------------------------------------------------------------ */
/* Bitmap pointer                                                           */
/* ------------------------------------------------------------------------ */
/*
    SUPERBLOCK.ulBitmaps is a sector that contains an array of sector
    numbers that consist of the actual starting sector number of the
    bitmap sectors.

    NOTE: One sector only allows 128*8Mb = 1Gb of disk space. If the disk
    is greater than 1Gb it does flow onto the next sector.
    The list is terminated by a zero entry.
*/

typedef struct _BITMAPBLOCK
    {
    LSN     ulStartBitmapSector[128] ;  /* Each bitmap block is 4 sectors long */
    } BITMAPBLOCK, FAR *PBITMAPBLOCK ;

/* ------------------------------------------------------------------------ */
/* Bad Block Information                                                    */
/* ------------------------------------------------------------------------ */
/*
    Bad Block

    Bad block lists are 4 sectors long. The initial bad block list is
    pointed to by SUPERBLOCK.ulBadBlockList. If the first block of 4
    sectors is overflowed, another block of 4 is allocated, and the
    first entry (DWORD) points to the next sector.
*/

typedef struct _BADBLOCKLIST
    {
    LSN     ulNextSector ;              /* 0 if no more, or next block of 4 */
    LSN     ulBadSector[511] ;          /* 0 - 510                          */
    } BADBLOCKLIST, FAR *PBADBLOCKLIST ;

/* ------------------------------------------------------------------------ */
/* Dynamic Hot Fix Information                                              */
/* ------------------------------------------------------------------------ */
/*
    Hot Fix

    The hotfix map is 4 sectors long. It is pointed to by
    SPAREBLOCK.ulHotFixMap. It looks like:

    LSN ulFrom[SPAREBLOCK.ulSpare]
    LSN ulTo[SPAREBLOCK.ulSpare]

    The To[] list is initialized to point to SPAREBLOCK.ulSpare preallocated
    empty sectors. The From[] list contains the sector numbers of bad blocks
    which have been remapped to corresponding sectors in the To[] list.
    SPAREBLOCK.ulSpareUsed gives the length of the From[] list.
*/

typedef struct _HOTFIXLIST
    {
    LSN     ulSector[512] ;
    } HOTFIXLIST, FAR *PHOTFIXLIST ;

/* ------------------------------------------------------------------------ */
/* Code Page Information                                                    */
/* ------------------------------------------------------------------------ */
/*
    Code Page

    The code page info is pointed to by the spare block. It consists of an
    index block and blocks containing character maps.

    The following is pretty vague.
*/
#define CP_DIR_MAGIC    0x494521F7L

typedef struct _CODEPAGEDIR
    {
    ULONG   ulMagic ;               /* F7, 21, 45, 49                       */
    ULONG   ulCodePageCount ;       /* number of pointers following         */
    ULONG   ulZero1[2] ;            /*                                      */
    struct _CODEPAGEENTRY
        {
        USHORT  usIndex ;           /* Index                                */
        USHORT  usCodePageNumber ;  /* Code Page Number                     */
        ULONG   ulBounds ;          /* Matches corresponding work in data block */
        LSN     ulCodePageData ;    /* sector no of a CODEPAGEDATA c.p. array   */
        ULONG   ulIndex ;           /* Index in c.p. array in that sector   */
        } CODEPAGEENTRY[31] ;       /* Unknown length                       */
    } CODEPAGEDIR, FAR *PCODEPAGEDIR ;

/*
    Code Page Data

These are pointed to by a CODEPAGEDIR structure.
*/
#define CP_DATA_MAGIC   0x894521F7L

typedef struct _CODEPAGEDATA
    {
    ULONG   ulMagic ;               /* F7, 21, 45, 89                       */
    ULONG   ulUsed ;                /* # elements used in c_p_data[]        */
    ULONG   ulBounds[3] ;           /* looks like (beg1,end1), (beg2,end2)  */
    USHORT  usOffsets[3] ;          /* offsets from start of sector to start of c_p_data[usIndex] */
    struct _CODEPAGE
        {
        USHORT  usIndex ;           /* Index                                */
        USHORT  usCodePageNumber ;  /* Code Page Number                     */
        USHORT  usZero1 ;           /*                                      */
        BYTE    byMap[128] ;        /* map for chars 80..FF                 */
        USHORT  usZero2 ;           /*                                      */
        } CODEPAGE[3] ;
    BYTE    byUnknown[78] ;
    } CODEPAGEDATA, FAR *PCODEPAGEDATA ;

/* ------------------------------------------------------------------------ */
/* Free Space Bitmap Information                                            */
/* ------------------------------------------------------------------------ */
/*
    Free space bitmaps are 4 sectors long, which is 16384 bits. 16384 sectors
    is 8Mb, and each 8Mb band has a 4 sector bitmap. Bit order in the maps
    is little-endian. 0 means taken, 1 means free.

    The four sectors are arranged as an array of 0..16383 bits. Each bit
    represents a sector.

    Bit map sectors are marked allocated in the bit maps, and so are sectors
    off the end of the partition.

    Band 0 is sectors 0000-3fff, its map is in sectors 0018-001b
    Band 1 is sectors 4000-7fff, its map is in sectors 7ffc-7fff
    Band 2 is sectors 8000-ffff, its map is in sectors 8000-8003

    The remaining bands have maps in their first (even) or last (odd) 4
    sectors. If the last, partial band is odd its map is in its last
    4 sectors.

    The bitmap locations are given in a table pointed to by the super block.
    SUPERBLOCK.ulBitmaps is the sector that holds the table of pointers
    that point to the actual free space bitmap sectors.
    No doubt they aren't constrained to be at 0018, 7ffc, 8000, ... ; that
    is just where they usually are.

    The "directory band" is a bunch of sectors preallocated for DIRCLUSTERS
    (DNODES). It has a 4 sector free space bitmap of its own. Each bit in
    the map corresponds to one 4 sector DIRCLUSTER, bit 0 of the map
    corresponding to the first 4 sectors of the directory band. The entire
    band is marked allocated in the main bitmap. The super block gives the
    locations of the main directory band and its bitmap. ("band" doesn't
    mean it is 8Mb long; it isn't).
*/

#define SECTOR_USED 0
#define SECTOR_FREE 1

/* ------------------------------------------------------------------------ */
/* Directory Entry Definitions                                              */
/* ------------------------------------------------------------------------ */
/*
    Flags used in the DIRENTRY.usFlags
*/
#define First       0x0001          /* set for ^A^A (".") entry             */
#define Flag2       0x0002          /*                                      */
#define Down        0x0004          /* down pointer present (after Name)    */
#define Last        0x0008          /* set on phony \377 entry              */
#define Flag5       0x0010          /*                                      */
#define Flag6       0x0020          /*                                      */
#define Flag7       0x0040          /*                                      */
#define Need_EA     0x0080          /* has NEEDEA flag set                  */
#define READ_ONLY   0x0100          /* DOS style attribute                  */
#define HIDDEN      0x0200          /* DOS style attribute                  */
#define SYSTEM      0x0400          /* DOS style attribute                  */
#define Flag12      0x0800          /* Would be VOLUME label attribute      */
#define DIRECTORY   0x1000          /* DOS style attribute                  */
#define ARCHIVE     0x2000          /* DOS style attribute                  */
#define NOT_8x3     0x4000          /* name is NOT 8.3                      */
#define Flag16      0x8000          /*                                      */

#define DIR_SPLITS  Down            /* For compatibility with other progs   */
#define END_OF_DIR  Last
#define FLAG_DIR    DIRECTORY

typedef struct _DIRENTRY
    {
    USHORT  uscDirEntrySize;        /* offset to next DIRENTRY              */
    USHORT  usFlags ;               /* see above                            */
    LSN     ulPointedSector;        /* FNODE giving allocation info         */
    ULONG   ulDate1;                /* Last Write                           */
    ULONG   ulcFileLength;          /* File length in bytes                 */
    ULONG   ulDate2;                /* Last Access                          */
    ULONG   ulDate3;                /* Creation                             */
    ULONG   ulcEALength ;           /* EA length in bytes                   */
    BYTE    Zero ;
    BYTE    byLocality ;            /* ??? 0=unk, 1=seq, 2=random, 3=both   */
    BYTE    bNameLength;            /* 1E                                   */
    BYTE    bName[1];               /* 1F   Variable length item            */
    } DIRENTRY, FAR *PDIRENTRY ;    /*      Formal length 0x20 bytes        */

/* ------------------------------------------------------------------------ */
/* DNODE Definitions                                                        */
/* ------------------------------------------------------------------------ */
/*
    DIRECTORY CLUSTERS or DNODES

    They are 4 sectors long.

    A directory is a tree of DNODES. There is one pointer to the root of
    the tree, in the FNODE for the directory. Every DNODE in the tree
    points up to this node, and the parent directory points down to it.
    The FNODE never moves, the DNODES do the B-tree thing, splitting
    and merging as files are added and removed.
*/

#define DNODE_MAGIC         0x77E40AAEL
#define DELETED_DNODE_MAGIC 0x77E40A00L

typedef struct _DNODE
    {
    ULONG   ulMagic ;               /* AE, 0A, E4, 77                       */
    ULONG   ulFirstFree ;           /* offset from start of DNODE to first free dir entry */
    ULONG   ulUsageCount ;          /* Is this used?                        */
    LSN     ulParentSector ;        /* Parent                               */
    LSN     ulMySelf ;              /* pointer to this sector               */
    BYTE    DirEntry[2028] ;        /* one or more DIRENTRY's               */
    } DNODE, FAR *PDNODE ;

/* ------------------------------------------------------------------------ */
/* B+ tree: allocation info in FNODEs and ANODEs                            */
/* ------------------------------------------------------------------------ */
/*
    DNODES point to fnodes which are responsible for listing the sectors
    assigned to the file.  This is done with trees of (length,address)
    pairs.  (Actually triples, of (length, file-address, disk-address)
    which can represent holes.  Find out if HPFS does that.)
    At any rate, fnodes contain a small tree; if subtrees are needed
    they occupy essentially a full block in anodes.  A leaf-level tree node
    has 3-word entries giving sector runs, a non-leaf node has 2-word
    entries giving subtree pointers.  A flag in the header says which.
*/

typedef struct _BPLUSLEAFNODE       /* For external sector runs             */
    {
    ULONG   ulFileSecno ;           /* first file sector in extent          */
    ULONG   ulLength ;              /* length in sectors                    */
    LSN     ulFragSector ;          /* first corresponding disk sector      */
    } BPLUSLEAFNODE, FAR *PBPLUSLEAFNODE ;

typedef struct _BPLUSINTERNALNODE   /* For internal ANODE lists             */
    {
    ULONG   ulFileSecno ;           /* subtree maps sectors < this          */
    LSN     ulFragSector ;          /* pointer to ANODE subtree             */
    } BPLUSINTERNALNODE, FAR *PBPLUSINTERNALNODE ;

/*
    Flags used in the BPLUSHEADER.byBPHeader
*/
#define FNODEPARENT     0x20        /* ? we're pointed to by an fnode,      */
                                    /* the data btree or some ea or the     */
                                    /* main ea bootage pointer ea_secno     */
                                    /* also can get set in fnodes, which    */
                                    /* may be a chkdsk bug or may mean      */
                                    /* this bit is irrelevant in fnodes,    */
                                    /* or this interpretation is all wet    */
#define INTERNAL        0x80        /* 1 -> (internal) tree of anodes       */
                                    /* 0 -> (leaf) list of extents          */
typedef struct _BPLUSHEADER
    {
    BYTE    byBPHeader ;            /* Flags for the BPLUS HEADER           */
    BYTE    byFill[3] ;             /* Padding                              */
    BYTE    bFreeNodes ;            /* free nodes in following array        */
    BYTE    bUsedNodes ;            /* used nodes in following array        */
    USHORT  usFirstFree ;           /* offset from start of header to first free node in array */
#if 0
    /*
    C6 has a hernia with these and a zero length array so we leave them
    out of the definition, but leave them here for reference.
    */
    union
        {
        BPLUSLEAFNODE       External[0] ; /* (external) 3-word entries giving sector runs      */
        BPLUSINTERNALNODE   Internal[0] ; /* (internal) 2-word entries giving subtree pointers */
        } ;
#endif
    } BPLUSHEADER, FAR *PBPLUSHEADER ;

/* ------------------------------------------------------------------------ */
/* FNODE: root of allocation b+ tree, and EA's                              */
/* ------------------------------------------------------------------------ */
/*
    Every file and every directory has one fnode, pointed to by the directory
    entry and pointing to the file's sectors or directory's root dnode.  EA's
    are also stored here, and there are said to be ACL's somewhere here too.
*/

#define FNODE_MAGIC         0xF7E40AAEL
#define DELETED_FNODE_MAGIC 0xF7E40A00L

/*
    Flags used in the FNODE.usFlags
*/
#define FN_EA_ANODE 0x0002          /* 1 -> ulEASector is an anode          */
#define FN_DIR_FLAG 0x0100          /* 1 -> A directory entry               */

typedef struct _FNODE
    {
    ULONG   ulMagic ;               /* AE, 0A, E4, F7                       */
    BYTE    bReserved1[8] ;         /*                                      */
    BYTE    bEntryNameLen ;         /* true length                          */
    BYTE    sEntryName[15] ;        /* truncated name                       */
    LSN     ulParentSector ;        /* pointer to file's directory FNODE    */
    BYTE    bReserved2[12] ;        /*                                      */
    ULONG   ulExtendedAttributes ;  /* length of disk-resident ea's (bytes) */
    LSN     ulEASector ;            /* first sector of disk-resident ea's   */
    USHORT  usExtendedAttributes ;  /* length of fnode-resident ea's (bytes)*/
    USHORT  usFlags ;               /*                                      */
    BPLUSHEADER bphBTree ;          /* b+tree, 40 extents or 60 subtrees    */
    union
        {
        BPLUSLEAFNODE     External[8] ;
        BPLUSINTERNALNODE Internal[12] ;
        } FragTable ;
    ULONG   ulFileSize ;            /* File size in bytes                   */
    ULONG   ulNoEA ;                /* number of required EA's              */
    BYTE    bReserved3[16] ;        /*                                      */
    ULONG   ulEAOffset ;            /* offset from start of fnode to first fnode-resident ea */
    BYTE    bReserved4[8] ;         /*                                      */
    BYTE    bEAData[316] ;          /* zero or more EA's, packed together   */
                                    /* with no alignment padding.           */
                                    /* (Do not use this field, get here     */
                                    /* via fnode + ea_offs. I think.)       */
    } FNODE, FAR *PFNODE ;

/* ------------------------------------------------------------------------ */
/* ANODE: Extra allocation node for heavily fragmented files                */
/* ------------------------------------------------------------------------ */
/*
    ANODE: 99.44% pure allocation tree

    ANODEs are 1 sectors long.
*/

#define ANODE_MAGIC         0x37E40AAEL
#define DELETED_ANODE_MAGIC 0x37E40A00L /* NOT VERIFIED */

typedef struct _ANODE
    {
    ULONG   ulMagic ;               /* AE, 0A, E4, 37                       */
    LSN     ulMySelf ;              /* pointer to this ANODE                */
    LSN     ulParent ;              /* parent ANODE or FNODE                */
    BPLUSHEADER bphBTree ;          /* b+tree, 40 extents or 60 subtrees    */
    union
        {
        BPLUSLEAFNODE     External[40] ;
        BPLUSINTERNALNODE Internal[60] ;
        } FragTable ;
    ULONG   ulFill[3] ;             /* unused                               */
    } ANODE, FAR *PANODE ;

/* ------------------------------------------------------------------------ */
/* EXTENDED ATTRIBUTE Definitions                                           */
/* ------------------------------------------------------------------------ */
/*
    A file's EA info is stored as a list of (name,value) pairs.  It is
    usually in the fnode, but (if it's large) it is moved to a single
    sector run outside the fnode, or to multiple runs with an anode tree
    that points to them.

    The value of a single EA is stored along with the name, or (if large)
    it is moved to a single sector run, or multiple runs pointed to by an
    anode tree, pointed to by the value field of the (name,value) pair.

    Flags in the EA tell whether the value is immediate, in a single sector
    run, or in multiple runs.  Flags in the fnode tell whether the EA list
    is immediate, in a single run, or in multiple runs.

    If the sector runs are ANODEs, the data pointed to by the ANODES is
    CONTIGUOUS. IT DOES NOT FIT DISCRETELY WITHIN EACH SECTOR. You
    will have to buffer it to get the data.
*/

/*
    Flags used in the EXTENDEDATTRIBUTE.bFlag
*/
#define EA_INDIRECT 0x01            /* 1 -> value gives sector number where real value starts  */
#define EA_ANODE    0x02            /* 1 -> sector is an anode that points to fragmented value */
#define EA_NEEDEA   0x80            /* 1 -> required EA                                        */

typedef struct _EXTENDEDATTRIBUTE
    {
    BYTE    bFlag ;                 /* as defined above                     */
    BYTE    bNameLength ;           /* length of name, bytes                */
    USHORT  usValueLength ;         /* length of value, bytes               */
    /*
    BYTE    bName[bNameLength] ;    // ascii attrib name
    BYTE    bNull ;                 // terminating '\0', not counted
    BYTE    bValue[usValueLength] ; //

    if this.indirect & valuelen is 8 and the value is:

    ULONG   ulLength ;              // real length of bValue in bytes
    LSN     ulSector ;              // sector address where it starts

    if this.anode, the above sector number is the root of an anode tree
    which points to the value.
    */
    } EXTENDEDATTRIBUTE, FAR *PEXTENDEDATTRIBUTE ;

/*
    This structure is used for the external EA sector work.
    It is at the same point as bValue.
*/
typedef struct _EXTEA
    {
    ULONG   ulLength ;              /* real length of bValue in bytes       */
    LSN     ulSector ;              /* sector address where it starts       */
    } EXTEA, FAR *PEXTEA ;

#pragma pack()

/* ------------------------------------------------------------------------ */
/* External references                                                      */
/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */
/* Foreward references                                                      */
/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */
/* Constant Local Data                                                      */
/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */
/* Global Data                                                              */
/* ------------------------------------------------------------------------ */


