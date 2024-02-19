#ifndef __IDE_H__
#define __IDE_H__




#define HD_DATA		0x1f0	/* _CTL when writing */
#define HD_ERROR	0x1f1	/* see err-bits */
#define HD_NSECTOR	0x1f2	/* nr of sectors to read/write */
#define HD_SECTOR	0x1f3	/* starting sector */
#define HD_LCYL		0x1f4	/* starting cylinder */
#define HD_HCYL		0x1f5	/* high byte of starting cyl */
#define HD_CURRENT	0x1f6	/* 101dhhhh , d=drive, hhhh=head */
#define HD_STATUS	0x1f7	/* see status-bits */
#define HD_PRECOMP HD_ERROR	/* same io address, read=error, write=precomp */
#define HD_COMMAND HD_STATUS	/* same io address, read=status, write=cmd */

#define HD_CMD		0x3f6

/* Bits of HD_STATUS */
#define ERR_STAT	0x01
#define INDEX_STAT	0x02
#define ECC_STAT	0x04	/* Corrected error */
#define DRQ_STAT	0x08
#define SEEK_STAT	0x10
#define WRERR_STAT	0x20
#define READY_STAT	0x40
#define BUSY_STAT	0x80

typedef struct {
	unsigned short	config;		/* lots of obsolete bit flags */
	unsigned short	cyls;		/* "physical" cyls */
	unsigned short	reserved2;	/* reserved (word 2) */
	unsigned short	heads;		/* "physical" heads */
	unsigned short	track_bytes;	/* unformatted bytes per track */
	unsigned short	sector_bytes;	/* unformatted bytes per sector */
	unsigned short	sectors;	/* "physical" sectors per track */
	unsigned short	vendor0;	/* vendor unique */
	unsigned short	vendor1;	/* vendor unique */
	unsigned short	vendor2;	/* vendor unique */
	unsigned char	serial_no[20];	/* 0 = not_specified */
	unsigned short	buf_type;
	unsigned short	buf_size;	/* 512 byte increments; 0 = not_specified */
	unsigned short	ecc_bytes;	/* for r/w long cmds; 0 = not_specified */
	unsigned char	fw_rev[8];	/* 0 = not_specified */
	unsigned char	model[40];	/* 0 = not_specified */
	unsigned char	max_multsect;	/* 0=not_implemented */
	unsigned char	vendor3;	/* vendor unique */
	unsigned short	dword_io;	/* 0=not_implemented; 1=implemented */
	unsigned char	vendor4;	/* vendor unique */
	unsigned char	capability;	/* bits 0:DMA 1:LBA 2:IORDYsw 3:IORDYsup*/
	unsigned short	reserved50;	/* reserved (word 50) */
	unsigned char	vendor5;	/* vendor unique */
	unsigned char	tPIO;		/* 0=slow, 1=medium, 2=fast */
	unsigned char	vendor6;	/* vendor unique */
	unsigned char	tDMA;		/* 0=slow, 1=medium, 2=fast */
	unsigned short	field_valid;	/* bits 0:cur_ok 1:eide_ok */
	unsigned short	cur_cyls;	/* logical cylinders */
	unsigned short	cur_heads;	/* logical heads */
	unsigned short	cur_sectors;	/* logical sectors per track */
	unsigned short	cur_capacity0;	/* logical total sectors on drive */
	unsigned short	cur_capacity1;	/*  (2 words, misaligned int)     */
	unsigned char	multsect;	/* current multiple sector count */
	unsigned char	multsect_valid;	/* when (bit0==1) multsect is ok */
	unsigned int	lba_capacity;	/* total number of sectors */
	unsigned short	dma_1word;	/* single-word dma info */
	unsigned short	dma_mword;	/* multiple-word dma info */
	unsigned short  eide_pio_modes; /* bits 0:mode3 1:mode4 */
	unsigned short  eide_dma_min;	/* min mword dma cycle time (ns) */
	unsigned short  eide_dma_time;	/* recommended mword dma cycle time (ns) */
	unsigned short  eide_pio;       /* min cycle time (ns), no IORDY  */
	unsigned short  eide_pio_iordy; /* min cycle time (ns), with IORDY */
	unsigned short  word69;
	unsigned short  word70;
	/* HDIO_GET_IDENTITY currently returns only words 0 through 70 */
	unsigned short  word71;
	unsigned short  word72;
	unsigned short  word73;
	unsigned short  word74;
	unsigned short  word75;
	unsigned short  word76;
	unsigned short  word77;
	unsigned short  word78;
	unsigned short  word79;
	unsigned short  word80;
	unsigned short  word81;
	unsigned short  command_sets;	/* bits 0:Smart 1:Security 2:Removable 3:PM */
	unsigned short  word83;		/* bits 14:Smart Enabled 13:0 zero */
	unsigned short  word84;
	unsigned short  word85;
	unsigned short  word86;
	unsigned short  word87;
	unsigned short  dma_ultra;
	unsigned short	word89;		/* reserved (word 89) */
	unsigned short	word90;		/* reserved (word 90) */
	unsigned short	word91;		/* reserved (word 91) */
	unsigned short	word92;		/* reserved (word 92) */
	unsigned short	word93;		/* reserved (word 93) */
	unsigned short	word94;		/* reserved (word 94) */
	unsigned short	word95;		/* reserved (word 95) */
	unsigned short	word96;		/* reserved (word 96) */
	unsigned short	word97;		/* reserved (word 97) */
	unsigned short	word98;		/* reserved (word 98) */
	unsigned short	word99;		/* reserved (word 99) */
	unsigned short	word100;	/* reserved (word 100) */
	unsigned short	word101;	/* reserved (word 101) */
	unsigned short	word102;	/* reserved (word 102) */
	unsigned short	word103;	/* reserved (word 103) */
	unsigned short	word104;	/* reserved (word 104) */
	unsigned short	word105;	/* reserved (word 105) */
	unsigned short	word106;	/* reserved (word 106) */
	unsigned short	word107;	/* reserved (word 107) */
	unsigned short	word108;	/* reserved (word 108) */
	unsigned short	word109;	/* reserved (word 109) */
	unsigned short	word110;	/* reserved (word 110) */
	unsigned short	word111;	/* reserved (word 111) */
	unsigned short	word112;	/* reserved (word 112) */
	unsigned short	word113;	/* reserved (word 113) */
	unsigned short	word114;	/* reserved (word 114) */
	unsigned short	word115;	/* reserved (word 115) */
	unsigned short	word116;	/* reserved (word 116) */
	unsigned short	word117;	/* reserved (word 117) */
	unsigned short	word118;	/* reserved (word 118) */
	unsigned short	word119;	/* reserved (word 119) */
	unsigned short	word120;	/* reserved (word 120) */
	unsigned short	word121;	/* reserved (word 121) */
	unsigned short	word122;	/* reserved (word 122) */
	unsigned short	word123;	/* reserved (word 123) */
	unsigned short	word124;	/* reserved (word 124) */
	unsigned short	word125;	/* reserved (word 125) */
	unsigned short	word126;	/* reserved (word 126) */
	unsigned short	word127;	/* reserved (word 127) */
	unsigned short	security;	/* bits 0:support 1:enabled 2:locked 3:frozen */
	unsigned short	reserved[127];
}hd_driveid;


