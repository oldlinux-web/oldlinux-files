/* provided by hlu, modified slightly by obz */
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * $Header: /proj/X11/mit/server/ddx/x386/RCS/x386Io.c,v 1.2 1991/06/27 00:01:38 root Exp $
 */

#define NEED_EVENTS
#if 0
#include "X.h"
#include "Xproto.h"
#include "inputstr.h"
#include "scrnintstr.h"

#include "compiler.h"

#include "x386Procs.h"
#include "x386OSD.h"
#include "atKeynames.h"
#else
#include "mouse.h"
#endif

static void
Error (const char *str)
{
  fprintf (stderr, "%s\n", str);
}

static void
x386PostMseEvent (int button, int dx, int dy)
{
  fprintf (stderr, "button: %d, dx: %d, dy: %d\n", button, dx, dy);
}

static void
x386SetSpeed(old, new, cflag)
     int old, new;
     unsigned short cflag;
{
  struct termio  tty;
  char           *c;

  (void) ioctl(x386Info.mseFd, TCGETA, &tty);
  tty.c_iflag = IGNBRK | IGNPAR ;     
  tty.c_oflag = 0;           
  tty.c_lflag = 0;
  tty.c_line = 0;
  tty.c_cc[VTIME]=0; 
  tty.c_cc[VMIN]=1;
  
  switch (old) {
  case 9600:  tty.c_cflag = cflag | B9600;  break;
  case 4800:  tty.c_cflag = cflag | B4800;  break;
  case 2400:  tty.c_cflag = cflag | B2400;  break;
  case 1200:
  default:    tty.c_cflag = cflag | B1200;  break;
  }
  (void) ioctl(x386Info.mseFd, TCSETAW, &tty);  
  
  switch(new) {
  case 9600:  c = "*q";  tty.c_cflag = cflag | B9600;  break;
  case 4800:  c = "*p";  tty.c_cflag = cflag | B4800;  break;
  case 2400:  c = "*o";  tty.c_cflag = cflag | B2400;  break;
  case 1200:
  default:    c = "*n";  tty.c_cflag = cflag | B1200;  break;
  }

  write(x386Info.mseFd, c, 2);  
  usleep(100000);

  (void) ioctl(x386Info.mseFd, TCSETAW, &tty);  
#ifdef TCMOUSE
  (void) ioctl(x386Info.mseFd, TCMOUSE, 1);  
#endif
}


/*
 * x386MseProc --
 *      Handle the initialization, etc. of a mouse
 */

