/*
 *	ultrastor.c	Copyright (C) 1992 David B. Gentzel
 *	Low-level SCSI driver for UltraStor 14F
 *	by David B. Gentzel, Whitfield Software Services, Carnegie, PA
 *	    (gentzel@nova.enet.dec.com)
 *	Thanks to UltraStor for providing the necessary documentation
 */

/*
 * NOTES:
 *    The UltraStor 14F is an intelligent, high performance ISA SCSI-2 host
 *    adapter.  It is essentially an ISA version of the UltraStor 24F EISA
 *    adapter.  It supports first-party DMA, command queueing, and
 *    scatter/gather I/O.  It can also emulate the standard AT MFM/RLL/IDE
 *    interface for use with OS's which don't support SCSI.
 *
 *    This driver may also work (with some small changes) with the UltraStor
 *    24F.  I have no way of confirming this...
 *
 *    Places flagged with a triple question-mark are things which are either
 *    unfinished, questionable, or wrong.
 */

/*
 * CAVEATS: ???
 *    This driver is VERY stupid.  It takes no advantage of much of the power
 *    of the UltraStor controller.  I hope to go back and beat it into shape,
 *    but PLEASE, anyone else who would like to, please make improvements!
 *
 *    By defining NO_QUEUEING in ultrastor.h, you disable the queueing feature
 *    of the mid-level SCSI driver.  Once I'm satisfied that the queueing
 *    version is as stable as the non-queueing version, I'll eliminate this
 *    option.
 */

#include <linux/config.h>

#ifdef CONFIG_SCSI_ULTRASTOR

#include <linux/stddef.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/kernel.h>

#include <asm/io.h>
#include <asm/system.h>

#define ULTRASTOR_PRIVATE	/* Get the private stuff from ultrastor.h */
#include "ultrastor.h"
#include "scsi.h"
#include "hosts.h"

#define VERSION "1.0 beta"

#define ARRAY_SIZE(arr) (sizeof (arr) / sizeof (arr)[0])
#define BIT(n) (1ul << (n))
#define BYTE(num, n) ((unsigned char)((unsigned int)(num) >> ((n) * 8)))

/* Simply using "unsigned long" in these structures won't work as it causes
   alignment.  Perhaps the "aligned" attribute may be used in GCC 2.0 to get
   around this, but for now I use this hack. */
typedef struct {
    unsigned char bytes[4];
} Longword;

/* Used to fetch the configuration info from the config i/o registers.  We
   then store (in a friendlier format) in config. */
struct config_1 {
    unsigned char bios_segment: 3;
    unsigned char reserved: 1;
    unsigned char interrupt: 2;
    unsigned char dma_channel: 2;
};
struct config_2 {
    unsigned char ha_scsi_id: 3;
    unsigned char mapping_mode: 2;
    unsigned char bios_drive_number: 1;
    unsigned char tfr_port: 2;
};

/* Used to store configuration info read from config i/o registers.  Most of
   this is not used yet, but might as well save it. */
struct config {
    unsigned short port_address;
    const void *bios_segment;
    unsigned char interrupt: 4;
    unsigned char dma_channel: 3;
    unsigned char ha_scsi_id: 3;
    unsigned char heads: 6;
    unsigned char sectors: 6;
    unsigned char bios_drive_number: 1;
};

/* MailBox SCSI Command Packet.  Basic command structure for communicating
   with controller. */
struct mscp {
    unsigned char opcode: 3;		/* type of command */
    unsigned char xdir: 2;		/* data transfer direction */
    unsigned char dcn: 1;		/* disable disconnect */
    unsigned char ca: 1;		/* use cache (if available) */
    unsigned char sg: 1;		/* scatter/gather operation */
    unsigned char target_id: 3;		/* target SCSI id */
    unsigned char ch_no: 2;		/* SCSI channel (always 0 for 14f) */
    unsigned char lun: 3;		/* logical unit number */
    Longword transfer_data;		/* transfer data pointer */
    Longword transfer_data_length;	/* length in bytes */
    Longword command_link;		/* for linking command chains */
    unsigned char scsi_command_link_id;	/* identifies command in chain */
    unsigned char number_of_sg_list;	/* (if sg is set) 8 bytes per list */
    unsigned char length_of_sense_byte;
    unsigned char length_of_scsi_cdbs;	/* 6, 10, or 12 */
    unsigned char scsi_cdbs[12];	/* SCSI commands */
    unsigned char adapter_status;	/* non-zero indicates HA error */
    unsigned char target_status;	/* non-zero indicates target error */
    Longword sense_data;
};

