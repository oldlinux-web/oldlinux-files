enum inputtype { FD, STRING };

#define EOF (-1)

extern void initinput(void);
extern Node *parseline(char *);
extern int gchar(void);
extern void ugchar(int);
extern Node *doit(boolean);
extern void flushu(void);
extern void pushinput(int /*enum inputtype*/,...);
extern void popinput(void);
extern int last;

extern boolean rcrc;
extern char *histstr;
extern int histfd;
