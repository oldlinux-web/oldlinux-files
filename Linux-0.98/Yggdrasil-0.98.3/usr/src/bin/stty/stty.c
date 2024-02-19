/* 
 Copyright (c) Lawrence Foard, 1992 
 This software may be used and distributed under the terms of the gnu 
 public license.
 This version of stty is made to work with LINUX but should work on any
 os with termios (I hope).
 Please report any bugs, or send suggestions to:
  E-mail: entropy@wintermute.wpi.edu
 
  Snail-mail:
   Lawrence Foard
   Virtual World Information Systems
   Suite #791
   1415 Main St.
   Worcester MA 01603 (USA)
*/
/* Version 0.1 */
#include <stdio.h>
#include <string.h>
#include <termio.h>
#include <fcntl.h>

struct alist 
 {
  int num;
  char *str;
 };

/* baud rates */
struct alist bauds[]=
 {
  {B0,"0"},
  {B50,"50"},
  {B75,"75"},
  {B110,"110"},
  {B134,"134"},
  {B150,"150"},
  {B200,"200"},
  {B300,"300"},
  {B600,"600"},
  {B1200,"1200"},
  {B1800,"1800"},
  {B2400,"2400"},
  {B4800,"4800"},
  {B9600,"9600"},
  {B19200,"19200"},
  {B38400,"38400"},
  {B19200,"EXTA"},
  {B38400,"EXTB"},
  {0,NULL}
 };

/* character size */
struct alist bits[]=
 {
  {CS5,"CS5"},
  {CS6,"CS6"},
  {CS7,"CS7"},
  {CS8,"CS8"},
  {0,NULL}
 };

/* lists of flag bits */

/* input flags */
struct alist iflags[]=
 {  
  {IGNBRK,"IGNBRK"},
  {BRKINT,"BRKINT"},
  {IGNPAR,"IGNPAR"},
  {PARMRK,"PARMRK"},
  {INPCK,"INPCK"},
  {ISTRIP,"ISTRIP"},
  {INLCR,"INLCR"},
  {IGNCR,"IGNCR"},
  {ICRNL,"ICRNL"},
  {IUCLC,"IUCLC"},
  {IXON,"IXON"},
  {IXANY,"IXANY"},
  {IXOFF,"IXOFF"},
  {IMAXBEL,"IMAXBEL"},
  {0,NULL}
 };

struct alist oflags[]=
 {
  {OPOST,"OPOST"},
  {OLCUC,"OLCUC"},
  {ONLCR,"ONLCR"},
  {ONOCR,"ONOCR"},
  {ONLRET,"ONLRET"},
  {OFILL,"OFILL"},
  {OFDEL,"OFDEL"},
  {0,NULL}
 };

/* delay modes <yuck> */

struct alist nldly[]=
 {
  {NL0,"NL0"},
  {NL1,"NL1"},
  {0,NULL}
 };

struct alist crdly[]=
 {
  {CR0,"CR0"},
  {CR1,"CR1"},
  {CR2,"CR2"},
  {CR3,"CR3"},
  {0,NULL}
 };

struct alist tabdly[]=
 {
  {TAB0,"TAB0"},
  {TAB1,"TAB1"},
  {TAB2,"TAB2"},
  {TAB3,"TAB3"},
  {0,NULL}
 };

struct alist bsdly[]=
 {
  {BS0,"BS0"},
  {BS1,"BS1"},
  {0,NULL}
 };

struct alist vtdly[]=
 {
  {VT0,"VT0"},
  {VT1,"VT1"},
  {0,NULL}
 };

struct alist ffdly[]=
 {
  {FF0,"FF0"},
  {FF1,"FF1"},
  {0,NULL}
 };

/* composite of all delay modes */
struct alist *delaymodes[]=
 {
  nldly,
  crdly,
  tabdly,
  bsdly,
  vtdly,
  ffdly,
  NULL
 };

/* the mask for each mode */
int delaymasks[]=
 {
  NLDLY,
  CRDLY,
  TABDLY,
  BSDLY,
  VTDLY,
  FFDLY,
  0
 };

