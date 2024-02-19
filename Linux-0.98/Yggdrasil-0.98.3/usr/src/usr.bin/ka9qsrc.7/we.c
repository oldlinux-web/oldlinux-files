/* we.c an wd8003 ethernet driver for linux. */
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
/* The bsd386 version was used as an example in order to write this
   code */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/tty.h>
#include <linux/types.h>
/* #include <linux/driver.h>*/
#include <linux/eth.h>
#include <asm/system.h>
#include <asm/segment.h>
#include <asm/io.h>
#include <asm/memory.h>
#include <linux/errno.h>
#include <linux/fcntl.h>

#include "wereg.h"

static unsigned char interrupt_mask;
/* format of status byte. 
   bit 
    0	start
    1	open
    2   transmitter in use */

#define START 1
#define OPEN  2
#define TRS_BUSY 4

static unsigned int status;

struct driver {
  struct wait_queue **d_rwait;
  struct wait_queue **d_wwait;
} builtin_dev;


static unsigned char we_addr[ETHER_ADDR_LEN];
static struct driver *mdev;
static int recv_errors=0;
static int trans_errors=0;
static int spackets=0;
static int collisions=0;
static struct wait_queue *wwait_ptr=NULL;
static struct wait_queue *rwait_ptr=NULL;

static inline int
min(int a, int b)
{
  if (a<b) return (a);
  return (b);
}

static inline int
max(int a, int b)
{
  if (a>b) return (a);
  return (b);
}

struct bufhead
{
  long len;
};

struct buf 
{
  short head, tail;
  unsigned char *memory;
};

static unsigned char rbuf_c[ETH_BUFF_SIZE];
static unsigned char wbuf_c[ETH_BUFF_SIZE];
static struct buf Buf[]=
{
  {0,0,rbuf_c},
  {0,0,wbuf_c}
};

static struct buf *rbuf = Buf;
static struct buf *wbuf = Buf+1;

static inline int
buf_free(struct buf *b)
{
  if (b->head >= b->tail)
    return(ETH_BUFF_SIZE-b->head +b->tail);
  else
    return (b->tail-b->head);
}

static inline void
clear_buf(struct buf *b)
{
  b->head=0;
  b->tail=0;
}

static inline int
copy_to_buf(struct buf *b, void *vptr, long len,
	    void *end_buff, callback split)
{
  struct bufhead *head;
  long end;
  long bend;
  long total=0;
  unsigned char *ptr;
  /* check to see if there is enough space. */
  if (len+3*sizeof(*head) >= 
      buf_free(b)) 
    {
      return(0);
    }

  head = (struct bufhead *)(b->memory + b->head);
  head->len = len;
  b->head += sizeof (*head);
  ptr = vptr;

  while (len > 0)
    {
      end = ETH_BUFF_SIZE - b->head;
      bend = (unsigned long)end_buff - (unsigned long)ptr;
      if (bend <= 0)
	{
	  ptr =split(ptr);
	  bend = (unsigned long)end_buff - (unsigned long)ptr;
	}
      /* copy up to the end of a buffer. */
      end = min(end,bend);
      end = min (end,len);
      if (end < 0)
	{
	  printk ("copy_to_buf end=%d\n",end);
	  end=0;
	}
      (void) memcpy (b->memory+b->head, ptr,end);
      ptr += end;
      len -= end;
      b->head+=end;
      total += end;
      if (b->head >= ETH_BUFF_SIZE)
	b->head = 0;
    }
  if (b->head >= ETH_BUFF_SIZE-2*sizeof(*head))
    b->head = 0;
  return (total);
}


