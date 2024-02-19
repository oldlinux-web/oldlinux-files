/* wereg.h */
/*
    Copyright (C) 1992  Ross Biro

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 

    The Author may be reached as bir7@leland.stanford.edu or
    C/O Department of Mathematics; Stanford University; Stanford, CA 94305
*/
/* This is based on if_wereg.h from bsd386 */

/* Uncomment the next line if you are having troubles with your 
   8 bit card being recognized as 16 bit. */
/* #define FORCE_8BIT */

struct wd_ring
{
  unsigned char status; /* status */
  /* format of status 
     bit
     0		packet ok
     1		crc error
     2		frame alignment error
     3		fifo overrun
     */
#define STRECVD 0xf1
  unsigned char next;   /* pointer to next packet. */
  unsigned short count; /*packet lenght in bytes + 4 */
};

/* Format of command register. 
   bits
    0		stop
    1		start
    2		transmit packet
   3-5		Remote DMA command		
   6-7		Page Select */

#define CSTOP 0x1
#define CSTART  0x2
#define CTRANS 0x4
#define CRDMA 0x38
#define CRDMA_SHIFT 3
#define CPAGE 0xc0
#define CPAGE_SHIFT 6

#define CPAGE1 0x40


#define CPAGE1 0x40

/* interrupt status defenitions
   bits
     0		Recv.
     1		Transmit
     2		RcvErr
     3		Transmit  Err
     4		Overwrite warning
     5		Counter overflow
     6		Remote DMA complete
     7		Reset Status */

#define IRCV		0x1
#define ITRS		0x2
#define IRCE		0x4
#define ITRE		0x8
#define IOVER		0x10
#define ICOUNTERS	0x20
#define IDMA 		0x40
#define IRESET 		0x80
#define IOVER		0x10
#define ICOUNTERS	0x20
#define IDMA 		0x40
#define IRESET 		0x80

/* transmit status format
   bits
    0		Packet transmitted ok.
    1		Non Deferred transmition
    2		Transmit collied
    3		Transmit aborted
    4		Carrier Sense Lost
    5		Fifo Underrun
    6		CD Heartbeat
    7		Out of Window Collision */

#define TROK 0x1
#define TRAB 0x4

/* Some ID stuff */
#define WD_ID1 0x03
#define WD_ID2 0x05
#define WD_CHECK 0xff
#define WD_PAGE 256 /* page size in bytes. */
#define WD_TXBS 6   /* size of transmit buffer in pages. */
#define WD_MAX_PAGES 32 /* Number of pages off ram on card (8k) */
#define WD_NIC 16   /* i/o base offset to NIC */

/* Some configuration stuff. */
/* where the memory is mapped in. */
#define WD_MEM (dev->mem_start)
#define WD_BUFFEND (dev->mem_end)
#define WD_MEMSIZE (WD_BUFFEND-WD_MEM)
#define WD_BASE (dev->base_addr)

#define TRANS_MASK 0xa
#define RECV_MASK  0x5
#define WD_DCONFIG 0x48
#define WD_RCONFIG 0x4
#define WD_MCONFIG 0x20
#define WD_TCONFIG 0x0
#define WD_IMEM (((WD_MEM>>13) & 0x3f)|0x40)

/* WD registers. */
#define WD_ROM (WD_BASE+8)
#define WD_CTL (WD_BASE+0)