struct alist cflags[]=
 {
  {CSTOPB,"CSTOPB"},
  {CREAD,"CREAD"},
  {PARENB,"PARENB"},
  {PARODD,"PARODD"},
  {HUPCL,"HUPCL"},
  {CLOCAL,"CLOCAL"},
  {CIBAUD,"CIBAUD"},
  {CRTSCTS,"CRTSCTS"},
  {0,NULL}
 };

struct alist lflags[]=
 {
  {ISIG,"ISIG"},
  {ICANON,"ICANON"},
  {XCASE,"XCASE"},
  {ECHO,"ECHO"},
  {ECHOE,"ECHOE"},
  {ECHOK,"ECHOK"},
  {ECHONL,"ECHONL"},
  {NOFLSH,"NOFLSH"},
  {TOSTOP,"TOSTOP"},
  {ECHOCTL,"ECHOCTL"},
  {ECHOPRT,"ECHOPRT"},
  {ECHOKE,"ECHOKE"},
  {FLUSHO,"FLUSHO"},
  {PENDIN,"PENDIN"},
  {IEXTEN,"IEXTEN"},
  {0,NULL}
 };

/* special characters */
struct alist special[]=
 {
  {VINTR,"INTR"},
  {VQUIT,"QUIT"},
  {VERASE,"ERASE"},
  {VKILL,"KILL"},
  {VEOF,"EOF"},
  {VTIME,"TIME"},
  {VMIN,"MIN"},
  {VSWTC,"SWTC"},
  {VSTART,"START"},
  {VSTOP,"STOP"},
  {VSUSP,"SUSP"},
  {VEOL,"EOL"},
  {VREPRINT,"REPRINT"},
  {VDISCARD,"DISCARD"},
  {VWERASE,"WERASE"},
  {VLNEXT,"LNEXT"},
  {VEOL2,"EOL2"},
  {0,NULL}
 };

/* look up a number in a list and return the string */
char *numtostr(struct alist *list,int num)
 {
  int a;
  for(a=0;list[a].str;a++)
   if (list[a].num==num)
    return(list[a].str);
  return("UNKNOWN");
 }

/* look up a string in a list and return the number */
int strtonum(struct alist *list,char *str) 
 {
  int a;
  char buff[100],*s;
  strcpy(buff,str);
  for(s=buff;*s;s++)
   if ((*s>='a') && (*s<='z'))
    *s=*s-'a'+'A';
  for(a=0;list[a].str;a++)
   if (!strcmp(buff,list[a].str))
    return(list[a].num);
  return(-1);
 }

char *print_ctrl(char c)
 {
  static char buff[10];
  if (c<32)
   sprintf(buff," '^%c' ",c+'@');
  else if (c==127)
   sprintf(buff," '^?' ");
  else sprintf(buff," '%c'  ",c);
  return(buff);
 }

char strtoctrl(char *s)
 {
  if (!s[0])
   return(0);
  if (!s[1])
   return(s[0]);
  if (s[0]=='^')
   {
    if ((s[1]>'a') && (s[1]<'z'))
     s[1]=s[1]-'a'+'A';
    return((s[1]=='?') ? 127 : s[1]-'@');
   }
  fprintf(stderr,"stty: invalid character %s\n",s);
  exit(1);
 }

#define TFI(f) ((f & term->c_iflag) ? ' ' : '-')
#define TFO(f) ((f & term->c_oflag) ? ' ' : '-')
#define TFC(f) ((f & term->c_cflag) ? ' ' : '-')
#define TFL(f) ((f & term->c_lflag) ? ' ' : '-')