int
x386MseProc(what)
     int        what;
{
  unchar                map[4];

  static unsigned short cflag[5] =
    {
#ifdef linux_hack /* not needed with patch3 - server doesn't do this! */
      (CS7 | ISTRIP          | CREAD | CLOCAL | HUPCL ),   /* MicroSoft */
#else
      (CS7 		     | CREAD | CLOCAL | HUPCL ),   /* MicroSoft */
#endif
      (CS8 | CSTOPB          | CREAD | CLOCAL | HUPCL ),   /* MouseSystems */
      (CS8 | PARENB | PARODD | CREAD | CLOCAL | HUPCL ),   /* MMSeries */
      (CS8 | CSTOPB          | CREAD | CLOCAL | HUPCL ),   /* Logitech */
      0,                                                   /* BusMouse */
    };
  
  switch (what)
    {
    case DEVICE_INIT: 
      map[1] = 3;
      map[2] = 2;
      map[3] = 1;
      break;
      
    case DEVICE_ON:
      if ((x386Info.mseFd = open(x386Info.mseDevice, O_RDWR | O_NDELAY)) < 0)
	{
	  Error ("Cannot open mouse");
	  return (!Success);
	}

      if (x386Info.mseType != P_BM) 
	{
	  x386SetSpeed(9600, x386Info.baudRate, cflag[x386Info.mseType]);
	  x386SetSpeed(4800, x386Info.baudRate, cflag[x386Info.mseType]);
	  x386SetSpeed(2400, x386Info.baudRate, cflag[x386Info.mseType]);
	  x386SetSpeed(1200, x386Info.baudRate, cflag[x386Info.mseType]);

	  if (x386Info.mseType == P_LOGI)
	    {
	      write(x386Info.mseFd, "S", 1);
	      x386SetSpeed(x386Info.baudRate, x386Info.baudRate, cflag[P_MM]);
	    }

	  if      (x386Info.sampleRate <=   0)  write(x386Info.mseFd, "O", 1);
	  else if (x386Info.sampleRate <=  15)  write(x386Info.mseFd, "J", 1);
	  else if (x386Info.sampleRate <=  27)  write(x386Info.mseFd, "K", 1);
	  else if (x386Info.sampleRate <=  42)  write(x386Info.mseFd, "L", 1);
	  else if (x386Info.sampleRate <=  60)  write(x386Info.mseFd, "R", 1);
	  else if (x386Info.sampleRate <=  85)  write(x386Info.mseFd, "M", 1);
	  else if (x386Info.sampleRate <= 125)  write(x386Info.mseFd, "Q", 1);
	  else                                  write(x386Info.mseFd, "N", 1);
	}
      
      x386Info.lastButtons = 0;
      x386Info.emulateState = 0;
      break;
      
    case DEVICE_OFF:
    case DEVICE_CLOSE:
      if (x386Info.mseType == P_LOGI)
	{
	  write(x386Info.mseFd, "U", 1);
	  x386SetSpeed(x386Info.baudRate, 1200, cflag[P_LOGI]);
	}
      close(x386Info.mseFd);
      break;
      
    }

  return Success;
}



/*
 * x386MseEvents --
 *      Read the new events from the device, and pass them to the eventhandler.
 */

