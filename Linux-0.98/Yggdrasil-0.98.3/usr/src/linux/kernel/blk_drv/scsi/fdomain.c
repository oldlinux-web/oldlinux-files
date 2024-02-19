/* fdomain.c -- Future Domain TMC-1660/TMC-1680 driver
 * Created: Sun May  3 18:53:19 1992
 * Revised: Tue Jul 28 19:45:25 1992 by root
 * Author: Rickard E. Faith, faith@cs.unc.edu
 * Copyright 1992 Rickard E. Faith
 *
 * $Log$

 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.

 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.

 * WARNING: THIS IS A BETA VERSION!
 *          USE AT YOUR OWN RISK!
 *          BACKUP YOUR SYSTEM BEFORE USING!

 * I would like to thank Maxtor, whose *free* 206 page manual on the LXT
 * drives was very helpful: "LXT SCSI Products: Specifications and OEM
 * Technical Manual (Revision B/September 1991)"

 * I wish that I could thank Future Domain for the necessary documentation,
 * but I can't.  I used the $25 "TMC-1800 SCSI Chip Specification" document
 * (FDC-1800T), which documents the *chip* and not the board.  Without it,
 * I would have been totally lost, but it would have been nice to have some
 * example source.  (The DOS BIOS source cost $250 and the UN*X driver
 * source was $750 [both required a non-disclosure agreement].  Ever wonder
 * why there are no freely available Future Domain drivers?)

 * Thanks to Todd Carrico (todd@wutc.wustl.edu), Dan Poirier
 * (poirier@cs.unc.edu ), Ken Corey (kenc@sol.acs.unt.edu), and C. de Bruin
 * (bruin@dutiba.tudelft.nl) for alpha testing.  Also thanks to Drew
 * Eckhardt (drew@cs.colorado.edu) for answering questions. */

#include <linux/config.h>

#ifdef CONFIG_SCSI_FUTURE_DOMAIN

#include <linux/sched.h>
#include <asm/io.h>
#include "fdomain.h"
#include "scsi.h"
#include "hosts.h"
#if QUEUE
#include <asm/system.h>
#include <linux/errno.h>
#endif

#define VERSION          "1.9"	/* Change with each revision */
#define DEBUG            1	/* Enable debugging output */
#define SEND_IDENTIFY    0	/* Send IDENTIFY message -- DOESN'T WORK! */
#define USE_FIFO         1	/* Use the FIFO buffer for I/O */
#define FAST_SYNCH       1	/* Enable Fast Synchronous */
#define ALLOW_ALL_IRQ    0	/* Allow all IRQ's -- NOT RECOMMENDED */
#define NEW_IRQ          1	/* Enable new IRQ handling */
#define DECREASE_IL      1	/* Try to decrease interrupt latency */

#if DEBUG
#define EVERY_ACCESS     0	/* Write a line on every scsi access */
#define ERRORS_ONLY      1	/* Only write a line if there is an error */
#define DEBUG_DETECT     0	/* Debug fdomain_16x0_detect() */
#else
#define EVERY_ACCESS     0	/* LEAVE THESE ALONE--CHANGE THE ONES ABOVE */
#define ERRORS_ONLY      0
#define DEBUG_DETECT     0
#endif

/* Errors are reported on the line, so we don't need to report them again */
#if EVERY_ACCESS
#undef ERRORS_ONLY
#define ERRORS_ONLY      0
#endif

static int               port_base = 0;
static void              *bios_base = NULL;
static int               interrupt_level = 0;
static volatile int      aborted = 0;

static int               Data_Mode_Cntl_port;
static int               FIFO_Data_Count_port;
static int               Interrupt_Cntl_port;
static int               Read_FIFO_port;
static int               Read_SCSI_Data_port;
static int               SCSI_Cntl_port;
static int               SCSI_Status_port;
static int               TMC_Cntl_port;
static int               TMC_Status_port;
static int               Write_FIFO_port;
static int               Write_SCSI_Data_port;

#if QUEUE
static unsigned char     current_target = 0;
static unsigned char     current_cmnd[10] = { 0, };
static void              *current_buff = NULL;
static int               current_bufflen = 0;
static void              (*current_done)(int,int) = NULL;

volatile static int      in_command = 0;
volatile static int      current_phase;
static int               this_host = 0;

enum { in_arbitration, in_selection, in_other };

#if NEW_IRQ
extern void              fdomain_16x0_intr( int unused );
#else
extern void              fdomain_16x0_interrupt();
#endif

static const char        *cmd_pt;
static const char        *the_command;
static unsigned char     *out_buf_pt;
static unsigned char     *in_buf_pt;
volatile static int      Status;
volatile static int      Message;
volatile static unsigned data_sent;
volatile static int      have_data_in;

volatile static int      in_interrupt_code = 0;

#endif


enum in_port_type { Read_SCSI_Data = 0, SCSI_Status = 1, TMC_Status = 2,
			  LSB_ID_Code = 5, MSB_ID_Code = 6, Read_Loopback = 7,
			  SCSI_Data_NoACK = 8, Option_Select = 10,
			  Read_FIFO = 12, FIFO_Data_Count = 14 };

