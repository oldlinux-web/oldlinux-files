/* Ok, I admit it... what I've done here is a mortal sin, but the XOBBS module
   for ax_mbx.c is so different from the W9NK mailbox, and I hate forcing folks
   to move files around before running a make, so here it is... the two cat'ed
   together with an "if XOBBS ... else ... endif" wrapper... sick, but it
   works...   Bdale */

#ifdef XOBBS
/* XOBBS.C  module to link NET thru IPC message queues to the XOBBS mailbox. */
/* Jim Durham, W2XO, 1-5-89 */
/* requires a call to axchk() to be inserted in the main commutator loop of
    the net main.c code */

#include <stdio.h>
#include "global.h"
#include "config.h"
#include "mbuf.h"
#include "ax25.h"
#include "timer.h"
#include "lapb.h"
#include "cmdparse.h"
#include "iface.h"
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "netrom.h"
#include "nr4.h"
#include "ax_mbx.h"
#ifdef	UNIX
#undef	toupper
#undef	tolower
#include <memory.h>
#include <string.h>
#endif

#ifdef	BSD
char *sprintf();
#endif

/*#define DEBUG 	  1*/

void fwdxmit();
void fwdrecv();
void fwdstate();
void axbbs_xmit();
char xogetc();
static char conbuf[10][10];        /*buffers used in getcon() and fwdcon()*/
int conindex,conchar;
int fwdflag;			/*flag to do various forwarding tasks*/
int pid;
int fwdbbsstate = NOCON;	/* con/discon state of forwarding bbs process */
int smsgqid;			/* send message queue (from viewpoint of bbs) */
int rmsgqid;			/* receive message queue (from viewpoint of bbs) */
int scmsgqid;			/* send CONTROL message queue */
int rcmsgqid;			/* receive CONTROL message queue */
int lastindex;                  /*last bbs index used*/
int nsend;
int ax25mbox;
char temp[128];
char *bbsexe = "xobbs";		/*path to bbs executable*/
struct  ax25_cb *bbscb;		/*forwarding bbs control block*/
struct message  msg;

struct mboxsess * base = NULLMBS;  /*pointer to base of mailbox chain*/
struct mboxsess fwdstruct;	/*forwarding session*/

#ifdef SID2
extern struct ax25_addr bbscall;
#endif

static struct mboxsess *
newmbox()
{
	struct mboxsess * mbp ;

        if(base == NULLMBS){
            base =(struct mboxsess *)malloc(sizeof(struct mboxsess));
    	    base->next = NULLMBS;
	    initmbox(base);
    	    return base;
    	}
    	else{
	    mbp = base;
	    while(mbp->next != NULLMBS)       /*go up the chain to the top*/
	        mbp = mbp->next;
	    mbp->next =(struct mboxsess *)malloc(sizeof(struct mboxsess));
	    mbp->next->next = NULLMBS;
	    initmbox(mbp->next);
	    return mbp->next;
	}
}

initmbox(mbp)
    struct mboxsess * mbp;
{
	
    mbp->bytes = 0;
    mbp->gotbytes = 0;
    mbp->mtxtptr = mbp->rmsgbuf.mtext;
}

freembox(mbp)
    struct mboxsess * mbp;
{
    struct mboxsess *p;
    
    if(mbp == base){            /*special case for base session*/
        if(base->next != NULLMBS)/*if base is NOT the only session*/
            base = base->next;   /*then make base point to next session*/
        else
            base = NULLMBS;     /*otherwise,just set base to NULLMBS*/
        free(mbp);              /*free up the storage*/
        return;                 /*and go away*/
    }
    p = base;
    for(;;){
        if(p->next == mbp){     /*if the next upward session is THE one*/
            p->next = mbp->next; /*eliminate the next upward session*/
            free(mbp);
            return;
        }
        if(p->next == NULLMBS){ /*something is wrong here!*/
            free(mbp);          /*try to fix it*/
            return;
        }
        p = p->next;
    }
}
	
	        
axchk()		/* service any ax25 connections w2xo 9-1-88*/
{
    int rdcnt;
    struct mbuf *bp;
    struct mboxsess * mbp;
    char *cp;
    int testsize,size;
    void axinit();
    
/* initialization code removed.  call to axinit() direct from main now. */
    
    if(base == NULLMBS)                       /*no mailboxes?..then leave*/
        goto fcontchk;
    mbp = base;					/*point to the session base*/
    while(mbp != NULLMBS){			/*and climb the session links*/
        if(mbp->gotbytes < 1)			/*if we need input*/
            if((mbp->gotbytes=msgrcv(smsgqid,&mbp->rmsgbuf,1024,(long)mbp->pid,
               IPC_NOWAIT|0600)) < 1)		/*and there is none, ..punt*/
                 goto contchk;
            else				/*got input!*/
                mbp->mtxtptr = mbp->rmsgbuf.mtext; /*new message buffer, set to start*/

            testsize = min(mbp->bytes,mbp->cbadr.axbbscb->paclen+1);      /* get the minimum of the # */
            size = min(testsize, mbp->gotbytes) + 1;       /* of bytes avail and paclen*/
            bp = alloc_mbuf((int16)size);                  /* then min of avail and have*/
            cp = bp->data;
	    if(mbp->proto == AX25){
                *cp++ = PID_FIRST | PID_LAST | PID_NO_L3;
                bp->cnt =1;
            }
            else  bp->cnt = 0;
            
            while(bp->cnt < size && mbp->gotbytes){
                *cp++ = *mbp->mtxtptr++;
                bp->cnt++;
                mbp->gotbytes--;
            }

	    if(mbp->proto == AX25)
	        send_ax25(mbp->cbadr.axbbscb,bp);
	    else
	        send_nr4(mbp->cbadr.axnrcb,bp);
                /*now, check for control messages*/

contchk:    rdcnt=msgrcv(scmsgqid,&msg,1024,(long)mbp->pid,IPC_NOWAIT|0600);
#ifdef DEBUG
	    if(rdcnt > 0) printf("net:Control Message received: rdcnt=%d\n",rdcnt);
#endif
            switch(rdcnt){
                case 0:     break;
                case -1:    /*perror("axchk002");*/
                            break;
                default:    if(msg.mtext[0] == 'K'){
#ifdef DEBUG
			        printf("recbbs: K received\n");
#endif
				kill(mbp->pid,9);
				wait(NULLCHAR);
				clearque(mbp->pid);
 				if(mbp->proto == AX25)
 					disc_ax25(mbp->cbadr.axbbscb);
 				else
 					disc_nr4(mbp->cbadr.axnrcb);

				freembox(mbp);
                            }/* if K */
            }/* switch */
        mbp = mbp->next;
    }/*while loop*/

fcontchk:    chkfwd();			/*check the forwarder*/
}

dombox(argc, argv)
int argc ;
char *argv[] ;
{
	if (argc < 2) {
		domboxdisplay() ;
		return 0 ;
	}

	if (argv[1][0] == 'y' || (strcmp(argv[1],"on") == 0))
		ax25mbox = 1 ;
	else if (argv[1][0] == 'n' || (strcmp(argv[1],"off") == 0))
		ax25mbox = 0 ;
	else if (argv[1][0] == '?')
		printf("ax25 mailbox is %s\n", ax25mbox ? "on" : "off") ;
	else
		printf("usage: mbox [y|n|?]\n") ;

	return 0 ;
}

