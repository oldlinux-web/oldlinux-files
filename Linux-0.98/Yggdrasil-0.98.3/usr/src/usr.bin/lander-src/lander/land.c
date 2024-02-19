#include <stdio.h>
#include <math.h>
#include <curses.h>
#include "consts.h"
#include "funcs.h"
 
#define INIT_DX_VAL 166.0                     /* initial horizontal velocity */
#define INIT_DX_INC 13.0   /* rate of init hor vel increase between landings */
#define INIT_DY_VAL 0.0                         /* initial vertical velocity */
#define INIT_DY_INC 3.0    /* rate of decrease in vert velocity b/w landings */

#ifdef LINUX_HACK
#define M_PI_2 6.28
#define M_PI 3.14
#endif

extern int LastLegalY, LastLegalX;
extern int Score, BestScore;
extern double FuelDec;
 
void EndCurses();
void StartLander();
int FlyLander();
int CleanUp();
void Explode();
 
int Landings;
int BSLandings = 0;
 
main(argc, argv)
 
int argc;
char *argv[];
 
       {
       double init_dy;                  /* initial rate of fall per landing */
       double init_dx;           /* initial horizontal velocity per landing */
       WINDOW *screen;                        /* main display curses window */
       int LanderStatus;              /* status of lander at end of landing */
 
       InitialiseScreen(&screen);                   /* do basic screen init */
       InitScore();
       do      {
                                          /* init screen and lander for game */
               StartLander(&init_dy, &init_dx);
               do      {
                       InitMoves(screen);    /* init lander for one landing */
                       DrawScreen(screen);   /* init screen for one landing */
                       LanderStatus = FlyLander(screen, init_dy, init_dx);
                       init_dy -= INIT_DY_INC;       /* make landing harder */
                       init_dx += INIT_DX_INC;
                       } while (LanderStatus == LANDED);     /* until crash */
               } while (CleanUp(screen));  /* while user wants another game */
       EndCurses(screen);                           /* final screen cleanup */
       return 1;
       }
 
/*
** StartLander() - initialise a new game.
*/
 
extern int Score;
 
static void StartLander(init_dy, init_dx)
 
double *init_dy, *init_dx;
 
       {
       Landings = 0;
       Score = 0;
       FuelDec = 0.0;
       *init_dy = INIT_DY_VAL;
       *init_dx = INIT_DX_VAL;
       }
 
/*
** EndCurses() - final cleanup before program exit.
*/
 
static void EndCurses(screen)
 
       {
       nodelay(screen, FALSE);                        /* switch off nodelay */
       wmove(screen, 1, 0);
       wclrtoeol(screen);                             /* display best score */
       wprintw(screen, "--Best Score: %d ", BestScore);
       wprintw(screen, "with %d landing%s - press space--", BSLandings,
           BSLandings == 1 ? "" : "s");
       wrefresh(screen);
       while (wgetch(screen) != ' ');
       printf("\033[?25h");
       endwin();                                        /* shut down curses */
       }
 
static int CleanUp(screen)
 
WINDOW *screen;
 
       {
       char ch;
 
       nodelay(screen, FALSE);
       if (Score > BestScore)
               {
               BestScore = Score;
               BSLandings = Landings;
               }
       ScWrite(screen);
       ScReadDisplay(screen);
       wmove(screen, 1, 0);
       wprintw(screen, "--Game Over - Safe Landings: %d - another game?--",
               Landings);
       wrefresh(screen);
       do      {
               ch = wgetch(screen);
               } while (ch != 'Y' && ch != 'y' && ch != ' ' && ch != 'n');
       return ch == 'Y' || ch == 'y' || ch == ' ';
       }
 
static int FlyLander(screen, y_move, x_move)
 
WINDOW *screen;
double y_move, x_move;
 
       {
       int land_stat;
       double altitude = ALTITUDE_INIT;
       double longditude = 0.0;
 
       for (land_stat = FLYING; land_stat == FLYING;)
               {
               sleep(1);
               GetMove(screen, &y_move, &x_move);
               land_stat = MoveLander(screen, altitude, longditude);
               switch (land_stat)
                   {
                   case FLYING :
                       y_move -= GRAVITY;
                       altitude += y_move;
                       if (altitude < 0.0)
                               altitude = 0.0;
                       longditude += x_move;
                       break;
                   case LANDED :
                       if (y_move < -(ACCEPTABLE))
                               {
                               Explode(screen, LastLegalY, LastLegalX);
                               land_stat = CRASH;
                               }
                       else
                               UpdateScore(screen);
                       break;
                   case CRASH :
                       Explode(screen, LastLegalY, LastLegalX);
                       break;
                   }
               wmove(screen, 0, 0);
               wclrtoeol(screen);
               wprintw(screen,
                   "alt: %8.3f X: %8.2f dY: %7.3f dX: %7.3f Score: %5d",
                   altitude, longditude, y_move, x_move, Score);
               wrefresh(screen);
               }
       if (land_stat == LANDED)
               {
               ++Landings;
               nodelay(screen, FALSE);
               wmove(screen, 1, 0);
               wprintw(screen, "--Safe Landing Number: %d", Landings);
               waddstr(screen, " - press space bar--");
#ifdef BSD
               wrefresh(screen);
#endif
               while (wgetch(screen) != ' ');
               nodelay(screen, TRUE);
               }
       return land_stat;
       }
 
