#include <signal.h>
#ifdef VMS
#include <file.h>
#else
#include <fcntl.h>
#endif
#include <stdio.h>
#include <curses.h>
#include "consts.h"
#include "funcs.h"
 
#define ALT_ADD(screen, y, x, ch) \
       wmove(screen, y, x); \
       waddch(screen, ch);
#define STAND_ADD(screen, y, x, ch) \
       wmove(screen, y, x); \
       wstandout(screen); \
       waddch(screen, ch); \
       wstandend(screen);
#define STAND_ALT_ADD(screen, y, x, ch) \
       wstandout(screen); \
       ALT_ADD(screen, y, x, ch); \
       wstandend(screen);
 
char *Template[] = {
"                                                                            ",
".                                                                          /",
"<                                                                        /v+",
"<                                                                       /+++",
"<                                                                     /v++++",
"<                            /====.                                   >+++++",
"<                            `v30v<                                  /++++++",
"<                             >++++.                                 >++++++",
"<                             >++++<                                /+++++++",
"<                             >++++<                               /++++++++",
"+.                            >+++++.                              `++++++++",
"+<                           /++++++<                               `+++++++",
"+<                          /+++++++<                                >++++++",
"+<                          >+++++++<                          /v.   >++++++",
"+<                          >+++++++<                         /+++.  >++++++",
"+<                          >+++++++'                         >+++<  >++++++",
"++====.                /====+++++++<                          >+++<  >++++++",
"++v30v<            /vvv+v15v+++++++<          /v.           /v++++<  >++++++",
"+++++++v.       /vv++++++++++++++++<        /v+++====.      >++++++==+++++++",
"+++++++++=======+++++++++++++++++++<      /v+++++v20v+======+++++++50+++++++",
"+++++++++vv10vvv++++++++++++++++++++======++++++++++++vv15vv++++++++++++++++",
"++++++++++++++++++++++++++++++++++++vv25vv++++++++++++++++++++++++++++++++++"};
 
int PadScore[MAX_PADS] = {30, 30, 15, 20, 50, 10, 15, 25};
int LastLegalY, LastLegalX;
 
#ifdef BSD
typedef char chtype_port_t;
#else
typedef chtype chtype_port_t;
#endif
 
static chtype_port_t LineMap[128] = {0};
static int Old_Y, Old_X;
 
void Introduction();
void dumpcore();
void zap();
 
void InitialiseScreen(init_scr)
 
WINDOW **init_scr;
 
       {
       register WINDOW *screen;
 
       signal(SIGINT, zap);
       signal(SIGQUIT, dumpcore);
       initscr();
       printf("\033[?25l");
       noecho();
       cbreak();
       *init_scr = stdscr;
       screen = *init_scr;
#ifdef BSD
       LineMap['\''] = '+';
       LineMap['.'] = '+';
       LineMap['|'] = '|';
       LineMap['-'] = '-';
       LineMap['/'] = '+';
       LineMap['`'] = '+';
       LineMap['+'] = '+';
       LineMap['>'] = '+';
       LineMap['<'] = '+';
       LineMap['^'] = '+';
       LineMap['v'] = '+';
       LineMap[PAD] = '=';
#else
       LineMap['\''] = ACS_LRCORNER;
       LineMap['.'] = ACS_URCORNER;
       LineMap['|'] = ACS_VLINE;
       LineMap['-'] = ACS_HLINE;
       LineMap['/'] = ACS_ULCORNER;
       LineMap['`'] = ACS_LLCORNER;
       LineMap['+'] = ACS_PLUS;
       LineMap['>'] = ACS_LTEE;
       LineMap['<'] = ACS_RTEE;
       LineMap['^'] = ACS_BTEE;
       LineMap['v'] = ACS_TTEE;
       LineMap[PAD] = ACS_HLINE;
#endif
       Introduction(screen);
       printf("\033[?25l");
       }
 
void DrawScreen(screen)
 
WINDOW *screen;
 
       {
       register int i, j, scr_i;
       chtype_port_t map;
       char *line, ch;
 
       werase(screen);
       for (i = 0; i < SCR_Y; ++i)
               {
               line = Template[i];
               scr_i = i + 1;
               for (j = 0; j < SCR_X; ++j)
                       {
                       ch = *(line + j);
                       map = LineMap[ch];
                       if (map)
                               if (ch == PAD)
                                       {
                                       STAND_ALT_ADD(screen, scr_i, j, map);
                                       }
                               else
                                       {
                                       ALT_ADD(screen, scr_i, j, map);
                                       }
                       else
                               if (ch == PAD)
                                       {
                                       STAND_ADD(screen, scr_i, j, ch);
                                       }
                               else
                                       mvwaddch(screen, scr_i, j, ch);
                       }
               }
       wrefresh(screen);
       LastLegalY = SCR_ADJ(0);
       LastLegalX = 0;
       Old_Y = Old_X = -1;
       }
 