static domboxdisplay()
{

	struct mboxsess * mbp;
	printf("Process-ID     &cb      Protocol      User\n") ;
    	mbp = base;    
	while(mbp != NULLMBS){
            printf("%-13d%-13x",mbp->pid,mbp->cbadr.axbbscb);             
    	        if(mbp->proto == AX25)
	            printf("Ax25        %s\n",mbp->call);
	        else
	            printf("Net/Rom     %s\n",mbp->call);
    	    mbp = mbp->next;
        }/*while loop*/
    printf("\n\n");
    printf("Forwarder State is: ");
    switch(fwdflag){
      case 0: printf("NOFWD\n");
              break;
      case 1: printf("SPAWNBBS\n");
              break;
      case 2: printf("DOCONNECT\n");
              break;
      case 3: printf("READXMIT\n");
              break;
      case 4: printf("GETCON\n");
              break;
      case 5: printf("CLEARCON\n");
              break;
      case 6: printf("WAITING\n");
              break;
      default: printf("UNDEFINED\n");
              break;
    }
}
/* Incoming mailbox session via ax.25 */


/* * This is the ax25 receive upcall function
 *it gathers incoming data and stuff it down the IPC queue to the proper BBS
 */

void
mbx_incom(axp,cnt)	
struct ax25_cb *axp;
int16 cnt;
{
#ifndef SID2
        char arg1[10],arg2[10],arg3[10],arg4[10],arg5[10];	
	void mbx_tx(), mbx_rx(), mbx_state();
	int j;
	struct mboxsess *mbp;
	struct mbuf *bp;
	struct mbuf *recv_ax25();
	
                mbp =newmbox();		/*after this, this is a mailbox connection*/
        				/* so, make a new mailbox session*/
	        axp->r_upcall = mbx_rx ;
	        axp->t_upcall = mbx_tx ;
		axp->s_upcall = mbx_state;
                mbp->cbadr.axbbscb=axp;
                mbp->proto = AX25;
                mbp->bytes = 32;	/*jump start the upcall*/

		bp = recv_ax25(axp,cnt) ;	/* get the initial input */
		free_p(bp) ;			/* and throw it away to avoid confusion */


                for(j=0;j<6;j++){               /*now, get incoming call letters*/
                    arg1[j]=mbp->cbadr.axbbscb->addr.dest.call[j];
                    arg1[j]=arg1[j] >> 1;
                    arg1[j]=arg1[j] & (char)0x7f;
                    if(arg1[j]==' ') arg1[j]='\0';
                }
                arg1[6]='\0';			/*terminate call letters*/
	        strcpy(mbp->call,arg1);   /*Copy call to session*/
                sprintf(arg2,"%d\0",rmsgqid);   /*set up args telling what the*/
                sprintf(arg3,"%d\0",smsgqid);   /*message ques are for the */
                sprintf(arg4,"%d\0",rcmsgqid);   /*execed process*/
                sprintf(arg5,"%d\0",scmsgqid);   /*execed process*/

                                           /*now, fork and exec the bbs*/
                if((pid=fork()) == 0){              /*if we are the child*/
                    if(execlp(bbsexe,"xobbs",arg1,arg2,arg3,arg4,arg5,0) < 0){
#ifdef DEBUG
                        printf("Bad exec on BBS\n");
#endif
                        exit(1);
                    }
                }
                else                      /* parent process is still NET */
                    mbp->pid=pid;         /* save pid of new baby*/
#endif
}	

void
mbx_rx(axp,cnt)		/*w2xo  2-16-89  */
struct ax25_cb *axp;		/* send chars down pipe to bbs */
int16 cnt;
{
    struct mbuf *bp;
    struct mbuf *recv_ax25();
    struct mboxsess * mbp;

	
    if(base == NULLMBS)
        return;
    mbp = base;
    while(mbp != NULLMBS){
	if(mbp->proto == AX25){
            if(mbp->cbadr.axbbscb == axp){  /* match requested block? */
                if((bp = recv_ax25(axp,cnt)) == NULLBUF)  /*nothing there*/
		    continue;
                while(bp != NULLBUF){
                    strncpy(msg.mtext,bp->data,bp->cnt);
                    msg.mtype = (long)mbp->pid;
                    if(msgsnd(rmsgqid,&msg,bp->cnt,0600) < 0)
                        perror("axincoming");
                    bp=free_mbuf(bp);   /*free the mbuf and get the next */
                }
            } /* if matching axp */
        }/*if ax25 */
    mbp = mbp->next;
    }/*while loop*/
}

void
mbx_tx(axp,cnt)
struct ax25_cb *axp;
int16 cnt;
{
    struct mboxsess * mbp;
    if(base == NULLMBS)
        return;                     /*no sessions*/
    mbp = base;
    while(mbp != NULLMBS){
        if(mbp->cbadr.axbbscb == axp)
            mbp->bytes = cnt;
    mbp = mbp->next;
    }
}

void
mbx_state(axp,old,new)		/*w2xo  3-23-89 */
struct ax25_cb *axp;
int old,new;
{
#ifdef SID2
        char arg1[10],arg2[10],arg3[10],arg4[10],arg5[10];	
	void mbx_tx(), mbx_rx(), mbx_state();
	int j;
#endif
	struct mboxsess *mbp;

        switch(new){
            case DISCONNECTED:
		   if((old == DISCONNECTED) || (old == DISCPENDING))
		       return;
                if(base == NULLMBS)
                    break;
        	mbp = base;
                while(mbp != NULLMBS){
                    if(axp == mbp->cbadr.axbbscb && mbp->proto == AX25){
                        kill(mbp->pid,9);     /*kill bbs process*/
                        wait(NULLCHAR);
                        freembox(mbp);
                        break; /* from while loop */
                    } /*if*/
                    mbp = mbp->next;
                }/*while loop*/
                break;   /*end of DISCONNECTED case*/
					
            case CONNECTED:
#ifdef SID2
		if(old != DISCONNECTED)
		    return;
	        if(!calleq(axp,&bbscall)){ /*not for the mailbox*/
		    axp->s_upcall = NULLVFP;
		    return;
	        }
	
                mbp =newmbox();		/*after this, this is a mailbox connection*/
        				/* so, make a new mailbox session*/
	        axp->r_upcall = mbx_rx ;
	        axp->t_upcall = mbx_tx ;

                mbp->cbadr.axbbscb=axp;
                mbp->proto = AX25;
                mbp->bytes = 32;	/*jump start the upcall*/

                for(j=0;j<6;j++){               /*now, get incoming call letters*/
                    arg1[j]=mbp->cbadr.axbbscb->addr.dest.call[j];
                    arg1[j]=arg1[j] >> 1;
                    arg1[j]=arg1[j] & (char)0x7f;
                    if(arg1[j]==' ') arg1[j]='\0';
                }
                arg1[6]='\0';			/*terminate call letters*/
	        strcpy(mbp->call,arg1);   /*Copy call to session*/
                sprintf(arg2,"%d\0",rmsgqid);   /*set up args telling what the*/
                sprintf(arg3,"%d\0",smsgqid);   /*message ques are for the */
                sprintf(arg4,"%d\0",rcmsgqid);   /*execed process*/
                sprintf(arg5,"%d\0",scmsgqid);   /*execed process*/

                                           /*now, fork and exec the bbs*/
                if((pid=fork()) == 0){              /*if we are the child*/
                    if(execlp(bbsexe,"xobbs",arg1,arg2,arg3,arg4,arg5,0) < 0){
#ifdef DEBUG
                        printf("Bad exec on BBS\n");
#endif
                        exit(1);
                    }
                }
                else                      /* parent process is still NET */
                    mbp->pid=pid;         /* save pid of new baby*/
#endif
                break;
        }/* end of switch*/
}/*end of routine*/