/* WD NIC register offsets */
#define WD_COMM (WD_BASE+WD_NIC+0x00) 		/* command register */
#define WD_PSTRT (WD_BASE+WD_NIC+0x01)		/* page start register */
#define WD_PSTOP (WD_BASE+WD_NIC+0x02)		/* page stop register */
#define WD_BNDR  (WD_BASE+WD_NIC+0x03)		/* Boundary Pointer */
#define WD_TRST (WD_BASE+WD_NIC+0x04)		/* Transmit Status */
#define WD_TRPG (WD_BASE+WD_NIC+0x04)		/* Transmit Page */
#define WD_TB0  (WD_BASE+WD_NIC+0x05)		/* Transmit byte count, low */
#define WD_TB1	(WD_BASE+WD_NIC+0x06)		/* Transmit byte count, high */
#define WD_ISR  (WD_BASE+WD_NIC+0x07)		/* Interrupt status register */
#define WD_RBY0 (WD_BASE+WD_NIC+0x0a)            /* remote byte count low. */
#define WD_RBY1 (WD_BASE+WD_NIC+0x0b)            /* remote byte count high. */
#define WD_RCC	(WD_BASE+WD_NIC+0x0c)		/* receive configuration */
#define WD_TRC	(WD_BASE+WD_NIC+0x0d)		/* transmit configuration */
#define WD_DCR  (WD_BASE+WD_NIC+0x0e)            /* data configuration */
#define WD_IMR  (WD_BASE+WD_NIC+0x0f)            /* Interrupt Mask register. */
#define WD_PAR0 (WD_BASE+WD_NIC+0x01)
#define WD_CUR  (WD_BASE+WD_NIC+0x07)
#define WD_MAR0 (WD_BASE+WD_NIC+0x08)

/* rth additions */

#define EN_CMD (WD_BASE+0)
#define EN_REG1 (WD_BASE+1)
#define EN_REG5 (WD_BASE+5)
#define EN_SAPROM (WD_BASE+8)
#define EN_REGE (WD_BASE+0x0e)
#define EN_OFFSET 16

/* WD Commands for EN_CMD */
#define EN_RESET
#define EN_MEMEN 0x40
#define EN_MEM_MASK 0x3f

/* WD Bus Register bits */
#define BUS16 1

/* WD REG5 Commands */
#define MEM16ENABLE 0x80
#define LAN16ENABLE 0x40
#define MEMMASK 0x1f
#define BIT19 0x1

/* Memory test pattern to use */
#define TESTPATTERN 0x5a

/* Western Digital Additional Registers */

/* National Semiconductor Definitions */

/* Page 0 */
#define CR	(WD_BASE+WD_NIC+0)	/* RW - Command */
#define CLDA0 	(WD_BASE+WD_NIC+1)	/* R - Current Local DMA Address 0 */
#define PSTART	(WD_BASE+WD_NIC+1)	/* W - Page Start Register */
#define CLDA1	(WD_BASE+WD_NIC+2)	/* R - Current Local DMA Address 1 */
#define PSTOP	(WD_BASE+WD_NIC+2)	/* W - Page Stop Register */
#define BNRY	(WD_BASE+WD_NIC+3)	/* RW - Boundry Pointer */
#define TSR	(WD_BASE+WD_NIC+4)	/* R - Transmit Status Register */
#define TPSR	(WD_BASE+WD_NIC+4)	/* W - Transmit Page Start */
#define NCR	(WD_BASE+WD_NIC+5)	/* R - Number of Collisions */
#define TBCR0	(WD_BASE+WD_NIC+5)	/* W - Transmit Byte Count 0 */
#define FIFO	(WD_BASE+WD_NIC+6)	/* R - FIFO */
#define TBCR1	(WD_BASE+WD_NIC+6)	/* W - Transmit Byte Count 1 */
#define ISR	(WD_BASE+WD_NIC+7)	/* RW - Interrupt Status Register */
#define CRDA0	(WD_BASE+WD_NIC+8)	/* R - Current Remote DMA Address 0 */
#define RSAR0	(WD_BASE+WD_NIC+8)	/* W - Remote Start Address 0 */
#define CRDA1	(WD_BASE+WD_NIC+9)	/* R - Current Remote DMA Address 1 */
#define RSAR1	(WD_BASE+WD_NIC+9)	/* W - Remote Start Address 1 */
		/* R - Reserved */
#define RBCR0	(WD_BASE+WD_NIC+0x0a)	/* W - Remote Byte Count 0 */
		/* R - Reserved */
