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

#define IRCV 0x1
#define ITRS 0x2
#define IRCE 0x4
#define ITRE 0x8

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
#define WD_MEM 0xd0000
#define WD_BUFFEND 0xd2000
#define WD_MEMSIZE (WD_BUFFEND-WD_MEM)
#define WD_BASE 0x280

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