/* Incoming mailbox session via net/rom */

void
mbx_nr4incom(cb)
register struct nr4cb *cb ;
{
	void mbx_nr4tx(), mbx_nr4rx(), mbx_nr4state() ;
        char arg1[10],arg2[10],arg3[10],arg4[10],arg5[10];
        struct mboxsess * mbp;
	int j;
		
        mbp = newmbox();

	cb->r_upcall = mbx_nr4rx ;
	cb->s_upcall = mbx_nr4state ;
	cb->t_upcall = mbx_nr4tx ;

        mbp->cbadr.axnrcb = cb;
        mbp->proto = NTRM;
        mbp->bytes = 32;	/*jump start the upcall*/

        for(j=0;j<6;j++){               /*now, get incoming call letters*/
            arg1[j]=mbp->cbadr.axnrcb->user.call[j];
            arg1[j]=arg1[j] >> 1;
            arg1[j]=arg1[j] & (char)0x7f;
            if(arg1[j]==' ') arg1[j]='\0';
        }
        arg1[6]='\0';			/*terminate call letters*/
	strcpy(mbp->call,arg1);  /*copy call to session*/

        sprintf(arg2,"%d\0",rmsgqid);   /*set up args telling what the*/
        sprintf(arg3,"%d\0",smsgqid);   /*message ques are for the */
        sprintf(arg4,"%d\0",rcmsgqid);   /*execed process*/
        sprintf(arg5,"%d\0",scmsgqid);   /*execed process*/

                                         /*now, fork and exec the bbs*/
        if((pid=fork()) == 0){              /*if we are the child*/
            if(execlp(bbsexe,"xobbs",arg1,arg2,arg3,arg4,arg5,0) < 0){
#ifdef DEBUG
                printf("Bad exec on BBS\n");
#endif
                exit(1);
            }
        }
        else                            /* parent process is still NET */
            mbp->pid=pid;         /* save pid of new baby*/
}

/* receive upcall for net/rom */
/* mbx_nr4rx collects lines, and calls mbx_line when they are complete. */
/* If the lines get too long, it arbitrarily breaks them. */

void mbx_nr4rx(cb,cnt)
struct nr4cb *cb ;
int16 cnt ;
{
    struct mbuf *bp;
    struct mbuf *recv_ax25();
    struct mboxsess * mbp;


	
    if(base == NULLMBS)
        return;
    mbp = base;
    while(mbp != NULLMBS){
	if(mbp->proto == NTRM){
            if(mbp->cbadr.axnrcb == cb){  /* match requested block? */
                if((bp = recv_nr4(cb,cnt)) == NULLBUF)  /*nothing there*/
		    continue;
                while(bp != NULLBUF){
                    strncpy(msg.mtext,bp->data,bp->cnt);
                    msg.mtype = (long)mbp->pid;
                    if(msgsnd(rmsgqid,&msg,bp->cnt,0600) < 0)
                        perror("nrincoming");
                    bp=free_mbuf(bp);   /*free the mbuf and get the next */
                }
            } /* if matching axp */
        } /* if netrom */
    mbp = mbp->next;
    } /*while loop*/
}

void
mbx_nr4tx(cb,cnt)
struct nr4cb *cb;
int16 cnt;
{
    struct mboxsess *mbp;
    if(base == NULLMBS)
        return;
    mbp = base;
    while(mbp != NULLMBS){
        if(mbp->cbadr.axnrcb == cb)
            mbp->bytes = cnt;
        mbp = mbp->next;
    }
}

/* state upcall for net/rom */
void mbx_nr4state(cb,old,new)
struct nr4cb *cb ;
int old, new ;
{
    struct mboxsess * mbp;

    switch(new){
        case DISCONNECTED:
            if(base == NULLMBS)
                return;
            mbp = base;
            while(mbp != NULLMBS){
                if(cb == mbp->cbadr.axnrcb){
                    kill(mbp->pid,9);     /*kill bbs process*/
                    wait(NULLCHAR);
                    freembox(mbp);
                    break; /* from for loop */
                }
            mbp = mbp->next;
            }/*end of while loop*/
        break;   /*end of DISCONNECTED case*/
					
        case CONNECTED:
            break; /* end of connected case*/
    }/* end of switch*/
}/*end of routine*/

indexx(s,t)
char s[],t[];
{
	int i,j,k;
	
	for(i=0;s[i]!= '\0'; i++){
		for(j=i,k=0;t[k]!='\0' && s[j] == t[k]; j++,k++)
			;
		if(t[k] == '\0')
			return(i);
	}
	return(-1);
}


chkfwd()				/* check forwarding and take appropriate*/
{					/* actions*/
    int gotbytes;

    if(fwdbbsstate==WAIT)
        return;
    switch(fwdflag){
        case NOFWD:     /* check for forward request */
		  if(msgrcv(scmsgqid,&msg,1024,1L,IPC_NOWAIT|0600) > 0) {
		      switch(msg.mtext[0]){
			  case 'F': fwdflag = SPAWNBBS;  /* forwarding request */
				    break;
			  case 'B': sendbeac();
				    break;
			  default:  break;
		      }
		  }
		  break;
        case SPAWNBBS:  spawnfwd();
                        nsend = false;
                        break;
        case DOCONNECT: fwdcon();
                        break;
        case READXMIT : fwdsend();
                        break;
        case GETCON:    getcon();
                        break;
        case CLEARCON:  clearcon(nsend);
        		if(!(nsend))	/* only do clearcon without N once*/
        		    nsend = true;
                        break;
        case WAITING:   break;
        default: break;
    }
    if(fwdflag != NOFWD){ /*if forwarder, check for control*/
        gotbytes = msgrcv(scmsgqid,&msg,256,(long)fwdstruct.pid,IPC_NOWAIT|0600);
        switch(gotbytes){
            case 0:     break;
            case -1:    /*perror("axchk003");*/
                        break;
            default:    if(msg.mtext[0] == 'K'){
            	        kill(fwdstruct.pid,9);
            	        wait(NULLCHAR);
#ifdef DEBUG
			printf("clearing ques\n");
#endif
                        clearque(fwdstruct.pid);
			fwdflag = NOFWD;
			disc_ax25(fwdstruct.cbadr.axbbscb);
                	fwdstruct.cbadr.axbbscb = NULLFWD;
                        }
                        else if(msg.mtext[0] == 'N'){
#ifdef DEBUG
                            printf("net:N received from BBS\n");
#endif
                            if(fwdbbsstate==YESCON){ /*if we are really connected*/
                                fwdbbsstate=WAIT;  /*wait until disconnect before next*/
                                disc_ax25(fwdstruct.cbadr.axbbscb); /*do the disconnect */
#ifdef DEBUG
                            printf("net:disconnect issued\n");
#endif
                            }
#ifdef DEBUG
                            printf("net:setting flag to CLEARCON\n");
#endif
                            fwdflag=CLEARCON;  /*set up for next connect*/
                        }
                        break;
        } /*switch*/
    } /* if forwarder active */
}

