/*{{{  mailheader*/
/*
From berg Thu Apr 11 14:01:17 1991
Return-Path: <berg>
Received: by messua (4.1/CIP.3)
        id AA01154; Thu, 11 Apr 91 14:01:14 +0200
Date: Thu, 11 Apr 91 14:01:14 +0200
From: berg (Stephen R. van den Berg)
Message-Id: <9104111201.AA01154@messua>
To: u31b3hs
Subject: fork construction... (of procmail)
Cc: stumvoll
Status: R

I know it is a bit lengthy, but I wanted to give you some almost complete
program.  Almost all references are resolved.  The most important function
is of course 'pipthrough', it takes as arguments: command line for the filter
program, char ptr to source of block to be filtered, and length of the block.
pipthrough will return to the calling program with returncode 0 if the filter
has been started successfully (and things need to be read back), and will
return to the calling program with returncode =! 0 if the filter failed.

The forked of part of the program NEVER returns from pipthrough.

Hope this helps.
-----------------------------cut here----------------------------------------
------------------------cut here----------------------------------------------
Sincerely,                 berg@marvin.e17.physik.tu-muenchen.de
Stephen R. van den Berg.
"I code it in 5 min, optimize it in 90 min, because it's so well optimized:
it runs in only 5 min.  Actually, most of the time I optimize programs."
*/
/*}}}  */

/*{{{  defines*/
#define PRD1    poutfd[0]
#define PWR1    poutfd[1]
#define PRD2    pinfd[0]
#define PWR2    pinfd[1]
#define PRD3    pbackfd[0]
#define PWR3    pbackfd[1]
#define BLKSIZ  16384
/*}}}  */
/*{{{  vars*/
volatile int flaggerd=2;
pid_t thepid;
/*}}}  */

/*{{{  main*/
main(){
  thepid=getpid();
  signal(SIGQUIT,flagger);
  signal(SIGTERM,sterminate);
  if(!pipthrough(progname,startchar,tobesent)) readback();
}
/*}}}  */

