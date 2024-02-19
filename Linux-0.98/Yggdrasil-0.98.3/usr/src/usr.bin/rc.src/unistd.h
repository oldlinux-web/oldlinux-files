extern PID_T fork(void);
extern PID_T getpid(void);
extern int chdir(const char *);
extern int close(int);
extern int dup2(int, int);
extern int execve(const char *, const char **, const char **);
extern int execl(const char *, ...);
extern int getegid(void);
extern int geteuid(void);
extern int getgroups(int, int *);
extern int isatty(int);
#ifndef SYSVR4 /* declares AND defines this in sys/stat.h!! */
extern int mknod(const char *, int, int);
#endif
extern int pipe(int *);
extern int read(int, void *, unsigned int);
extern int setpgrp(int, PID_T);
extern int unlink(const char *);
extern int wait(int *);
extern int write(int, const void *, unsigned int);

extern int optind;
extern char *optarg;
extern int errno;

extern char *getenv(const char *);
