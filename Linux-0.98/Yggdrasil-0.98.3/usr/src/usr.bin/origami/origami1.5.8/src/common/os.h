/* os specific stuff which will be included in every module */
/*{{{  typedef SHORT*/
#ifdef _MINIX
typedef unsigned char SHORT;
#else
typedef unsigned int SHORT;
#endif
/*}}}  */
/*{{{  TOS*/
#ifdef TOS
#  ifdef MISC_C
#    include <ext.h>
#  endif
#  ifdef ORIGAMI_C
#    include <tos.h>
#  endif
 
#  define F_OK 1
#  define R_OK 2
#  define W_OK 4
 
  int access(char *file, int flag);
  void putenv(char *string);
  void st_redraw(void);
  void pre_shell(void);
  void post_shell(void);
  void set_path(char *filename);
  void st_init(void);
  void st_exit(void);
  void fileselect(char *str);
  void vputchar(char c);
  void newsystem(char *command);
  void newexit (int status);
  void alarm(unsigned long seconds);
  void putw(int w, FILE *s);
  int getw(FILE *s);
  void abort_scanner(void);
  char *mktemp(char *pattern);
  FILE *popen(char *command, char *mode);
  int pclose (FILE *stream);
#  undef putchar
#  define putchar(c) vputchar(c)
#  define strlen(a) ((int) strlen(a)) /* this seems to work in TurboC */
#  define sleep(a)
#  ifndef ST_UTIL_C
#    define signal(a,b)
#    define system(a) newsystem(a)
#    define exit(a) newexit(a)
#  endif
#  define _POSIX_PATH_MAX 120
#endif
/*}}}  */
/*{{{  stdio stuff*/
#ifndef TOS
/* set everything like it should be set */
#define READ "r"
#define READ_BIN "r"
#define WRITE "w"
#define WRITE_BIN "w"
#define PATH_SEP "/"
#else
#define READ "r"
#define READ_BIN "rb"
#define WRITE "w+"
#define WRITE_BIN "w+b"
#define PATH_SEP "\\"
#endif
/*}}}  */
