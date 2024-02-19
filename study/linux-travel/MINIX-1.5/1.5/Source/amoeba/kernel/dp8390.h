/*
 * National Semiconductor DP8390 Network Interface Controller
 */

typedef
union dp8390reg {
	struct pg0rd {			/* Page 0, for reading ------------- */
		char	dp_cr;		/* Read side of Command Register     */
		char	dp_clda0;	/* Current Local Dma Address 0       */
		char	dp_clda1;	/* Current Local Dma Address 1       */
		char	dp_bnry;	/* Boundary Pointer                  */
		char	dp_tsr;		/* Transmit Status Register          */
		char	dp_ncr;		/* Number of Collisions Register     */
		char	dp_fifo;	/* Fifo ??                           */
		char	dp_isr;		/* Interrupt Status Register         */
		char	dp_crda0;	/* Current Remote Dma Address 0      */
		char	dp_crda1;	/* Current Remote Dma Address 1      */
		char	dp_dum1;	/* unused                            */
		char	dp_dum2;	/* unused                            */
		char	dp_rsr;		/* Receive Status Register           */
		char	dp_cntr0;	/* Tally Counter 0                   */
		char	dp_cntr1;	/* Tally Counter 1                   */
		char	dp_cntr2;	/* Tally Counter 2                   */
	} dp_pg0rd;
	struct pg0wr {			/* Page 0, for writing ------------- */
		char	dp_cr;		/* Write side of Command Register    */
		char	dp_pstart;	/* Page Start Register               */
		char	dp_pstop;	/* Page Stop Register                */
		char	dp_bnry;	/* Boundary Pointer                  */
		char	dp_tpsr;	/* Transmit Page Start Register      */
		char	dp_tbcr0;	/* Transmit Byte Count Register 0    */
		char	dp_tbcr1;	/* Transmit Byte Count Register 1    */
		char	dp_isr;		/* Interrupt Status Register         */
		char	dp_rsar0;	/* Remote Start Address Register 0   */
		char	dp_rsar1;	/* Remote Start Address Register 1   */
		char	dp_rbcr0;	/* Remote Byte Count Register 0      */
		char	dp_rbcr1;	/* Remote Byte Count Register 1      */
		char	dp_rcr;		/* Receive Configuration Register    */
		char	dp_tcr;		/* Transmit Configuration Register   */
		char	dp_dcr;		/* Data Configuration Register       */
		char	dp_imr;		/* Interrupt Mask Register           */
	} dp_pg0wr;
	struct pg1rdwr {		/* Page 1, read/write -------------- */
		char	dp_cr;		/* Command Register                  */
		char	dp_par0;	/* Physical Address Register 0       */
		char	dp_par1;	/* Physical Address Register 1       */
		char	dp_par2;	/* Physical Address Register 2       */
		char	dp_par3;	/* Physical Address Register 3       */
		char	dp_par4;	/* Physical Address Register 4       */
		char	dp_par5;	/* Physical Address Register 5       */
		char	dp_curr;	/* Current Page Register             */
		char	dp_mar0;	/* Multicast Address Register 0      */
		char	dp_mar1;	/* Multicast Address Register 1      */
		char	dp_mar2;	/* Multicast Address Register 2      */
		char	dp_mar3;	/* Multicast Address Register 3      */
		char	dp_mar4;	/* Multicast Address Register 4      */
		char	dp_mar5;	/* Multicast Address Register 5      */
		char	dp_mar6;	/* Multicast Address Register 6      */
		char	dp_mar7;	/* Multicast Address Register 7      */
	} dp_pg1rdwr;
} dp8390;

/* Bits in dp_cr */

#define CR_STP		0x01		/* Stop: software reset              */
#define CR_STA		0x02		/* Start: activate NIC               */
#define CR_TXP		0x04		/* Transmit Packet                   */
#define CR_DMA		0x38		/* Mask for DMA control              */
#	define CR_DM_NOP	0x00	/* DMA: No Operation                 */
#	define CR_DM_RR		0x08	/* DMA: Remote Read                  */
#	define CR_DM_RW		0x10	/* DMA: Remote Write                 */
#	define CR_DM_SP		0x18	/* DMA: Send Packet                  */
#	define CR_DM_ABORT	0x20	/* DMA: Abort Remote DMA Operation   */
#define CR_PS		0xC0		/* Mask for Page Select              */
#	define CR_PS_P0		0x00	/* Register Page 0                   */
#	define CR_PS_P1		0x40	/* Register Page 1                   */
#	define CR_PS_T0		0x80	/* Test Mode Register Map ??         */
#	define CR_SP_T1		0xC0	/* Test Mode Register Map ??         */

/* Bits in dp_isr */