static inline int
copy_to_buf_fs(struct buf *b, void *vptr, unsigned long len)
{
  struct bufhead *head;
  unsigned long end;
  unsigned char *ptr;

  /* check to see if there is enough space. */
  if (len+3*sizeof(*head) >= 
      buf_free(b)) 
    {
      printk ("copy_to_buf_fs failed %d\n",len);
      return(0);
    }

  head = (struct bufhead *)(b->memory + b->head);
  head->len = len;
  b->head += sizeof (*head);

  end = ETH_BUFF_SIZE - b->head;
  ptr = vptr;
  /* copy up to the end of the buffer. */
  end = min(end,len);
  memcpy_fromfs (b->memory+b->head, ptr,end);
  ptr += end;
  len -= end;
  b->head+=end;
  if (b->head >= ETH_BUFF_SIZE)
    b->head = 0;
  /* now copy the rest. */
  memcpy_fromfs (b->memory+b->head, ptr, len);
  b->head += len;
  if (b->head >= ETH_BUFF_SIZE-2*sizeof(*head))
    b->head = 0;
  return (end + len);
}

static inline int
copy_from_buf(struct buf *b, void *vptr, long len)
{
  struct bufhead *head;
  long end;
  unsigned char *ptr;

  /* check to see if there is anything in the buffer. */
  if (b->head == b->tail)
    return(0);

  head = (struct bufhead *)(b->memory + b->tail);
  b->tail += sizeof (*head);
  len = min (head->len,len);

  end = ETH_BUFF_SIZE - b->tail;
  ptr = vptr;
  /* copy up to the end of the buffer. */
  end = min(end,len);
  if (end < 0)
    {
      printk ("copy from buf end = %d\n",end);
      end = 0;
    }
  (void) memcpy (ptr, b->memory+b->tail, end);
  ptr += end;
  len -= end;
  b->tail+=end;
  if (b->tail >= ETH_BUFF_SIZE)
    b->tail = 0;
  /* now copy the rest. */
  (void )memcpy (ptr, b->memory+b->tail, len);
  /* now forget about any leftover stuff. */
  b->tail += head->len - end;
  if (b->tail >= ETH_BUFF_SIZE)
    b->tail-=ETH_BUFF_SIZE;
  if (b->tail >= ETH_BUFF_SIZE-2*sizeof(*head))
    b->tail = 0;
  return (end + len);
}

static inline int
copy_from_buf_fs(struct buf *b, void *vptr, unsigned long len)
{
  struct bufhead *head;
  unsigned long end;
  unsigned char *ptr;

  /* check to see if there is anything in the buffer. */
  if (b->head == b->tail)
    return(0);

  head = (struct bufhead *)(b->memory + b->tail);
  b->tail += sizeof (*head);
  len = min (head->len,len);

  end = ETH_BUFF_SIZE - b->tail;
  ptr = vptr;
  /* copy up to the end of the buffer. */
  end = min(end,len);
  memcpy_tofs (ptr, b->memory+b->tail, end);
  ptr += end;
  len -= end;
  b->tail+=end;
  if (b->tail >= ETH_BUFF_SIZE)
    b->tail = 0;
  /* now copy the rest. */
  if (len != 0)
    {
      memcpy_tofs (ptr,b->memory+b->tail, len);
    }
  /* now forget about any leftover stuff. */
  b->tail += head->len - end;
  if (b->tail >= ETH_BUFF_SIZE)
    b->tail-=ETH_BUFF_SIZE;
  if (b->tail >= ETH_BUFF_SIZE-2*sizeof(*head))
    b->tail = 0;
  return (end + len);
}

void
wd_stop(void)
{
  unsigned char cmd;
  cli();
  cmd = inb_p(WD_COMM);
  cmd |= CSTOP;
  cmd &= ~(CSTART|CPAGE);
  outb_p(cmd, WD_COMM);
  outb(0,WD_IMR);
  sti();
  interrupt_mask = 0;
  status &= ~START;
}

static inline void
wd_start(void)
{
  unsigned char cmd;
  interrupt_mask=RECV_MASK;
  cli();
  cmd = inb_p(WD_COMM);
  cmd &= ~(CSTOP|CPAGE);
  cmd |= CSTART;
  outb_p(cmd, WD_COMM);
  outb(interrupt_mask,WD_IMR);
  sti();
  status |= START;
}