struct systypes {
    unsigned char type;
    char *name;
} sys_types[] = {
    {0, "Empty"},
    {1, "DOS 12-bit FAT"},		/* Primary DOS with 12-bit FAT */
    {2, "XENIX /"},			/* XENIX / filesystem */
    {3, "XENIX /usr"},			/* XENIX /usr filesystem */
    {4, "DOS 16-bit FAT <32M"},		/* Primary DOS with 16-bit FAT */
    {5, "DOS Extended"},		/* DOS 3.3+ extended partition */
    {6, "DOS 16-bit FAT >=32M"},
    {7, "OS/2 IFS (e.g., HPFS) or NTFS or QNX2 or Advanced UNIX"},
    {8, "AIX boot or SplitDrive"},
    {9, "AIX data or Coherent"},
    {0x0a, "OS/2 Boot Manager or Coherent swap"},
    {0x0b, "Windows FAT32"},
    {0x0c, "Windows FAT32 (lba)"},
    {0x0d, "Windows FAT16(lba)"},
    {0x0e, "DOS (16-bit FAT), CHS-mapped"},
    {0x0f, "Ext. partition, CHS-mapped"},
    {0x10, "OPUS"},
    {0x11, "OS/2 BM: hidden DOS 12-bit FAT"},
    {0x12, "Compaq diagnostics"},
    {0x14, "OS/2 BM: hidden DOS 16-bit FAT <32M"},
    {0x16, "OS/2 BM: hidden DOS 16-bit FAT >=32M"},
    {0x17, "OS/2 BM: hidden IFS"},
    {0x18, "AST Windows swapfile"},
    {0x24, "NEC DOS"},
    {0x3c, "PartitionMagic recovery"},
    {0x40, "Venix 80286"},
    {0x41, "Linux/MINIX (sharing disk with DRDOS)"},
    {0x42, "SFS or Linux swap (sharing disk with DRDOS)"},
    {0x43, "Linux native (sharing disk with DRDOS)"},
    {0x50, "DM (disk manager)"},
    {0x51, "DM6 Aux1 (or Novell)"},
    {0x52, "CP/M or Microport SysV/AT"},
    {0x53, "DM6 Aux3"},
    {0x54, "DM6"},
    {0x55, "EZ-Drive (disk manager)"},
    {0x56, "Golden Bow (disk manager)"},
    {0x5c, "Priam Edisk (disk manager)"}, /* according to S. Widlake */
    {0x61, "SpeedStor"},
    {0x63, "GNU HURD or Mach or Sys V/386 (such as ISC UNIX)"},
    {0x64, "Novell Netware 286"},
    {0x65, "Novell Netware 386"},
    {0x70, "DiskSecure Multi-Boot"},
    {0x75, "PC/IX"},
    {0x77, "QNX4.x"},
    {0x78, "QNX4.x 2nd part"},
    {0x79, "QNX4.x 3rd part"},
    {0x80, "MINIX until 1.4a"},
    {0x81, "MINIX since 1.4b, early Linux, Mitac dmgr"},
    {0x82, "Linux swap"},
    {0x83, "Linux native"},
    {0x84, "OS/2 hidden C: drive"},
    {0x85, "Linux extended"},
    {0x86, "NTFS volume set??"},
    {0x87, "NTFS volume set??"},
    {0x93, "Amoeba"},
    {0x94, "Amoeba BBT"},		/* (bad block table) */
    {0xa0, "IBM Thinkpad hibernation"}, /* according to dan@fch.wimsey.bc.ca */
    {0xa5, "BSD/386"},			/* 386BSD */
    {0xa7, "NeXTSTEP 486"},
    {0xb7, "BSDI fs"},
    {0xb8, "BSDI swap"},
    {0xc1, "DRDOS/sec (FAT-12)"},
    {0xc4, "DRDOS/sec (FAT-16, < 32M)"},
    {0xc6, "DRDOS/sec (FAT-16, >= 32M)"},
    {0xc7, "Syrinx"},
    {0xdb, "CP/M or Concurrent CP/M or Concurrent DOS or CTOS"},
    {0xe1, "DOS access or SpeedStor 12-bit FAT extended partition"},
    {0xe3, "DOS R/O or SpeedStor"},
    {0xe4, "SpeedStor 16-bit FAT extended partition < 1024 cyl."},
    {0xf1, "SpeedStor"},
    {0xf2, "DOS 3.3+ secondary"},
    {0xf4, "SpeedStor large partition"},
    {0xfe, "SpeedStor >1024 cyl. or LANstep"},
    {0xff, "Xenix Bad Block Table"}
};






#endif /*__IDE_H__*/