print_settings(struct termios *term,struct winsize *win)
 {
  fprintf(stderr,"---------Characters----------\n");
  fprintf(stderr,"INTR:  %s ",print_ctrl(term->c_cc[VINTR]));
  fprintf(stderr,"QUIT:  %s ",print_ctrl(term->c_cc[VQUIT]));
  fprintf(stderr,"ERASE: %s ",print_ctrl(term->c_cc[VERASE]));
  fprintf(stderr,"KILL:  %s ",print_ctrl(term->c_cc[VKILL]));
  fprintf(stderr,"EOF:   %s\n",print_ctrl(term->c_cc[VEOF]));
  fprintf(stderr,"TIME:  %3d    ",term->c_cc[VTIME]);
  fprintf(stderr,"MIN:   %3d    ",term->c_cc[VMIN]);
  fprintf(stderr,"SWTC:  %s ",print_ctrl(term->c_cc[VSWTC]));
  fprintf(stderr,"START: %s ",print_ctrl(term->c_cc[VSTART]));
  fprintf(stderr,"STOP:  %s\n",print_ctrl(term->c_cc[VSTOP]));
  fprintf(stderr,"SUSP:  %s ",print_ctrl(term->c_cc[VSUSP]));
  fprintf(stderr,"EOL:   %s ",print_ctrl(term->c_cc[VEOL]));
  fprintf(stderr,"EOL2:  %s ",print_ctrl(term->c_cc[VEOL2]));
  fprintf(stderr,"LNEXT: %s\n",print_ctrl(term->c_cc[VLNEXT]));
  fprintf(stderr,"DISCARD: %s ",print_ctrl(term->c_cc[VDISCARD]));
  fprintf(stderr,"REPRINT: %s ",print_ctrl(term->c_cc[VREPRINT]));
  fprintf(stderr,"RWERASE: %s\n",print_ctrl(term->c_cc[VWERASE]));
  fprintf(stderr,"----------Control Flags---------\n");
  fprintf(stderr,
"%cCSTOPB %cCREAD %cPARENB %cPARODD %cHUPCL %cCLOCAL %cCRTSCTS\n",
 TFC(CSTOPB),TFC(CREAD),TFC(PARENB),TFC(PARODD),TFC(HUPCL),TFC(CLOCAL),
 TFC(CRTSCTS));
  fprintf(stderr,"Baud rate: %s Bits: %s\n",
  numtostr(bauds,term->c_cflag & CBAUD),numtostr(bits,term->c_cflag & CSIZE));
  fprintf(stderr,"----------Input Flags----------\n");
  fprintf(stderr,
"%cIGNBRK %cBRKINT %cIGNPAR %cPARMRK %cINPCK %cISTRIP  %cINLCR %cIGNCR\n",
 TFI(IGNBRK),TFI(BRKINT),TFI(IGNPAR),TFI(PARMRK),TFI(INPCK),TFI(ISTRIP),
  TFI(INLCR),TFI(IGNCR));
  fprintf(stderr,
"%cICRNL  %cIUCLC  %cIXON   %cIXANY  %cIXOFF %cIMAXBEL\n",
 TFI(ICRNL),TFI(IUCLC),TFI(IXON),TFI(IXANY),TFI(IXOFF),TFI(IMAXBEL));
  fprintf(stderr,"---------Output Flags---------\n");
  fprintf(stderr,
"%cOPOST %cOLCUC %cONLCR %cOCRNL %cONOCR %cONLRET %cOFILL %cOFDEL\n",
  TFO(OPOST),TFO(OLCUC),TFO(ONLCR),TFO(OCRNL),TFO(ONOCR),TFO(ONLRET),
  TFO(OFILL),TFO(OFDEL));
  fprintf(stderr,"Delay modes: CR%d NL%d TAB%d BS%d FF%d VT%d\n",
   (term->c_oflag & CRDLY)/CR1,
   (term->c_oflag & NLDLY)/NL1,
   (term->c_oflag & TABDLY)/TAB1,
   (term->c_oflag & BSDLY)/BS1,
   (term->c_oflag & FFDLY)/FF1,
   (term->c_oflag & VTDLY)/VT1);
  fprintf(stderr,"-----------Local Flags---------\n");
  fprintf(stderr,
"%cISIG %cICANON %cXCASE %cECHO %cECHOE %cECHOK %cECHONL %cNOFLSH\n",
 TFL(ISIG),TFL(ICANON),TFL(XCASE),TFL(ECHO),TFL(ECHOE),TFL(ECHOK),
 TFL(ECHONL),TFL(NOFLSH));
  fprintf(stderr,
"%cTOSTOP %cECHOCTL %cECHOPRT %cECHOKE %cFLUSHO %cPENDIN %cIEXTEN\n",
 TFL(TOSTOP),TFL(ECHOCTL),TFL(ECHOPRT),TFL(ECHOKE),TFL(FLUSHO),TFL(PENDIN),
 TFL(IEXTEN));
  fprintf(stderr,"rows %d cols %d\n",win->ws_row,win->ws_col);
 }

