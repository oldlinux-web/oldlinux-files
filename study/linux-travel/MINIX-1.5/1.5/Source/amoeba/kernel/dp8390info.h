/*
 * parameters for driver for
 * National Semiconductor DP8390 Network Interface Controller
 */

extern
struct dp8390info {
	vir_bytes	dpi_devaddr;	/* device address */
	char		dpi_pstart;	/* start of recv ring */
	char		dpi_pstop;	/* end of recv ring */
	phys_bytes	dpi_membase;	/* memory address of page 0 */
	phys_bytes	dpi_tbuf;	/* memory address of transmit buffer */
} dp8390info;

