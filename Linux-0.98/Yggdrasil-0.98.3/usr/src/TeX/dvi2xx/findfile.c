#include "config.h"
#include <string.h>
#include <stdio.h>
#ifndef vms
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <types.h>
#include <stat.h>
#endif
int stat();
char *path_segment();
extern bool    G_quiet;

bool
findfile(path,n,fontmag,name,AfterMakeTexPk)
char path[STRSIZE];  /* PIXEL path */
char n[STRSIZE];     /* name of font */
long fontmag;        /* magnification */
char name[STRSIZE];  /* full name of PXL file  (returned) */
bool AfterMakeTexPk;  /* was the routine called after MakeTeXPK */ 
{
    char local_path[STRSIZE];
#ifdef unix
    char MakePKCommand[STRSIZE];
    bool success;
#endif
    char *pathpt;
    struct stat s;
    int rc = -1;
    int resolution, i;

    resolution = (int)(fontmag/5.0 +0.5) ;

#ifdef vms
#ifdef USEPXL
    sprintf(name,"%s[%d]%s.pk;",path,resolution,n);
    if ((rc = stat(name,&s))!=0) {
      sprintf(name,"%s%s.%dpk;",path,n,resolution);
      if ((rc = stat(name,&s))!=0) {
        sprintf(name,"%s:[%d]%s.pk;",path,resolution,n);
        if ((rc = stat(name,&s))!=0) {
          sprintf(name,"%s:%s.%dpk;",path,n,resolution);
          if ((rc = stat(name,&s))!=0) {
            sprintf(name,"%s[%d]%s.pxl;",path,fontmag,n);
            if ((rc = stat(name,&s))!=0) {
              sprintf(name,"%s%s.%dpxl;",path,n,fontmag);
              if ((rc = stat(name,&s))!=0) {
                sprintf(name,"%s:[%d]%s.pxl;",path,fontmag,n);
                if ((rc = stat(name,&s))!=0) {
                  sprintf(name,"%s:%s.%dpxl;",path,n,fontmag);
                  rc = stat(name,&s);
                }
              }
            }
          }
        }
      }
    }
#else
    sprintf(name,"%s%s.%dgf;",path,n,resolution);
    if ((rc = stat(name,&s))!=0) {
      sprintf(name,"%s:%s.%dgf;",path,n,resolution);
      if ((rc = stat(name,&s))!=0) {
        sprintf(name,"%s%s.%ldgf;",path,n,fontmag);
        if ((rc = stat(name,&s))!=0) {
          sprintf(name,"%s:%s.%ldgf;",path,n,fontmag);
          rc = stat(name,&s);
        }
      }
    }
#endif
    if (rc==0) return(TRUE);
#else /* not vms */
    for(i=0; (pathpt=path_segment((bool)(i==0),path,local_path))!=NULL;i++) {
#ifdef USEPXL
       sprintf(name,"%s/dpi%d/%s.pk",pathpt,resolution,n);
       if ((rc = stat(name,&s))!=0) {
           sprintf(name,"%s/dpi%d/%s.pxl",pathpt,resolution,n);
           if ((rc = stat(name,&s))!=0) {
               sprintf(name,"%s/pxl%ld/%s.pk",pathpt,fontmag,n);
               if ((rc = stat(name,&s))!=0) {
                   sprintf(name,"%s/pxl%ld/%s.pxl",pathpt,fontmag,n);
                   if ((rc = stat(name,&s))!=0) {
#ifndef MSDOS
                      sprintf(name,"%s/%s.%dpk",pathpt,n,resolution);
                      if ((rc = stat(name,&s))!=0) {
                         sprintf(name,"%s/%s.%dpxl",pathpt,n,resolution);
                         if ((rc = stat(name,&s))!=0) {
#endif
                            sprintf(name,"%s/%s.%d",pathpt,n,resolution);
                            rc = stat(name,&s);
#ifndef MSDOS
                         }
                      }
#endif
                   }
               }
           }
       }
#else
       sprintf(name,"%s/%s.%dgf",pathpt,n,resolution);
       if ((rc = stat(name,&s))!=0) {
           sprintf(name,"%s/%s.%ldgf",pathpt,n,fontmag);
           rc = stat(name,&s);
       }
#endif
       if (rc==0) return(TRUE);
     };
#endif /* vms */

#ifdef FUTURE
    for(i=0; (pathpt=path_segment((bool)(i==0),VFPATH,local_path))!=NULL;i++) {
       sprintf(name,"%s/%s.vfm",pathpt,n);
       printf("searching virtual font <%s>\n",name);
       if (stat(name,&s) == 0) return(TRUE);
    }
#endif

#ifndef USEPXL
    /* return error message */
    sprintf(name,"font not found: <%s>/%s.<%d;%ld>gf",
                  path,n,resolution,fontmag);
#else
#ifdef unix
#ifdef MAKETEXPK
    if (!AfterMakeTexPk) {
      sprintf(MakePKCommand,"%s %s %d %d %.4f %s",MAKETEXPK,
	      n,resolution,RESOLUTION,(float)((float)resolution/RESOLUTION),MODE);
      if (!G_quiet) fprintf(stderr,"calling: %s\n",MakePKCommand);
      system(MakePKCommand);
      if (findfile(path,n,fontmag,name,TRUE)) return(TRUE);
    }
#endif
#endif
    sprintf(name,"font not found: <%s>/<dpi%d;pxl%ld>/%s.<pk;pxl>",
                  path,resolution,fontmag,n);
#endif

return(FALSE);
}

char *
path_segment(first,full_path,local_path)
bool first;
char *full_path, *local_path;
{
	static char *pppt;
	char *pathpt;

	if (first) pathpt = strcpy(local_path,full_path);
	else pathpt = pppt;
	if (pathpt != NULL) {
#ifdef unix
       	        pppt = strchr(pathpt , ':' );
#else
                pppt = strchr(pathpt , ';' );
#endif
                if (pppt != NULL) {
                   *pppt = '\0';
                   pppt++;
                   }
        }
return pathpt;
}



