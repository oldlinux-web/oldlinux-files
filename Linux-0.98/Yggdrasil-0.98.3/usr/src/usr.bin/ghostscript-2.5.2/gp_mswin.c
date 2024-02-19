/* Copyright (C) 1992 Aladdin Enterprises.  All rights reserved.
   Distributed by Free Software Foundation, Inc.

This file is part of Ghostscript.

Ghostscript is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.  Refer
to the Ghostscript General Public License for full details.

Everyone is granted permission to copy, modify and redistribute
Ghostscript, but only under the conditions described in the Ghostscript
General Public License.  A copy of this license is supposed to have been
given to you along with Ghostscript so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies.  */

/* gp_mswin.c */
/*
 * Microsoft Windows 3.n platform support for Ghostscript.
 * Original version by Russell Lang and Maurice Castro with help from
 * Programming Windows, 2nd Ed., Charles Petzold, Microsoft Press;
 * initially created from gp_dosfb.c and gp_itbc.c 5th June 1992.
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "gp_mswin.h"

#include "memory_.h"
#include "gx.h"
#include "gp.h"
#include "gpcheck.h"
#include "gserrors.h"
#include "gxdevice.h"

#include "dos_.h"
#include <fcntl.h>
#include <io.h>
#include <signal.h>
#include "string_.h"

/* Library routines not declared in a standard header */
extern char *getenv(P1(const char *));

/* ------ from gnuplot winmain.c plus new stuff ------ */


/* limits */
#define MAXSTR 255
#define BIGBLK 4096

/* Linked list of currently open window devices */
gx_device *gdev_win_open_list;

/* public handles */
HWND FAR hwndeasy;
HANDLE FAR phInstance;

const char FAR szAppName[] = "Ghostscript";
char win_prntmp[MAXSTR];	/* filename of PRN temporary file */

/* printer structure */
struct PrStr{
	char far *dev;
	char far *out;
	char far *drv;
	struct PrStr * far next;
	};

struct PrStr * far prlst = NULL;
struct PrStr * far defprlst = NULL;
static HANDLE FAR lib = 0;

int FAR PASCAL printselproc();
BOOL FAR PASCAL AbortProc();

/* EasyWin */
extern POINT _ScreenSize;
extern BOOL _KeyPressed();

int main(int argc, char *argv[], char *env[]);

int win_init = 0;	/* flag to know if gp_exit has been called */

/* our exit handler */
void win_exit(void)
{
	/* if we didn't exit through gs_exit() then do so now */
	if (win_init)
		gs_exit(0);

	fcloseall();
	DestroyWindow(hwndeasy);
}

extern long far PASCAL WndProc(HWND, WORD, WORD, LONG);

int PASCAL 
WinMain(HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpszCmdLine, int cmdShow)
{
	WNDCLASS wndclass;

	char modulename[MAXSTR];

	/* Initialize the list of open Windows devices */
	gdev_win_open_list = 0;

	/* copy the hInstance into a variable so it can be used */
	phInstance = hInstance;

        /* start up the text window */
	_ScreenSize.y = 50;
	_InitEasyWin();

	/* fix up the EasyWindows window provided by Borland */
	GetModuleFileName(hInstance, (LPSTR) modulename, MAXSTR);
	hwndeasy = FindWindow("BCEasyWin", modulename);
	SetWindowText(hwndeasy, szAppName);            /* change title */
	SetClassWord(hwndeasy, GCW_HICON, LoadIcon(hInstance, "texticon"));

	/* if this is the first Ghostscript then register the window class */
	/* for graphics */
	if (!hPrevInstance) {
		wndclass.style = CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = WndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = hInstance;
		wndclass.hIcon = LoadIcon(hInstance,"grpicon");
		wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndclass.hbrBackground = GetStockObject(WHITE_BRUSH);
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = szAppName;
		RegisterClass(&wndclass);
	}

	(void) atexit(win_exit); /* setup exit handler */

	main(_argc, _argv, environ);

	/* never reached */
	win_exit(); 
	return 0;
}

void destroyproflist()
{
	struct PrStr far *cur;
	struct PrStr far *next;

	cur = prlst;
	prlst = NULL;
	while (cur != NULL)
	{
		free(cur->dev);
		free(cur->out);
		free(cur->drv);
		next = cur->next;
		free(cur);
		cur = next;
		}
	if (defprlst != NULL)
	{
		free(defprlst->dev);
		free(defprlst->out);
		free(defprlst->drv);
		free(defprlst);
		defprlst = NULL;
		}
	}