spawnfwd()
{
    char arg1[20],arg2[20],arg3[20],arg4[20],arg5[20];

      			/*fork and exec the forwarder*/
    if((fwdstruct.pid=fork()) == 0){              /* if this is the child*/
        strcpy(arg1,"forward");
        sprintf(arg2,"%d\0",rmsgqid);    /*tell the bbs what the message*/
        sprintf(arg3,"%d\0",smsgqid); 	/* que ids to use are*/
        sprintf(arg4,"%d\0",rcmsgqid);
        sprintf(arg5,"%d\0",scmsgqid);

			/*check for bad exec */
        if(execlp(bbsexe,"xobbs",arg1,arg2,arg3,arg4,arg5,0) < 0){
	    fprintf(stderr,"Bad Exec on forwarder\n");
	    exit(1);
	}
    }/* if fwdstruct.pid is 0 */
    else{
        fwdflag=CLEARCON;  /*not the child, so set flag for next level*/
	fwdstruct.gotbytes = 0;
	fwdstruct.bytes = 0;
	fwdstruct.proto = AX25;
	fwdstruct.mtxtptr = fwdstruct.rmsgbuf.mtext;
    }
}
    
clearcon(nsend)
    int nsend;
{
    int i;

#ifdef DEBUG
    printf("net:clearcon:entering routine\n");
#endif
    for(i=0;i<10;i++)
        conbuf[i][0]='\0';
#ifdef DEBUG
    printf("net:clearcon:Clearing ques\n");
#endif
    conindex=conchar=0;			/*clear the connect buffer*/
    fwdflag=GETCON;
    clearque(fwdstruct.pid);
    if(nsend && fwdstruct.cbadr.axbbscb != NULLFWD){  /*don't send N to new bbs or no bbs */
        msg.mtype = (long)fwdstruct.pid;
	sprintf(msg.mtext,"N\0");
#ifdef DEBUG
        printf("net:Sending %s to bbs\n",msg.mtext);
#endif
        msgsnd(rcmsgqid,&msg,2,0600);
#ifdef DEBUG
        printf("net:clearcon:sent N to bbs process ID %d\n",fwdstruct.pid);
#endif
    }
}
    

getcon()                        /*routine called when fwdflag is "GETCON"*/
{                               /* to read the connect request*/

    fwdstruct.gotbytes = msgrcv(smsgqid,&fwdstruct.rmsgbuf,256,(long)fwdstruct.pid,IPC_NOWAIT|0600);  /* is there anything in the pipe?*/
    switch(fwdstruct.gotbytes){
        case 0:     return;
        case -1:    /*perror("getcon001");*/
                    break;
        default:    strncpy(conbuf[conindex],fwdstruct.rmsgbuf.mtext,fwdstruct.gotbytes);
		    if(fwdstruct.rmsgbuf.mtext[0] != 0x04){
                        conbuf[conindex++][fwdstruct.gotbytes] = '\0';
                        break;
                   }
                   else
                   fwdflag = DOCONNECT;
    }
    fwdstruct.gotbytes=0;
}

fwdcon()
{
    char *argv[10];

    char buf[128];
    static char temp[] = "con";

    int numdigis;
    int i;
      
    argv[1] = &conbuf[0][0];	/* get device to use*/
    argv[2] = &conbuf[1][0];	/* get who to connect to*/
    numdigis=atoi(&conbuf[2][0]);
    for(i=0; i < numdigis;i++)
    	argv[3+i] = &conbuf[3+i][0];

    argv[0]= temp;              /* set up for the doconfwd() call */
 
    fflush(stdout);
     if(!doconfwd(3+numdigis,argv)){               /*do the connect*/
         sprintf(msg.mtext,"OK\n");
         msg.mtype = (long)fwdstruct.pid;
         if(msgsnd(rmsgqid,&msg,3,0600) < 0)
             perror("fwdcon001");
	 fwdflag=WAITING;
         return 0;
     }
     else{
         sprintf(buf,"NOGOOD\n");
	 msg.mtype = (long)fwdstruct.pid;
         if(msgsnd(rmsgqid,&msg,7,0600) < 0)
             perror("fwdcon002");
         fwdflag=CLEARCON;
         return 1;
    }
}

void
fwdstate(axp,old,new)		/*w2xo      10-24-88 */
struct ax25_cb *axp;
int old,new;
{


    switch(new){
	case DISCONNECTED:  if(fwdflag==READXMIT){
				fwdflag=CLEARCON;
			    }
			    fwdbbsstate=NOCON;
			    kill(fwdstruct.pid,SIGUSR1); /*tell bbs about discon*/
			    break;
	                    	
	case CONNECTED:	    fwdflag=READXMIT;
			    fwdstruct.bytes = 32; /*jump start t_upcall*/
			    fwdbbsstate=YESCON;
			    break;
        default:            break;
    }
}

void
fwdxmit(axp,cnt)
struct ax25_cb *axp;
int16 cnt;
{
		/*all we want to do here is establish the max number of*/
		/* bytes to send at any one time*/

	if(axp == fwdstruct.cbadr.axbbscb)
	    fwdstruct.bytes = cnt;
}

void
fwdrecv(axp,cnt)		/*w2xo  10-24-88  */
struct ax25_cb *axp;		/* send chars down pipe to forwarding bbs */
int16 cnt;
{
    struct mbuf *bp;
    struct mbuf *recv_ax25();
	
    if((bp = recv_ax25(axp,cnt)) == NULLBUF)  /*nothing there*/
        return;

    while(bp != NULLBUF){
        strncpy(msg.mtext,bp->data,bp->cnt);
	msg.mtype = (long)fwdstruct.pid;
        if(msgsnd(rmsgqid,&msg,bp->cnt,0600) < 0)
            perror("fwdrecv");
        bp=free_mbuf(bp);
    }
}

fwdsend()		/* service bbs ax25 connections w2xo 10-24-88*/
{
    int size,testsize;
    struct mbuf *bp;
    char *cp;

    if(fwdstruct.cbadr.axbbscb != NULLFWD){
        if(fwdstruct.gotbytes < 1)
            if((fwdstruct.gotbytes=msgrcv(smsgqid,&fwdstruct.rmsgbuf,256,(long)fwdstruct.pid,
               IPC_NOWAIT|0600)) < 1)
                 return;   /*return if nothing to send*/
            else
                fwdstruct.mtxtptr = fwdstruct.rmsgbuf.mtext; /*new message buffer, set to start*/

        testsize = min(fwdstruct.bytes,fwdstruct.cbadr.axbbscb->paclen+1);      /* get the minimum of the # */
        size = min(testsize, fwdstruct.gotbytes) +1;       /* of bytes avail and paclen*/
        bp = alloc_mbuf((int16)size);                  /* then min of avail and have*/
        cp = bp->data;
        *cp++ = PID_FIRST | PID_LAST | PID_NO_L3;
        bp->cnt = 1;
        while(bp->cnt < size){
            *cp++ = *fwdstruct.mtxtptr++;
            bp->cnt++;
            fwdstruct.gotbytes--;
        }
        send_ax25(fwdstruct.cbadr.axbbscb,bp);
    } /*if active */
}

/* Initiate AX.25 forwarding connection to remote bbs */
doconfwd(argc,argv)
int argc;
char **argv;
 {
	void fwdrecv(),fwdxmit(),fwdstate();
	struct ax25_addr dest;
	struct ax25 addr;
	struct ax25_cb *open_ax25();
	struct interface *ifp;
	struct mboxsess *mbp;
	extern int16 axwindow;
	int i;

	for(ifp = ifaces; ifp != NULLIF; ifp = ifp->next)
		if(strcmp(argv[1],ifp->name) == 0)
			break;
	if(ifp == NULLIF){
		printf("Interface %s unknown\n",argv[1]);
		return 1;
	}

	mbp = base;			/* point to the session base */
	while (mbp != NULLMBS) {	/* and climb the session */
		if (!strncmp(mbp->call,argv[2],strlen(mbp->call)))
			return 1;
		mbp = mbp->next;	/* bomb out if session to this bbs */
	}				/* exists */

	setcall(&dest,argv[2]);
#ifndef NETROM
	ASSIGN(addr.source,mycall);
#else
	memcpy(&addr.source,ifp->hwaddr,AXALEN);
#endif
	setcall(&addr.dest,argv[2]);
	for(i=3; i < argc; i++)
		setcall(&addr.digis[i-3],argv[i]);

	addr.ndigis = i - 3;
	fwdstruct.cbadr.axbbscb = open_ax25(&addr,axwindow,fwdrecv,fwdxmit,fwdstate,ifp,(char *)0);
	return 0;
}