#define RBCR1	(WD_BASE+WD_NIC+0x0b)	/* W - Remote Byte Count 1 */
#define RSR	(WD_BASE+WD_NIC+0x0c)	/* R - Receive Status Register */
#define RCR	(WD_BASE+WD_NIC+0x0c)	/* W - Receive Configuration */
#define CNTR0	(WD_BASE+WD_NIC+0x0d)	/* R - Frame Alignment Errors 0 */
#define TCR	(WD_BASE+WD_NIC+0x0d)	/* W - Transmit Configuration */
#define CNTR1	(WD_BASE+WD_NIC+0x0e)	/* R - Frame Alignment Errors 1 */ 
#define DCR	(WD_BASE+WD_NIC+0x0e)	/* W - Data Configuration */
#define CNTR2	(WD_BASE+WD_NIC+0x0f)	/* R - Missed Packet Errors */
#define IMR	(WD_BASE+WD_NIC+0x0f)	/* W - Interrupt Mask Register */

/* Page 1 */
		/* RW - Command */
#define PAR0	(WD_BASE+WD_NIC+0x01)	/* RW - Physical Address 0 */
#define PAR1	(WD_BASE+WD_NIC+0x02)	/* RW - Physical Address 1 */
#define PAR2	(WD_BASE+WD_NIC+0x03)	/* RW - Physical Address 2 */
#define PAR3	(WD_BASE+WD_NIC+0x04)	/* RW - Physical Address 3 */
#define PAR4	(WD_BASE+WD_NIC+0x04)	/* RW - Physical Address 4 */
#define PAR5	(WD_BASE+WD_NIC+0x05)	/* RW - Physical Address 5 */
#define PAR6	(WD_BASE+WD_NIC+0x06)	/* RW - Physical Address 6 */
#define CURR	(WD_BASE+WD_NIC+0x07)	/* RW - Current Page */
#define MAR0	(WD_BASE+WD_NIC+0x08)	/* RW - Multicast Address 0 */
#define MAR1	(WD_BASE+WD_NIC+0x09)	/* RW - Multicast Address 1 */
#define MAR2	(WD_BASE+WD_NIC+0x0a)	/* RW - Multicast Address 2 */
#define MAR3	(WD_BASE+WD_NIC+0x0b)	/* RW - Multicast Address 3 */
#define MAR4	(WD_BASE+WD_NIC+0x0c)	/* RW - Multicast Address 4 */
#define MAR5	(WD_BASE+WD_NIC+0x0d)	/* RW - Multicast Address 5 */
#define MAR6	(WD_BASE+WD_NIC+0x0e)	/* RW - Multicast Address 6 */
#define MAR7	(WD_BASE+WD_NIC+0x0f)	/* RW - Multicast Address 7 */

/* Page 2 */
/* Page 2 Registers are RW opposite Page 0 */
/* and should be used for diagnostic purposes only */

/* Command Register bits */
#define STOP	1	/* In progress jobs finished, software reset */
#define STA	2	/* Activate the NIC */
#define TXP	4	/* Initiate TX packet */
#define RD0	8	/* Remote DMA commands */
#define RD1	0x10	
#define RD2	0x20
#define PS0	0x40	/* Page Select */
#define  PS1	0x80	/* 00 = 0, 01 = 1, 10 = 2, 11 = reserved */

#define PAGE0	~(PS0|PS1)	/* Remember to AND this */
#define PAGE1	PS0		/* these can be OR'd */
#define PAGE2	PS1
#define NO_DMA	RD2

/* Interrupt Status Register bits */
#define PRX	1	/* Packet received with no errors */
#define PTX	2	/* Packet transmitted with no errors */
#define RXE	4	/* Packet received with errors */
#define TXE	8	/* Transmit aborted with errors */
#define OVW	0x10	/* Overwrite warning */
#define CNT	0x20	/* Counter overflow warning */
#define RDC	0x40	/* Remote DMA complete */
#define RST	0x80	/* Reset status - does not cause interrupts */