int wd8003_close(struct inode *inode, struct file *filep)
{
  int minor;
  minor = MINOR(inode->i_rdev);
  if (minor != 0) return (-ENODEV);
  wd_stop();
  status = 0;
  return (0);
}

int
wd8003_open(struct inode *inode, struct file *filep)
{
  unsigned char cmd;
  int i;
  int minor;

  minor = MINOR(inode->i_rdev);
  if (minor != 0) return (-ENODEV);
  if (status & OPEN) return (-EBUSY);

  /* clear the buffs. */
  clear_buf(rbuf);
  clear_buf(wbuf);

  /* we probably don't want to be interrupted here. */
  cli();
  /* This section of code is mostly copied from the bsd driver which is
     mostly copied from somewhere else. */
  cmd=inb_p(WD_COMM);
  cmd|=CSTOP;
  cmd &= ~(CSTART|CPAGE);
  outb_p(cmd, WD_COMM);
  outb_p(0, WD_IMR);
  sti();
  outb_p(WD_DCONFIG,WD_DCR);
  /*Zero the remote byte count. */
  outb_p(0, WD_RBY0);
  outb_p(0, WD_RBY1);
  outb_p(WD_MCONFIG,WD_RCC);
  outb_p(WD_TCONFIG,WD_TRC);
  /* Set the transmit page */
  outb_p(0,WD_TRPG);
  outb_p(WD_TXBS,WD_PSTRT);
  outb_p(WD_MAX_PAGES,WD_PSTOP);
  outb_p(WD_TXBS,WD_BNDR);
  /* clear interrupt status. */
  outb_p(0xff,WD_ISR);
  /* we don't want no stinking interrupts. */
  outb_p(0 ,WD_IMR);
  cmd|=1<<CPAGE_SHIFT;
  outb_p(cmd,WD_COMM);
  /* set the either address. */
  for (i=0; i < ETHER_ADDR_LEN; i++)
    {
      outb_p(we_addr[i],WD_PAR0+i);
    }
  /* set the multicast address. */
  for (i=0; i < ETHER_ADDR_LEN; i++)
    {
      outb_p(0xff,WD_MAR0+i);
    }
  outb_p(WD_TXBS,WD_CUR);
  cmd&=~(CPAGE|CRDMA);
  cmd|= 4<<CRDMA_SHIFT;
  outb_p(cmd, WD_COMM);
  outb_p(WD_RCONFIG,WD_RCC);
  status = OPEN;
  wd_start(); 
  return (0);
}

static void *
wd_callback(void *ptr)
{
  if (ptr >= (void *)WD_BUFFEND)
    return ((void *)(WD_MEM + (WD_TXBS<<8)));
  return (ptr);
}
/*This routine just copies the stuff from the ring into the
  buffer.  It remove the ring headers, and makes sure that
  everything fits. */

static inline int
wdget(struct wd_ring *ring)
{
  unsigned char *fptr;
  unsigned long len;
  int i;
  fptr = (unsigned char *)(ring +1);
  len = ring->count-4;
  for (i =0; i < ETHER_ADDR_LEN; i++)
    if (fptr[i] != we_addr[i])
      {
	return (0);
      }
  if (copy_to_buf (rbuf, fptr, len,(void *) WD_BUFFEND, wd_callback) 
      != len) return (1);
  return (0);
}
/* wd_strans attempts to start a transmission.  It is
   only called when the transmit buffer is free. */
static inline void
wd_strans(void)
{
  unsigned long len;
  unsigned char cmd;

  len = copy_from_buf(wbuf,(void *)WD_MEM,WD_TXBS<<8);
  /* see if there is anything to send. */
  if (len == 0)
    {
      interrupt_mask &= ~TRANS_MASK;
      status &= ~(TRS_BUSY);
      return;
    }
  status |= TRS_BUSY;
  /* now we need to set up the card info. */
  len=max(len, ETHER_MIN_LEN);
  cmd=inb_p(WD_COMM);
  outb_p(len&0xff,WD_TB0);
  outb_p(len>>8,WD_TB1);
  cmd |= CTRANS;
  outb(cmd,WD_COMM);
  interrupt_mask |= TRANS_MASK;
}

