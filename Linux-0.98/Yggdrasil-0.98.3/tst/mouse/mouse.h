
/*
 * SYS V/386 dependent includes
 */
#include    <stdio.h>
#include    <fcntl.h>
#include    <ctype.h>
#include    <sys/types.h>
#include    <sys/ioctl.h>
#include    <sys/param.h>
#include    <time.h>
#include    <termio.h>
#include    <string.h>
#include    <errno.h>
#include    "compiler.h"
extern int errno;

#define DEVICE_INIT	0
#define DEVICE_CLOSE	1
#define DEVICE_ON	2
#define DEVICE_OFF	3

/* 
 * structure common for every Mouse
  X386DeviceRec device;
 */
typedef struct {
  int		Fd;
  int           x,y,buttons;          /* internal state of this mouse */
  int           threshold, num, den;  /* acceleration */
  int           nobuttons;            /* button number */
  int           packages;             /* packages / sec */
  int           baudrate;             /* targeted speed */
  int           protocol;             /* which way of data transmitting */
  int           cflag;                /* default setting for protocol */
  struct termio tty;                  /* termio struct of device */
  unchar        hd_mask, hd_id;       /* identification of the first byte */
  unchar        dp_mask, dp_id;       /* identification of a data byte */
  int           nobytes;              /* package length */
  unchar        sbuf[4];              /* state buffer for logitech */
  int           emulate;              /* automata state for 2 button mode */
  int           bufp;                 /* package buffer */
  unchar        buf[8];
} X386MouseRec, *X386MousePtr;

#define P_MS    0                     /* Microsoft */
#define P_MSC   1                     /* Mouse Systems Corp */
#define P_MM    2                     /* MMseries */
#define P_LOGI  3                     /* Logitech */
#define P_BM    4                     /* BusMouse ??? */

typedef int Bool;

typedef struct {
  /* pointer part */
  int           mseFd;
  char          *mseDevice;
  int           mseType;
  int           baudRate;
  int           sampleRate;
  int           lastButtons;
  int           threshold, num, den;  /* acceleration */
  int           emulateState;         /* automata state for 2 button mode */
  Bool          emulate3Buttons;

} x386InfoRec, *x386InfoPtr;

x386InfoRec x386Info;

#define P_MS    0                     /* Microsoft */
#define P_MSC   1                     /* Mouse Systems Corp */
#define P_MM    2                     /* MMseries */
#define P_LOGI  3                     /* Logitech */
#define P_BM    4                     /* BusMouse ??? */

#define Success	0
#define TRUE	1
#define FALSE	0
