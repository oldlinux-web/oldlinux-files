/*
 * gv.h -- Main include file for ghostview.
 * Copyright (C) 1992  Timothy O. Theisen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Author: Tim Theisen           Systems Programmer
 * Internet: tim@cs.wisc.edu       Department of Computer Sciences
 *     UUCP: uwvax!tim             University of Wisconsin-Madison
 *    Phone: (608)262-0438         1210 West Dayton Street
 *      FAX: (608)262-9777         Madison, WI   53706
 */

#include <stdio.h>
#include <X11/Xos.h>
#include <sys/stat.h>
#include <X11/Intrinsic.h>
#define XtSetFloatArg(arg, n, d) \
    if (sizeof(float) > sizeof(XtArgVal)) { \
	XtSetArg(arg, n, &(d)); \
    } else { \
	XtArgVal *ld = (XtArgVal *)&(d); \
	XtSetArg(arg, n, *ld); \
    }
#include "Ghostview.h"

/* Application resources */
typedef struct _AppResources {
    Boolean show_title;		/* whether to show title */
    Boolean show_date;		/* whether to show date */
    Boolean show_locator;	/* whether to show locator */
    Boolean install_std_cmap;	/* whether to install standard colormap */
    Boolean retain_std_cmap;	/* whether to retain standard colormap */
    Boolean private_cmap;	/* whether to use private colormap */
    Boolean auto_center;	/* whether to automatically center the page */
    int wm_horiz_margin;	/* Space taken by window manager */
    int wm_vert_margin;		/* Space taken by window manager */
    int minimum_magstep;	/* smallest magstep allowed */
    int maximum_magstep;	/* largest magstep allowed */
    int magstep;		/* default magstep */
    XtPageOrientation orientation;	/* default orientation */
    String page;		/* first page to show */
    String pagemedia;		/* default page media */
    Boolean force_orientation;	/* use default to override document comments */
    Boolean force_pagemedia;	/* use default to override document comments */
    Boolean swap_landscape;	/* Landscape comment maps to Seascape */
    String print_command;	/* command used to print doc, usually "lpr" */
    String printer_variable;	/* env varaible to use, usually "PRINTER" */
    String default_printer;	/* printer to use if no PRINTER is not set*/
    String print_prompt;	/* string to prompt user for printer name */
    String print_fail;		/* string to inform user that print failed */
    String open_prompt;		/* string to prompt for file name to open */
    String open_fail;		/* string to inform user that open failed */
    String save_prompt;		/* string to prompt for file name to save */
    String save_fail;		/* string to inform user that save failed */
    /* Work arounds for others' bugs */
    Boolean openwindows;	/* whether to work around openwindow bug */
    Boolean ncdwm;		/* whether to work around ncdwm bug */
} AppResources;

extern float	default_xdpi;
extern float	default_ydpi;

extern int	num_ghosts;
extern FILE	*psfile;
extern String	filename;
extern String	oldfilename;
extern int	current_page;
extern int	current_magstep;
extern XtPageOrientation	current_orientation;
extern int	default_pagemedia;
extern int	current_pagemedia;
extern Boolean	force_document_media;
extern int	document_media;
extern int	current_llx;
extern int	current_lly;
extern int	current_urx;
extern int	current_ury;
extern int	base_papersize;
extern Boolean	info_up;
extern int	force_setting;
extern Pixmap	dot_bitmap;
extern Pixmap	menu16_bitmap;
extern Pixmap	tie_fighter_bitmap;
extern String	toc_text;
extern int	toc_length;
extern int	toc_entry_length;
extern int	info_length;
extern time_t	mtime;
extern struct document *doc;
extern struct document *olddoc;
extern Colormap	cmap;
extern Boolean	special_cmap;
extern Pixel	white;
extern Pixel	black;
extern Atom	wm_delete_window;
extern int	catch_Xerror();
extern XErrorHandler	old_Xerror;
extern Boolean	dying;
extern XErrorEvent	bomb;