HDC FAR getprinter()
{
	/* See Petzold, "Programming Windows", ed 2, p721 */
	/* Use Default device and allow others */
	char name[MAXSTR];
	LPSTR dev;
	LPSTR drv;
	LPSTR out;
	FARPROC prslinst;
	int dlgret;
	char alldev[BIGBLK];
	struct PrStr far *cur = NULL; 
	struct PrStr far *last = NULL; 
	char *strptr;
	HDC retdc;

	/* get the useful info on the default printer */
	GetProfileString("windows", "device", ",,,", name, MAXSTR);
	dev = strtok(name, ",");
	drv = strtok(NULL, ", ");
	out = strtok(NULL, ", ");
	/* if OK store it as default */
	if (dev && drv && out)
	{
		defprlst = (struct PrStr far *) malloc(sizeof(struct PrStr));
		defprlst->next = NULL;
		defprlst->dev = (char far *) malloc(strlen(dev)+1);
		defprlst->out = (char far *) malloc(strlen(out)+1);
		defprlst->drv = (char far *) malloc(strlen(drv)+1);
		_fstrcpy(defprlst->dev, (LPSTR) dev);
		_fstrcpy(defprlst->out, (LPSTR) out);
		_fstrcpy(defprlst->drv, (LPSTR) drv);
		}
	
	/* find out about the names of other devices */
	GetProfileString("devices", NULL, "", alldev, BIGBLK);
	strptr = alldev;
	while (*strptr)
	{
		if (cur == NULL)
		{
			if (prlst == NULL)
				cur = last = prlst = (struct PrStr far *) malloc(sizeof(struct PrStr));
			else
				cur = last = prlst->next = (struct PrStr far *) malloc(sizeof(struct PrStr));
			}
		else
		{
			last = last->next = (struct PrStr far *) malloc(sizeof(struct PrStr));
			}
		last->dev = (char far *) malloc(strlen(strptr)+1);
		_fstrcpy(last->dev, (LPSTR) strptr);
		last->next = NULL;
		last->drv = NULL;
		last->out = NULL;
		strptr += strlen(strptr) + 1;
		}

	/* find out the details */
	while (cur)
	{
		GetProfileString("devices", cur->dev, "", name, MAXSTR);
		drv = strtok(name, ",");
		out = strtok(NULL, ", ");
		cur->out = (char far *) malloc(strlen(out)+1);
		cur->drv = (char far *) malloc(strlen(drv)+1);
		_fstrcpy(cur->out, (LPSTR) out);
		_fstrcpy(cur->drv, (LPSTR) drv);
		cur = cur->next;
		}

	/* open dialog box */
	prslinst = MakeProcInstance(printselproc, phInstance);
	dlgret = DialogBox(phInstance, "PRSEL", hwndeasy, prslinst);
	if (dlgret == IDCANCEL)
	{
		destroyproflist();
		return NULL;
		}
	dlgret = dlgret - IDOK - IDCANCEL;

	if (dlgret == 0)
	{
		dev = defprlst->dev;
		drv = defprlst->drv;
		out = defprlst->out;
		}
	else
	{
		dlgret--;
		cur = prlst;
		while (dlgret > 0)
		{
			cur = cur->next;
			dlgret--;
			}

		dev = cur->dev;
		drv = cur->drv;
		out = cur->out;
		}

	/* create device context */
	retdc = CreateDC(drv, dev, out, (void *) NULL);
	destroyproflist();

	return retdc;
	}

