
#include "kernel.h"

#ifndef NONET

#include "minix/com.h"
#include "internet.h"
#include "etherformat.h"
#include "dp8390.h"
#include "dp8390info.h"
#include "dp8390stat.h"
#include "assert.h"

/* macros for device I/O */

#define input(devaddr, dp_register) \
	in_byte((vir_bytes)&((union dp8390reg *) devaddr)->dp_pg0rd.dp_register)
#define input1(devaddr, dp_register) \
	in_byte((vir_bytes)&((union dp8390reg *) devaddr)->dp_pg1rdwr.dp_register)
#define output(devaddr, dp_register, value) \
	out_byte((vir_bytes)&((union dp8390reg *) devaddr)->dp_pg0wr.dp_register, value)
#define output1(devaddr, dp_register, value) \
	out_byte((vir_bytes)&((union dp8390reg *) devaddr)->dp_pg1rdwr.dp_register, value)

#define MAX_WAIT	10000

#ifdef DPSTAT
struct dpstat dpstat;
#endif

static int (*bufread)();	/* call when packet came in */
static int (*bufwritten)();	/* call when packet has been written */

static disabled;
static phys_bytes curopacket;	/* packet being transmitted */
static phys_bytes Curbuff;	/* address of next read buffer to release */

static
chipinit(myaddr)
Eth_addr *myaddr;
{
	register vir_bytes device;

	device = dp8390info.dpi_devaddr;
	output(device, dp_cr, CR_PS_P0|CR_DM_ABORT);	/* back to main register set */
	output(device, dp_pstart, dp8390info.dpi_pstart);
	output(device, dp_pstop, dp8390info.dpi_pstop);
	output(device, dp_bnry, dp8390info.dpi_pstart);
	output(device, dp_rcr, RCR_MON);
	output(device, dp_tcr, TCR_NORMAL);
	output(device, dp_dcr, DCR_BYTEWIDE|DCR_8BYTES);
        output(device, dp_rbcr0, 0);
        output(device, dp_rbcr1, 0);
        output(device, dp_isr, 0xFF);
	output(device, dp_cr, CR_PS_P1|CR_DM_ABORT);	/* switch to register set 1 */
	output1(device, dp_par0, myaddr->e[0]);
	output1(device, dp_par1, myaddr->e[1]);
	output1(device, dp_par2, myaddr->e[2]);
	output1(device, dp_par3, myaddr->e[3]);
	output1(device, dp_par4, myaddr->e[4]);
	output1(device, dp_par5, myaddr->e[5]);
	output1(device, dp_curr, dp8390info.dpi_pstart+1);
        output1(device, dp_cr, CR_PS_P0|CR_DM_ABORT);
        output(device, dp_rcr, RCR_AB);
        input(device, dp_cntr0);
        input(device, dp_cntr1);
        input(device, dp_cntr2);

#ifdef TRMTINT
	output(device, dp_imr, IMR_TXEE|IMR_PTXE|IMR_PRXE|IMR_CNTE|IMR_OVWE);
#endif    
	output(device, dp_imr, IMR_PRXE|IMR_CNTE|IMR_OVWE);
	output(device, dp_cr, CR_STA|CR_DM_ABORT);		/* fire it up */
}

/*

 * Interrupt handling
 */

static
dp_xmit_intr()
#ifdef TRMTINT
{
	register tsr;

	if (curopacket == 0) {
		printf("Bogus transmit interrupt\n");
		STINC(ds_btint);
		return;
	}
	tsr = input(dp8390info.dpi_devaddr, dp_tsr);
	if (tsr&TSR_PTX)
		STINC(ds_written);	/* It went OK! */
	if (tsr&TSR_DFR)
		STINC(ds_deferred);
	if (tsr&TSR_COL)
		STINC(ds_collisions);
	if (tsr&TSR_ABT)
		STINC(ds_xcollisions);
	if (tsr&TSR_CRS) {
		printf("Ethernet carrier sense lost\n");
		STINC(ds_carlost);
	}
	if (tsr&TSR_FU) {
		printf("Ethernet Fifo Underrun\n");
		STINC(ds_fifo);
	}
	if (tsr&TSR_CDH) {
		printf("Ethernet Heartbeat failure\n");
		STINC(ds_heartbeat);
	}
	if (tsr&TSR_OWC) {
		printf("Ethernet late collision\n");
		STINC(ds_lcol);
	}
	(*bufwritten)(curopacket);
	curopacket = 0;
}
#else
{}
#endif


static
recvintr()
{
	register vir_bytes device;
	register phys_bytes paddr;
	struct rcvdheader pkthead;
	char pageno, curr, next;
	int length;

	device = dp8390info.dpi_devaddr;
	pageno=input(device, dp_bnry)+1;
        if (pageno == dp8390info.dpi_pstop)
            pageno = dp8390info.dpi_pstart;
	while (!(disabled)) {
		output(device, dp_cr, CR_PS_P1);/* switch to register set 1 */
		curr = input1(device, dp_curr);
		output1(device, dp_cr, CR_PS_P0);/* back to main register set*/
		if (pageno==curr){
			break;
		}
		STINC(ds_read);
		paddr = dp8390info.dpi_membase+(pageno<<8);
		getheader(paddr, &pkthead);

		next = pkthead.rp_next;
		if (pkthead.rp_status&RSR_PRX) {
			if (next < pageno && next > dp8390info.dpi_pstart) {
				/*
				 * We copy end of packet to avoid break.
				 */
				phys_copy(dp8390info.dpi_membase+
					   (dp8390info.dpi_pstart<<8),
					  dp8390info.dpi_membase+
					   (dp8390info.dpi_pstop<<8),
					  (phys_bytes) (next-dp8390info.dpi_pstart)<<8);
			}
			length = (pkthead.rp_rbcl&0xFF)|(pkthead.rp_rbch<<8);
			Curbuff = paddr + sizeof (pkthead);
			disabled = 1;
			(*bufread)(Curbuff, length-4);
		}
		pageno = pkthead.rp_next;
		if (pageno >= dp8390info.dpi_pstop || 
		    pageno < dp8390info.dpi_pstart)
			printf("page no %x\n", pageno);
/*		assert(pageno >= dp8390info.dpi_pstart);
		assert(pageno < dp8390info.dpi_pstop);*/
	}
}			