#define SEQ_COUNT (sizeof(sequenceA) / sizeof(sequenceA[0]))
#define AVERAGE 9
#define DEVIATION 4
#define MAX_PARTS (AVERAGE + DEVIATION)
static char sequenceA[] = {'-', '/', '|', '\\'};
static char sequenceB[] = {'-', '\\', '|', '/'};
 
static void Explode(screen, Y_bang, X_bang)
 
WINDOW *screen;
int Y_bang, X_bang;
 
       {
       int particles, i, new_y, new_x, draw_y, draw_x, touched, toy, tox;
       int overlay[SCR_Y][SCR_X];
#ifdef BSD
       int old_chs[SCR_Y][SCR_X];
#else
       chtype old_chs[SCR_Y][SCR_X];
#endif
       double x_inc;
       struct paths_t {
               double x_mult;
               double y_mult;
               double x;
               int old_y;
               int old_x;
               int seq_no;
               char *sequence;
               } paths[MAX_PARTS];
       struct paths_t *path;
       long lrand48(), time();
       double drand48();
       void srand48();
 
       wstandout(screen);
       mvwaddch(screen, SCR_ADJ(Y_bang), X_bang, '*');
       wstandend(screen);
       wrefresh(screen);
       srand48(time((long *) 0));
       memset((char *)overlay, 0, sizeof(overlay));
       particles = (AVERAGE + lrand48() % DEVIATION);
       x_inc = M_PI_2 * (drand48() * 0.01 + 0.01);
       for (i = 0; i < particles; ++i)
               {
               path = &paths[i];
               path->x = 0.0;
               path->x_mult = drand48() * (double) SCR_X * 0.2;
               path->y_mult = drand48() * (double) SCR_Y * 1.1 + 5.0;
               path->old_y = -1;
               path->old_x = -1;
               path->seq_no = lrand48() % SEQ_COUNT;
               path->sequence = i & 1 ? sequenceA : sequenceB;
               flash();
               }
       while (paths[0].x < M_PI)
               for (i = 0; i < particles; ++i)
                       {
                       touched = 0;
                       path = &paths[i];
                       new_x = path->x * path->x_mult + 0.5;
                       if (i & 1)
                               new_x = -new_x;
                       new_y = sin(path->x) * path->y_mult + 0.5;
                       draw_y = Y_bang - new_y;
                       draw_x = X_bang + new_x;
                       toy = path->old_y;
                       tox = path->old_x;
                       if (LEGAL_YX(toy, tox))
                               if (!(--overlay[toy][tox]))
                                       {
                                       touched = 1;
                                       mvwaddch(screen, SCR_ADJ(toy), tox,
                                           old_chs[toy][tox]);
                                       }
                       if (LEGAL_YX(draw_y, draw_x))
                               {
                               wmove(screen, SCR_ADJ(draw_y), draw_x);
                               if (!overlay[draw_y][draw_x])
                                       old_chs[draw_y][draw_x] = winch(screen);
                               waddch(screen,
                                   path->sequence[path->seq_no++ % SEQ_COUNT]);
                               ++overlay[draw_y][draw_x];
                               touched = 1;
                               }
                       if (touched)
                               {
#ifdef SYS5_3
                               flushinp();
#endif
                               wrefresh(screen);
                               }
                       path->old_y = draw_y;
                       path->old_x = draw_x;
                       path->x += x_inc;
                       }
       for (i = 0; i < particles; ++i)
               {
               path = &paths[i];
               toy = path->old_y;
               tox = path->old_x;
               if (LEGAL_YX(toy, tox))
                       mvwaddch(screen, SCR_ADJ(toy), tox, old_chs[toy][tox]);
               }
       wrefresh(screen);
       }
 
#if defined(RAND_BSD)
static long lrand48()
 
       {
       long random();
 
       return random();
       }
 
static void srand48(seed)
 
long seed;
 
       {
       srandom((int) seed);
       }
 
#define PERIOD (4096 - 1)
 
static double drand48()
 
       {
       return random() % PERIOD / (double) PERIOD;
       }
#endif