int
rw_wd8003(int rw, unsigned minor, char *buf, int count, unsigned short flags )
{
  unsigned long len=0;
  unsigned char cmd;
  if (minor != 0) return (-ENODEV);

  if (rw == READ)
    {
      while (len == 0)
	{
	  verify_area(buf, count);
	  len = copy_from_buf_fs(rbuf,buf, count);
	  if (len == 0) 
	    {
	      clear_buf(rbuf);
	      if ( flags & O_NONBLOCK)
		{
		  return (-EAGAIN);
		}
	      interruptible_sleep_on(mdev->d_rwait);
	    }
	}
    }
  else
    {
      while (len == 0)
	{
	  verify_area(buf, count);
	  len = copy_to_buf_fs(wbuf,buf, count);
	  if (len == 0) 
	    {
	      clear_buf(wbuf);
	      if ( flags & O_NONBLOCK)
		{
		  return (-EAGAIN);
		}
	      interruptible_sleep_on(mdev->d_wwait);
	    }
	}
      /* see if we need to start the transmission. */
      if (!(status & TRS_BUSY))
	  {
	    cli();
	    cmd = inb_p(WD_COMM);
	    cmd &= ~(CPAGE);
	    outb_p(cmd, WD_COMM);
	    outb_p(0, WD_IMR);
	    sti();
	    wd_strans();
	    outb_p(interrupt_mask,WD_IMR);
	  }
    }
  return (len);
}

static int
wd8003_read (struct inode *inode, struct file *file, char *buf, int count)
{
  return (rw_wd8003(READ, MINOR(inode->i_rdev), buf, count, file->f_flags));
}

static int
wd8003_write (struct inode *inode, struct file *file, char *buf, int count)
{
  return (rw_wd8003(WRITE, MINOR(inode->i_rdev), buf, count, file->f_flags));
}

/* tell the card about the new boundary. */

static inline void
wd_put_bnd(unsigned char bnd )
{

	unsigned char cmd;

	/* Ensure page 0 selected */
	cmd = inb_p( WD_COMM );
	if (cmd & 0x40) {
		outb_p(cmd & (~CPAGE1), WD_COMM);	/* select page 0 */
		outb_p(bnd, WD_BNDR);
		outb_p(cmd | CPAGE1, WD_COMM);	/* reselect page 1 */
	} else {
		outb_p(bnd, WD_BNDR);
	}
}

static inline unsigned char
wd_get_bnd( )
{

	unsigned char   cmd, bnd;

	/* Ensure page 0 selected */
	cmd = inb_p(WD_COMM);
	if (cmd & 0x40) {
		outb_p(cmd & (~CPAGE1), WD_COMM);	/* select page 0 */
		bnd = inb_p(WD_BNDR);
		outb_p(cmd | CPAGE1, WD_COMM);	/* reselect page 1 */
		return (bnd);
	} else {
		return (inb_p(WD_BNDR));
	}
}

static inline unsigned char
wd_get_cur( )
{

	unsigned char   cmd, cur;

	/* Ensure page 1 selected */
	cmd = inb_p(WD_COMM);
	if (cmd & 0x40) {
		return (inb_p(WD_CUR));
	} else {
		outb_p(cmd | CPAGE1, WD_COMM);	/* select page 1 */
		cur = inb_p(WD_CUR);
		outb_p(cmd & (~CPAGE1), WD_COMM);	/* reselect page 0 */
		return (cur);
	}
}

/* This routine handles the packet recieved interrupt. */
/* Debug routines slow things down, but reveal bugs... */

int wd_debug = 0;