/* Allowed BIOS base addresses for 14f (NULL indicates reserved) */
static const void *const bios_segment_table[8] = {
    NULL,	     (void *)0xC4000, (void *)0xC8000, (void *)0xCC000,
    (void *)0xD0000, (void *)0xD4000, (void *)0xD8000, (void *)0xDC000,
};

/* Allowed IRQs for 14f */
static const unsigned char interrupt_table[4] = { 15, 14, 11, 10 };

/* Allowed DMA channels for 14f (0 indicates reserved) */
static const unsigned char dma_channel_table[4] = { 5, 6, 7, 0 };

/* Head/sector mappings allowed by 14f */
static const struct {
    unsigned char heads;
    unsigned char sectors;
} mapping_table[4] = { { 16, 63 }, { 64, 32 }, { 64, 63 }, { 0, 0 } };

/* Config info */
static struct config config;

/* Our index in the host adapter array maintained by higher-level driver */
static int host_number;

/* PORT_ADDRESS is first port address used for i/o of messages. */
#ifdef PORT_OVERRIDE
# define PORT_ADDRESS PORT_OVERRIDE
#else
# define PORT_ADDRESS (config.port_address)
#endif

static volatile int aborted = 0;

#ifndef PORT_OVERRIDE
static const unsigned short ultrastor_ports[] = {
    0x330, 0x340, 0x310, 0x230, 0x240, 0x210, 0x130, 0x140,
};
#endif

static void ultrastor_interrupt(int cpl);

static void (*ultrastor_done)(int, int) = 0;

static const struct {
    const char *signature;
    size_t offset;
    size_t length;
} signatures[] = {
    { "SBIOS 1.01 COPYRIGHT (C) UltraStor Corporation,1990-1992.", 0x10, 57 },
};