enum out_port_type { Write_SCSI_Data = 0, SCSI_Cntl = 1, Interrupt_Cntl = 2,
			   Data_Mode_Cntl = 3, TMC_Cntl = 4, Write_Loopback = 7,
			   Write_FIFO = 12 };

static void *addresses[] = {
   (void *)0xc8000,
   (void *)0xca000,
   (void *)0xce000,
   (void *)0xde000 };
#define ADDRESS_COUNT (sizeof( addresses ) / sizeof( unsigned ))
		       
static unsigned short ports[] = { 0x140, 0x150, 0x160, 0x170 };
#define PORT_COUNT (sizeof( ports ) / sizeof( unsigned short ))

static unsigned short ints[] = { 3, 5, 10, 11, 12, 14, 15, 0 };

/*

  READ THIS BEFORE YOU ADD A SIGNATURE!

  READING THIS SHORT NOTE CAN SAVE YOU LOTS OF TIME!

  READ EVERY WORD, ESPECIALLY THE WORD *NOT*

  This driver works *ONLY* for Future Domain cards using the
  TMC-1600 chip.  This includes models TMC-1660 and TMC-1680
  *ONLY*.

  The following is a BIOS signature for a TMC-950 board, which
  looks like it is a 16 bit board (based on card edge), but
  which only uses the extra lines for IRQ's (not for data):

  FUTURE DOMAIN CORP. (C) 1986-1990 V7.009/18/90

  THIS WILL *NOT* WORK WITH THIS DRIVER!

  Here is another BIOS signature for yet another Future
  Domain board WHICH WILL *NOT* WORK WITH THIS DRIVER:

  FUTURE DOMAIN CORP. (C) 1986-1990 V6.0209/18/90

  Here is another BIOS signature for the TMC-88x series:

  FUTURE DOMAIN COPR. (C) 1986-1989 V6.0A7/28/90

  THIS WILL *NOT* WORK WITH THIS DRIVER, but it *WILL*
  work with the *SEAGATE* ST-01/ST-02 driver.

  */

struct signature {
   char *signature;
   int  sig_offset;
   int  sig_length;
} signatures[] = {
   { "FUTURE DOMAIN CORP. (C) 1986-1990 1800-V2.0 7/28/89", 5, 50 },
   { "FUTURE DOMAIN CORP. (C) 1986-1990 1800", 5, 37 },
   /* READ NOTICE ABOVE *BEFORE* YOU WASTE YOUR TIME ADDING A SIGANTURE */
};

#define SIGNATURE_COUNT (sizeof( signatures ) / sizeof( struct signature ))


/* These functions are based on include/asm/io.h */

#if 1
static unsigned short inline inw( unsigned short port )
{
   unsigned short _v;
   
   __asm__ volatile ("inw %1,%0"
		     :"=a" (_v):"d" ((unsigned short) port));
   return _v;
}

static void inline outw( unsigned short value, unsigned short port )
{
   __asm__ volatile ("outw %0,%1"
		     ::"a" ((unsigned short) value),
		     "d" ((unsigned short) port));
}
#else

#define inw( port ) \
      ({ unsigned short _v; \
	       __asm__ volatile ("inw %1,%0" \
				: "=a" (_v) : "d" ((unsigned short) port)); \
				      _v; })

#define outw( value ) \
      __asm__ volatile \
      ("outw %0,%1" : : "a" ((unsigned short) value), \
       "d" ((unsigned short) port))
#endif


/* These defines are copied from kernel/blk_drv/hd.c */

#define insw( buf, count, port ) \
      __asm__ volatile \
      ( "cld;rep;insw"::"d" (port),"D" (buf),"c" (count):"cx","di" )

#define outsw( buf, count, port) \
      __asm__ volatile \
      ("cld;rep;outsw"::"d" (port),"S" (buf),"c" (count):"cx","si")


static void do_pause( unsigned amount )	/* Pause for amount*10 milliseconds */
{
   unsigned long the_time = jiffies + amount; /* 0.01 seconds per jiffy */

   while (jiffies < the_time);
}

static void inline fdomain_make_bus_idle( void )
{
   outb( 0, SCSI_Cntl_port );
   outb( 0, Data_Mode_Cntl_port );
   outb( 1, TMC_Cntl_port );
}

static int fdomain_is_valid_port( int port )
{
   int options;

#if DEBUG_DETECT 
   printk( " (%x%x),",
	  inb( port + MSB_ID_Code ), inb( port + LSB_ID_Code ) );
#endif

   /* The MCA ID is a unique id for each MCA compatible board.  We
      are using ISA boards, but Future Domain provides the MCA ID
      anyway.  We can use this ID to ensure that this is a Future
      Domain TMC-1660/TMC-1680.
    */

   if (inb( port + LSB_ID_Code ) != 0xe9) { /* test for 0x6127 id */
      if (inb( port + LSB_ID_Code ) != 0x27) return 0;
      if (inb( port + MSB_ID_Code ) != 0x61) return 0;
   } else {			            /* test for 0xe960 id */
      if (inb( port + MSB_ID_Code ) != 0x60) return 0;
   }

   /* We have a valid MCA ID for a TMC-1660/TMC-1680 Future Domain board.
      Now, check to be sure the bios_base matches these ports.
      If someone was unlucky enough to have purchased more than one
      Future Domain board, then they will have to modify this code, as
      we only detect one board here.  [The one with the lowest bios_base].
    */

   options = inb( port + Option_Select );

#if DEBUG_DETECT
   printk( " Options = %x,", options );
#endif

   if (addresses[ (options & 0xc0) >> 6 ] != bios_base) return 0;
   interrupt_level = ints[ (options & 0x0e) >> 1 ];

   return 1;
}