int FAR PASCAL printselproc(hdlg, wmsg, wparam, lparam)
HWND hdlg;
WORD wmsg;
WORD wparam;
DWORD lparam;
{
	struct PrStr far *cur; 
	LPSTR proutstr;
	HWND menu;
	int val;
	LPSTR dev;
	LPSTR drv;
	LPSTR out;
	char *drvnam;
	void (FAR PASCAL *dm)(HWND, HANDLE, LPSTR, LPSTR);

	switch (wmsg)
	{
		case WM_INITDIALOG:
			/* default message */
			if (defprlst != NULL)
			{
				menu = GetDlgItem(hdlg, DEFPRNSTR);
				proutstr = (LPSTR) malloc(strlen(defprlst->dev) + strlen(defprlst->out) + 17);
				_fstrcpy(proutstr, (LPSTR) "(Currently ");
				_fstrcat(proutstr, defprlst->dev);
				_fstrcat(proutstr, (LPSTR) " on ");
				_fstrcat(proutstr, defprlst->out);
				_fstrcat(proutstr, (LPSTR) ")");
				SetWindowText(menu, (LPSTR) proutstr);
				EnableWindow(GetDlgItem(hdlg, DEFPRN), TRUE);
				SendMessage(GetDlgItem(hdlg, DEFPRN), BM_SETCHECK, 1, 0L);
				}
			else
				EnableWindow(GetDlgItem(hdlg, DEFPRN), FALSE);
				

			/* list of all other printers */
			menu = GetDlgItem(hdlg, PRMNU);
			if (prlst == NULL)
				EnableWindow(GetDlgItem(hdlg, SPECPRN), FALSE);
			else
			{
				EnableWindow(GetDlgItem(hdlg, SPECPRN), TRUE);
				if (defprlst == NULL)
					SendMessage(GetDlgItem(hdlg, SPECPRN), BM_SETCHECK, 1, 0L);
				}
			cur = prlst;
			while (cur != NULL)
			{
				proutstr = (LPSTR) malloc(strlen(cur->dev) + strlen(cur->out) + 5);
				_fstrcpy(proutstr, cur->dev);
				_fstrcat(proutstr, (LPSTR) " on ");
				_fstrcat(proutstr, cur->out);
				SendMessage(menu,CB_INSERTSTRING, -1, (LONG) (LPSTR) proutstr);
				free(proutstr);
				cur = cur->next;
				}
			SendMessage(menu,CB_SETCURSEL, 0, 0);
			return TRUE;
		case WM_COMMAND:
			switch (wparam)
			{
				case IDOK:
					if (SendMessage(GetDlgItem(hdlg, DEFPRN), BM_GETCHECK, 0, 0L))
						val = 0;
					else if (SendMessage(GetDlgItem(hdlg, SPECPRN), BM_GETCHECK, 0, 0L))
						{
							menu = GetDlgItem(hdlg, PRMNU);
							val = SendMessage(menu,CB_GETCURSEL, 0, 0L)+1;
							}
					else
						EndDialog(hdlg, IDCANCEL);
					if (val == CB_ERR)
						EndDialog(hdlg, IDCANCEL);		/* catch all */
					else
						EndDialog(hdlg, val + IDOK + IDCANCEL);
					return TRUE;
				case IDCANCEL:
					EndDialog(hdlg, IDCANCEL);
					return TRUE;
				case PRSET:
					dev = NULL;		/* clear the options */
					drv = NULL;
					out = NULL;
					/* if button DEFAULT get options */
					if (SendMessage(GetDlgItem(hdlg, DEFPRN), BM_GETCHECK, 0, 0L))
					{
						dev = defprlst->dev;
						drv = defprlst->drv;
						out = defprlst->out;
						}
					/* if button SPECIFIC get options */
					if (SendMessage(GetDlgItem(hdlg, SPECPRN), BM_GETCHECK, 0, 0L))
					{
						menu = GetDlgItem(hdlg, PRMNU);
						val = SendMessage(menu,CB_GETCURSEL, 0, 0);
						cur = prlst;
						while (val > 0)
						{
							cur = cur->next;
							val--;
							}
						dev = cur->dev;
						drv = cur->drv;
						out = cur->out;
						}
			
					if (dev != NULL)
					{	
						/* allow configuration of printer */
						drvnam = (char *) malloc(strlen(drv) + 5);
						strcpy(drvnam, drv);
						strcat(drvnam, ".DRV");
						lib = LoadLibrary(drvnam);
						free(drvnam);
						if (lib >= 32)
						{
							dm = (void (FAR PASCAL *)()) GetProcAddress(lib, (LPSTR) "DEVICEMODE");
							(*dm)((HWND) hdlg, (HANDLE)lib, (LPSTR) dev, (LPSTR) out);
							FreeLibrary(lib);
							}
						}
					return TRUE;
				}
			break;
		}
	return FALSE;
	}

BOOL FAR PASCAL AbortProc(HDC hdcPrn, int code)
{
    MSG msg;

    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        }
    return(TRUE);
}
  
/* ------ Process message loop ------ */
/*
 * Check messages and interrupts; return true if interrupted.
 * This is called frequently - it must be quick!
 */
int
gp_check_interrupts()
{
	_KeyPressed();	/* Borland EasyWin message loop */
	return 0;
}

