/*
  ST_UTILS.C
  ST-specific routines for ORIGAMI / Turbo C
  (C) 1991 M. Schwingen
*/

#include <stdio.h>
#include <stdlib.h>
#include <ext.h>
#include <string.h>
#include <tos.h>
#include <vdi.h>
#include <aes.h>
#include "ori_rsc.h"

#define ST_UTIL_C
#include "local\bool.h"
#include "..\origami\origami.h"

/*{{{  int access(char *file, int flag)*/
#define READONLY 0x01
#define HIDDEN   0x02
#define SYSTEM   0x04
#define VOLUME   0x08
#define SUBDIR   0x10
#define ARCHIVE  0x20
int access(char *file, int flag)
{
  int result;
 
  result = Fattrib(file,0,0);
  if (result < 0)
    return -1; /* file does not exist */
  if (result & (VOLUME | SUBDIR))
    return -1; /* exists, but is volume / subdir -> no access */
 
  if ((flag & W_OK) && (result & READONLY))
    return -1; /* no write access */
 
  return 0;
}
/*}}}  */
/*{{{  void putenv(char *string) - set environment variables*/
/*
   This implementation of putenv simply does a system("setenv ..."). This
   works well with MUPFEL, it should also work with other shells which stay
   resident in the background while executing ORIGAMI and which maintain one
   global environment for all sub-shells.
*/
void putenv(char *string)
{
  char buf[256];
  char *p=buf;
 
  /* build command: setenv var "value" from string var=value */
  strcpy(buf,"setenv ");
  while (*p)
  	p++;
  while (*string && *string != '=')
    *p++ = *string++;
  string++;
  *p++ = ' ';
  *p++ = '"';
  while (*string)
  	*p++ = *string++;
  *p++ = '"';
  *p = 0;
  system(buf);
}
/*}}}  */

/*{{{  extern GEM variables*/
extern int work_in[12],work_out[57],
           phys_handle,handle,ap_id,
           wind_handle,
           gl_hchar,gl_wchar,gl_hbox,gl_wbox,
           wind_x,wind_y,wind_w,wind_h,
           _x,_y;
/*}}}  */

/*{{{  extern ressource structures and init routine, defined in ORI_RSC.C*/
extern OBJECT *rs_trindex[];
void rsrc_init(void);
/*}}}  */
/*{{{  void st_redraw(void)*/
void st_redraw(void)
{
  menu_bar(rs_trindex[MENU],0);
  menu_bar(rs_trindex[MENU],1);
  graf_mouse(ARROW,NULL);
}
/*}}}  */

/*{{{  void pre_shell(void) - prepare shell system call*/
void pre_shell(void)
{
  int xy[4];
 
  menu_bar(rs_trindex[MENU],0); /* remove menu bar */
  /* it seems we have to close our window to avoid system hangups */
  wind_close(wind_handle);
  graf_mouse(M_ON,NULL);        /* display mouse */
  wind_update(END_UPDATE);      /* enable AES screen update */
  /*{{{  clear complete screen*/
  wind_get(0,WF_WORKXYWH,&xy[0],&xy[1],&xy[2],&xy[3]);
  xy[2] += xy[0];
  xy[3] += xy[1];
  xy[0] = xy[1] = 0;
  vr_recfl(handle,xy);
  /*}}}  */
  /*{{{  initialize GEMDOS and BIOS cursor to upper left corner*/
  Cconws("\033H");
  Bconout(2,27);
  Bconout(2,'H');
  /*}}}  */
}
/*}}}  */
/*{{{  void post_shell(void) - actions after shell call*/
void post_shell(void)
{
  graf_mouse(M_OFF,NULL);       /* disable mouse */
  wind_update(BEG_UPDATE);      /* disable AES screen update */
  menu_bar(rs_trindex[MENU],1); /* re-draw menu bar */
  /* re-open window (closed in pre_shell) - window handle is still
     allocated! */
  wind_open(wind_handle,wind_x-1,wind_y-1,wind_w+2,wind_h+2);
}
/*}}}  */
int shellwait = TRUE;
/*{{{  void newsystem(char *command) - prepare & do system call*/
void newsystem(char *command)
{
  pre_shell();
  system(command);
  if (shellwait)
  {
    Cconws("[END]");
    Cconin();
  }
  shellwait = TRUE;
  post_shell();
}
/*}}}  */

void newexit (int status)
{
  if (status != 0)
  {
    puts("\npress a key ...");
    Cconin();
  }
  exit(status);
}
 