/* This routine handles the packet recieved interrupt. */
/* It is called with interrupts enabled, but with
   the wd card configured not to return interrupts. */
/* It is enterred with page one selected, and it must
   leave that way. */
/* I think there might be an infinite loop here, so I'm putting
   in a maximum number of packets we can look at at once. 
   Someone with good docs can try to fix this one. */

static inline void
wd_rcv(void)
{
  unsigned char bnd;
  unsigned char cur;
  unsigned char cmd;
  struct wd_ring *ring;
  int count = 0;

	/* Update current and boundry values from board */
	while ((bnd = wd_get_bnd()) != (cur = wd_get_cur())) {

		/* Position pointer to packet in card ring buffer */
		ring = (struct wd_ring *) (WD_MEM + (bnd << 8));

		/* Ensure a valid packet */
		if( ring->status & 1 ) { /* Too small and too big packets are filtered by the board */
			if( wd_debug )
				printk("\nwd8013 - wdget: bnd = %d, cur = %d, status = %d, len = %d, next = %d", bnd, cur, ring->status, ring->count, ring->next);

			wdget(ring);
			wake_up(mdev->d_rwait);

			/* Compute new boundry */
			bnd = ring->next;
	
			wd_put_bnd(bnd);
	
			/* update our copy of cur. */
			cur = wd_get_cur();
		}
		else {	/* Bad packet in ring buffer - should not happen due to hardware filtering */
			printk("\nwd8013 - bad packet: len = %d, status = x%x, bnd = %d, cur = %d, loops = %d, trashing receive buffer!", ring->count, ring->status, bnd, cur, count );
			/* Reset bnd = cur */
			wd_put_bnd( wd_get_cur() );
			break; /* return */
		}

		/* infinite loop detection - should be removed! */
		if( (++count > ( WD_MAX_PAGES + 1 ) ) || ( bnd >= WD_MAX_PAGES ) ){
			/* Reset bnd = cur */
			printk("\nwd8013 - wd_rcv: infinite loop detected, loops = %d, bnd = %d, cur = %d, max pages = %d", count, bnd, cur, WD_MAX_PAGES );
			wd_put_bnd( wd_get_cur());
			break; /* return */
		}
	}

	/* reset to page 0 */
	cmd = inb_p(WD_COMM);
	if (cmd & 0x40) {
		outb_p(cmd & ~(CPAGE1), WD_COMM);	/* select page 0 */
	}
}

/* This get's the transmit interrupts. 
   It assume command page 0 is set, and
   returns with command page 0 set. */

static inline void
wd_trs(void)
{
  /*update the stats. */
  spackets++;
  collisions += inb(WD_TB0);
  /* attempt to start a transmission. */
  wd_strans();
  /* wakeup anything that was sleeping on writes. */
  if (*(mdev->d_wwait)) wake_up(mdev->d_wwait);
}
/* Currently this routine sometimes gets stuck in an infinite
   loop.  So I'm putting in a counter.  When the routine
   has looped 10 times, I'm just resetting everything,
   then leaving.  Someone who has real docs can fix this. */