/* This really needs a man page */
void usage()
 {
  fprintf(stderr,"Copyright (c), Lawrence Foard, 1992\n");
  fprintf(stderr,"May be used under the terms of the gnu public license\n");
  fprintf(stderr,"Version 0.1\n");
  exit(1);
 }

/* macro to set or clear bit(s) depending on setclr's value */
#define SETIT(flags,bit) \
 if (setclr) \
  flags&=~(bit);\
 else\
  flags|=bit;
                          

/* parse one option effect passed structure with it */
int parse_opts(char *options[],struct termios *term,struct winsize *winsize)
 {
  int a,b,c,d,setclr=0;
  char *option;
  for(a=0;options[a];a++)
   {
    option=options[a];
    /* special cases */
    if (!strcmp(option,"rows"))
     {
      winsize->ws_row=atoi(options[++a]);
      continue;
     }
    if (!strcmp(option,"cols"))
     {
      winsize->ws_col=atoi(options[++a]);
      continue;
     }
    if (!strcmp(option,"sane"))
     {
      int c;
      term->c_cflag=(term->c_cflag & ~CBAUD) | (term->c_cflag & CSIZE) | HUPCL;
      term->c_iflag=ICRNL | IXON;
      term->c_oflag=OPOST | ONLCR | 
       ((NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) & term->c_oflag);
      term->c_lflag=ISIG | ICANON | ECHO | ECHOCTL | ECHOKE;
      term->c_cc[VINTR]='C'-'@';
      term->c_cc[VQUIT]='\\'-'@';
      term->c_cc[VERASE]=127;
      term->c_cc[VEOF]='D'-'@';
      term->c_cc[VKILL]='U'-'@';
      term->c_cc[VSTART]='Q'-'@';
      term->c_cc[VSTOP]='S'-'@';
      term->c_cc[VSUSP]='Z'-'@';
      term->c_cc[VLNEXT]='V'-'@';
      term->c_cc[VDISCARD]='O'-'@';
      term->c_cc[VREPRINT]='R'-'@';
      term->c_cc[VWERASE]='W'-'@';
      continue;
     }
    /* is it a baud rate? */
    if (-1!=(b=strtonum(bauds,option)))
     {
      term->c_cflag=(term->c_cflag & ~CBAUD) | b;
      continue;
     }  
    /* is it a character size? */
    if (-1!=(b=strtonum(bits,option)))
     {
      term->c_cflag=(term->c_cflag & ~CSIZE) | b;
      continue;
     }
    /* is it one of the special characters? */
    if (-1!=(b=strtonum(special,option)))
     {
      if (!options[a+1])
       {
        fprintf(stderr,"stty: %s must be followed by a character.\n",option);
        usage();
        exit(1);
       }
      term->c_cc[b]=strtoctrl(options[++a]);
      continue;
     }
    /* scan delay mode lists */
    for(c=d=0;delaymodes[c];c++)
     if (-1!=(b=strtonum(delaymodes[c],option)))
      {
       term->c_oflag=(term->c_oflag & ~delaymasks[c]) | b;
       d=1;
      }
    if (d)
     continue;
     
    /* bit options */
    if (*option=='-')
     {
      setclr=1;
      option++;
      /* skip over single character args already parsed */
      if (!option[1]) 
       switch(option[0])
        {
         case 'b':
         case 'h':
          continue;
         case 'r':
         case 'a':
          a++;
          continue;
         case 'e':
          return;
        }
     }
    else
     setclr=0;
    /* check input flags */
    if (-1!=(b=strtonum(iflags,option)))
     {
      SETIT(term->c_iflag,b);
      continue;
     }
    /* check output flags */
    if (-1!=(b=strtonum(oflags,option)))
     {
      SETIT(term->c_oflag,b);
      continue;
     }
    /* check control flags */
    if (-1!=(b=strtonum(cflags,option)))
     {
      SETIT(term->c_cflag,b);
      continue;
     } 
    /* check local flags */
    if (-1!=(b=strtonum(lflags,option)))
     {
      SETIT(term->c_lflag,b);     
      continue;
     }
    fprintf(stderr,"stty: Invalid option %s\n",options[a]);
    usage();
    exit(1);
   }
 }

