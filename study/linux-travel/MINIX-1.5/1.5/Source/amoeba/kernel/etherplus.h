/*
 * Western Digital Ethercard Plus, or WD8003E card
 *
 * This information seems to be guarded like the crown jewels
 */

struct eplusreg {
	char	epl_ctlstatus;		/* Control(write) and status(read)   */
	char	epl_res1[7];
	char	epl_ea0;		/* Most significant eaddr byte       */
	char	epl_ea1;
	char	epl_ea2;
	char	epl_ea3;
	char	epl_ea4;
	char	epl_ea5;		/* Least significant eaddr byte      */
	char	epl_res2;
	char	epl_chksum;		/* sum from epl_ea0 upto here is 0xFF   */
	dp8390	epl_dp8390;		/* NatSemi chip                      */
};

/* Bits in epl_ctlstatus */

#define CTL_RESET	0x80		/* Software Reset                    */
#define CTL_MENABLE	0x40		/* Memory Enable                     */
#define CTL_MEMADDR	0x3F		/* Bits SA18-SA13, SA19 implicit 1   */

#define STA_IIJ		0x7		/* Interrupt Indication Jumpers      */
