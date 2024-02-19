#ifdef VMS
#include <file.h>
#else
#include <fcntl.h>
#endif
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <curses.h>
#include "consts.h"
#include "funcs.h"
 
#if defined(BSD) && ! defined(O_NDELAY)
#define NO_INP -1
#else
#define NO_INP EOF
#endif
 
#define FABS_M(x) ((x) >= -1.0 ? 1.0 : -(x))
#define FUEL_INIT 5000.0
#define FUEL_DRAIN 450.0
#define FUEL_MIN 100.0
 
double Fuel;
double FuelDec = 0.0;
 
static double Power;
static double PowerSet[] = {0.0, 0.01, 0.1, 0.5, 1.0, 2.0, 5.0, 10.0,
    15.0, 20.0};
static double att1p, att2p, att3p;
 
void InitMoves(screen)
 
WINDOW *screen;
 
       {
       att1p = 0.0;
       att2p = 0.0;
       att3p = 0.0;
       Power = 0.0;
       Fuel = FUEL_INIT - FuelDec;
       if (Fuel < FUEL_MIN)
               Fuel = FUEL_MIN;
       FuelDec += FUEL_DRAIN;
       nodelay(screen, TRUE);
       }
 
void GetMove(screen, y_delta, x_delta)
 
WINDOW *screen;
double *y_delta, *x_delta;
 
       {
       int ch, index;
       double y_delta_inc;
#ifdef VMS
#define getch mygetch
#endif
 
       if (Fuel > 0.0)
               {
               wrefresh(screen);
               while ((ch = getch()) != NO_INP)
                       {
#ifdef BSD
                       ch &= 0x7F;
#endif
                       if (isdigit(ch))
                               {
                               index = ch - '0';
                               Power = PowerSet[index];
                               }
                       else
                               switch (ch)
                               {
                               case 'z' :
                                       att1p = Power;
                                       break;
                               case 'x' :
                                       att2p = Power;
                                       break;
                               case 'c' :
                                       att3p = Power;
                                       break;
                               default :
                                       flash();
                                       break;
                               }
                       }
               *x_delta += att1p;
               y_delta_inc = att2p * (log10(FABS_M(*y_delta)) / GRAVITY);
               *y_delta += y_delta_inc;
               *x_delta -= att3p;
               Fuel -= att3p + att2p + att1p;
               if (Fuel < 0.0)
                       att3p = att2p = att1p = Power = Fuel = 0.0;
               }
       wmove(screen, LINES - 1, 0);
       wclrtoeol(screen);
       wprintw(screen,
            "Thrust - L: %5.2f vert: %7.4f R: %5.2f Pow: %5.2f Fuel: %7.2f",
           att1p, att2p, att3p, Power, Fuel);
       wrefresh(screen);
       }


#ifdef VMS
#include <descrip.h>
#include <ctype.h>
#include <iodef.h>
#include <ttdef.h>
#include <tt2def.h>

struct {       /* terminal mode buffers */
        int     page_width;
        int     basic_term;
        int     ext_term;
       } modebuf, savemode;

int     getch_chan = 0;

int getch()
{
static $DESCRIPTOR(tname,"SYS$INPUT");
char    c;
int     typeahdcnt;
int     sys$qiow();

    if (!getch_chan)
    {   sys$assign(&tname,&getch_chan,0,0);
        sys$qiow(0,getch_chan,IO$_SENSEMODE,0,0,0,&modebuf,12,0,0,0,0);
        savemode = modebuf;
        modebuf.basic_term = modebuf.basic_term & ~TT$M_WRAP;
        modebuf.ext_term = modebuf.ext_term & ~TT2$M_APP_KEYPAD;
        sys$qiow(0,getch_chan,IO$_SETMODE,0,0,0,&modebuf,12,0,0,0,0);
    }

    sys$qiow(0,getch_chan,IO$_SENSEMODE|IO$M_TYPEAHDCNT,
             0,0,0,&typeahdcnt,4,0,0,0,0);
    typeahdcnt &= 0xffff;
    if (!typeahdcnt) return NO_INP;

    sys$qiow(0,getch_chan,IO$_READVBLK | IO$M_NOECHO | IO$M_NOFILTR,
             0,0,0,&c,1,0,0,0,0);
    if (c == EOF)  exit(1);
    return c;
}
#endif