int ultrastor_14f_detect(int hostnum)
{
    size_t i;
    unsigned char in_byte;
    struct config_1 config_1;
    struct config_2 config_2;

#if (ULTRASTOR_DEBUG & UD_DETECT)
    printk("US14F: detect: called\n");
#endif

#ifndef PORT_OVERRIDE
    PORT_ADDRESS = 0;
    for (i = 0; i < ARRAY_SIZE(ultrastor_ports); i++) {
	PORT_ADDRESS = ultrastor_ports[i];
#endif

#if (ULTRASTOR_DEBUG & UD_DETECT)
	printk("US14F: detect: testing port address %03X\n", PORT_ADDRESS);
#endif

	in_byte = inb(PRODUCT_ID(PORT_ADDRESS + 0));
	if (in_byte != US14F_PRODUCT_ID_0) {
#if (ULTRASTOR_DEBUG & UD_DETECT)
# ifdef PORT_OVERRIDE
	    printk("US14F: detect: wrong product ID 0 - %02X\n", in_byte);
# else
	    printk("US14F: detect: no adapter at port %03X\n", PORT_ADDRESS);
# endif
#endif
#ifdef PORT_OVERRIDE
	    return FALSE;
#else
	    continue;
#endif
	}
	in_byte = inb(PRODUCT_ID(PORT_ADDRESS + 1));
	/* Only upper nibble is defined for Product ID 1 */
	if ((in_byte & 0xF0) != US14F_PRODUCT_ID_1) {
#if (ULTRASTOR_DEBUG & UD_DETECT)
# ifdef PORT_OVERRIDE
	    printk("US14F: detect: wrong product ID 1 - %02X\n", in_byte);
# else
	    printk("US14F: detect: no adapter at port %03X\n", PORT_ADDRESS);
# endif
#endif
#ifdef PORT_OVERRIDE
	    return FALSE;
#else
	    continue;
#endif
	}
#ifndef PORT_OVERRIDE
	break;
    }
    if (i == ARRAY_SIZE(ultrastor_ports)) {
# if (ULTRASTOR_DEBUG & UD_DETECT)
	printk("US14F: detect: no port address found!\n");
# endif
	return FALSE;
    }
#endif

#if (ULTRASTOR_DEBUG & UD_DETECT)
    printk("US14F: detect: adapter found at port address %03X\n",
	   PORT_ADDRESS);
#endif

    /* All above tests passed, must be the right thing.  Get some useful
       info. */
    *(char *)&config_1 = inb(CONFIG(PORT_ADDRESS + 0));
    *(char *)&config_2 = inb(CONFIG(PORT_ADDRESS + 1));
    config.bios_segment = bios_segment_table[config_1.bios_segment];
    config.interrupt = interrupt_table[config_1.interrupt];
    config.dma_channel = dma_channel_table[config_1.dma_channel];
    config.ha_scsi_id = config_2.ha_scsi_id;
    config.heads = mapping_table[config_2.mapping_mode].heads;
    config.sectors = mapping_table[config_2.mapping_mode].sectors;
    config.bios_drive_number = config_2.bios_drive_number;

    /* To verify this card, we simply look for the UltraStor SCSI from the
       BIOS version notice. */
    if (config.bios_segment != NULL) {
	int found = 0;

	for (i = 0; !found && i < ARRAY_SIZE(signatures); i++)
	    if (memcmp((char *)config.bios_segment + signatures[i].offset,
		       signatures[i].signature, signatures[i].length))
		found = 1;
	if (!found)
	    config.bios_segment = NULL;
    }
    if (!config.bios_segment) {
#if (ULTRASTOR_DEBUG & UD_DETECT)
	printk("US14F: detect: not detected.\n");
#endif
	return FALSE;
    }

    /* Final consistancy check, verify previous info. */
    if (!config.dma_channel || !(config_2.tfr_port & 0x2)) {
#if (ULTRASTOR_DEBUG & UD_DETECT)
	printk("US14F: detect: consistancy check failed\n");
#endif
	return FALSE;
    }

    /* If we were TRULY paranoid, we could issue a host adapter inquiry
       command here and verify the data returned.  But frankly, I'm
       exhausted! */

    /* Finally!  Now I'm satisfied... */
#if (ULTRASTOR_DEBUG & UD_DETECT)
    printk("US14F: detect: detect succeeded\n"
	   "  Port address: %03X\n"
	   "  BIOS segment: %05X\n"
	   "  Interrupt: %u\n"
	   "  DMA channel: %u\n"
	   "  H/A SCSI ID: %u\n",
	   PORT_ADDRESS, config.bios_segment, config.interrupt,
	   config.dma_channel, config.ha_scsi_id);
#endif
    host_number = hostnum;
    scsi_hosts[hostnum].this_id = config.ha_scsi_id;
#ifndef NO_QUEUEING
    {
	struct sigaction sa;

	sa.sa_handler = ultrastor_interrupt;
	sa.sa_mask = 0;
	sa.sa_flags = SA_INTERRUPT;	/* ??? Do we really need this? */
	sa.sa_restorer = 0;
	if (irqaction(config.interrupt, &sa)) {
	    printk("Unable to get IRQ%u for UltraStor controller\n",
		   config.interrupt);
	    return FALSE;
	}
    }
#endif
    return TRUE;
}

const char *ultrastor_14f_info(void)
{
    return "UltraStor 14F SCSI driver version "
	   VERSION
	   " by David B. Gentzel\n";
}

static struct mscp mscp = {
    OP_SCSI, DTD_SCSI, 0, 1, 0		/* This stuff doesn't change */
};

int ultrastor_14f_queuecommand(unsigned char target, const void *cmnd,
			       void *buff, int bufflen, void (*done)(int, int))
{
    unsigned char in_byte;

#if (ULTRASTOR_DEBUG & UD_COMMAND)
    printk("US14F: queuecommand: called\n");
#endif

    /* Skip first (constant) byte */
    memset((char *)&mscp + 1, 0, sizeof (struct mscp) - 1);
    mscp.target_id = target;
    /* mscp.lun = ???; */
    mscp.transfer_data = *(Longword *)&buff;
    mscp.transfer_data_length = *(Longword *)&bufflen,
    mscp.length_of_scsi_cdbs = ((*(unsigned char *)cmnd <= 0x1F) ? 6 : 10);
    memcpy(mscp.scsi_cdbs, cmnd, mscp.length_of_scsi_cdbs);

    /* Find free OGM slot (OGMINT bit is 0) */
    do
	in_byte = inb_p(LCL_DOORBELL_INTR(PORT_ADDRESS));
    while (!aborted && (in_byte & 1));
    if (aborted) {
#if (ULTRASTOR_DEBUG & (UD_COMMAND | UD_ABORT))
	printk("US14F: queuecommand: aborted\n");
#endif
	/* ??? is this right? */
	return (aborted << 16);
    }

    /* Store pointer in OGM address bytes */
    outb_p(BYTE(&mscp, 0), OGM_DATA_PTR(PORT_ADDRESS + 0));
    outb_p(BYTE(&mscp, 1), OGM_DATA_PTR(PORT_ADDRESS + 1));
    outb_p(BYTE(&mscp, 2), OGM_DATA_PTR(PORT_ADDRESS + 2));
    outb_p(BYTE(&mscp, 3), OGM_DATA_PTR(PORT_ADDRESS + 3));

    /* Issue OGM interrupt */
    outb_p(0x1, LCL_DOORBELL_INTR(PORT_ADDRESS));

    ultrastor_done = done;

#if (ULTRASTOR_DEBUG & UD_COMMAND)
    printk("US14F: queuecommand: returning\n");
#endif

    return 0;
}

