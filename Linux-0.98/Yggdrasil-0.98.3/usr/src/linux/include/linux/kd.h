#ifndef _LINUX_KD_H
#define _LINUX_KD_H

/* 0x4B is 'K', to avoid collision with termios and vt */

#define SWAPMONO	0x4B00	/* use mca as output device */
#define SWAPCGA		0x4B01	/* use cga as output device */
#define SWAPEGA		0x4B02	/* use ega as output device */
#define SWAPVGA		0x4B03	/* use vga as output device */
#define CONS_CURRENT	0x4B04	/* return current output device */
#define		MONO		0x01
#define		CGA		0x02
#define		EGA		0x03

#define SW_B40x25	0x4B05	/* 40x25 mono text (cga/ega) */
#define SW_C40x25	0x4B06	/* 40x24 color text (cga/ega) */
#define SW_B80x25	0x4B07	/* 80x25 mono text (cga/ega) */
#define SW_C80x25	0x4B08	/* 80x25 color text (cga/ega) */
#define SW_BG320	0x4B09	/* 320x200 mono graphics (cga/ega) */
#define SW_CG320	0x4B0A	/* 320x200 color graphics (cga/ega) */
#define SW_BG640	0x4B0B	/* 640x200 mono graphics (cga/ega) */
#define SW_CG320_D	0x4B0C	/* 320x200 graphics (ega mode d) */
#define SW_CG640_E	0x4B0D	/* 640x200 graphics (ega mode e) */
#define SW_EGAMONOAPA	0x4B0E	/* 640x350 graphics (ega mode f) */
#define SW_ENH_MONOAPA2	0x4B0F	/* 640x350 graphics extd mem (ega mode f*) */
#define SW_CG640x350	0x4B10	/* 640x350 graphics (ega mode 10) */
#define SW_ENH_CG640	0x4B11	/* 640x350 graphics extd mem (ega mode 10*) */
#define SW_EGAMONO80x25	0x4B12	/* 80x25 mono text (ega mode 7) */
#define SW_ENHB40x25	0x4B13	/* enhanced 40x25 mono text (ega) */
#define SW_ENHC40x25	0x4B14	/* enhanced 40x25 color text (ega) */
#define SW_ENHB80x25	0x4B15	/* enhanced 80x25 mono text (ega) */
#define SW_ENHC80x25	0x4B16	/* enhanced 80x25 color text (ega) */
#define SW_ENHB80x43	0x4B17	/* enhanced 80x43 mono text (ega) */
#define SW_ENHC80x43	0x4B18	/* enhanced 80x43 color text (ega) */
#define SW_MCAMODE	0x4B19	/* reinit mca */
#define SW_ATT640	0x4B1A	/* 640x400 16color */
/* should add more vga modes, etc */

#define CONS_GET	0x4B1B	/* get current display mode */
#define		M_B40x25	0	/* 40x25 mono (cga/ega) */
#define		M_C40x25	1	/* 40x25 color (cga/ega) */
#define		M_B80x25	2	/* 80x25 mono (cga/ega) */
#define		M_C80x25	3	/* 80x25 color (cga/ega) */
#define		M_BG320		4	/* 320x200 mono (cga/ega) */
#define		M_CG320		5	/* 320x200 color (cga/ega) */
#define		M_BG640		6	/* 640x200 mono (cga/ega) */
#define		M_EGAMONO80x25	7	/* 80x25 mono (ega) */
#define		M_CG320_D	13	/* ega mode d */
#define		M_CG640_E	14	/* ega mode e */
#define		M_EFAMONOAPA	15	/* ega mode f */
#define		M_CG640x350	16	/* ega mode 10 */
#define		M_ENHMONOAPA2	17	/* ega mode f with ext mem */
#define		M_ENH_CG640	18	/* ega mode 10* */
#define		M_ENH_B40x25	19	/* ega enh 40x25 mono */
#define		M_ENH_C40x25	20	/* ega enh 40x25 color */
#define		M_ENH_B80x25	21	/* ega enh 80x25 mono */
#define		M_ENH_C80x25	22	/* ega enh 80x25 color */
#define		M_ENH_B80x43	0x70	/* ega enh 80x43 mono */
#define		M_ENH_C80x43	0x71	/* ega enh 80x43 color */
#define		M_MCA_MODE	0xff	/* monochrome adapter mode */
#define MCA_GET		0x4B1C	/* get mca display mode */
#define CGA_GET		0x4B1D	/* get cga display mode */
#define EGA_GET		0x4B1E	/* get ega display mode */

#define MAPCONS		0x4B1F	/* map current video mem into address space */
#define MAPMONO		0x4B20	/* map mca video mem into address space */
#define MAPCGA		0x4B21	/* map cga video mem into address space */
#define MAPEGA		0x4B22	/* map ega video mem into address space */
#define MAPVGA		0x4B23	/* map vga video mem into address space */