#define ISR_PRX		0x01		/* Packet Received with no errors    */
#define ISR_PTX		0x02		/* Packet Transmitted with no errors */
#define ISR_RXE		0x04		/* Receive Error                     */
#define ISR_TXE		0x08		/* Transmit Error                    */
#define ISR_OVW		0x10		/* Overwrite Warning                 */
#define ISR_CNT		0x20		/* Counter Overflow                  */
#define ISR_RDC		0x40		/* Remote DMA Complete               */
#define ISR_RST		0x80		/* Reset Status                      */

/* Bits in dp_imr */

#define IMR_PRXE	0x01		/* Packet Received iEnable           */
#define IMR_PTXE	0x02		/* Packet Transmitted iEnable        */
#define IMR_RXEE	0x04		/* Receive Error iEnable             */
#define IMR_TXEE	0x08		/* Transmit Error iEnable            */
#define IMR_OVWE	0x10		/* Overwrite Warning iEnable         */
#define IMR_CNTE	0x20		/* Counter Overflow iEnable          */
#define IMR_RDCE	0x40		/* DMA Complete iEnable              */

/* Bits in dp_dcr */

#define DCR_WTS		0x01		/* Word Transfer Select              */
#	define DCR_BYTEWIDE	0x00	/* WTS: byte wide transfers          */
#	define DCR_WORDWIDE	0x01	/* WTS: word wide transfers          */
#define DCR_BOS		0x02		/* Byte Order Select                 */
#	define DCR_LTLENDIAN	0x00	/* BOS: Little Endian                */
#	define DCR_BIGENDIAN	0x02	/* BOS: Big Endian                   */
#define DCR_LAS		0x04		/* Long Address Select               */
#define DCR_BMS		0x08		/* Burst Mode Select                 */
#define DCR_AR		0x10		/* Autoinitialize Remote             */
#define DCR_FTS		0x60		/* Fifo Threshold Select             */
#	define DCR_2BYTES	0x00	/* 2 bytes                           */
#	define DCR_4BYTES	0x40	/* 4 bytes                           */
#	define DCR_8BYTES	0x20	/* 8 bytes                           */
#	define DCR_12BYTES	0x60	/* 12 bytes                          */

/* Bits in dp_tcr */

#define TCR_CRC		0x01		/* Inhibit CRC                       */
#define TCR_ELC		0x06		/* Encoded Loopback Control          */
#	define TCR_NORMAL	0x00	/* ELC: Normal Operation             */
#	define TCR_INTERNAL	0x02	/* ELC: Internal Loopback            */
#	define TCR_0EXTERNAL	0x04	/* ELC: External Loopback LPBK=0     */
#	define TCR_1EXTERNAL	0x06	/* ELC: External Loopback LPBK=1     */
#define TCR_ATD		0x08		/* Auto Transmit                     */
#define TCR_OFST	0x10		/* Collision Offset Enable (be nice) */

/* Bits in dp_tsr */

#define TSR_PTX		0x01		/* Packet Transmitted (without error)*/
#define TSR_DFR		0x02		/* Transmit Deferred                 */
#define TSR_COL		0x04		/* Transmit Collided                 */
#define TSR_ABT		0x08		/* Transmit Aborted                  */
#define TSR_CRS		0x10		/* Carrier Sense Lost                */
#define TSR_FU		0x20		/* Fifo Underrun                     */
#define TSR_CDH		0x40		/* CD Heartbeat                      */
#define TSR_OWC		0x80		/* Out of Window Collision           */

/* Bits in tp_rcr */

#define RCR_SEP		0x01		/* Save Errored Packets              */
#define RCR_AR		0x02		/* Accept Runt Packets               */
#define RCR_AB		0x04		/* Accept Broadcast                  */
#define RCR_AM		0x08		/* Accept Multicast                  */
#define RCR_PRO		0x10		/* Physical Promiscuous              */
#define RCR_MON		0x20		/* Monitor Mode                      */

/* Bits in dp_rsr */

#define RSR_PRX		0x01		/* Packet Received Intact            */
#define RSR_CRC		0x02		/* CRC Error                         */
#define RSR_FAE		0x04		/* Frame Alignment Error             */
#define RSR_FO		0x08		/* FIFO Overrun                      */
#define RSR_MPA		0x10		/* Missed Packet                     */
#define RSR_PHY		0x20		/* Multicast Address Match !!        */
#define RSR_DIS		0x40		/* Receiver Disabled                 */


struct rcvdheader {
	char	rp_status;		/* Copy of rsr                       */
	char	rp_next;		/* Pointer to next packet            */
	char	rp_rbcl;		/* Receive Byte Count Low            */
	char	rp_rbch;		/* Receive Byte Count High           */
};