enum {OPEN, PRINT_WHOLE, PRINT_MARKED, SAVE};
extern int	mode;

extern XtAppContext app_con;
extern AppResources app_res;

/* Widgets */
extern Widget toplevel;
extern Widget     form;
extern Widget         titlebutton;
extern Widget             titlemenu;
extern Widget         datebutton;
extern Widget             datemenu;
extern Widget         locator;
extern Widget         box;
extern Widget             filebutton;
extern Widget                 filemenu;
extern Widget                     openbutton;
extern Widget                     reopenbutton;
extern Widget                     printwholebutton;
extern Widget                     printmarkedbutton;
extern Widget                     savebutton;
extern Widget                     copyrightbutton;
extern Widget                     quitbutton;
extern Widget             pagebutton;
extern Widget                 pagemenu;
extern Widget                     nextbutton;
extern Widget                     showbutton;
extern Widget                     prevbutton;
extern Widget                     centerbutton;
extern Widget                     markbutton;
extern Widget                     unmarkbutton;
extern Widget             magstepbutton;
extern Widget                 magstepmenu;
extern Widget                     *magstepentry;
extern Widget             orientationbutton;
extern Widget                 orientationmenu;
extern Widget                     portraitbutton;
extern Widget                     landscapebutton;
extern Widget                     upsidedownbutton;
extern Widget                     seascapebutton;
extern Widget                     swapbutton;
extern Widget             pagemediabutton;
extern Widget                 pagemediamenu;
extern Widget                     *pagemediaentry;
extern Widget         toc;
extern Widget         pageview;
extern Widget             page;

/* Popup widgets */
extern Widget infopopup;
extern Widget     infoform;
extern Widget         infotext;
extern Widget         infobutton;
extern Widget copyrightpopup;
extern Widget     copyrightform;
extern Widget         copyrighttext;
extern Widget         copyrightbutton;
extern Widget dialogpopup;
extern Widget     dialog;

/* Dialogs */
extern Widget CreateDialog();
extern String GetDialogPrompt();
extern void SetDialogPrompt();
extern String GetDialogResponse();
extern void SetDialogResponse();
extern void ClearDialogResponse();

/* Callbacks */
extern void quit_ghostview();
extern void popup();
extern void popup_dialog();
extern void reopen_file();
extern void prev_page();
extern void this_page();
extern void next_page();
extern void center_page();
extern void mark_page();
extern void unmark_page();
extern void set_magstep();
extern void set_orientation();
extern void swap_landscape();
extern void set_pagemedia();
extern void track_and_zoom();
extern void message();
extern void output();
extern void okay();
extern void dismiss();
extern void destroy();
extern void destroy_ghost();

/* Actions */
extern void gv_copyright();
extern void gv_quit();
extern void gv_open();
extern void gv_reopen();
extern void gv_save();
extern void gv_print_whole();
extern void gv_print_marked();
extern void gv_prev();
extern void gv_show();
extern void gv_next();
extern void gv_center();
extern void gv_mark();
extern void gv_unmark();
extern void gv_set_magstep();
extern void gv_increase_magstep();
extern void gv_decrease_magstep();
extern void gv_set_orientation();
extern void gv_swap_landscape();
extern void gv_set_pagemedia();
extern void gv_default();
extern void gv_force();
extern void gv_delete_window();
extern void gv_delete_zoom();
extern void gv_dismiss();
extern void gv_scroll_up();
extern void gv_scroll_down();
extern void gv_scroll_left();
extern void gv_scroll_right();
extern void gv_erase_locator();
extern void gv_check_file();

/* Misc */
extern void show_page();
extern Boolean setup_ghostview();
extern void layout_ghostview();
extern void SetStandardColormap();
extern void magnify();
extern String open_file();
extern String save_file();
extern String print_file();
extern void pscopydoc();
extern void positionpopup();
extern Boolean set_new_magstep();
extern Boolean set_new_orientation();
extern Boolean set_new_pagemedia();
extern void build_pagemedia_menu();
extern Widget build_label_menu();
extern void new_file();