static
cntintr()
{
	register vir_bytes device;
        int n;

        printf("dp8390: counter overflow\n");		/*DEBUG*/
	device = dp8390info.dpi_devaddr;
        n = input(device, dp_cntr0);
	STADD(ds_fram, n);
        n = input(device, dp_cntr1);
	STADD(ds_crc, n); 
        n =input(device, dp_cntr2);
	STADD(ds_lost, n);
}

PUBLIC void
dp8390_int()
{
	register isr;
	register vir_bytes device;

	device = dp8390info.dpi_devaddr;
	for(isr=input(device, dp_isr); isr&(ISR_OVW|ISR_PRX|ISR_PTX|ISR_CNT);
						isr=input(device, dp_isr)) {
		if (isr&ISR_OVW) {
			printf("OVW, do something\n");
			output(device, dp_isr, ISR_OVW);	/* ack */
		}
		if (isr&ISR_PTX) {
			dp_xmit_intr();
			output(device, dp_isr, ISR_PTX);	/* ack */
		}
		if (isr&ISR_TXE) {
			dp_xmit_intr();
			output(device, dp_isr, ISR_TXE);	/* ack */
		}
		if (isr&ISR_PRX) {
			/*recvintr();*/
			got_packet();
			output(device, dp_isr, ISR_PRX);	/* ack */
		}
		if (isr&ISR_CNT) {
			cntintr();
			output(device, dp_isr, ISR_CNT);	/* ack */
		}
	}
}

eth_init(etheraddr, br, bw)
Eth_addr *etheraddr;
int (*br)(), (*bw)();
{
	bufread = br;
	bufwritten = bw;
	epl_init();		/* activate on board memory */
	chipinit(etheraddr);	/* start ethernet controller chip */
}



eth_write(bufaddr, bufcnt)
phys_bytes bufaddr;
{
	int bpageno;
	register vir_bytes device;


	device = dp8390info.dpi_devaddr;
/*	assert(curopacket==0);  */
	assert(((bufaddr-dp8390info.dpi_membase)&0xFF)==0);
	assert(bufcnt >= 60);	/* magic Ethernet requirement */
/*	assert(bufcnt <= 1514); /* another one */
	bpageno = ((bufaddr-dp8390info.dpi_membase)>>8) & 0xFF;
	curopacket = bufaddr;
	output(device, dp_tpsr, bpageno);
	output(device, dp_tbcr1, bufcnt>>8);
	output(device, dp_tbcr0, bufcnt&0xFF);
	output(device, dp_cr, CR_TXP);		/* there it goes */
}

eth_release(bufaddr) 
phys_bytes bufaddr;
{
	register vir_bytes device;
	register phys_bytes paddr;
	struct rcvdheader pkthead;
	char pageno;

	device = dp8390info.dpi_devaddr;
	paddr = bufaddr-sizeof(pkthead);
	assert(((paddr-dp8390info.dpi_membase)&0xFF)==0);
	getheader(paddr, &pkthead);
        pageno = pkthead.rp_next;
        if (pageno == dp8390info.dpi_pstart)
            pageno = dp8390info.dpi_pstop;
	if (bufaddr != Curbuff)
	    panic("eth_release: bad order", NO_NUM);
	output(device, dp_bnry, pageno-1);
	disabled = 0;
/*	recvintr(); */
}

phys_bytes
eth_getbuf()
{
	int t_cnt;
	register vir_bytes device;
	register tsr;

	device = dp8390info.dpi_devaddr;

        t_cnt = 0;
        while (input(device,dp_cr)&CR_TXP) {
            if (t_cnt++ > MAX_WAIT)
                printf("transmitter frozen\n"); 
		return (phys_bytes)0;
	}       

#ifndef TRMTINT
#ifdef DPSTAT
	tsr = input(device, dp_tsr);
	if (tsr&TSR_PTX)
		STINC(ds_written);	/* It went OK! */
	if (tsr&TSR_DFR)
		STINC(ds_deferred);
	if (tsr&TSR_COL)
		STINC(ds_collisions);
	if (tsr&TSR_ABT)
		STINC(ds_xcollisions);
	if (tsr&TSR_CRS) {
		printf("Ethernet carrier sense lost\n");
		STINC(ds_carlost);
	}
	if (tsr&TSR_FU) {
		printf("Ethernet Fifo Underrun\n");
		STINC(ds_fifo);
	}
	if (tsr&TSR_CDH) {
		printf("Ethernet Heartbeat failure\n");
		STINC(ds_heartbeat);
	}
	if (tsr&TSR_OWC) {
		printf("Ethernet late collision\n");
		STINC(ds_lcol);
	}
#endif
#endif
	return dp8390info.dpi_tbuf;	/* return pointer to xmit buffer */
}

#else NONET

PUBLIC void
dp8390_int()
{
}

#endif NONET

PUBLIC void
eth_stp()
{
/* called from reboot() (klib88.s) to stop the ethernet */
#ifndef NONET
	output(dp8390info.dpi_devaddr, dp_cr, CR_STP|CR_DM_ABORT);

#endif
}