fgetline(fd,s)
    int fd;
    char *s;
{
    int i;
    char c;
    i=0;
    while((c=xogetc(fd)) && ((c != '\n') && (c != '\r')))
        s[i++] = c;

    s[i] = '\0';
    return(i);
}


char xogetc(fd)
{
	char c;
	if(read(fd,&c,1)){
		c &= (char)0x7f;		/*strip 8th bit and up*/
		if(c=='\r')
		    c='\n';
		return(c);
	}
	else return('\0');
}


void
axinit()
{
	int fd;

	if((smsgqid=msgget(SENDKEY,IPC_CREAT|0600)) < 0){
		perror("msgget smsgqid:");
		exit(1);
	}
	else
	    printf("Send message que identifier is:%d\n",smsgqid);
	    
	if((rmsgqid=msgget(RCVKEY,IPC_CREAT|0600)) < 0){
		perror("msgget rmsgqid:");
		exit(1);
	}
	else
	    printf("Receive message que identifier is:%d\n",rmsgqid);

	if((scmsgqid=msgget(SCTLKEY,IPC_CREAT|0600)) < 0){
		perror("msgget scmsgqid:");
		exit(1);
	}
	else {
	    printf("Control send message que identifier is:%d\n",scmsgqid);
	}

	if((rcmsgqid=msgget(RCTLKEY,IPC_CREAT|0600)) < 0){
		perror("msgget rcmsgqid:");
		exit(1);
	}
	else
	    printf("Control receive message que identifier is:%d\n",rcmsgqid);

}

clearque(pid)
    int pid;
{
	while(msgrcv(smsgqid,&msg,256,(long)pid,IPC_NOWAIT|0600) > 0)
	    ;
	while(msgrcv(rmsgqid,&msg,256,(long)pid,IPC_NOWAIT|0600) > 0)
	    ;
	while(msgrcv(scmsgqid,&msg,256,(long)pid,IPC_NOWAIT|0600) > 0)
	    ;
	while(msgrcv(rcmsgqid,&msg,256,(long)pid,IPC_NOWAIT|0600) > 0)
	    ;
}

calleq(axp,addr)
    struct ax25_cb *axp;
    struct ax25_addr *addr;
{
	register int i;

	if(memcmp(axp->addr.source.call,addr->call,ALEN) != 0)
		return 0;
	if((axp->addr.source.ssid & SSID) != (addr->ssid & SSID))
		return 0;
	return 1;
}

sendbeac()
{
	int gotbytes;
	struct mbuf *bp;
	char *cp,*kp;
	struct interface *iface;
        extern struct interface *ifaces;
        		
	if((gotbytes = msgrcv(smsgqid,&msg,1024,(long)2,0600)) < 0){
		perror("sendbeac:receiving message");
		return;
	}
	
	for(iface = ifaces; iface->next != NULLIF ; iface = iface->next){
	    if((iface->name[0] != 'a') || (iface->name[1] != 'x'))
	    	continue;
            bp = alloc_mbuf((int16)(gotbytes + 2));                  /* then min of avail and have*/
            cp = bp->data;
            kp = msg.mtext;
            *cp++ = PID_FIRST | PID_LAST | PID_NO_L3;
            bp->cnt = 1;
            do{
                *cp++ = *kp++;
                bp->cnt++;
            }while(bp->cnt < (gotbytes + 1));

#ifdef SID2
	    ax_output(iface,(char*)&ax25_bdcst,(char *)&bbscall,(PID_FIRST |PID_LAST|PID_NO_L3),bp);
#else
	    ax_output(iface,(char*)&ax25_bdcst,(char *)mycall,(PID_FIRST |PID_LAST|PID_NO_L3),bp);
#endif

        }
}


#else /* not XOBBS */

#include <stdio.h>
#include <time.h>
#include <ctype.h>
#ifdef	UNIX
#ifdef	UNIX
#undef	toupper
#undef	tolower
#endif
#include <sys/types.h>
#endif
#include "global.h"
#include "mbuf.h"
#include "ax25.h"
#include "timer.h"
#include "iface.h"
#include "lapb.h"
#include "netrom.h"
#include "nr4.h"
#include "ax_mbx.h"
#include "cmdparse.h"
#ifdef	UNIX
#include <memory.h>
#include <string.h>
#endif

#ifdef	BSD
char *sprintf();
#endif

/*
#define MBDEBUG
*/

struct mbx *mbox[NUMMBX] ;
int ax25mbox ;

static char mbbanner[] =
	"[NET-$]\rWelcome to the %s TCP/IP Mailbox\r(C)hat, (S)end, (B)ye >\r" ;
static char mbmenu[] = "(C)hat, (S)end, (B)ye >\r" ;

dombox(argc, argv)
int argc ;
char *argv[] ;
{
	if (argc < 2) {
		domboxdisplay() ;
		return 0 ;
	}

	if (argv[1][0] == 'y' || (strcmp(argv[1],"on") == 0))
		ax25mbox = 1 ;
	else if (argv[1][0] == 'n' || (strcmp(argv[1],"off") == 0))
		ax25mbox = 0 ;
	else if (argv[1][0] == '?')
		printf("ax25 mailbox is %s\n", ax25mbox ? "on" : "off") ;
	else
		printf("usage: mbox [y|n|?]\n") ;

	return 0 ;
}

static domboxdisplay()
{
	int i ;
	struct mbx *m ;
	static char *states[] = {"NONE","CMD","SUBJ","DATA"} ;
	static char *mbtype[] = {"NONE","AX25 ","NET/ROM"} ;
	
	printf(" User     State    Type    &cb\n") ;

	for (i = 0 ; i < NUMMBX ; i++)
		if ((m = mbox[i]) != NULLMBX)
			printf("%10s %4s  %7s   %04x\n", m->name, states[m->state],
					mbtype[m->type],
					m->type == MBX_AX25 ? (int)m->cb.ax25_cb : (int)m->cb.nr4_cb) ;
}

	
static struct mbx *
newmbx()
{
	int i ;
	struct mbx *m ;

	for (i = 0 ; i < NUMMBX ; i++)
		if (mbox[i] == NULLMBX) {
			if ((m = mbox[i] = (struct mbx *)calloc(1,sizeof(struct mbx)))
				== NULLMBX)
				return NULLMBX ;
			m->mbnum = i ;
			return m ;
		}

	/* If we get here, there are no free mailbox sessions */

	return NULLMBX ;
}


/* Incoming mailbox session via ax.25 */

