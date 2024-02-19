#include "kernel.h"
#include "minix/com.h"
#include "dp8390.h"
#include "internet.h"
#include "etherformat.h"
#include "etherplus.h"
#include "dp8390info.h"
#include "eplinfo.h"
#include "assert.h"

/* macros for device I/O */

#define input(devaddr, ep_register) \
	in_byte((vir_bytes)&((struct eplusreg *) devaddr)->ep_register)
#define output(devaddr, ep_register, value) \
	out_byte((vir_bytes)&((struct eplusreg *) devaddr)->ep_register, value)

epl_init() {
	register vir_bytes device;
	register sum;

	device = eplinfo.epi_devaddr;
	assert((dp8390info.dpi_membase&0x81FFF)==0x80000);
	sum =
		input(device, epl_ea5) +
		input(device, epl_ea4) +
		input(device, epl_ea3) +
		input(device, epl_ea2) +
		input(device, epl_ea1) +
		input(device, epl_ea0) +
		input(device, epl_res2) +
		input(device, epl_chksum);
	if ((sum&0xFF) != 0xFF)
		panic("No ethernet board", NO_NUM);
	output(device, epl_ctlstatus, CTL_RESET);
	output(device, epl_ctlstatus, CTL_MENABLE|((dp8390info.dpi_membase>>13)&CTL_MEMADDR));
}


etheraddr(eaddr) Eth_addr *eaddr; {
	register vir_bytes device;

	device = eplinfo.epi_devaddr;
	eaddr->e[0] = input(device, epl_ea0);
	eaddr->e[1] = input(device, epl_ea1);
	eaddr->e[2] = input(device, epl_ea2);
	eaddr->e[3] = input(device, epl_ea3);
	eaddr->e[4] = input(device, epl_ea4);
	eaddr->e[5] = input(device, epl_ea5);
}