#ifdef NO_QUEUEING
int ultrastor_14f_command(unsigned char target, const void *cmnd,
			  void *buff, int bufflen)
{
    unsigned char in_byte;

#if (ULTRASTOR_DEBUG & UD_COMMAND)
    printk("US14F: command: called\n");
#endif

    (void)ultrastor_14f_queuecommand(target, cmnd, buff, bufflen, 0);

    /* Wait for ICM interrupt */
    do
	in_byte = inb_p(SYS_DOORBELL_INTR(PORT_ADDRESS));
    while (!aborted && !(in_byte & 1));
    if (aborted) {
#if (ULTRASTOR_DEBUG & (UD_COMMAND | UD_ABORT))
	printk("US14F: command: aborted\n");
#endif
	/* ??? is this right? */
	return (aborted << 16);
    }

    /* Clean ICM slot (set ICMINT bit to 0) */
    outb_p(0x1, SYS_DOORBELL_INTR(PORT_ADDRESS));

#if (ULTRASTOR_DEBUG & UD_COMMAND)
    printk("US14F: command: returning %08X\n",
	   (mscp.adapter_status << 16) | mscp.target_status);
#endif

    /* ??? not right, but okay for now? */
    return (mscp.adapter_status << 16) | mscp.target_status;
}
#endif

int ultrastor_14f_abort(int code)
{
#if (ULTRASTOR_DEBUG & UD_ABORT)
    printk("US14F: abort: called\n");
#endif

    aborted = (code ? code : DID_ABORT);

#if (ULTRASTOR_DEBUG & UD_ABORT)
    printk("US14F: abort: returning\n");
#endif

    return 0;
}

int ultrastor_14f_reset(void)
{
    unsigned char in_byte;

#if (ULTRASTOR_DEBUG & UD_RESET)
    printk("US14F: reset: called\n");
#endif

    /* Issue SCSI BUS reset */
    outb_p(0x20, LCL_DOORBELL_INTR(PORT_ADDRESS));

    /* Wait for completion... */
    do
	in_byte = inb_p(LCL_DOORBELL_INTR(PORT_ADDRESS));
    while (in_byte & 0x20);

    aborted = DID_RESET;

#if (ULTRASTOR_DEBUG & UD_RESET)
    printk("US14F: reset: returning\n");
#endif
    return 0;
}

#ifndef NO_QUEUEING
static void ultrastor_interrupt(int cpl)
{
#if (ULTRASTOR_DEBUG & UD_INTERRUPT)
    printk("US14F: interrupt: called: status = %08X\n",
	   (mscp.adapter_status << 16) | mscp.target_status);
#endif

    if (ultrastor_done == 0)
	panic("US14F: interrupt: unexpected interrupt!\n");
    else {
	void (*done)(int, int);

	/* Save ultrastor_done locally and zero before calling.  This is needed
	   as once we call done, we may get another command queued before this
	   interrupt service routine can return. */
	done = ultrastor_done;
	ultrastor_done = 0;

	/* Clean ICM slot (set ICMINT bit to 0) */
	outb_p(0x1, SYS_DOORBELL_INTR(PORT_ADDRESS));

	/* Let the higher levels know that we're done */
	/* ??? status is wrong here... */
	done(host_number, (mscp.adapter_status << 16) | mscp.target_status);
    }

#if (ULTRASTOR_DEBUG & UD_INTERRUPT)
    printk("US14F: interrupt: returning\n");
#endif
}
#endif

#endif