void
x386MseEvents()
{
  unchar               rBuf[64];
  int                  i,nBytes, buttons, dx, dy;
  static int           pBufP = 0;
  static unsigned char pBuf[8];

  static unsigned char proto[5][5] = {
    /*  hd_mask hd_id   dp_mask dp_id   nobytes */
    { 	0x40,	0x40,	0x40,	0x00,	3 	},  /* MicroSoft */
    {	0xf8,	0x80,	0x00,	0x00,	5	},  /* MouseSystems */
    {	0xe0,	0x80,	0x80,	0x00,	3	},  /* MMSeries */
    {	0xe0,	0x80,	0x80,	0x00,	3	},  /* Logitech */
    {	0xf8,	0x80,	0x00,	0x00,	5	},  /* BusMouse */
  };
  
  if (!(nBytes = read(x386Info.mseFd, (char *)rBuf, sizeof(rBuf)))) return;

  for ( i=0; i < nBytes; i++) {
    /*
     * Hack for resyncing: We check here for a package that is:
     *  a) illegal (detected by wrong data-package header)
     *  b) invalid (0x80 == -128 and that might be wrong for MouseSystems)
     *  c) bad header-package
     *
     * NOTE: b) is a voilation of the MouseSystems-Protocol, since values of
     *       -128 are allowed, but since they are very seldom we can easily
     *       use them as package-header with no button pressed.
     */
    if (pBufP != 0 && 
	((rBuf[i] & proto[x386Info.mseType][2]) != proto[x386Info.mseType][3]
	 || rBuf[i] == 0x80))
      {
	pBufP = 0;          /* skip package */
      }

    if (pBufP == 0 &&
	(rBuf[i] & proto[x386Info.mseType][0]) != proto[x386Info.mseType][1])
      {
	/*
	 * Hack for Logitech MouseMan Mouse - Middle button
	 *
	 * Unfortunately this mouse has variable length packets: the standard
	 * Microsoft 3 byte packet plus an optional 4th byte whenever the
	 * middle button status changes.
	 *
	 * We have already processed the standard packet with the movement
	 * and button info.  Now post an event message with the old status
	 * of the left and right buttons and the updated middle button.
	 */
	if (x386Info.mseType == P_MS && (rBuf[i] == 0x20 || rBuf[i] == 0))
	  {
	    buttons = ((int)(rBuf[i] & 0x20) >> 4)
	      | (x386Info.lastButtons & 0x05);
	    x386PostMseEvent(buttons, 0, 0);
	  }

	continue;            /* skip package */
      }


    pBuf[pBufP++] = rBuf[i];
    if (pBufP != proto[x386Info.mseType][4]) continue;

    /*
     * assembly full package
     */
    switch(x386Info.mseType) {
      
    case P_MS:              /* Mircosoft */
      buttons = (x386Info.lastButtons & 2)
	| ((int)(pBuf[0] & 0x20) >> 3)
	  | ((int)(pBuf[0] & 0x10) >> 4);
      dx = (char)(((pBuf[0] & 0x03) << 6) | (pBuf[1] & 0x3F));
      dy = (char)(((pBuf[0] & 0x0C) << 4) | (pBuf[2] & 0x3F));
      break;
      
    case P_MSC:             /* Mouse Systems Corp */
      buttons = (~pBuf[0]) & 0x07;
      dx =    (char)(pBuf[1]) + (char)(pBuf[3]);
      dy = - ((char)(pBuf[2]) + (char)(pBuf[4]));
      break;
      
    case P_MM:              /* MM Series */
    case P_LOGI:            /* Logitech Mice */
      buttons = pBuf[0] & 0x07;
      dx = (pBuf[0] & 0x10) ?   pBuf[1] : - pBuf[1];
      dy = (pBuf[0] & 0x08) ? - pBuf[2] :   pBuf[2];
      break;
      
    case P_BM:              /* BusMouse */
      buttons = (~pBuf[0]) & 0x07;
      dx =   (char)pBuf[1];
      dy = - (char)pBuf[2];
      break;
    }

    x386PostMseEvent(buttons, dx, dy);
    pBufP = 0;
  }
}

void
x386MseConfig (char *dev, int type)
{
  x386Info.mseType = type;
  x386Info.mseDevice = dev;
  x386Info.sampleRate = 150;
  x386Info.emulate3Buttons = FALSE;
}

int stop = FALSE;

void
stopit(int foo)
{
	stop = TRUE;
}

main (int argc, char **argv)
{
  int i = 0;
  char *str, *dev;

  if (argc != 3) {
    fprintf (stderr, "Usage: %s [Microsoft|MouseSystem|MMseries|Logitech] device\n", argv [0]);
    exit (1);
  }

  i = P_MS;
  if (!strcasecmp (argv [1], "Microsoft")) i = P_MS;
  if (!strcasecmp (argv [1], "MouseSystem")) i = P_MSC;
  if (!strcasecmp (argv [1], "MMseries")) i = P_MM;
  if (!strcasecmp (argv [1], "Logitech")) i = P_LOGI;

  dev = argv[2];

  switch (i) {
  case P_MS:
    str = "Microsoft";
    break;
  case P_MSC:
    str = "MouseSystem";
    break;
  case P_MM:
    str= "MMseries";
    break;
  case P_LOGI:
    str = "Logitech";
    break;
  }
  fprintf (stderr, "You are testing %s on %s.\n", str, dev);
  x386MseConfig (dev, i);

  x386MseProc(DEVICE_INIT);
  x386MseProc(DEVICE_ON);
/*   outb (0x03fb, 0x03); */

  signal(SIGINT, stopit);
  fprintf(stderr, "Play with the mouse - hit ctrl-c to stop\n");
  for (;;) {
    if (stop)
	    break;
    x386MseEvents();
  }
  x386MseProc(DEVICE_CLOSE);
  fprintf(stderr, "finished.\n");
}
