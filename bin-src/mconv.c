/*
 * Microsoft serial mouse emulator for PS/2 mouse using a named pipe.
 *
 * This program reads packets from a PS/2 mouse, transforms them 
 * into the corresponding Microsoft serial mouse packets and writes
 * them to a named pipe (fifo). Application programs not supporting
 * the PS/2 mouse protocol (e.g. X11) can then read the packets from
 * the fifo, which to them looks like a serial port with a Microsoft
 * mouse on it.
 *
 * Create a named pipe with suitable permissions with mkfifo,
 * for instance
 *
 *      mkfifo -m 666 /dev/mouse
 * 
 * Make sure you have an entry in /dev for the PS/2 pointing device.
 * If not, create with
 *
 *      mknod /dev/psaux c 10 1
 *
 * Start up the conversion program with:
 *
 *      mconv /dev/psaux /dev/mouse &
 *
 * The program takes two arguments: the real mouse device and the
 * name of the fifo.
 *
 * In Xconfig, fool X into thinking you have a Microsoft mouse
 * on /dev/mouse (or whatever you called it):
 *
 * Microsoft "/dev/mouse"
 * 
 * Johan Myreen
 * jem@cs.hut.fi
 */


#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

static char *fifo_name, *prog_name;
static int fifo, mouse;

void handler()
{
  fprintf(stderr, "\n%s: Mouse killed!\n", prog_name);
  close(fifo);
  close(mouse);
  exit(0);
}

void sigpipe_handler()
{
  close(fifo);
  fifo = open(fifo_name, O_WRONLY);
  if (fifo < 0) {
    fprintf(stderr, "%s: Error reopening fifo.\n", prog_name);
    close(mouse);
    exit(1);
  }
  signal(SIGPIPE, sigpipe_handler);
}
  

unsigned char getbyte(void)
{
  static unsigned char buf[1024];
  static unsigned char *bp = buf, *ep = buf;
  int n;

  if (bp == ep) {
    bp = ep = buf;
    n = read(mouse, buf, 1024);
    if (n>0) {
      ep += n;
    }
  }
  return *bp++;
}


void track_mouse(void)
{
  unsigned char byte1, byte2, byte3, out[3], outbyte;
  int ret;

  while (1) {
    byte1 = getbyte();
    if (byte1 & 0xc0)
      continue;			        /* Resynchronize */
    byte2 = getbyte();
    byte3 = getbyte();
    byte3 = -byte3;
    outbyte = 0x40;
    outbyte |= ((byte2 >> 6)&0x03);
    outbyte |= ((byte3 >> 4)&0x0c);
    outbyte |= (byte1&0x01) << 5;	/* Left button */
    outbyte |= (byte1&0x02) << 3;	/* Right button */
    out[0] = outbyte;
    out[1] = (byte2)&0x3f;
    out[2] = (byte3)&0x3f;
    ret = write(fifo, out, 3);
    if (ret < 0 && errno != EPIPE)
      return;
  }
}


int main(int argc, char **argv)
{

  prog_name = argv[0];
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <mouse dev> <fifo>\n", argv[0]);
    exit(1);
  }

  mouse = open(argv[1], O_RDONLY);

  if (mouse < 0) {
    fprintf(stderr, "%s: error %d opening mouse. Exiting.\n", prog_name, errno);
    close(fifo);
    exit(1);
  }

  fifo_name = argv[2];
  fifo = open(fifo_name, O_WRONLY);

  if (fifo < 0) {
    fprintf(stderr, "%s: error %d opening fifo. Exiting.\n", prog_name, errno);
    exit(1);
  }

  signal(SIGTERM, handler);
  signal(SIGINT, handler);
  signal(SIGHUP, handler);
  signal(SIGPIPE, sigpipe_handler);

  track_mouse();

  fprintf(stderr, "%s: error %d reading or writing.\n", prog_name, errno);
  close(fifo);
  close(mouse);
}