/* Interrupt Mask Register - 1 = enabled */
#define PRXE	1	/* Packet received */
#define PTXE	2	/* Packet transmitted */
#define RXEE	4	/* Receive error */
#define TXEE	8	/* Transmit error */
#define OVWE	0x10	/* Overwrite error */
#define CNTE	0x20	/* Counter overflow */
#define RDCE	0x40	/* Remote DMA complete */

/* Data Configuration Register */
#define WTS	1	/* Word Transfer 0 = byte, 1 = word */
#define BOS	2	/* Byte Order 0 = 8086, 1 = 68000 */
#define LAS	4	/* Long Address 0 = 16bit, 1 = 32 bit DMA */
#define LS	8	/* Loopback = 0, 1 = Normal */
#define AR	0x10	/* Autoinitialize = 1 DMA, 0 = software */
#define FT0	0x20	/* FIFO Threshold (word mode /2 ) */
#define FT1	0x40	/* 00 = 2, 01 = 4, 10 = 8, 11 = 12 bytes */

/* Transmit Configuration Register */
#define CRCI	1	/* CRC inhibit = 1, append = 0 */
#define LB0	2	/* Loopback control 00 = normal loopback */
#define LB1	4	/* 01 = internal, 10 = external1, 11 = external2 */
#define ATD	8	/* Auto Transmit Enable = 1 tx inhibit enabled */
#define OFST	0x10	/* Collision offset 1 = modify to low priority mode */

/* Transmitter Status Register */
#define PTXOK	1	/* Packet transmitted without error */
		/* reserved */
#define COL	4	/* Transmit collided, check NCR for count */
#define ABT	8	/* Transmit aborted - 16 tries */
#define CRS	0x10	/* Carrier Sense lost */
#define FU	0x20	/* FIFO underrun */
#define CDH	0x40	/* CD Heartbeat failed */
#define OWC	0x80	/* Out of window collision */

/* Receive configuration Register */
#define SEP	1	/* Save error packets = 1 */
#define ARUNT	2	/* Accept RUNT packets < 64 bytes */
#define AB	4	/* Accept Broadcast packets */
#define AM	8	/* Accept Multicast packets */
#define PRO	0x10	/* Promiscuous mode */
#define MON	0x20	/* Monitor mode */

/* Receive Status Register */
#define PRX	1	/* Packet received without error */
#define CRC	2	/* CRC error */
#define FAE	4	/* Frame Alignment error */
#define FO	8	/* FIFO overrun error */
#define MPA	0x10	/* Missed packet */
#define PHY	0x20	/* Physical = 0, Multicast/Broadcast = 1 */
#define DIS	0x40	/* Receiver disabled (monitor mode) */
#define DFR	0x80	/* Deferring - jabber on line */


/* rth additions */

#define EN_CMD (WD_BASE+0)
#define EN_REG1 (WD_BASE+1)
#define EN_REG5 (WD_BASE+5)
#define EN_SAPROM (WD_BASE+8)
#define EN_REGE (WD_BASE+0x0e)
#define EN_OFFSET 16

/* WD Commands for EN_CMD */
#define EN_RESET
#define EN_MEMEN 0x40
#define EN_MEM_MASK 0x3f

/* WD Bus Register bits */
#define BUS16 1

/* WD REG5 Commands */
#define MEM16ENABLE 0x80
#define LAN16ENABLE 0x40
#define MEMMASK 0x1f
#define BIT19 0x1

/* Memory test pattern to use */
#define TESTPATTERN 0x5a

/* Western Digital Additional Registers */

/* National Semiconductor Definitions */