void
mbx_incom(axp,cnt)
register struct ax25_cb *axp ;
int16 cnt ;
{
	struct mbx *m ;
	struct mbuf *bp, *recv_ax25() ;
	char *cp ;
	extern char hostname[] ;
	void mbx_rx(), mbx_state() ;
	extern char *index() ;
	
	if ((m = newmbx()) == NULLMBX) {
		disc_ax25(axp) ;	/* no memory! */
		return ;
	}

	m->state = MBX_CMD ;	/* start in command state */
	m->type = MBX_AX25 ;	/* this is an ax.25 mailbox session */
	m->cb.ax25_cb = axp ;

	pax25(m->name,&axp->addr.dest) ;
	cp = index(m->name,'-') ;
	if (cp != NULLCHAR)			/* get rid of SSID */
		*cp = '\0' ;

	m->lp = m->line ;		/* point line pointer at buffer */
	axp->r_upcall = mbx_rx ;
	axp->s_upcall = mbx_state ;
	axp->user = (char *)m ;

	/* The following is necessary because we didn't know we had a */
	/* "real" ax25 connection until a data packet came in.  We    */
	/* can't be spitting banners out at every station who connects, */
	/* since they might be a net/rom or IP station.  Sorry.  */
	
	bp = recv_ax25(axp,cnt) ;		/* get the initial input */
	free_p(bp) ;					/* and throw it away to avoid confusion */

	/* Now say hi */
	
	if ((bp = alloc_mbuf((int16)(strlen(hostname) + strlen(mbbanner) + 2))) == NULLBUF) {
		disc_ax25(axp) ; /* mbx_state will fix stuff up */
		return ;
	}

	*bp->data = PID_FIRST | PID_LAST | PID_NO_L3 ;	/* pid */
	(void)sprintf(bp->data+1,mbbanner,hostname) ;
	bp->cnt = strlen(bp->data+1) + 1 ;

	send_ax25(axp,bp) ;					/* send greeting message and menu */
}

/* receive upcall for ax.25 */
/* mbx_rx collects lines, and calls mbx_line when they are complete. */
/* If the lines get too long, it arbitrarily breaks them. */

void mbx_rx(axp,cnt)
struct ax25_cb *axp ;
int16 cnt ;
{
	struct mbuf *bp, *recv_ax25() ;
	struct mbx *m ;
	char c ;
	int mbx_line() ;
	
	m = (struct mbx *)axp->user ;
	
	if ((bp = recv_ax25(axp,cnt)) == NULLBUF)
		return ;

	while (pullup(&bp,&c,1) == 1) {
		if (c == '\r') {
			*m->lp = '\0' ;			/* null terminate */
			if (mbx_line(m) == -1) {	/* call the line processor */
				free_p(bp) ;		/* toss the rest */
				break ;				/* get out - we're obsolete */
			}
			m->lp = m->line ;		/* reset the pointer */
		}
		else if ((m->lp - m->line) == (MBXLINE - 1)) {
			*m->lp++ = c ;
			*m->lp = '\0' ;
			if (mbx_line(m) == -1) {
				free_p(bp) ;
				break ;
			}
			m->lp = m->line ;
		}
		else
			*m->lp++ = c ;
	}
}

/* state upcall for ax.25 */

/*ARGSUSED*/
void mbx_state(axp,old,new)
struct ax25_cb *axp ;
int old, new ;
{
	struct mbx *m ;
	void free_mbx() ;
	
	m = (struct mbx *)axp->user ;

	/* dummy for now ... */
	if (new == DISCONNECTED) {
		axp->user = NULLCHAR ;
		free_mbx(m) ;
	}
}


/* Incoming mailbox session via net/rom */

void
mbx_nr4incom(cb)
register struct nr4cb *cb ;
{
	struct mbx *m ;
	struct mbuf *bp ;
	char *cp ;
	extern char hostname[] ;
	void mbx_nr4rx(), mbx_nr4state() ;
	extern char *index() ;
	
	if ((m = newmbx()) == NULLMBX) {
		disc_nr4(cb) ;	/* no memory! */
		return ;
	}

	m->state = MBX_CMD ;	/* start in command state */
	m->type = MBX_NETROM ;	/* mailbox session type is net/rom */
	m->cb.nr4_cb = cb ;

	pax25(m->name,&cb->user) ;
	cp = index(m->name,'-') ;
	if (cp != NULLCHAR)			/* get rid of SSID */
		*cp = '\0' ;

	m->lp = m->line ;		/* point line pointer at buffer */
	cb->r_upcall = mbx_nr4rx ;
	cb->s_upcall = mbx_nr4state ;
	cb->puser = (char *)m ;

	/* Say hi */
	
	if ((bp = alloc_mbuf((int16)(strlen(hostname) + strlen(mbbanner) + 1))) == NULLBUF) {
		disc_nr4(cb) ; /* mbx_nr4state will fix stuff up */
		return ;
	}

	(void)sprintf(bp->data,mbbanner,hostname) ;
	bp->cnt = strlen(bp->data) ;

	send_nr4(cb,bp) ;					/* send greeting message and menu */
}

/* receive upcall for net/rom */
/* mbx_nr4rx collects lines, and calls mbx_line when they are complete. */
/* If the lines get too long, it arbitrarily breaks them. */

void mbx_nr4rx(cb,cnt)
struct nr4cb *cb ;
int16 cnt ;
{
	struct mbuf *bp ;
	struct mbx *m ;
	char c ;
	int mbx_line() ;
	
	m = (struct mbx *)cb->puser ;
	
	if ((bp = recv_nr4(cb,cnt)) == NULLBUF)
		return ;

	while (pullup(&bp,&c,1) == 1) {
		if (c == '\r') {
			*m->lp = '\0' ;			/* null terminate */
			if (mbx_line(m) == -1) {	/* call the line processor */
				free_p(bp) ;		/* toss the rest */
				break ;				/* get out - we're obsolete */
			}
			m->lp = m->line ;		/* reset the pointer */
		}
		else if ((m->lp - m->line) == (MBXLINE - 1)) {
			*m->lp++ = c ;
			*m->lp = '\0' ;
			if (mbx_line(m) == -1) {
				free_p(bp) ;
				break ;
			}
			m->lp = m->line ;
		}
		else
			*m->lp++ = c ;
	}
}

/* state upcall for net/rom */

/*ARGSUSED*/
void mbx_nr4state(cb,old,new)
struct nr4cb *cb ;
int old, new ;
{
	struct mbx *m ;
	void free_mbx() ;
	
	m = (struct mbx *)cb->puser ;

	if (new == NR4STDISC) {
		cb->puser = NULLCHAR ;
		free_mbx(m) ;
	}
}

static void
free_mbx(m)
struct mbx *m ;
{
	if (m->to != NULLCHAR)
		free(m->to) ;


	if (m->tofrom != NULLCHAR)
		free(m->tofrom) ;

	if (m->tomsgid != NULLCHAR)
		free(m->tomsgid) ;
		
	if (m->tfile != NULLFILE)
		fclose(m->tfile) ;

	mbox[m->mbnum] = NULLMBX ;
	
	free(m) ;
}


static 
mbx_line(m)
struct mbx *m ;
{
	void ax_session(), nr4_session(), mbx_disc() ;
	char *host ;
	extern char hostname[] ;
	char fullfrom[80] ;
	
