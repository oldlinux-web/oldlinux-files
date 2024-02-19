/*
 * offsets from base for the Portable (Plus) FTP package
 *				       P+ serial    serial/modem
 */
#define   BAUDLO		0	/* 0x44 	or 0xa4 */
#define   BAUDHI		2	/* 0x46 	or 0xa6 */
#define   STATUS		4	/* 0x48 	or 0xa8 */
#define   FORMATCONTROL		4	/* 0x48 	or 0xa8 */
#define   RXDATA		6	/* 0x4a 	or 0xaa */
#define   TXDATA		6	/* 0x4a 	or 0xaa */
#define   INTRCONTROL		8	/* 0x4c 	or 0xac */
#define   INTRSTATUS		10	/* 0x4e 	or 0xae */
#define   SERIAL_STATUS		0x48
#define   SERIAL_FMTCNTL	0x48
#define   SERIAL_INTRCNTL	0x4c
#define   MODEM_STATUS		0xa8
#define   MODEM_FMTCNTL		0xa8
#define   MODEM_INTRCNTL	0xac

extern unsigned char serialconfig, modemconfig;
typedef unsigned char byte;
extern byte model;
extern void serial_power(int on_off);
extern void modem_power(int on_off);
extern int init_com(int dev, int wordlen, char parity);
#define ASY_MAX 2  /* Two asynch ports allowed on the P+ ( serial & modem) */
extern unsigned nasy;

struct plusasy {
	interrupt (*oldrx)();/* original interrupt rx vector    */
	interrupt (*oldtx)();/* original interrupt tx vector    */
};

extern struct plusasy plusasy[];