/* Page 0 */
#define CR	(WD_BASE+WD_NIC+0)	/* RW - Command */
#define CLDA0 	(WD_BASE+WD_NIC+1)	/* R - Current Local DMA Address 0 */
#define PSTART	(WD_BASE+WD_NIC+1)	/* W - Page Start Register */
#define CLDA1	(WD_BASE+WD_NIC+2)	/* R - Current Local DMA Address 1 */
#define PSTOP	(WD_BASE+WD_NIC+2)	/* W - Page Stop Register */
#define BNRY	(WD_BASE+WD_NIC+3)	/* RW - Boundry Pointer */
#define TSR	(WD_BASE+WD_NIC+4)	/* R - Transmit Status Register */
#define TPSR	(WD_BASE+WD_NIC+4)	/* W - Transmit Page Start */
#define NCR	(WD_BASE+WD_NIC+5)	/* R - Number of Collisions */
#define TBCR0	(WD_BASE+WD_NIC+5)	/* W - Transmit Byte Count 0 */
#define FIFO	(WD_BASE+WD_NIC+6)	/* R - FIFO */
#define TBCR1	(WD_BASE+WD_NIC+6)	/* W - Transmit Byte Count 1 */
#define ISR	(WD_BASE+WD_NIC+7)	/* RW - Interrupt Status Register */
#define CRDA0	(WD_BASE+WD_NIC+8)	/* R - Current Remote DMA Address 0 */
#define RSAR0	(WD_BASE+WD_NIC+8)	/* W - Remote Start Address 0 */
#define CRDA1	(WD_BASE+WD_NIC+9)	/* R - Current Remote DMA Address 1 */
#define RSAR1	(WD_BASE+WD_NIC+9)	/* W - Remote Start Address 1 */
		/* R - Reserved */
#define RBCR0	(WD_BASE+WD_NIC+0x0a)	/* W - Remote Byte Count 0 */
		/* R - Reserved */
#define RBCR1	(WD_BASE+WD_NIC+0x0b)	/* W - Remote Byte Count 1 */
#define RSR	(WD_BASE+WD_NIC+0x0c)	/* R - Receive Status Register */
#define RCR	(WD_BASE+WD_NIC+0x0c)	/* W - Receive Configuration */
#define CNTR0	(WD_BASE+WD_NIC+0x0d)	/* R - Frame Alignment Errors 0 */
#define TCR	(WD_BASE+WD_NIC+0x0d)	/* W - Transmit Configuration */
#define CNTR1	(WD_BASE+WD_NIC+0x0e)	/* R - Frame Alignment Errors 1 */ 
#define DCR	(WD_BASE+WD_NIC+0x0e)	/* W - Data Configuration */
#define CNTR2	(WD_BASE+WD_NIC+0x0f)	/* R - Missed Packet Errors */
#define IMR	(WD_BASE+WD_NIC+0x0f)	/* W - Interrupt Mask Register */

/* Page 1 */
		/* RW - Command */
#define PAR0	(WD_BASE+WD_NIC+0x01)	/* RW - Physical Address 0 */
#define PAR1	(WD_BASE+WD_NIC+0x02)	/* RW - Physical Address 1 */
#define PAR2	(WD_BASE+WD_NIC+0x03)	/* RW - Physical Address 2 */
#define PAR3	(WD_BASE+WD_NIC+0x04)	/* RW - Physical Address 3 */
#define PAR4	(WD_BASE+WD_NIC+0x04)	/* RW - Physical Address 4 */
#define PAR5	(WD_BASE+WD_NIC+0x05)	/* RW - Physical Address 5 */
#define PAR6	(WD_BASE+WD_NIC+0x06)	/* RW - Physical Address 6 */
#define CURR	(WD_BASE+WD_NIC+0x07)	/* RW - Current Page */
#define MAR0	(WD_BASE+WD_NIC+0x08)	/* RW - Multicast Address 0 */
#define MAR1	(WD_BASE+WD_NIC+0x09)	/* RW - Multicast Address 1 */
#define MAR2	(WD_BASE+WD_NIC+0x0a)	/* RW - Multicast Address 2 */
#define MAR3	(WD_BASE+WD_NIC+0x0b)	/* RW - Multicast Address 3 */
#define MAR4	(WD_BASE+WD_NIC+0x0c)	/* RW - Multicast Address 4 */
#define MAR5	(WD_BASE+WD_NIC+0x0d)	/* RW - Multicast Address 5 */
#define MAR6	(WD_BASE+WD_NIC+0x0e)	/* RW - Multicast Address 6 */
#define MAR7	(WD_BASE+WD_NIC+0x0f)	/* RW - Multicast Address 7 */