/*{{{  void st_exit(void)*/
void st_exit(void)
{
  menu_bar(rs_trindex[MENU],0);
  if (wind_handle >=0)
  /*{{{  close & delete window*/
  {
    wind_close(wind_handle);
    wind_delete(wind_handle);
  }
  /*}}}  */
  wind_update(END_UPDATE);
  graf_mouse(M_ON,NULL);
  v_clsvwk(handle);
  appl_exit();
}
/*}}}  */
/*{{{  void st_init(void)*/
void st_init(void)
{
  register int i;
  int xyarray[4];
 
  ap_id = appl_init();
  if (ap_id <0)
  {
    Cconws("\nGEM error at appl_init()\nprogram aborted.\n");
    exit(1);
  }
  handle = work_in[0] = graf_handle(&gl_wchar,&gl_hchar,&gl_wbox,&gl_hbox);
  for (i = 1; i < 10; work_in[i++] = 1 )
    ;
  work_in[10] = 2; /* 2 = RC */
  v_opnvwk( work_in, &handle, work_out );
  /*{{{  if handle == 0: complain & exit*/
  if (handle == 0)
  {
    form_alert(1,"[1][error at v_opnvwk!][OK]");
    appl_exit();
    exit(1);
  }
  /*}}}  */
  atexit(st_exit);
  /*{{{  initialize VDI parameters: writemode, clip etc.*/
  vswr_mode(handle,MD_REPLACE);
  vst_color(handle,1);
  vst_effects(handle,0);
  vsf_color(handle,0);
  vsf_interior(handle,FIS_SOLID);
  vsf_perimeter(handle,0);
  vsl_color(handle,1);
  vsl_type(handle,1);
  vsl_width(handle,1);
  vsl_ends(handle,0,0);
  xyarray[0] = xyarray[1] = 0;
  xyarray[2] = work_out[0];
  xyarray[3] = work_out[1];
  vs_clip(handle,1,xyarray);
  vst_alignment(handle,0,5,xyarray,xyarray);
  /*}}}  */
  /* get size of background window */
  wind_get(0,WF_WORKXYWH,&wind_x,&wind_y,&wind_w,&wind_h);
  /* initialize ressource */
  rsrc_init();
  menu_bar(rs_trindex[MENU],1);
  graf_mouse(ARROW,NULL);
  graf_mouse(M_OFF,NULL);
  wind_update(BEG_UPDATE);
  /* open window, size = full screen, border is invisible (out of screen)
     this is only necessary to get redraw messages */
  if ((wind_handle = wind_create(0,wind_x-1,wind_y-1,wind_w+2,wind_h+2)) <0)
  /*{{{  complain & exit*/
  {
    form_alert(1,"[1][Cannot open window.][OK]");
    exit(1);
  }
  /*}}}  */
  wind_open(wind_handle,wind_x-1,wind_y-1,wind_w+2,wind_h+2);
}
/*}}}  */

/*{{{  void fileselect(char *str)*/
void fileselect(char *str)
{
  char *p;
  int button;
  static char fs_name[20] = "";
  static char fs_path[256] = "";
 
  if (*fs_path == 0)
  {
    fs_path[0] = Dgetdrv() + 'A';
    fs_path[1] = ':';
    Dgetpath(fs_path+2,0);
    strcat(fs_path,"\\*.*");
  }
  graf_mouse(M_ON,NULL);
  wind_update(END_UPDATE);
  if (fsel_input(fs_path,fs_name,&button) == 0 || button==0)
  {
    graf_mouse(M_OFF,NULL);
    wind_update(BEG_UPDATE);
    *fs_path = 0;
    *fs_name = 0;
    *str = 0;
    return;
  }
  graf_mouse(M_OFF,NULL);
  wind_update(BEG_UPDATE);
  if (*fs_name == 0)
  {
    *str = 0;
    return;
  }
  strcpy(str,fs_path);
  p = str;
  while (*p) p++;
  while (*--p != '\\')
    ;
  *++p = 0;
  strcat(str,fs_name);
}
/*}}}  */

char * mktemp(char *str)
{
	static int tmpcnt = 0;
	char buf[10];
	char *p1,*p2;
	
	sprintf(buf,"%08d",tmpcnt++);
	p1 = buf;
	while (*p1)
		p1++;
	p2 = str;
	p1--;
	while (*p2)
	{
		if (*p2 == 'X')
			*p2 = *p1--;
		p2++;
	}
	return str;
}

char base_name[]=".origami";
/*{{{  fileprompt*/
#ifdef STD_C
char *fileprompt(char n[_POSIX_PATH_MAX+1])
#else
char *fileprompt(n) char n[_POSIX_PATH_MAX+1];
#endif
{
  readprompt(n,M_FILENAME,_POSIX_PATH_MAX);
  if (*n == '\0' && !aborted)
  	fileselect(n);
  return(n);
}
/*}}}  */