/*{{{  pipthrough*/
pipthrough(line,source,len)char*line,*source;
const long len;
{
        int pinfd[2],poutfd[2];
        pipe(pbackfd);
        pipe(pinfd);
        flaggerd=0;                   /* main pipes setup */
        /*{{{  create a sending procmail*/
        if(!(pidchild=fork())){
          backblock=source;
          backlen=len;
          signal(SIGTERM,stermchild);
          signal(SIGINT,stermchild);
          signal(SIGHUP,stermchild);
          signal(SIGQUIT,stermchild);
          rclose(PRD2);
          rclose(PRD3);
          pipe(poutfd);
          rclose(STDOUT);
          /*{{{  create the filter*/
          if(!(pidfilt=fork())){
            rclose(PWR1);
            rclose(PWR3);
            dup(PWR2);
            rclose(PWR2);
            getstdin(PRD1);
            callnewprog(line);
          }
          /*}}}  */
          rclose(PWR2);
          rclose(PRD1);
          if(forkerr(pidfilt,line)){
            rclose(PWR1);
            stermchild();
          }
          /*{{{  send text to be filtered*/
          if(dump(PWR1,source,len)){
            writeerr(line);
            stermchild();
          }
          /*}}}  */
          /*{{{  check the exitcode of the filter*/
          if(pwait&&waitfor(pidfilt)!=EX_OK){
            progerr(line);
            stermchild();
          }
          /*}}}  */
          rclose(PWR3);
          kill(thepid,SIGQUIT);
          exit(EX_OK);
        } /* tell parent to proceed */
        /*}}}  */
        rclose(PWR2);
        rclose(PWR3);
        getstdin(PRD2);
        if(forkerr(pidchild,"procmail")) return 1;
        return 0;
}              /* we stay behind to read back in the filtered text */
/*}}}  */
/*{{{  waitflagger    wait for SIGQUIT from child*/
waitflagger(){
  while(!flaggerd) sleep(16);
} /* to prevent polling */
/*}}}  */
/*{{{  flagger        hey, we received a SIGQUIT*/
void flagger(){
  signal(SIGQUIT,flagger);
  flaggerd=1;
}
/*}}}  */
/*{{{  dump*/
long dump(fd,source,len)const int fd;
const char*source;
long len;
{
  int i;
  if(fd>=0){
    while(i=rwrite(s,source,BLKSIZ<len?BLKSIZ:(int)len)){
      if(i<0){
        i=0;
        goto writefin;
      }
      len-=i;
      source+=i;
    }
    writefin:
    rclose(s);
    return len-i;
  }
  return len?len:-1;
}       /* return an error even if nothing was to be sent */
/*}}}  */
/*{{{  readdyn*/
char*readdyn(bf,filled)char*bf;
long*const filled;
{
  /*{{{  variables*/
  int i;
  long oldsize;
  /*}}}  */
 
  oldsize=*filled;
  goto jumpin;
  do{
    *filled+=i;
jumpin:
    bf=realloc(bf,*filled+BLKSIZ);  /* dynamic adjust */
jumpback:
    ;
  } while(0<(i=rread(STDIN,bf+*filled,BLKSIZ)));
  switch(flaggerd){
    case 0:
      waitflagger();
    case 1:
      getstdin(PRD3);
      if(1==rread(STDIN,buf,1)){
        bf=realloc(bf,(*filled=oldsize+1)+BLKSIZ);
        bf[oldsize]=*buf;
        flaggerd=2;
        goto jumpback;
      }
    default:
      ;
  }
  if(!*filled) return realloc(bf,1);  /* +1 for housekeeping purposes */
  return realloc(bf,*filled+1);
}                 /* minimize the buffer space */
/*}}}  */
char*backblock;         /* what is to be recovered in case of filter failure */
long backlen;                                                  /* its length */
pid_t pidfilt,pidchild;
int pbackfd[2];              /* the emergency pipe (no, not the backpipe :-) */
/*{{{  sterminate*/
void sterminate(){
        signal(SIGTERM,SIG_IGN);
        signal(SIGHUP,SIG_IGN);
        signal(SIGINT,SIG_IGN);
        kill(pidchild,SIGTERM);
        terminate();
}
/*}}}  */
/*{{{  stermchild*/
void stermchild(){
        signal(SIGHUP,SIG_IGN);
        signal(SIGINT,SIG_IGN);
        signal(SIGQUIT,SIG_IGN);
        signal(SIGTERM,SIG_IGN);
        kill(pidfilt,SIGTERM);
        kill(thepid,SIGQUIT);
        log("Rescue of unfiltered data ");
        if(dump(PWR3,backblock,backlen))
                log("failed\n");
        else
                log("succeeded\n");
        exit(EX_UNAVAILABLE);
}
/*}}}  */
/*{{{  waitfor*/
waitfor(pid)const pid_t pid;
{
        int i;           /* wait for a specific process */
        while(pid!=wait(&i)||(i&127)==127);
        return i>>8&255;
}
/*}}}  */
/*{{{  getstdin*/
getstdin(pip)const int pip;
{
        rclose(STDIN);
        dup(pip);
        rclose(pip);
}
/*}}}  */
/*{{{  callnewprog*/
callnewprog(newname)char*const newname;
{
        int argc;
        char*endp,**newargv;
        register char*p;
        yell("Executing",p=newname);
        if(sh){
                const char*newargv[4];                   /* should we start a shell? */
                newargv[3]=0;
                newargv[2]=p;
                newargv[1]=tgetenv(shellflags);
                *newargv=tgetenv(shell);
                shexec(newargv);
        }
        argc=2;
        while(*p)     /* If no shell, we'll have to chop up the arguments ourselves */
                if(*p==' '||*p=='\t'){
                        argc++;
                        *p='\0';
                        while(*++p==' '||*p=='\t')
                                *p='\0';
                }
                else
                        p++;
        endp=p;
        *(newargv=malloc(argc*sizeof*newargv))=p=newname;
        argc=1;
        for(;;){
                while(*p)
                        p++;
                while(!*p){
                        if(p==endp){
                                newargv[argc]=0;
                                shexec(newargv);
                        }
                        p++;
                }
                newargv[argc++]=p;
        }
}
/*}}}  */
/*{{{  writeerr*/
writeerr(line)const char*const line;
{
        log("Error while writing to");
        logqnl(line);
}
/*}}}  */
/*{{{  forkerr*/
forkerr(pid,a)const pid_t pid;
const char*const a;
{
        if(pid==-1){
                log("Failed forking");
                logqnl(a);
                return 1;
        }
        return 0;
}
/*}}}  */
/*{{{  progerr*/
progerr(line)const char*const line;
{
        log("Program failure of");
        logqnl(line);
}
/*}}}  */
/*{{{  log(a)const char*const a;*/
log(a)const char*const a;
{
        rwrite(STDERR,a,strlen(a));
}
/*}}}  */
/*{{{  shexec(argv)const char *const*argv;*/
shexec(argv)const char *const*argv;
{
        int i;
        const char**newargv,**p;
        execvp(*argv,argv);     /* if this one fails, we retry it as a shell script */
        for(p=argv,i=1;i++,*p++;);
        newargv=malloc(i*sizeof*p);
        for(*(p=newargv)=binsh;*++p=*++argv;);
        execve(*newargv,newargv,environ);            /* no shell script? -> trouble */
        log("Failed to execute");
        logqnl(*argv);
        exit(EX_UNAVAILABLE);
}
/*}}}  */
/*{{{  rclose(fd)const int fd;*/
rclose(fd)const int fd;
{
        int i;        /* a sysV secure close (signal immune) */
        while((i=close(fd))&&errno==EINTR);
        return i;
}
/*}}}  */
/*{{{  rwrite(fd,a,len)const int fd,len;*/
rwrite(fd,a,len)const int fd,len;
void*const a;
{
        int i; /* a sysV secure write */
        while(0>(i=write(fd,a,(size_t)len))&&errno==EINTR);
        return i;
}
/*}}}  */
/*{{{  rread(fd,a,len)const int fd,len;*/
rread(fd,a,len)const int fd,len;
void*const a;
{
        int i;   /* a sysV secure read */
        while(0>(i=read(fd,a,(size_t)len))&&errno==EINTR);
        return i;
}
/*}}}  */
/*{{{  ropen(name,mode,mask)const char*const name;*/
ropen(name,mode,mask)const char*const name;
const mode_t mask;
{
        int i;
        while(0>(i=open(name,mode,mask))&&errno==EINTR);      /* a sysV secure open */
        return i;
}
/*}}}  */