void
wd8003_interrupt(void)
{
  unsigned char cmd;
  unsigned char isr;
  int count=0;
  /* Acknowledge the interrupt. */
/*  outb_p(0x20,0x20); */
  /* now we can be interrupted, but since we have
     done nothing yet it doesn't matter. */
  /*we don't want to be interrupted after we
    read the command byte. */
  cli();
  cmd=inb_p(WD_COMM);
  cmd&=~(CPAGE);
  outb_p(cmd,WD_COMM);
  isr=inb(WD_ISR);
  do {
    outb_p(0,WD_IMR);
    /* we have turned the wd8003 interrupts off, so we can reenable
       them at the processor level. */
    sti();
    /* From now on other devices will be able to interrupt us, so
       some care is probably required. */
    count++;
    if (count > 10)
      {
	printk ("wd8003:Warning loop broken\n");
	/* we are caught in a loop, reset everything that we can think
	   of and return.  This needs to be fixed. */
	      cmd |= CSTOP;
	      outb_p(cmd,WD_COMM);
	      outb_p(WD_IMEM|0x80,WD_CTL);
	      outb_p(WD_IMEM,WD_CTL);
	      outb_p(WD_MCONFIG,WD_RCC);
	      outb_p(WD_TXBS,WD_BNDR);
	      cmd |= 1 <<CPAGE_SHIFT;
	      outb_p(cmd, WD_COMM);
	      outb_p(WD_TXBS,WD_CUR);
	      cmd &= ~(CSTOP|CPAGE);
	      cmd |= CSTART;
	      outb_p(cmd, WD_COMM);
	      outb_p(WD_RCONFIG,WD_RCC);
	      outb_p (0,WD_ISR);
	      outb_p(interrupt_mask,WD_IMR);
	      return;
      }
    /* see what to do about the interrupt. */
    if (isr & IRCV) /* got a packet. */
      wd_rcv();
    if (isr & ITRS) /* finished sending a packet. */
      wd_trs();

    if (isr & IRCE) /* recieve error */
      {
	/* reading these registers might clear the error condition. */
	collisions += inb(WD_TB0);
	(void) inb(WD_RCC);
	(void) inb(WD_DCR);
	recv_errors++;
      }
    if (isr & ITRE) /* transmit error. */
      {
	collisions += inb(WD_TB0);
	trans_errors++;
      }

    /* acknowledge the intterupt. */
    outb_p(isr, WD_ISR);

    /* Now we will try to reenable onboard interrupts. */
    cli();
    outb_p(interrupt_mask,WD_IMR);
    isr=inb_p(WD_ISR)&interrupt_mask;
  } while (isr != 0);
  sti();
}

int
wd8003_ioctl(struct inode *inode, struct file *file,
	     unsigned int cmd, unsigned int arg)
{
  int dev;
  dev=MINOR(inode->i_rdev);
  if (dev != 0) return (-ENODEV);
  switch (cmd)
    {
    case ETH_START:
      wd_start();
      return (0);

    case ETH_STOP:
      wd_stop();
      return(0);

    default:
      return (-EINVAL);

    }
}
static struct file_operations wd_fops = 
{
  NULL,
  wd8003_read,
  wd8003_write,
  NULL,
  NULL,
  wd8003_ioctl,
  wd8003_open,
  wd8003_close
};

void
wd8003_init(struct driver *dev)
{
  unsigned char csum;
  int i;
  chrdev_fops[10]=&wd_fops;
  mdev = &builtin_dev;
  mdev->d_rwait = &rwait_ptr;
  mdev->d_wwait = &wwait_ptr;
  csum = 0;
  for (i = 0; i < 8; i++)
    {
      csum += inb_p(WD_ROM+i);
    }
  if (csum != WD_CHECK)
    {
      printk ("Warning no WD8003 board installed.\n");
      /* make sure no one can attempt to open the device. */
      status = OPEN;
      return;
    }

  /* mapin the interface memory. */
  outb(WD_IMEM,WD_CTL);

  /* clear the interface memory */
  for (i = 0; i < WD_MEMSIZE; i++)
    {
      ((unsigned char *)WD_MEM)[i] = 0;
      if (((unsigned char *)WD_MEM)[i] != 0) 
	{
	  printk ("WD Memory error.\n");
	  status = OPEN;
	}
    }

  /* print the initialization message, and the
     ethernet address. */
  printk ("WD8003 ethernet Address ");
  for (i = 0; i <ETHER_ADDR_LEN; i++)
    {
      we_addr[i]=inb_p(WD_ROM+i);
      printk ("%2.2X ",we_addr[i]);
    }
  printk ("\n");
  status = 0;
  if (request_irq(5, wd8003_interrupt))
    printk("Unable to get irq 5\n");
}