/* hangup the line (doesn't work in this version of LINUX) */
dohangup()
 {
  struct termios term,oldterm;
  tcgetattr(1,&term);
  oldterm=term;
  term.c_cflag=(term.c_cflag & ~CBAUD) | B0;
  tcsetattr(1,TCSANOW,&term);
  /* leave dtr down for 1 second */
  sleep(1);
  tcsetattr(1,TCSANOW,&oldterm);
 }

main(int argc,char *argv[])
 {
  struct termios term;
  struct termios oldterm;
  struct winsize win,oldwin;
  int a,hangupb=0,hangupa=0,execc=-1;
  char *command=NULL,*tty=NULL;
  /* if no args just print current state */
  if (!argv[1])
   {
    tcgetattr(1,&term);
    ioctl(1,TIOCGWINSZ,&win);
    print_settings(&term,&win);
    exit(0);
   }
  /* !!! The getty like features arn't quite working !!! */
  /* check for letter options needed before we do this */
  for(a=0;argv[a];a++)
   if ((argv[a][0]=='-') && (argv[a][2]==0))
    switch(argv[a][1]) 
     {
      case 'e': /* exec program */
       execc=++a;
       a=argc;
       break;
      case 'r': /* system run program */
       command=argv[++a];
       break;
      case 'a': /* attach to tty */
       tty=argv[++a];
       break;
      case 'b': /* hangup before command */
       hangupb=1;
       break;
      case 'h': /* hangup after */
       hangupa=1;
       break;
     }
  /* attach to remote tty? */
  if (tty)
   {
    int tfd;
    /* POSIX version of setpgrp() */
    setsid();
    /* open new controlling terminal */
    if (-1==(tfd=open(tty,O_RDWR)))
     { 
      fprintf(stderr,"stty: unable to open %s\n",tty);
      perror("");
      exit(1);
     }
    close(0);
    close(1);
    close(2); 
    dup(tfd);
    dup(tfd); 
    dup(tfd);
    close(tfd);
   }
  /* hangup before doing anything? */
  if (hangupb)
   dohangup();
  tcgetattr(1,&term);
  ioctl(1,TIOCGWINSZ,&win);
  oldterm=term;  
  oldwin=win;
  parse_opts(argv+1,&term,&win);
  tcsetattr(1,TCSANOW,&term);
  ioctl(1,TIOCSWINSZ,&win);
  /* check for an exec */
  if (execc!=-1)
   {
    if (-1==execv(argv[execc],argv+execc))
     {
      perror("stty: exec failed");
      exit(1);
     }
    exit(1);  /* just incase */
   }
  /* check for a command to run using system() */ 
  if (command) 
   {
    /* if this is running on a different tty fork off */
    if (tty && fork())
     exit(0);
    system(command);
    /* restore old terminal attributes */
    tcsetattr(1,TCSANOW,&oldterm);
    ioctl(1,TIOCSWINSZ,&oldwin);
   }
  /* handup after */
  if (hangupa)
   dohangup();
 }