static int fdomain_test_loopback( void )
{
   int i;
   int result;

   for (i = 0; i < 255; i++) {
      outb( i, port_base + Write_Loopback );
      result = inb( port_base + Read_Loopback );
      if (i != result) return 1;
   }
   return 0;
}

#if !NEW_IRQ
static void fdomain_enable_interrupt( void )
{
   if (!interrupt_level) return;

#if ALLOW_ALL_IRQ
   if (interrupt_level < 8) {
      outb( inb_p( 0x21 ) & ~(1 << interrupt_level), 0x21 );
   } else
#endif
	 {
	    outb( inb_p( 0xa1 ) & ~(1 << (interrupt_level - 8)), 0xa1 );
	 }
}

static void fdomain_disable_interrupt( void )
{
   if (!interrupt_level) return;

#if ALLOW_ALL_IRQ
   if (interrupt_level < 8) {
      outb( inb_p( 0x21 ) | (1 << interrupt_level), 0x21 );
   } else
#endif
	 {
	    outb( inb_p( 0xa1 ) | (1 << (interrupt_level - 8)), 0xa1 );
	 }
}
#endif

int fdomain_16x0_detect( int hostnum )
{
   int           i, j;
   int           flag;
   unsigned char do_inquiry[] =       { 0x12, 0, 0, 0, 255, 0 };
   unsigned char do_request_sense[] = { 0x03, 0, 0, 0, 255, 0 };
   unsigned char do_read_capacity[] = { 0x25, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   unsigned char buf[256];
   unsigned      retcode;

#if DEBUG_DETECT
   printk( "SCSI: fdomain_16x0_detect()," );
#endif

   for (i = 0; !bios_base && i < ADDRESS_COUNT; i++) {
#if DEBUG_DETECT
      printk( " %x(%x),", (unsigned)addresses[i], (unsigned)bios_base );
#endif
      for (j = 0; !bios_base && j < SIGNATURE_COUNT; j++) {
	 if (!memcmp( ((char *)addresses[i] + signatures[j].sig_offset),
		    signatures[j].signature, signatures[j].sig_length )) {
	    bios_base = addresses[i];
	 }
      }
   }

   if (!bios_base) {
#if DEBUG_DETECT
      printk( " FAILED: NO BIOS\n" );
#endif
      return 0;
   }

   /* The TMC-1660/TMC-1680 has a RAM area just after the BIOS ROM.
      Assuming the ROM is enabled (otherwise we wouldn't have been
      able to read the ROM signature :-), then the ROM set up the
      RAM area with some magic numbers, such as a list of port
      base addresses and a list of the disk "geometry" reported to
      DOS (this geometry has nothing to do with physical geometry).
    */

   port_base = *((char *)bios_base + 0x1fcc)
	 + (*((char *)bios_base + 0x1fcd) << 8);
   
#if DEBUG_DETECT
   printk( " %x,", port_base );
#endif

   for (flag = 0, i = 0; !flag && i < PORT_COUNT; i++) {
      if (port_base == ports[i]) ++flag;
   }

   if (flag) flag = fdomain_is_valid_port( port_base );

   if (!flag) {			/* Cannot get port base from BIOS RAM */
      
      /* This is a bad sign.  It usually means that someone patched the
	 BIOS signature list (the signatures variable) to contain a BIOS
	 signature for a board *OTHER THAN* the TMC-1660/TMC-1680.
       */
      
#if DEBUG_DETECT
      printk( " RAM FAILED, " );
#endif
      /* Anyway, the alternative to finding the address in the RAM is
	 to just search through every possible port address for one
	 that is attached to the Future Domain card.  Don't panic,
	 though, about reading all these random port addresses--there
	 are rumors that the Future Domain BIOS does something very
	 similar.
       */

      for (flag = 0, i = 0; !flag && i < PORT_COUNT; i++) {
	 port_base = ports[i];
#if DEBUG_DETECT
	 printk( " %x,", port_base );
#endif
	 flag = fdomain_is_valid_port( port_base );
      }
   }

   if (!flag) {
#if DEBUG_DETECT
      printk( " FAILED: NO PORT\n" );
#endif
      return 0;		/* Cannot find valid set of ports */
   }

#if DEBUG_DETECT
   printk( "\n" );
   printk( "SCSI: bios_base = %x, port_base = %x, interrupt_level = %d\n",
	  (unsigned)bios_base, port_base, interrupt_level );
#endif

   if (interrupt_level) {
      printk( "Future Domain BIOS at %x; port base at %x; using IRQ %d\n",
	     (unsigned)bios_base, port_base, interrupt_level );
   } else {
      printk( "Future Domain BIOS at %x; port base at %x; *NO* IRQ\n",
	     (unsigned)bios_base, port_base );
   }
   
   Data_Mode_Cntl_port  = port_base + Data_Mode_Cntl;
   FIFO_Data_Count_port = port_base + FIFO_Data_Count;
   Interrupt_Cntl_port  = port_base + Interrupt_Cntl;
   Read_FIFO_port       = port_base + Read_FIFO;
   Read_SCSI_Data_port  = port_base + Read_SCSI_Data;
   SCSI_Cntl_port       = port_base + SCSI_Cntl;
   SCSI_Status_port     = port_base + SCSI_Status;
   TMC_Cntl_port        = port_base + TMC_Cntl;
   TMC_Status_port      = port_base + TMC_Status;
   Write_FIFO_port      = port_base + Write_FIFO;
   Write_SCSI_Data_port = port_base + Write_SCSI_Data;

   fdomain_16x0_reset();

   if (fdomain_test_loopback()) {
#if DEBUG_DETECT
      printk( "SCSI: LOOPBACK TEST FAILED, FAILING DETECT!\n" );
#endif
      return 0;
   }

   /* These routines are here because of the way the SCSI bus behaves
      after a reset.  This appropriate behavior was not handled correctly
      by the higher level SCSI routines when I first wrote this driver.
    */
   
   printk( "Future Domain detection routine scanning for devices:\n" );
   for (i = 0; i < 8; i++) {
      if (i == 6) continue;	/* The host adapter is at SCSI ID 6 */
      retcode = fdomain_16x0_command( i, do_request_sense, buf, 255 );
      if (!retcode) {
	 retcode = fdomain_16x0_command( i, do_inquiry, buf, 255 );
	 if (!retcode) {
	    printk( "     SCSI ID %d: ", i );
	    for (j = 8; j < 32; j++) printk( "%c", buf[j] );
	    retcode = fdomain_16x0_command( i, do_read_capacity, buf, 255 );
	    if (!retcode) {
	       unsigned long blocks, size, capacity;
	       
	       blocks = (buf[0] << 24) | (buf[1] << 16)
		     | (buf[2] << 8) | buf[3];
	       size = (buf[4] << 24) | (buf[5] << 16) | (buf[6] << 8) | buf[7];
	       capacity = +(blocks * size * 10) / +(1024L * 1024L);

	       printk( "%lu MB (%lu byte blocks)",
		      ((capacity + 5L) / 10L), size );
	    }
	    printk ("\n" );
	 }
      }
   }

#if QUEUE
#if !ALLOW_ALL_IRQ
   if (interrupt_level < 8) {
      printk( "Future Domain: WILL NOT USE IRQ LESS THAN 8 FOR QUEUEING!\n" );
      scsi_hosts[hostnum].can_queue = 0;
   } else
#endif
#if NEW_IRQ
	 {
	    int              retcode;
	    struct sigaction sa;

	    this_host      = hostnum;

	    sa.sa_handler  = fdomain_16x0_intr;
	    sa.sa_flags    = SA_INTERRUPT;
	    sa.sa_mask     = 0;
	    sa.sa_restorer = NULL;

	    retcode = irqaction( interrupt_level, &sa );

	    if (retcode < 0) {
	       if (retcode == -EINVAL) {
		  printk( "Future Domain: IRQ %d is bad!\n", interrupt_level );
		  printk( "       This shouldn't happen: REPORT TO RIK!\n" );
	       } else if (retcode == -EBUSY) {
		  printk( "Future Domain: IRQ %d is already in use!\n",
			 interrupt_level );
		  printk( "       Please use another IRQ for the FD card!\n" );
	       } else {
		  printk( "Future Domain: Error getting IRQ %d\n",
			 interrupt_level );
		  printk( "       This shouldn't happen: REPORT TO RIK!\n" );
	       }
	       printk( "       IRQs WILL NOT BE USED!\n" );
	       
	       scsi_hosts[this_host].can_queue = 0;
	    } else {
	       printk( "Future Domain: IRQ %d selected with retcode = %d\n",
		       interrupt_level, retcode );
	    }
	 }
#else
	 {
	    this_host = hostnum;
	    set_intr_gate( 0x20 + interrupt_level, &fdomain_16x0_interrupt );
	    fdomain_enable_interrupt();
	 }
#endif
#endif

   return 1;
}

char *fdomain_16x0_info(void)
{
   static char buffer[] =
	 "Future Domain TMC-1660/TMC-1680 SCSI driver version "
	       VERSION
		     "\n";
   return buffer;
}

static int fdomain_arbitrate( void )
{
   int           status = 0;
   unsigned long timeout;

#if VERBOSE
   printk( "SCSI: fdomain_arbitrate()\n" );
#endif
   
   outb( 0x00, SCSI_Cntl_port );              /* Disable data drivers */
   outb( 0x40, port_base + SCSI_Data_NoACK ); /* Set our id bit */
   outb( 0x04, TMC_Cntl_port );	              /* Start arbitration */

   timeout = jiffies + 50;	              /* 500 mS */
   while (jiffies < timeout) {
      status = inb( TMC_Status_port );        /* Read adapter status */
      if (status & 0x02) return 0;            /* Arbitration complete */
   }

   /* Make bus idle */
   fdomain_make_bus_idle();

#if EVERY_ACCESS
   printk( "Arbitration failed, status = %x\n", status );
#endif
#if ERRORS_ONLY
   printk( "SCSI: Arbitration failed, status = %x", status );
#endif
   return 1;
}

static int fdomain_select( int target )
{
   int           status;
   unsigned long timeout;

   outb( 0x80, SCSI_Cntl_port );	/* Bus Enable */
   outb( 0x8a, SCSI_Cntl_port );	/* Bus Enable + Attention + Select */

   /* Send our address OR'd with target address */
#if SEND_IDENTIFY
   outb( 0x40 | (1 << target), port_base + SCSI_Data_NoACK );
#else
   outb( (1 << target), port_base + SCSI_Data_NoACK );
#endif

    /* Stop arbitration (also set FIFO for output and enable parity) */
   outb( 0xc8, TMC_Cntl_port ); 

   timeout = jiffies + 25;	        /* 250mS */
   while (jiffies < timeout) {
      status = inb( SCSI_Status_port ); /* Read adapter status */
      if (status & 1) {		        /* Busy asserted */
	 /* Enable SCSI Bus (on error, should make bus idle with 0) */
#if SEND_IDENTIFY
	 /* Also, set ATN so that the drive will make a MESSAGE OUT phase */
	 outb( 0x88, SCSI_Cntl_port );
#else
	 outb( 0x80, SCSI_Cntl_port );
#endif
	 return 0;
      }
   }
   /* Make bus idle */
   fdomain_make_bus_idle();
#if EVERY_ACCESS
   if (!target) printk( "Select failed\n" );
#endif
#if ERRORS_ONLY
   if (!target) printk( "SCSI: Select failed" );
#endif
   return 1;
}

#if QUEUE

#if !USE_FIFO
#pragma error QUEUE requires USE_FIFO
#endif

void my_done( int error )
{
   if (in_command) {
      in_command = 0;
      in_interrupt_code = 0;
      outb( 0x00, Interrupt_Cntl_port );
      fdomain_make_bus_idle();
      if (current_done) current_done( this_host, error );
      else panic( "SCSI (Future Domain): current_done() == NULL" );
   } else {
      panic( "SCSI (Future Domain): my_done() called outside of command\n" );
   }
}

#if NEW_IRQ
void fdomain_16x0_intr( int unused )
#else
void fdomain_16x0_intr( void )
#endif
{
   int      status;
   int      done = 0;
   unsigned data_count;

#if NEW_IRQ
   sti();
#endif

   if (in_interrupt_code)
	 panic( "SCSI (Future Domain): fdomain_16x0_intr() NOT REENTRANT!\n" );
   else
	 ++in_interrupt_code;
   
   outb( 0x00, Interrupt_Cntl_port );

#if EVERY_ACCESS
   printk( "aborted = %d, ", aborted );
#endif

   if (aborted) {
      /* Force retry for timeouts after selection complete */
      if (current_phase == in_other)
	    my_done( DID_BUS_BUSY << 16 );
      else
	    my_done( aborted << 16 );
#if NEW_IRQ && !DECREASE_IL
      cli();
#endif
      return;
   }

   /* We usually have one spurious interrupt after each command.  Ignore it. */
   if (!in_command) {		/* Spurious interrupt */
      in_interrupt_code = 0;
#if NEW_IRQ && !DECREASE_IL
      cli();
#endif
      return;
   }
   
   if (current_phase == in_arbitration) {
      status = inb( TMC_Status_port );        /* Read adapter status */
      if (!(status & 0x02)) {
#if EVERY_ACCESS
	 printk( " AFAIL " );
#endif
	 my_done( DID_TIME_OUT << 16 );
#if NEW_IRQ && !DECREASE_IL
	 cli();
#endif
	 return;
      }
      current_phase = in_selection;

      outb( 0x80, SCSI_Cntl_port );	/* Bus Enable */
      outb( 0x8a, SCSI_Cntl_port );	/* Bus Enable + Attention + Select */
      
      outb( (1 << current_target), port_base + SCSI_Data_NoACK );

      outb( 0x40, Interrupt_Cntl_port );
      /* Stop arbitration (also set FIFO for output and enable parity) */
      in_interrupt_code = 0;
      outb( 0xd8, TMC_Cntl_port );
#if NEW_IRQ && !DECREASE_IL
      cli();
#endif
      return;
   } else if (current_phase == in_selection) {
      status = inb( SCSI_Status_port );
      if (!(status & 0x01)) {
#if EVERY_ACCESS
	 printk( " SFAIL " );
#endif
	 my_done( DID_NO_CONNECT << 16 );
#if NEW_IRQ && !DECREASE_IL
	 cli();
#endif
	 return;
      }
      current_phase = in_other;
#if FAST_SYNCH
      outb( 0xc0, Data_Mode_Cntl_port );
#endif
      in_interrupt_code = 0;
      outb( 0x90, Interrupt_Cntl_port );
      outb( 0x80, SCSI_Cntl_port );
#if NEW_IRQ && !DECREASE_IL
      cli();
#endif
      return;
   }

   /* current_phase == in_other: this is the body of the routine */

   switch ((unsigned char)*the_command) {
   case 0x04: case 0x07: case 0x0a: case 0x15: case 0x2a:
   case 0x2e: case 0x3b: case 0xea: case 0x3f:
      data_count = 0x2000 - inw( FIFO_Data_Count_port );
      if (current_bufflen - data_sent < data_count)
	    data_count = current_bufflen - data_sent;
      if (data_count > 0) {
/* 	 if (data_count > 512) data_count = 512; */
#if EVERY_ACCESS
	 printk( "%d OUT, ", data_count );
#endif
	 if (data_count == 1) {
	    outb( *out_buf_pt++, Write_FIFO_port );
	    ++data_sent;
	 } else {
	    data_count >>= 1;
	    outsw( out_buf_pt, data_count, Write_FIFO_port );
	    out_buf_pt += 2 * data_count;
	    data_sent += 2 * data_count;
	 }
      }
      break;
   default:
      if (!have_data_in) {
	 outb( 0x98, TMC_Cntl_port );
	 ++have_data_in;
      } else {
	 data_count = inw( FIFO_Data_Count_port );
/* 	 if (data_count > 512) data_count = 512; */
	 if (data_count) {
#if EVERY_ACCESS
	    printk( "%d IN, ", data_count );
#endif
	    if (data_count == 1) {
	       *in_buf_pt++ = inb( Read_FIFO_port );
	    } else {
	       data_count >>= 1; /* Number of words */
	       insw( in_buf_pt, data_count, Read_FIFO_port );
	       in_buf_pt += 2 * data_count;
	    }
	 }
      }
      break;
   }

   status = inb( SCSI_Status_port );

   if (status & 0x10) {	/* REQ */
      
      switch (status & 0x0e) {
      case 0x08:		/* COMMAND OUT */
	 outb( *cmd_pt++, Write_SCSI_Data_port );
#if EVERY_ACCESS
	 printk( "CMD = %x,", (unsigned char)cmd_pt[-1] );
#endif
	 break;
      case 0x0c:		/* STATUS IN */
	 Status = inb( Read_SCSI_Data_port );
#if EVERY_ACCESS
	 printk( "Status = %x, ", Status );
#endif
#if ERRORS_ONLY
	 if (Status) {
	    printk( "SCSI: target = %d, command = %x, Status = %x\n",
		   current_target, (unsigned char)*the_command, Status );
	 }
#endif
	 break;
      case 0x0a:		/* MESSAGE OUT */
#if SEND_IDENTIFY
	 /* On the first request, send an Identify message */
	 if (!sent_identify) {
	    outb( 0x80, SCSI_Cntl_port );          /* Lower ATN */
	    outb( 0x80, Write_SCSI_Data_port );    /* Identify */
	    ++sent_identify;
	 } else
#else
	       outb( 0x07, Write_SCSI_Data_port ); /* Reject */
#endif
	 break;
      case 0x0e:		/* MESSAGE IN */
	 Message = inb( Read_SCSI_Data_port );
#if EVERY_ACCESS
	 printk( "Message = %x, ", Message );
#endif
	 if (!Message) ++done;
	 break;
      }
   }

   if (done) {
#if EVERY_ACCESS
      printk( " ** IN DONE ** " );
#endif

      if (have_data_in) {
	 while (data_count = inw( FIFO_Data_Count_port )) {
	    if (data_count == 1) {
	       *in_buf_pt++ = inb( Read_FIFO_port );
	    } else {
	       data_count >>= 1; /* Number of words */
	       insw( in_buf_pt, data_count, Read_FIFO_port );
	       in_buf_pt += 2 * data_count;
	    }
	 }
      }
#if EVERY_ACCESS
      printk( "AFTER DATA GET\n" );
#endif
      
#if ERRORS_ONLY
      if (*the_command == REQUEST_SENSE && !Status) {
	 if ((unsigned char)(*((char *)current_buff + 2)) & 0x0f) {
	    printk( "SCSI REQUEST SENSE: Sense Key = %x, Sense Code = %x\n",
		   (unsigned char)(*((char *)current_buff + 2)) & 0x0f,
		   (unsigned char)(*((char *)current_buff + 12)) );
	 }
      }
#endif
#if EVERY_ACCESS
      printk( "BEFORE MY_DONE\n" );
#endif
      my_done( (Status & 0xff) | ((Message & 0xff) << 8) | (DID_OK << 16) );
   } else {
      in_interrupt_code = 0;
      outb( 0x90, Interrupt_Cntl_port );
   }

#if NEW_IRQ && !DECREASE_IL
   cli();
#endif
   return;
}

int fdomain_16x0_queue( unsigned char target, const void *cmnd,
			 void *buff, int bufflen, void (*done)(int,int) )
{
   if (in_command) {
      panic( "SCSI (Future Domain): fdomain_16x0_queue() NOT REENTRANT!\n" );
   }
#if EVERY_ACCESS
   printk( "queue %d %x\n", target, *(unsigned char *)cmnd );
#endif

   fdomain_make_bus_idle();

   aborted         = 0;
   current_target  = target;
   memcpy( current_cmnd, cmnd, ((*(unsigned char *)cmnd) <= 0x1f ? 6 : 10 ) );
   current_buff    = buff;
   current_bufflen = bufflen;
   current_done    = done;

   /* Initialize static data */
   cmd_pt          = current_cmnd;
   the_command     = current_cmnd;
   out_buf_pt      = current_buff;
   in_buf_pt       = current_buff;
   
   Status          = 0;
   Message         = 0;
   data_sent       = 0;
   have_data_in    = 0;

   /* Start arbitration */
   current_phase = in_arbitration;
   outb( 0x00, Interrupt_Cntl_port );
   outb( 0x00, SCSI_Cntl_port );              /* Disable data drivers */
   outb( 0x40, port_base + SCSI_Data_NoACK ); /* Set our id bit */
   ++in_command;
   outb( 0x20, Interrupt_Cntl_port );
   outb( 0x1c, TMC_Cntl_port );	              /* Start arbitration */

   return 0;
}
#endif

int fdomain_16x0_command( unsigned char target, const void *cmnd,
			 void *buff, int bufflen )
{
   const char     *cmd_pt = cmnd;
   const char     *the_command = cmnd;
   unsigned char  *out_buf_pt = buff;
   unsigned char  *in_buf_pt = buff;
   int            Status = 0;
   int            Message = 0;
   int            status;
   int            done = 0;
   unsigned long  timeout;
   unsigned       data_sent = 0;
   unsigned       data_count;
#if USE_FIFO
   int            have_data_in = 0;
#endif
#if SEND_IDENTITY
   int            sent_identify = 0;
#endif

#if EVERY_ACCESS
   printk( "fdomain_command(%d, %x): ", target, (unsigned char)*the_command );
#endif

   if (fdomain_arbitrate()) {
#if ERRORS_ONLY
      printk( ", target = %d, command = %x\n",
	     target, (unsigned char)*the_command );
#endif
      return DID_TIME_OUT << 16;
   }

   if (fdomain_select( target )) {
#if ERRORS_ONLY
      if (!target) printk( ", target = %d, command = %x\n",
			 target, (unsigned char)*the_command );
#endif
      return DID_NO_CONNECT << 16;
   }

   timeout = jiffies + 500;	/* 5000 mS -- For Maxtor after a RST */
   aborted = 0;			/* How is this supposed to get reset??? */

#if FAST_SYNCH
   outb( 0xc0, Data_Mode_Cntl_port );
#endif

#if USE_FIFO
   switch ((unsigned char)*the_command) {
   case 0x04: case 0x07: case 0x0a: case 0x15: case 0x2a:
   case 0x2e: case 0x3b: case 0xea: case 0x3f:
      data_count = 0x2000 - inw( FIFO_Data_Count_port );
      if (bufflen - data_sent < data_count)
	    data_count = bufflen - data_sent;
      if (data_count == 1) {
	 outb( *out_buf_pt++, Write_FIFO_port );
	 ++data_sent;
      } else {
	 data_count >>= 1;
	 outsw( out_buf_pt, data_count, Write_FIFO_port );
	 out_buf_pt += 2 * data_count;
	 data_sent += 2 * data_count;
      }
      break;
   default:
      outb( 0x88, TMC_Cntl_port );
      ++have_data_in;
      break;
   }
#endif
   
   while (((status = inb( SCSI_Status_port )) & 1)
	  && !done && !aborted && jiffies < timeout) {
      
      if (status & 0x10) {	/* REQ */

	 switch (status & 0x0e) {
	 case 0x00:		/* DATA OUT */
#if USE_FIFO
	    data_count = 0x2000 - inw( FIFO_Data_Count_port );
	    if (bufflen - data_sent < data_count)
		  data_count = bufflen - data_sent;
	    if (data_count == 1) {
	       outb( *out_buf_pt++, Write_FIFO_port );
	       ++data_sent;
	    } else {
	       data_count >>= 1;
	       outsw( out_buf_pt, data_count, Write_FIFO_port );
	       out_buf_pt += 2 * data_count;
	       data_sent += 2 * data_count;
	    }
#else
	    outb( *out_buf_pt++, Write_SCSI_Data_port );
#endif
	    break;
	 case 0x04:		/* DATA IN */
#if USE_FIFO
	    if (!have_data_in) {
	       outb( 0x88, TMC_Cntl_port );
	       ++have_data_in;
	    }
	    data_count = inw( FIFO_Data_Count_port );
	    if (data_count == 1) {
	       *in_buf_pt++ = inb( Read_FIFO_port );
	    } else {
	       data_count >>= 1; /* Number of words */
	       insw( in_buf_pt, data_count, Read_FIFO_port );
	       in_buf_pt += 2 * data_count;
	    }
#else
	    *in_buf_pt++ = inb( Read_SCSI_Data_port );
#endif
	    break;
	 case 0x08:		/* COMMAND OUT */
	    outb( *cmd_pt++, Write_SCSI_Data_port );
#if EVERY_ACCESS
	    printk( "%x,", (unsigned char)cmd_pt[-1] );
#endif
	    break;
	 case 0x0c:		/* STATUS IN */
	    Status = inb( Read_SCSI_Data_port );
#if EVERY_ACCESS
	    printk( "Status = %x, ", Status );
#endif
#if ERRORS_ONLY
	    if (Status) {
	       printk( "SCSI: target = %d, command = %x, Status = %x\n",
		      target, (unsigned char)*the_command, Status );
	    }
#endif
	    break;
	 case 0x0a:		/* MESSAGE OUT */
#if SEND_IDENTIFY
	    /* On the first request, send an Identify message */
	    if (!sent_identify) {
	       outb( 0x80, SCSI_Cntl_port );          /* Lower ATN */
	       outb( 0x80, Write_SCSI_Data_port );    /* Identify */
	       ++sent_identify;
	    } else
#else
		  outb( 0x07, Write_SCSI_Data_port ); /* Reject */
#endif
	    break;
	 case 0x0e:		/* MESSAGE IN */
	    Message = inb( Read_SCSI_Data_port );
#if EVERY_ACCESS
	    printk( "Message = %x, ", Message );
#endif
	    if (!Message) ++done;
	    break;
	 }
      }
   }

   if (jiffies >= timeout) {
#if EVERY_ACCESS
      printk( "Time out, status = %x\n", status );
#endif
#if ERRORS_ONLY
      printk( "SCSI: Time out, status = %x (target = %d, command = %x)\n",
	     status, target, (unsigned char)*the_command );
#endif
      fdomain_make_bus_idle();
      return DID_BUS_BUSY << 16;
   }

   if (aborted) {
#if EVERY_ACCESS
      printk( "Aborted\n" );
#endif
#if ONLY_ERRORS
      printk( "SCSI: Aborted (command = %x)\n", (unsigned char)*the_command );
#endif
      fdomain_16x0_reset();
      return DID_ABORT << 16;
   }
   
#if USE_FIFO
   if (have_data_in) {
      while (data_count = inw( FIFO_Data_Count_port )) {
	 if (data_count == 1) {
	    *in_buf_pt++ = inb( Read_FIFO_port );
	 } else {
	    data_count >>= 1; /* Number of words */
	    insw( in_buf_pt, data_count, Read_FIFO_port );
	    in_buf_pt += 2 * data_count;
	 }
      }
   }
#endif

   fdomain_make_bus_idle();

#if EVERY_ACCESS
   printk( "Retcode = %x\n",
	  (Status & 0xff) | ((Message & 0xff) << 8) | (DID_OK << 16) );
#endif
#if ERRORS_ONLY
   if (*the_command == REQUEST_SENSE && !Status) {
      if ((unsigned char)(*((char *)buff + 2)) & 0x0f) {
	 printk( "SCSI REQUEST SENSE: Sense Key = %x, Sense Code = %x\n",
		(unsigned char)(*((char *)buff + 2)) & 0x0f,
		(unsigned char)(*((char *)buff + 12)) );
      }
   }
#endif

   return (Status & 0xff) | ((Message & 0xff) << 8) | (DID_OK << 16);
}

int fdomain_16x0_abort( int code )
{

#if EVERY_ACCESS
   printk( " ABORT " );
#endif

#if QUEUE
   cli();
   if (!in_command) {
      sti();
      return 0;
   }

   aborted = code ? code : DID_ABORT;

   sti();
   fdomain_make_bus_idle();
#else
   aborted = code ? code : DID_ABORT;
#endif

   return 0;
}

int fdomain_16x0_reset( void )
{
   outb( 1, SCSI_Cntl_port );
   do_pause( 2 );
   outb( 0, SCSI_Cntl_port );
   do_pause( 115 );
   outb( 0, Data_Mode_Cntl_port );
   outb( 0, TMC_Cntl_port );

   aborted = DID_RESET;

   return 0;
}

#if QUEUE && !NEW_IRQ

/* This is copied from kernel/sys_calls.s
   and from kernel/blk_drv/scsi/aha1542.c */

__asm__("
_fdomain_16x0_interrupt:
	cld
	push %gs
	push %fs
	push %es
	push %ds
	pushl %eax
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	movl $0x17,%edx
	mov %dx,%fs

	movl $_fdomain_disable_interrupt,%edx
	call *%edx

	movb $0x20,%al
	outb %al,$0xA0		# EOI to interrupt controller #1
	jmp 1f			# give port chance to breathe
1:	jmp 1f
1:	outb %al,$0x20

	sti
	movl $_fdomain_16x0_intr,%edx
	call *%edx		# ``interesting'' way of handling intr.
	cli

	movl $_fdomain_enable_interrupt,%edx
	call *%edx

	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popl %eax
	pop %ds
	pop %es
	pop %fs
	pop %gs
	iret
");
#endif

#endif