	if (m->state == MBX_CMD) {
		switch (tolower(m->line[0])) {
			case 'b':	/* bye - bye */
				mbx_disc(m) ;
				return -1 ;	/* tell line processor to quit */
				break ;
			case 'c':	/* chat */
				switch (m->type) {
				  case MBX_AX25:
					m->cb.ax25_cb->user = NULLCHAR ;
					ax_session(m->cb.ax25_cb,0) ;	/* make it a chat session */
					break ;
				  case MBX_NETROM:
				    m->cb.nr4_cb->puser = NULLCHAR ;
					nr4_session(m->cb.nr4_cb) ;
					break ;
				}
				free_mbx(m) ;
				return -1 ;
				break ;
			case 's': {
				int badsubj = 0 ;

				/* Get S-command type (B,P,T, etc.) */
				
				if (m->line[1] == '\0')
					m->stype = ' ' ;
				else
					m->stype = toupper(m->line[1]) ;
					
				if (mbx_to(m) == -1) {
					if (m->sid & MBX_SID)
						mbx_msg(m,"NO\r") ;
					else {
						mbx_msg(m,
							"S command syntax error - format is:\r") ;
						mbx_msg(m,
						  "  S name [@ host] [< from_addr] [$bulletin_id]\r") ;
					}
					badsubj++ ;
				}
				else if (validate_address(m->to) == 0)	 {
					if (m->sid & MBX_SID)
						mbx_msg(m, "NO\r") ;
					else
						mbx_msg(m, "Bad user or host name\r") ;
					free(m->to) ;
					m->to = NULLCHAR ;
					if (m->tofrom) {
						free(m->tofrom) ;
						m->tofrom = NULLCHAR ;
					}
					if (m->tomsgid) {
						free(m->tomsgid) ;
						m->tomsgid = NULLCHAR ;
					}
					badsubj++ ;
				}

				if (badsubj)
					mbx_msg(m, (m->sid & MBX_SID) ? ">\r" : mbmenu) ;
				else {
					m->state = MBX_SUBJ ;
					mbx_msg(m,	(m->sid & MBX_SID) ? "OK\r" : "Subject:\r") ;
				}
				break ;
			}
			case '[':	/* This is a BBS - say "OK", not "Subject:" */
			  {
				int len = strlen(m->line) ;
				
				if (m->line[len - 1] == ']') { /* must be an SID */
					m->sid = MBX_SID ;
					/* Now check to see if this is an RLI board. */
					/* As usual, Hank does it a bit differently from */
					/* the rest of the world. */
					if (len >= 5)		/* [RLI] at a minimum */
						if (strncmp(&m->line[1],"RLI",3) == 0)
							m->sid |= MBX_SID_RLI ;
					
					mbx_msg(m,">\r") ;
				}
			  }
			  break ;
			case 'f':
				if (m->line[1] == '>' && (m->sid & MBX_SID)) {
					/* RLI BBS' expect us to disconnect if we */
					/* have no mail for them, which of course */
					/* we don't, being rather haughty about our */
					/* protocol superiority. */
					if (m->sid & MBX_SID_RLI) {
						mbx_disc(m) ;
						return -1 ;
					} else
						mbx_msg(m,">\r") ;
					break ;
				}
				/* Otherwise drop through to "huh?" */
			default:
				mbx_msg(m,"Huh?\r") ;
				mbx_msg(m, (m->sid & MBX_SID) ? ">\r" : mbmenu) ;
		}
	return 0 ;
	}
	else if (m->state == MBX_SUBJ) {
		if (mbx_data(m) == -1) {
			mbx_msg(m,"Can't create temp file for mail\r") ;
			mbx_msg(m, (m->sid & MBX_SID) ? ">\r" : mbmenu) ;
			free(m->to) ;
			m->to = NULLCHAR ;
			if (m->tofrom) {
				free(m->tofrom) ;
				m->tofrom = NULLCHAR ;
			}
			if (m->tomsgid) {
				free(m->tomsgid) ;
				m->tomsgid = NULLCHAR ;
			}
			m->state = MBX_CMD ;
			return 0 ;
		}
		m->state = MBX_DATA ;
		if ((m->sid & MBX_SID) == 0)
			mbx_msg(m,
			  "Enter message.  Terminate with /EX or ^Z in first column:\r") ;
		return 0 ;
	}
	else if (m->state == MBX_DATA) {
		if (m->line[0] == 0x1a ||
			strcmp(m->line, "/ex") == 0 ||
			strcmp(m->line, "/EX") == 0) {
			if ((host = index(m->to,'@')) == NULLCHAR)
				host = hostname ;		/* use our hostname */
			else
				host++ ;				/* use the host part of address */

			/* make up full from name for work file */
			(void)sprintf(fullfrom,"%s@%s",m->name,hostname) ;
			
			fseek(m->tfile,0L,0) ;		/* reset to beginning */
			if (queuejob((struct tcb *)0,m->tfile,host,m->to,fullfrom) != 0)
				mbx_msg(m,
						"Couldn't queue message for delivery\r") ;

			free(m->to) ;
			m->to = NULLCHAR ;
			if (m->tofrom) {
				free(m->tofrom) ;
				m->tofrom = NULLCHAR ;
			}
			if (m->tomsgid) {
				free(m->tomsgid) ;
				m->tomsgid = NULLCHAR ;
			}
			fclose(m->tfile) ;
			m->tfile = NULLFILE ;
			m->state = MBX_CMD ;
			mbx_msg(m, (m->sid & MBX_SID) ? ">\r" : mbmenu) ;
			return 0 ;
		}
		/* not done yet! */
		fprintf(m->tfile,"%s\n",m->line) ;
		return 0 ;
	}
	return 0 ;
}

static
mbx_msg(m,msg)
struct mbx *m ;
char msg[] ;
{
	int len ;
	struct mbuf *bp ;
	struct ax25_cb *axp ;
	struct nr4cb *cb ;

	len = strlen(msg) ;

	switch (m->type) {
	  case MBX_AX25:
	    axp = m->cb.ax25_cb ;
			
		if ((bp = alloc_mbuf((int16)(len+1))) == NULLBUF) {
			disc_ax25(axp) ;
			return -1 ;
		}

		bp->cnt = len + 1 ;
	
		*bp->data = PID_FIRST | PID_LAST | PID_NO_L3 ;
	
		memcpy(bp->data+1, msg, len) ;

		send_ax25(axp,bp) ;

		break ;

	  case MBX_NETROM:
	    cb = m->cb.nr4_cb ;

		if ((bp = alloc_mbuf((int16)len)) == NULLBUF) {
			disc_nr4(cb) ;
			return -1 ;
		}

		bp->cnt = len ;

		memcpy(bp->data, msg, len) ;

		send_nr4(cb, bp) ;

		break ;
	}
	return 0 ;
}


/* States for send line parser state machine */

#define		SKIP_CMD		1
#define		LOOK_FOR_USER	2
#define		IN_USER			3
#define		AFTER_USER		4
#define		LOOK_FOR_HOST	5
#define		IN_HOST			6
#define		AFTER_HOST		7
#define		LOOK_FOR_FROM	8
#define		IN_FROM			9
#define		AFTER_FROM		10
#define		LOOK_FOR_MSGID	11
#define		IN_MSGID		12
#define		FINAL_STATE		13
#define		ERROR_STATE		14

/* Prepare the addressee.  If the address is bad, return -1, otherwise
 * return 0
 */
static
mbx_to(m)
struct mbx *m ;
{
	register char *cp ;
	int state ;
	char *user, *host, *from, *msgid ;
	int userlen = 0, hostlen = 0, fromlen = 0, msgidlen = 0 ;
	
	cp = m->line ;