struct port_io_struc {
	char dir;			/* direction in vs out */
	unsigned short port;
	char data;
};
#define		IN_ON_PORT	0x00
#define		OUT_ON_PORT	0x01
struct port_io_arg {
	struct port_io_struc args[4];
};
#define MCAIO		0x4B24	/* i/o to mca video board */
#define CGAIO		0x4B25	/* i/o to cga video board */
#define EGAIO		0x4B26	/* i/o to ega video board */
#define VGAIO		0x4B27	/* i/o to vga video board */

#define GIO_FONT8x8	0x4B28	/* gets current 8x8 font used */
#define PIO_FONT8x8	0x4B29	/* use supplied 8x8 font */
#define GIO_FONT8x14	0x4B2A	/* gets current 8x14 font used */
#define PIO_FONT8x14	0x4B2B	/* use supplied 8x14 font */
#define GIO_FONT8x16	0x4B2C	/* gets current 8x16 font used */
#define PIO_FONT8x16	0x4B2D	/* use supplied 8x16 font */

#define MKDIOADDR	32	/* io bitmap size from <linux/sched.h> */
struct kd_disparam {
	long type;			/* type of display */
	char *addr;			/* display mem address */
	ushort ioaddr[MKDIOADDR];	/* valid i/o addresses */
};
#define KDDISPTYPE	0x4B2E	/* gets display info */
#define		KD_MONO		0x01
#define		KD_HERCULES	0x02
#define		KD_CGA		0x03
#define		KD_EGA		0x04

#define KIOCSOUND	0x4B2F	/* start sound generation (0 for off) */
#define KDMKTONE	0x4B30	/* generate tone */

#define KDGETLED	0x4B31	/* return current led flags */
#define KDSETLED	0x4B32	/* set current led flags */
#define 	LED_SCR		0x01	/* scroll lock */
#define 	LED_CAP		0x04	/* caps lock */
#define 	LED_NUM		0x02	/* num lock */

#define KDGKBTYPE	0x4B33	/* get keyboard type */
#define 	KB_84		0x01
#define 	KB_101		0x02
#define 	KB_OTHER	0x03

#define KDADDIO		0x4B34	/* add i/o port as valid */
#define KDDELIO		0x4B35	/* del i/o port as valid */
#define KDENABIO	0x4B36	/* enable i/o to video board */
#define KDDISABIO	0x4B37	/* disable i/o to video board */

struct kd_quemode {
	int qsize;		/* desired # elem in queue */
	int signo;		/* signal to send when queue not empty */
	char *qaddr;		/* user virt addr of queue */
};
#define KDQUEMODE	0x4B38	/* enable/disable special queue mode */

#define KDSBORDER	0x4B39	/* set screen boarder in ega text mode */

#define KDSETMODE	0x4B3A	/* set text/grahics mode */
#define		KD_TEXT		0x00
#define		KD_GRAPHICS	0x01
#define		KD_TEXT0	0x02	/* ? */
#define		KD_TEXT1	0x03	/* ? */
#define KDGETMODE	0x4B3B	/* get current mode */

struct kd_memloc {
	char *vaddr;		/* virt addr to map to */
	char *physaddr;		/* phys addr to map from */
	long length;		/* number of bytes */
	long ioflg;		/* enable i/o if set */
};
#define KDMAPDISP	0x4B3C	/* map display into address space */
#define KDUNMAPDISP	0x4B3D	/* unmap display from address space */

#define KDVDCTYPE	0x4B3E	/* return vdc controller/display info */

#define KIOCINFO	0x4B3F	/* tell what the device is */

typedef char scrnmap_t;
#define		E_TABSZ		256
#define GIO_SCRNMAP	0x4B40	/* get screen mapping from kernel */
#define PIO_SCRNMAP	0x4B41	/* put screen mapping table in kernel */

#define GIO_ATTR	0x4B42	/* get screen attributes */
#define GIO_COLOR	0x4B43	/* return nonzero if display is color */

#define		K_RAW		0x00
#define		K_XLATE		0x01
#define KDGKBMODE	0x4B44	/* gets current keyboard mode */
#define KDSKBMODE	0x4B45	/* sets current keyboard mode */

struct kbentry {
	u_char kb_table;
	u_char kb_index;
	u_char kb_value;
};
#define		K_NORMTAB	0x00
#define		K_SHIFTTAB	0x01
#define		K_ALTTAB	0x02
#define		K_ALTSHIFTTAB	0x03
#define		K_SRQTAB	0x04
#define KDGKBENT	0x4B46	/* gets one entry in translation table */
#define KDSKBENT	0x4B47	/* sets one entry in translation table */

#endif /* _LINUX_KD_H */
