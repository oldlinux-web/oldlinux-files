#include "config.h"
#ifdef MULPORT
#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "iface.h"
#include "ax25.h"
#include <ctype.h>
#ifdef	UNIX
#include <string.h>
#endif

/**************************************************************************
* The struct mulrep holds calls and interface names for multiport         *
* repeating                                                               *
* port[] holds the interface name passed back to hack in ax25.c           *
**************************************************************************/
typedef struct {
     struct    ax25_addr dcall;    /* digipeater callsign */
     char port[4];            /* port name */
} mulrep;


#define TRUE 1
#define FALSE 0



/****************************************************************************
* Multiport system globals                                                  *
* mport is the multiport flag initialized to false until mulport is called. *
****************************************************************************/
int       mport = FALSE;
mulrep    destrpt[10], extbl[10], lan;

/*************************************************************************
* mulport fills the array of structures of digi calls to which packets   *
* may  be repeated and sets the global flag mport TRUE. It reads from    *
* the file DIGILIST which must be in the ROOT directory and is formatted *
* <callsign+ssid> <interface_name>                                       *
* example:                                                               *
* KE4ZV-1 ax0                                                            *
* KD4NC-1 ax1                                                            *
* mulport also builds an array of destination calls from the file EXLIST *
* in the root directory for destinations not fitting the general rules.  *
* This is the function called when the command "mulport on" is given.    *
*************************************************************************/
/*ARGSUSED*/
int mulport(argc,argv)
int argc;
char *argv[];
{
 FILE *fp, *fopen();
 int       x;
 char tcall[12], tport[4];
 char str[257];
 int tmp;

 if (strncmp(argv[1],"on",2)==0){
      if ((fp = fopen("/digilist","r")) == 0) {
          printf("Could not open file DIGILIST\n");
          return(FALSE);
      }
      setcall(&(lan.dcall),"lan");
      strcpy(lan.port,"");
      printf("\nDigi Call          Interface\n");
      for (x=0; (x <= 9) && (fgets(str, 256, fp)); x++) {
          /* make sure we clear out the whole line to the \n   */
          /* otherwise, any other  whitespace after the port  */
          /* designator will fubar the parser                 */
          tmp = sscanf(str, "%s %s", tcall, tport);
          if (tmp!= 2) {    /* make sure we don't do a partial entry */
            printf("Invalid record in DIGILIST encountered, record ignored\n");
            continue;
          }
          printf("%9.9s       %6.6s\n",tcall,tport);

/*  we really need to verify that the port  exists before we put it into the
    table or direct a stream to it. Current default behavior is to send the
    packet back out the port it came in on if there is no match.
*/
          setcall(&(destrpt[x].dcall),tcall);
          strcpy(destrpt[x].port,tport);
#ifdef MULBUG
          printf("%9.9s      %6.6s\n",destrpt[x].dcall.call,destrpt[x].port);
#endif
     }
      fclose(fp);
/*finish out the array with null entries */
      strcpy(tcall,'\0');
      strcpy(tport,'\0');
      for(;x<=9;x++){
          setcall(&(destrpt[x].dcall),tcall);
          strcpy(destrpt[x].port,tport);
      }
      /*****************************************************************
      *    This code builds the exception list extbl[]                 *
      *    This list handles destination calls who need behavior that  *
      *    does not follow the mulport rules ie: a user station on the *
      *    high speed trunk.                                           *
      *****************************************************************/
      if ((fp = fopen("/exlist","r")) == 0) {
          printf("Could not open file EXLIST\n");
          return(FALSE);
      }
      printf("\nException Call          Interface\n");
      for (x=0; (x <= 9) && (fgets(str, 256, fp)); x++) {
          tmp = sscanf(str, "%s %s", tcall, tport);
          if (tmp!= 2) {    /* make sure we don't do a partial entry */
            printf("Invalid record in EXLIST encountered, record ignored\n");
            continue;
          }
          printf("%9.9s       %6.6s\n",tcall,tport);
          setcall(&(extbl[x].dcall),tcall);
          strcpy(extbl[x].port,tport);
#ifdef MULBUG
          printf("%9.9s      %6.6s\n",extbl[x].dcall.call,extbl[x].port);
#endif
     }
      fclose(fp);
/*finish out the array with null entries */
      strcpy(tcall,'\0');
      strcpy(tport,'\0');
      for(;x<=9;x++){
          setcall(&(destrpt[x].dcall),tcall);
          strcpy(destrpt[x].port,tport);
      }
      mport = TRUE;
      return(TRUE);
 } else {
     mport = FALSE;
     return(FALSE);
 }

}