/* Page 2 */
/* Page 2 Registers are RW opposite Page 0 */
/* and should be used for diagnostic purposes only */

/* Command Register bits */
#define STOP	1	/* In progress jobs finished, software reset */
#define STA	2	/* Activate the NIC */
#define TXP	4	/* Initiate TX packet */
#define RD0	8	/* Remote DMA commands */
#define RD1	0x10	
#define RD2	0x20
#define PS0	0x40	/* Page Select */
#define  PS1	0x80	/* 00 = 0, 01 = 1, 10 = 2, 11 = reserved */

#define PAGE0	~(PS0|PS1)	/* Remember to AND this */
#define PAGE1	PS0		/* these can be OR'd */
#define PAGE2	PS1
#define NO_DMA	RD2

/* Interrupt Status Register bits */
#define PRX	1	/* Packet received with no errors */
#define PTX	2	/* Packet transmitted with no errors */
#define RXE	4	/* Packet received with errors */
#define TXE	8	/* Transmit aborted with errors */
#define OVW	0x10	/* Overwrite warning */
#define CNT	0x20	/* Counter overflow warning */
#define RDC	0x40	/* Remote DMA complete */
#define RST	0x80	/* Reset status - does not cause interrupts */

/* Interrupt Mask Register - 1 = enabled */
#define PRXE	1	/* Packet received */
#define PTXE	2	/* Packet transmitted */
#define RXEE	4	/* Receive error */
#define TXEE	8	/* Transmit error */
#define OVWE	0x10	/* Overwrite error */
#define CNTE	0x20	/* Counter overflow */
#define RDCE	0x40	/* Remote DMA complete */

/* Data Configuration Register */
#define WTS	1	/* Word Transfer 0 = byte, 1 = word */
#define BOS	2	/* Byte Order 0 = 8086, 1 = 68000 */
#define LAS	4	/* Long Address 0 = 16bit, 1 = 32 bit DMA */
#define LS	8	/* Loopback = 0, 1 = Normal */
#define AR	0x10	/* Autoinitialize = 1 DMA, 0 = software */
#define FT0	0x20	/* FIFO Threshold (word mode /2 ) */
#define FT1	0x40	/* 00 = 2, 01 = 4, 10 = 8, 11 = 12 bytes */

/* Transmit Configuration Register */
#define CRCI	1	/* CRC inhibit = 1, append = 0 */
#define LB0	2	/* Loopback control 00 = normal loopback */
#define LB1	4	/* 01 = internal, 10 = external1, 11 = external2 */
#define ATD	8	/* Auto Transmit Enable = 1 tx inhibit enabled */
#define OFST	0x10	/* Collision offset 1 = modify to low priority mode */

/* Transmitter Status Register */
#define PTXOK	1	/* Packet transmitted without error */
		/* reserved */
#define COL	4	/* Transmit collided, check NCR for count */
#define ABT	8	/* Transmit aborted - 16 tries */
#define CRS	0x10	/* Carrier Sense lost */
#define FU	0x20	/* FIFO underrun */
#define CDH	0x40	/* CD Heartbeat failed */
#define OWC	0x80	/* Out of window collision */

/* Receive configuration Register */
#define SEP	1	/* Save error packets = 1 */
#define ARUNT	2	/* Accept RUNT packets < 64 bytes */
#define AB	4	/* Accept Broadcast packets */
#define AM	8	/* Accept Multicast packets */
#define PRO	0x10	/* Promiscuous mode */
#define MON	0x20	/* Monitor mode */

/* Receive Status Register */
#define PRX	1	/* Packet received without error */
#define CRC	2	/* CRC error */
#define FAE	4	/* Frame Alignment error */
#define FO	8	/* FIFO overrun error */
#define MPA	0x10	/* Missed packet */
#define PHY	0x20	/* Physical = 0, Multicast/Broadcast = 1 */
#define DIS	0x40	/* Receiver disabled (monitor mode) */
#define DFR	0x80	/* Deferring - jabber on line */