int MoveLander(screen, land_y, land_x)
 
WINDOW *screen;
double land_y, land_x;
 
       {
       int y, x, touchup = 0, screen_y, new_legal;
       double y_real, x_real;
       char ch;
 
       y_real = (ALTITUDE_INIT - land_y) / ALTITUDE_INIT * (double) SCR_Y;
       x_real = land_x / LANDSCAPE_WIDTH * (double) SCR_X;
       y = y_real + 0.5;
       x = x_real + 0.5;
       new_legal = LEGAL_YX(y, x);
       if (y != Old_Y || x != Old_X)
               {
               if (LEGAL_YX(Old_Y, Old_X))
                       {
                       mvwaddch(screen, SCR_ADJ(Old_Y), Old_X, '.');
                       touchup = 1;
                       }
               if (new_legal)
                       {
                       screen_y = SCR_ADJ(y);
#ifdef BSD
                       ALT_ADD(screen, screen_y, x, '$');
#else
                       ALT_ADD(screen, screen_y, x, ACS_TTEE);
#endif
                       wmove(screen, screen_y, x);
                       LastLegalY = y;
                       LastLegalX = x;
                       touchup = 1;
                       }
               }
       if (touchup)
               wrefresh(screen);
       Old_Y = y;
       Old_X = x;
       if (new_legal)
               {
               ch = *(Template[y] + x);
               if (ch == PAD)
                       return LANDED;
               if (ch != ' ')
                       return CRASH;
               }
       else
               return CRASH;
       return FLYING;
       }
 
static void zap()
 
       {
       printf("\033[?25h");
       endwin();
#ifndef VMS
       fcntl(fileno(stdin), F_SETFL, 0);
#endif
       exit(1);
       }
 
static void dumpcore()
 
       {
       printf("\033[?25h");
#ifndef VMS
       fcntl(fileno(stdin), F_SETFL, 0);
#endif
       endwin();
       signal(SIGQUIT, SIG_DFL);
       kill(getpid(), SIGQUIT);
       pause();
       }
 
#define L0 "Lunar Lander"
#ifdef VMS
#define L1 "The classic arcade game comes to VMS."
#else
#define L1 "The classic arcade game comes to Unix."
#endif
#define L2 "Controls:"
#define L3 "'0'-'9' - power level"
#define L4 "'z' - set left retro rocket to power level"
#define L5 "'x' - set vertical thrust to power level"
#define L6 "'c' - set right retro to power level"
#define L7 "--press space to start game--"
#define L8 "Stacey Campbell at HCR, 1989"
#define CENTRE(win, line, str) mvwaddstr(win, line, (COLS - sizeof(str)) / 2, \
       str)
 
static void Introduction(screen)
 
WINDOW *screen;
 
       {
       werase(screen);
       CENTRE(screen, 2, L0);
       CENTRE(screen, 5, L1);
       CENTRE(screen, 9, L2);
       CENTRE(screen, 11, L3);
       CENTRE(screen, 12, L4);
       CENTRE(screen, 13, L5);
       CENTRE(screen, 14, L6);
       CENTRE(screen, 18, L8);
       CENTRE(screen, 20, L7);
#ifdef BSD
       wrefresh(screen);
#endif
       while (wgetch(screen) != ' ');
       }

#ifdef BSD
 
int flash()
 
       {
#ifndef VMS
       putchar(7);
#endif
       }
 
#ifndef FNDELAY
#define FNDELAY O_NDELAY
#endif

int nodelay(win, flag)
 
WINDOW *win;
int flag;
 
#ifdef VMS
{ return 0; }
#else
       {
       int res;
 
       res = fcntl(fileno(stdin), F_GETFL, 0);
       if (flag)
               {
               res |=  FNDELAY;
               fcntl(fileno(stdin), F_SETFL, res);
               }
       else
               {
               res &= ~FNDELAY;
               fcntl(fileno(stdin), F_SETFL, res);
               }
       return 0;
       }
#endif
#endif