	for (state = SKIP_CMD ; state < FINAL_STATE ; cp++) {
#ifdef MBDEBUG
		printf("State is %d, char is %c\n", state, *cp) ;
#endif
		switch (state) {
			case SKIP_CMD:
				if (*cp == '\0')
					state = ERROR_STATE ;		/* no user */
				else if (isspace(*cp))
					state = LOOK_FOR_USER ;
				break ;
			case LOOK_FOR_USER:
				if (*cp == '\0' || *cp == '@' || *cp == '<' || *cp == '$')
					state = ERROR_STATE ;		/* no user */
				else if (!isspace(*cp)) {		/* found start of user */
					user = cp ;					/* point at start */
					userlen++ ;					/* start counting */
					state = IN_USER ;
				}
				break ;
			case IN_USER:
				switch (*cp) {
					case '\0':					/* found username only */
						state = FINAL_STATE ;
						break ;
					case '@':
						state = LOOK_FOR_HOST ;	/* hostname should follow */
						break ;
					case '<':
						state = LOOK_FOR_FROM ;	/* from name should follow */
						break ;
					case '$':
						state = LOOK_FOR_MSGID ; /* message id should follow */
						break ;
					default:
						if (isspace(*cp))
							state = AFTER_USER ;	/* white space */
						else
							userlen++ ;				/* part of username */
				}
				break ;
			case AFTER_USER:
				switch (*cp) {
					case '\0':
						state = FINAL_STATE ;		/* found username only */
						break ;
					case '@':
						state = LOOK_FOR_HOST ;		/* hostname follows */
						break ;
					case '<':
						state = LOOK_FOR_FROM ;		/* fromname follows */
						break ;
					case '$':
						state = LOOK_FOR_MSGID ;	/* message id follows */
						break ;
					default:
						if (!isspace(*cp))
							state = ERROR_STATE ;
				}
				break ;
			case LOOK_FOR_HOST:
				switch (*cp) {
					case '\0':					/* user@? */
					case '@':					/* user@@ */
					case '<':					/* user@< */
					case '$':					/* user@$ */
						state = ERROR_STATE ;
						break ;
					default:
						if (!isspace(*cp)) {
							host = cp ;
							hostlen++ ;
							state = IN_HOST ;
						}
				}
				break ;
			case IN_HOST:
				switch (*cp) {
					case '\0':
						state = FINAL_STATE ;		/* found user@host */
						break ;
					case '@':
						state = ERROR_STATE ;		/* user@host@? */
						break ;
					case '<':
						state = LOOK_FOR_FROM ;		/* fromname follows */
						break ;
					case '$':
						state = LOOK_FOR_MSGID ;	/* message id follows */
						break ;
					default:
						if (isspace(*cp))
							state = AFTER_HOST ;
						else
							hostlen++ ;
				}
				break ;
			case AFTER_HOST:
				switch (*cp) {
					case '\0':
						state = FINAL_STATE ;		/* user@host */
						break ;
					case '@':
						state = ERROR_STATE ;		/* user@host @ */
						break ;
					case '<':
						state = LOOK_FOR_FROM ;		/* user@host < */
						break ;
					case '$':
						state = LOOK_FOR_MSGID ;	/* user@host $ */
						break ;
					default:
						if (!isspace(*cp))
							state = ERROR_STATE ;	/* user@host foo */
				}
				break ;
			case LOOK_FOR_FROM:
				switch (*cp) {
					case '\0':					/* user@host <? */
					case '@':					/* user@host <@ */
					case '<':					/* user@host << */
					case '$':					/* user@host <$ */
						state = ERROR_STATE ;
						break ;
					default:
						if (!isspace(*cp)) {
							from = cp ;
							fromlen++ ;
							state = IN_FROM ;
						}
				}
				break ;
			case IN_FROM:
				switch (*cp) {
					case '\0':
						state = FINAL_STATE ;		/* user@host <foo */
						break ;
					case '<':
						state = ERROR_STATE ;		/* user@host <foo< */
						break ;
					case '$':
						state = LOOK_FOR_MSGID ;	/* message id follows */
						break ;
					default:
						if (isspace(*cp))
							state = AFTER_FROM ;
						else
							fromlen++ ;
				}
				break ;
			case AFTER_FROM:
				switch (*cp) {
					case '\0':
						state = FINAL_STATE ;		/* user@host <foo */
						break ;
					case '@':						/* user@host <foo @ */
					case '<':						/* user@host <foo < */
						state = ERROR_STATE ;
						break ;
					case '$':
						state = LOOK_FOR_MSGID ;	/* user@host <foo $ */
						break ;
					default:
						if (!isspace(*cp))
							state = ERROR_STATE ;	/* user@host foo */
				}
				break ;
			case LOOK_FOR_MSGID:
				if (*cp == '\0')
					state = ERROR_STATE ;			/* msgid = $? */
				else if (isspace(*cp))
					state = ERROR_STATE ;			/* user@host <foo $ bar */
				else {
					msgid = cp ;
					msgidlen++ ;
					state = IN_MSGID ;
				}
				break ;
			case IN_MSGID:
				if (*cp == '\0')
					state = FINAL_STATE ;
				else if (isspace(*cp))
					state = FINAL_STATE ;
				else
					msgidlen++ ;
				break ;
			default:
				/* what are we doing in this state? */
				state = ERROR_STATE ;
		}
	}

	if (state == ERROR_STATE)
		return -1 ;		/* syntax error */

	if ((m->to = malloc((unsigned)(userlen + hostlen + 2))) == NULLCHAR)
		return -1 ;		/* no room for to address */

	strncpy(m->to, user, userlen) ;
	m->to[userlen] = '\0' ;
	
	if (hostlen) {
		m->to[userlen] = '@' ;
		strncpy(m->to + userlen + 1, host, hostlen) ;
		m->to[userlen + hostlen + 1] = '\0' ;
	}

	if (fromlen) {
		if ((m->tofrom = malloc((unsigned)(fromlen + 1))) == NULLCHAR) {
			free(m->to) ;
			m->to = NULLCHAR ;
			return -1 ;
		}
		strncpy(m->tofrom, from, fromlen) ;
		m->tofrom[fromlen] = '\0' ;
	}

	if (msgidlen) {
		if ((m->tomsgid = malloc((unsigned)(msgidlen + 1))) == NULLCHAR) {
			free(m->to) ;
			m->to = NULLCHAR ;
			if (fromlen) {
				free(m->tofrom) ;
				m->tofrom = NULLCHAR ;
			}
			return -1 ;
		}
		strncpy(m->tomsgid, msgid, msgidlen) ;
		m->tomsgid[msgidlen] = '\0' ;
	}
	
	return 0 ;
}

/* This opens the data file and writes the mail header into it.
 * Returns 0 if OK, and -1 if not.
 */

static
mbx_data(m)
struct mbx *m ;
{
	time_t t, time() ;
	char *ptime() ;
	extern char hostname[] ;
	extern FILE *tmpfile();
	extern long get_msgid() ;
	
	if ((m->tfile = tmpfile()) == NULLFILE)
		return -1 ;

	time(&t) ;
	fprintf(m->tfile,"Date: %s",ptime(&t)) ;
	if (m->tomsgid)
		fprintf(m->tfile, "Message-Id: <%s@%s>\n", m->tomsgid, hostname) ;
	else
		fprintf(m->tfile,"Message-Id: <%ld@%s>\n",get_msgid(),hostname) ;
	fprintf(m->tfile,"From: %s%%%s.bbs@%s\n",
			m->tofrom ? m->tofrom : m->name, m->name, hostname) ;
	fprintf(m->tfile,"To: %s\n",m->to) ;
	fprintf(m->tfile,"Subject: %s\n",m->line) ;
	if (m->stype != ' ')
		fprintf(m->tfile,"X-BBS-Msg-Type: %c\n", m->stype) ;
	fprintf(m->tfile,"\n") ;
	
	return 0 ;
}

/* Shut down the mailbox communications connection */

static void
mbx_disc(m)
struct mbx *m ;
{
	switch (m->type) {
	  case MBX_AX25:
		disc_ax25(m->cb.ax25_cb) ;
		break ;
	  case MBX_NETROM:
	  	disc_nr4(m->cb.nr4_cb) ;
		break ;
	}
}
#endif /* XOBBS */
