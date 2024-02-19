#include <stdio.h>
#include <curses.h>
#include "consts.h"
#include "funcs.h"
 
#define HEADER "Scores"
#define NAME_LEN 20
#define HS_ENTRIES 10
#define SC_WIN_LEN 16
#define SPC_LINE (SC_WIN_LEN - 2)
 
typedef struct score_pad_t {
       int y;
       int start_x;
       int end_x;
       } score_pad_t;
typedef struct score_t {
       int score;
       char name[NAME_LEN];
       } score_t;
 
extern double Fuel;
extern int PadScore[];
extern int BSLandings, Landings;
extern char *Template[];
extern int LastLegalY, LastLegalX;
 
score_pad_t ScorePad[MAX_PADS];
int Score;
int BestScore = 0;
 
static char *HSFile = HS_FILE;
static int TotalPads;
 
void ScDisplayErr();
int ScCmp();
 
void ScReadDisplay(back_win)
 
WINDOW *back_win;
 
       {
       int old_y, old_x, items, i;
       WINDOW *score_win;
       FILE *fp;
       score_t score_pad[HS_ENTRIES];
 
       if ((fp = fopen(HSFile, "r")) == NULL)
               {
               ScDisplayErr(back_win, "Unable to display HS file.");
	       fclose(fp);
               return;
               }
       getyx(back_win, old_y, old_x);
       wmove(back_win, 0, 0);
       wrefresh(back_win);
       score_win = newwin(SC_WIN_LEN, 25, 2, 5);
       werase(score_win);
       box(score_win, 0, 0);
       items = fread((char *)score_pad, sizeof(score_t), HS_ENTRIES, fp);
       fclose(fp);
       if (items == 0)
               {
               mvwaddstr(score_win, 5, 1, "HS file empty.");
               PressSpace(back_win, old_y, old_x, score_win, SPC_LINE, 1);
               return;
               }
       mvwaddstr(score_win, 1, (25 - sizeof(HEADER)) / 2, HEADER);
       wrefresh(score_win);
       for (i = 0; i < items; ++i)
               {
               wmove(score_win, 3 + i, 1);
               wprintw(score_win, "%4d %s", score_pad[i].score,
                   score_pad[i].name);
               }
       PressSpace(back_win, old_y, old_x, score_win, SPC_LINE, 1);
       }
 
void ScWrite(back_win)
 
WINDOW *back_win;
 
       {
       int items;
       FILE *fp;
       char *user;
       score_t score_pad[HS_ENTRIES + 1];
       void qsort();
       char *getenv();
 
       if (Score == 0)
               return;
       if ((fp = fopen(HSFile, "r")) == NULL)
               {
               ScDisplayErr(back_win, "Unable to read HS file.");
               ScDisplayErr(back_win, "Attempting to create HS file.");
               if (creat(HSFile, 0777) == -1)
                       {
                       ScDisplayErr(back_win,
                           "Unable to create HS file, check pathname.");
		       fclose(fp);
                       return;
                       }
               }
       fclose(fp);
       if ((fp = fopen(HSFile, "r")) == NULL)
               {
               ScDisplayErr(back_win, "Unable to read new HS file.");
	       fclose(fp);
               return;
               }
       items = fread((char *)score_pad, sizeof(score_t), HS_ENTRIES, fp);
       fclose(fp);
       #ifdef LINUX_HACK
       if ((user = getenv("LOGNAME")) == NULL)
               {
               ScDisplayErr(back_win, "Environment variable LOGNAME must be set.");
               return;
               }
       #else
       if ((user = getenv("USER")) == NULL) {
	 ScDisplayErr(back_win, "Environment variable USER must be set.");
	 return;
       }
       #endif
       strcpy(score_pad[items].name, user);
       score_pad[items].score = Score;
       ++items;
       qsort((char *)score_pad, items, sizeof(score_t), ScCmp);
       if (items > HS_ENTRIES)
               items = HS_ENTRIES;
       if ((fp = fopen(HSFile, "r+")) == NULL)
               {
               ScDisplayErr(back_win, "Unable to write HS file.");
	       fclose(fp);
               return;
               }
       if (fwrite((char *)score_pad, sizeof(score_t), items, fp) == 0)
               ScDisplayErr(back_win, "No HS entries written.");
       fclose(fp);
       }
 
static int ScCmp(sc_rec1, sc_rec2)
 
score_t *sc_rec1, *sc_rec2;
 
       {
       if (sc_rec1->score < sc_rec2->score)
               return 1;
       if (sc_rec1->score > sc_rec2->score)
               return -1;
       return 0;
       }
 
void PressSpace(back_win, y, x, cur_win, sy, sx)
 
WINDOW *back_win, *cur_win;
int y, x, sy, sx;
 
       {
       mvwaddstr(cur_win, sy, sx, "--press space--");
       wrefresh(cur_win);
       while (wgetch(cur_win) != ' ');
       delwin(cur_win);
       wmove(back_win, y, x);
       touchwin(back_win);
       wrefresh(back_win);
       }
 
static void ScDisplayErr(back_win, str)
 
WINDOW *back_win;
char *str;
 
       {
       int old_y, old_x, win_len;
       WINDOW *err_win;
 
       getyx(back_win, old_y, old_x);
       wmove(back_win, 0, 0);
       wrefresh(back_win);
       win_len = strlen(str) + 2;
       err_win = newwin(3, win_len, LINES / 2 - 2, (COLS - win_len - 2) / 2);
       box(err_win, 0, 0);
       mvwaddstr(err_win, 1, 1, str);
       wrefresh(err_win);
       sleep(3);
       delwin(err_win);
       wmove(back_win, old_y, old_x);
       touchwin(back_win);
       wrefresh(back_win);
       }
 
void UpdateScore(screen)
 
WINDOW *screen;
 
       {
       int pad_i = 0, found = 0, fuel_bonus, diff_bonus;
       char scr_buf[128];
 
       while (pad_i < SCR_X && ! found)
               {
               found = LastLegalX >= ScorePad[pad_i].start_x &&
                       LastLegalX <= ScorePad[pad_i].end_x &&
                       ScorePad[pad_i].y == LastLegalY;
               ++pad_i;
               }
       --pad_i;
       fuel_bonus = Fuel / 100.0 + 0.5;
       sprintf(scr_buf, "Fuel bonus: %d", fuel_bonus);
       mvwaddstr(screen, 3, 25, scr_buf);
       wrefresh(screen);
       diff_bonus = (Landings + 1) * 10;
       sprintf(scr_buf, "Consecutive Landings Bonus: %d", diff_bonus);
       mvwaddstr(screen, 4, 9, scr_buf);
       wrefresh(screen);
       Score += PadScore[pad_i] + fuel_bonus + diff_bonus;
       }
 
void InitScore()
 
       {
       int i, j, pad_count = 0;
       char *line;
 
       for (i = 0; i < SCR_Y; ++i)
               {
               j = 0;
               line = Template[i];
               while (j < SCR_X)
                       {
                       if (line[j] == PAD)
                               {
                               ScorePad[pad_count].y = i;
                               ScorePad[pad_count].start_x = j;
                               while (j < SCR_X && line[j] == PAD)
                                       ++j;
                               if (j >= SCR_X)
                                       --j;
                               ScorePad[pad_count].end_x = j;
                               ++pad_count;
                               }
                       ++j;
                       }
               }
       TotalPads = pad_count;
       }