/**************************************************************************
* Here is the repeater hack called from ax_recv in module ax25.c.         *
* Repeater searches the array of digi calls created by mulport for the    *
* digi call following ours in the packet header. If a match occurs the    *
* corresponding interface name is found by comparing the string in        *
* port[] to the names assigned to interfaces with the attach command.     *
* A pointer to the interface is returned by repeater. Default behavior on *
* match failure is to return a pointer to the interface the packet came   *
* in on. If our call is the last digi call in the header, the destination *
* call is compared to the array of exception calls and, if a match, the   *
* corresponding interface is returned by repeater. Default behavior on    *
* match failure is to return a pointer to the interface referenced by the *
* pseudo call "lan" in digilist.                                          *
* If no matches at all are found, default behavior is to return a pointer *
* to the interface the packet came in on.                                 *
**************************************************************************/
struct interface *
repeater(ap,interface,hdr)
     struct ax25_addr *ap;
     struct interface *interface;
     struct ax25 *hdr;

{
     struct interface *intport;
     int x, flg, match;

 if (++ap < &hdr->digis[hdr->ndigis]){
#ifdef MULBUG
                  printf("mport && there is a call after ours\n");
#endif
                  for (x=0, flg=0; (!flg) && (x <= 9) && (strcmp('\0',destrpt[x].dcall.call)!=0); x++){  /* 10 mports max */
#ifdef MULBUG
                    printf("stepping thru destrpt at %d %9.9s   %9.9s\n",x,ap->call,&(destrpt[x].dcall.call));
#endif
                      if (addreq(ap,&(destrpt[x].dcall))){
#ifdef MULBUG
                         printf("dcall match\n");
#endif
                         for (intport=ifaces; !flg && (intport!=NULLIF); intport=intport->next){
#ifdef MULBUG
                             printf("stepping thru interfaces %9.9s  %9.9s\n",intport->name,&(destrpt[x].port));
#endif
                             if (strcmp(intport->name,destrpt[x].port)==0){
#ifdef MULBUG
                                printf("interface match on %s\n",&(destrpt[x].port));
#endif
                                interface=intport;
                                flg = 1;
                             }
                         }
                      }
                  }
                  ap--;
               }
               else{
                   for (match=0,x=0, flg=0; (!flg) && (x <= 9) && (strcmp('\0',extbl[x].dcall.call)!=0); x++){  /* 10 mports max */
#ifdef MULBUG
                       printf("scanning for hdr.dest %9.9s  %9.9s\n",&(hdr->dest.call),&(extbl[x].dcall.call));
#endif
                       if (addreq(&(hdr->dest),&(extbl[x].dcall))){
#ifdef MULBUG
                          printf("dest match\n");
#endif
                          for (intport=ifaces; !flg && (intport!=NULLIF); intport=intport->next){
#ifdef MULBUG
                              printf("stepping thru interfaces %9.9s  %9.9s\n",intport->name,&(extbl[x].port));
#endif
                              if (strcmp(intport->name,extbl[x].port)==0){
#ifdef MULBUG
                                 printf("interface match\n");
#endif
                                 interface=intport;
                                 flg = 1;
                                 match=1;
                              }
                          }
                      }
                    }
                    if (match!=1){
                       for (x=0, flg=0; (!flg) && (x <= 9) && (strcmp('\0',destrpt[x].dcall.call)!=0); x++){  /* 10 mports max */
#ifdef MULBUG
                            printf("scanning for lan.dcall %9.9s  %9.9s\n",&(lan.dcall.call),&(destrpt[x].dcall.call));
#endif
                            if (addreq(&(lan.dcall),&(destrpt[x].dcall))){
#ifdef MULBUG
                                 printf("dcall match\n");
#endif
                                 for (intport=ifaces; !flg && (intport!=NULLIF); intport=intport->next){
#ifdef MULBUG
                                      printf("stepping thru interfaces %9.9s  %9.9s\n",intport->name,&(destrpt[x].port));
#endif
                                      if (strcmp(intport->name,destrpt[x].port)==0){
#ifdef MULBUG
                                          printf("interface match\n");
#endif
                                          interface=intport;
                                          flg = 1;
                                      }
                                   }
                             }
                         }
                      }
                      ap--;
                  }
return(interface);
}
#endif