/* ------ from gp_dosfb.c ------ */

/* ------ Screen management ------ */

/* Write a string to the console. */
void
gp_console_puts(const char *str, uint size)
{	fwrite(str, 1, size, stdout);
}

/* Make the console current on the screen. */
int
gp_make_console_current(struct gx_device_s *dev)
{	return 0;
}

/* Make the graphics current on the screen. */
int
gp_make_graphics_current(struct gx_device_s *dev)
{	return 0;
}

/* ------ from gp_itbc.c ------ */

/* Do platform-dependent initialization. */
void
gp_init()
{
	win_init = 1;
}

/* Do platform-dependent cleanup. */
void
gp_exit()
{
	win_init = 0;
}

/* ------ Printer accessing ------ */

/* Open a connection to a printer.  A null file name means use the */
/* standard printer connected to the machine, if any. */
/* Return NULL if the connection could not be opened. */
extern void gp_set_printer_binary(P1(int));
FILE *
gp_open_printer(char *fname)
{	if ( strlen(fname) == 0 || !strcmp(fname, "PRN") )
	{	FILE *pfile;
		pfile = gp_open_scratch_file(gp_scratch_file_name_prefix, 
			win_prntmp, "wb");
		return pfile;
	}
	else
		return fopen(fname, "wb");
}

/* Close the connection to the printer. */
void
gp_close_printer(FILE *pfile, const char *fname)
{
HDC printer;
char *buf;
int *bufcount, count;
FARPROC lpfnAbortProc;
	fclose(pfile);
	if (strlen(fname) && strcmp(fname,"PRN"))
	    return;		/* a file, not a printer */
	printer = getprinter();
	if (printer != (HDC)NULL) {
	  if ( (pfile = fopen(win_prntmp,"rb")) != (FILE *)NULL) {
	    if ( (buf = malloc(4096+2)) != (char *)NULL ) {
	    	bufcount = (int *)buf;
		EnableWindow(hwndeasy,FALSE);
		lpfnAbortProc = MakeProcInstance(AbortProc,phInstance); 
	    	if (Escape(printer, STARTDOC, strlen(szAppName),szAppName, NULL) > 0) {
		    while (!feof(pfile)) {
		        count = fread(buf+2,1,4096,pfile);
		        *bufcount = count;
		        Escape(printer, PASSTHROUGH, count+2, (LPSTR)buf, NULL);
		    }
		    Escape(printer,ENDDOC,0,NULL,NULL);
		}
		FreeProcInstance(lpfnAbortProc);
		EnableWindow(hwndeasy,TRUE);
		free(buf);
	    }
	    fclose(pfile);
	  }
	  DeleteDC(printer);
	}
	unlink(win_prntmp);
}

/* ------ File names ------ */

/* Create and open a scratch file with a given name prefix. */
/* Write the actual file name at fname. */
FILE *
gp_open_scratch_file(const char *prefix, char *fname, const char *mode)
{	char *temp;
	if ( (temp = getenv("TEMP")) == NULL )
		*fname = 0;
	else
	{	strcpy(fname, temp);
		/* Prevent X's in path from being converted by mktemp. */
		for ( temp = fname; *temp; temp++ )
			*temp = tolower(*temp);
		strcat(fname, "\\");
	}
	strcat(fname, prefix);
	strcat(fname, "XXXXXX");
	mktemp(fname);
	return fopen(fname, mode);
}

/* ------ File operations ------ */

/* If the file given by fname exists, fill in its status and return 1; */
/* otherwise return 0. */
int
gp_file_status(const char *fname, file_status *pstatus)
{	FILE *f = fopen(fname, "r");
	long flen;
	struct ftime ft;
	if ( f == NULL ) return 0;
	if ( getftime(fileno(f), &ft) < 0 )
	   {	fclose(f);
		return 0;
	   }
	fseek(f, 0, SEEK_END);
	flen = ftell(f);
	pstatus->size_pages = (flen + 1023) >> 10;
	pstatus->size_bytes = flen;
	/* Make a single long value from the ftime structure. */
	pstatus->time_referenced = pstatus->time_created =
	  ((long)((ft.ft_year << 9) + (ft.ft_month << 5) + ft.ft_day) << 16) +
	  ((ft.ft_hour << 11) + (ft.ft_min << 5) + ft.ft_tsec);
	fclose(f);
	return 1;
}
