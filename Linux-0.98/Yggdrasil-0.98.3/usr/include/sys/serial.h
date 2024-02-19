#ifndef SYS_SERIAL_H
#define SYS_SERIAL_H
/*
 * Defines for PC AT serial port.
 */

/* 
 * serial port addresses and IRQs
 */

#define PORT_0		0x03F8
#define PORT_1		0x02F8
#define IRQ_0		0x04
#define IRQ_1		0x03

/*
 * Definitions for INS8250 / 16550  chips
 */

/* defined as offsets from the port address (data port) */
#define DAT	0	/* receive/transmit data */
#define ICR	1	/* interrupt control register */
#define ISR	2	/* interrupt status register */
#define LCR	3	/* line control register */
#define MCR	4	/* modem control register */
#define LSR	5	/* line status register */
#define MSR	6	/* modem status register */
#define DLL	0	/* divisor latch (lsb) */
#define DLH	1	/* divisor latch (msb) */


/* ICR */

#define RIEN	0x01	/* enable receiver interrupt */
#define TIEN	0x02	/* enable transmitter interrupt */
#define SIEN	0x04	/* enable receiver line status interrupt */
#define MIEN	0x08	/* enable modem status interrupt */


/* ISR */

#define FFTMOUT	0x0c	/* fifo rcvr timeout */
#define RSTATUS	0x06	/* change in receiver line status */
#define RxRDY	0x04	/* receiver data available */
#define TxRDY	0x02	/* transmitter holding register empty */
#define MSTATUS	0x00	/* change in modem status */


/* LCR 3 */

/* number of data bits per received/transmitted character */
#define RXLEN	0x03
#define STOP1	0x00
#define STOP2	0x04
#define PAREN	0x08
#define PAREVN	0x10
#define PARMARK	0x20
#define SNDBRK	0x40
#define DLAB	0x80

/* baud rate definitions */
#define ASY9600	12

/* definitions for character length (data bits) in RXLEN field */
#define BITS5	0x00
#define BITS6	0x01
#define BITS7	0x02
#define BITS8	0x03

/* MCR */

#define DTR	0x01	/* bring up DTR */
#define RTS	0x02	/* bring up RTS */
#define OUT1	0x04
#define OUT2	0x08
#define LOOP	0x10	/* put chip into loopback state */


/* LSR */

#define RCA	0x01	/* receive char available */
#define OVRRUN	0x02	/* receive overrun */
#define PARERR	0x04	/* parity error */
#define FRMERR	0x08	/* framing/CRC error */
#define BRKDET	0x10	/* break detected (null char + frame error) */
#define XHRE	0x20	/* transmit holding register empty */
#define XSRE	0x40	/* transmit shift register empty */


/* MSR */

#define DCTS	0x01	/* CTS has changed state */
#define DDSR	0x02	/* DSR has changed state */
#define DRI	0x04	/* RI has changed state */
#define DDCD	0x08    /* DCD has changed state */
#define CTS	0x10	/* state of CTS */
#define DSR	0x20	/* state of DSR */
#define RI      0x40    /* state of RI */
#define DCD     0x80    /* state of DCD */


#define DELTAS(x)	((x)&(DCTS|DDSR|DRI|DDCD))
#define STATES(x)	((x)(CTS|DSR|RI|DCD))

#endif /* SYS_SERIAL_H */
