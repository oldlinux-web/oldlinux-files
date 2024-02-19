/*
 * callbacks.c -- X11 callbacks for ghostview.
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
#ifndef BUFSIZ
#define BUFSIZ 1024
#endif

#ifdef VMS
#define getenv _getenv
#endif

extern char *getenv();

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>

#include "Ghostview.h"
#include "gv.h"
#include "ps.h"

/* Start application folding up by Destroying the top level widget. */
/* The application exits when the last interpreter is killed during */
/* a destroy callback from ghostview widgets. */
void
quit_ghostview(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XtDestroyWidget(toplevel);
}

/* Popup a window. */
void
popup(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    positionpopup((Widget)client_data);
    XtPopup((Widget)client_data, XtGrabNone);
    XRaiseWindow(XtDisplay((Widget)client_data), XtWindow((Widget)client_data));
}

/* Popup a dialog box. */
void
popup_dialog(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
#ifdef SELFILE
    Widget button;
    String okay, cancel;
    String name, init_path;
    Arg args[1];
    FILE* fp;
    struct stat sbuf;
    extern FILE *XsraSelFile();

    button = XtNameToWidget(dialog, "cancel");
    if (button) {
	XtSetArg(args[0], XtNlabel, &cancel);
	XtGetValues(button, args, ONE);
    }
    button = XtNameToWidget(dialog, "okay");
    if (button) {
	XtSetArg(args[0], XtNlabel, &okay);
	XtGetValues(button, args, ONE);
    }
#endif

    mode = (int) client_data;
    switch (mode) {
    case PRINT_WHOLE:
    case PRINT_MARKED:
	SetDialogPrompt(dialog, app_res.print_prompt);
	if (app_res.default_printer)
	    SetDialogResponse(dialog, app_res.default_printer);
	else
	    ClearDialogResponse(dialog);
	popup(w, (XtPointer)dialogpopup, call_data);
	break;
    case OPEN:
#ifdef SELFILE
	if (filename && strcmp(filename, "-")) init_path = filename;
	else init_path = NULL;
	fp = XsraSelFile(toplevel, app_res.open_prompt, okay, cancel,
			 app_res.open_fail, init_path, "r", NULL, &name);
	if (fp == NULL) break;
	if (oldfilename) XtFree(oldfilename);
	oldfilename = filename;
	filename = name;
	if (psfile) fclose(psfile);
	psfile = fp;
	stat(filename, &sbuf);
	mtime = sbuf.st_mtime;
	new_file(0);
	show_page(0);
#else
	SetDialogPrompt(dialog, app_res.open_prompt);
	if (filename && strcmp(filename, "-"))
	    SetDialogResponse(dialog, filename);
	else
	    ClearDialogResponse(dialog);
	popup(w, dialogpopup, call_data);
#endif
	break;
    case SAVE:
#ifdef SELFILE
	fp = XsraSelFile(toplevel, app_res.save_prompt, okay, cancel,
			 app_res.save_fail, "", "w", NULL, &name);
	if (fp == NULL) break;
	pscopydoc(fp);
	fclose(fp);
	XtFree(name);
#else
	SetDialogPrompt(dialog, app_res.save_prompt);
	ClearDialogResponse(dialog);
	popup(w, dialogpopup, call_data);
#endif
	break;
    }
}

/* Explicitly reopen the file. */
void
reopen_file(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    struct stat sbuf;
    int number = current_page;

    fclose(psfile);
    psfile = fopen(filename, "r");
    mtime = sbuf.st_mtime;
    if (oldfilename) XtFree(oldfilename);
    oldfilename = XtNewString(filename);
    new_file(number);
    show_page(number);
}

/* Get the selection, if no selection, get the insertion point. */
/* If the new_page is different from the current page show it.  */
/* If not at the first page, show the previous page. */
void
prev_page(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XawTextPosition pos, end;
    int new_page;

    XawTextGetSelectionPos(toc, &pos, &end);
    if (pos == end) {		/* Nothing selected */
	pos = XawTextGetInsertionPoint(toc);
    }
    if ((new_page = pos/toc_entry_length) == current_page) {
	new_page = current_page - 1;
    }
    if (new_page < 0) return;
    show_page(new_page);
}

/* Get the selection, if no selection, get the insertion point. */
/* Show this page.  */
void
this_page(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    if (toc_text) {
	XawTextPosition pos, end;
	int new_page;

	XawTextGetSelectionPos(toc, &pos, &end);
	if (pos == end) {		/* Nothing selected */
	    pos = XawTextGetInsertionPoint(toc);
	}
	new_page = pos/toc_entry_length;
	show_page(new_page);
    } else {
	GhostviewDisableInterpreter(page);
	show_page(0);
    }
}

/* Get the selection, if no selection, get the insertion point. */
/* If the new_page is different from the current page show it.  */
/* If not at the last page, show the next page. */
void
next_page(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XawTextPosition pos, end;
    int new_page = 0;

    if (toc_text) {
	XawTextGetSelectionPos(toc, &pos, &end);
	if (pos == end) {		/* Nothing selected */
	    pos = XawTextGetInsertionPoint(toc);
	}
	if ((new_page = pos/toc_entry_length) == current_page) {
	    new_page = current_page + 1;
	}
	if (new_page >= doc->numpages) return;
    }
    show_page(new_page);
}

/* Center the viewport over the page */
void
center_page(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    Arg args[2];
    Widget scroll;
    float top, shown;

    scroll = XtNameToWidget(pageview, "vertical");
    if (scroll) {
	XtSetArg(args[0], XtNshown, &shown);
	XtSetArg(args[1], XtNtopOfThumb, &top);
	XtGetValues(scroll, args, TWO);

	top = (1.0 - shown) / 2.0;
	XtCallCallbacks(scroll, XtNjumpProc, &top);
    }

    scroll = XtNameToWidget(pageview, "horizontal");
    if (scroll) {
	XtSetArg(args[0], XtNshown, &shown);
	XtSetArg(args[1], XtNtopOfThumb, &top);
	XtGetValues(scroll, args, TWO);

	top = (1.0 - shown) / 2.0;
	XtCallCallbacks(scroll, XtNjumpProc, &top);
    }
}

/* Get the selection, if no selection, get the insertion point. */
/* Mark all pages in range, and cause toc to update. */
void
mark_page(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XawTextPosition begin, end;
    int i;

    XawTextGetSelectionPos(toc, &begin, &end);
    if (begin == end) {		/* Nothing selected */
	begin = end = XawTextGetInsertionPoint(toc);
    } else {
	end--;			/* Sometimes end spills onto next line */
    }
    for (i = begin/toc_entry_length; i <= end/toc_entry_length; i++) {
	toc_text[i*toc_entry_length] = '*';
	XawTextInvalidate(toc, i*toc_entry_length, i*toc_entry_length+1);
    }
}

/* Get the selection, if no selection, get the insertion point. */
/* Unmark all pages in range, and cause toc to update. */
void
unmark_page(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XawTextPosition begin, end;
    int i;

    XawTextGetSelectionPos(toc, &begin, &end);
    if (begin == end) {		/* Nothing selected */
	begin = end = XawTextGetInsertionPoint(toc);
    } else {
	end--;			/* Sometimes end spills onto next line */
    }
    for (i = begin/toc_entry_length; i <= end/toc_entry_length; i++) {
	toc_text[i*toc_entry_length] = ' ';
	XawTextInvalidate(toc, i*toc_entry_length, i*toc_entry_length+1);
    }
}

/* Set new magstep.  Reshow the current page if magstep changed. */
void
set_magstep(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    app_res.magstep = (int)client_data;
    if (set_new_magstep()) {
	layout_ghostview();
	show_page(current_page);
    }
}

/* Set new orientation.  Reshow the current page if orientation changed. */
void
set_orientation(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    app_res.force_orientation = force_setting;
    app_res.orientation = (XtPageOrientation) client_data;
    if (set_new_orientation(current_page)) {
	layout_ghostview();
	show_page(current_page);
    }
}

/* Swap the landscape labels and change the flag. */
void
swap_landscape(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    Arg args[1];
    String s1, s2;

    app_res.swap_landscape = !app_res.swap_landscape;

    XtSetArg(args[0], XtNlabel, &s1);
    XtGetValues(landscapebutton, args, ONE);
    s1 = XtNewString(s1);
    XtSetArg(args[0], XtNlabel, &s2);
    XtGetValues(seascapebutton, args, ONE);
    s2 = XtNewString(s2);
    XtSetArg(args[0], XtNlabel, s2);
    XtSetValues(landscapebutton, args, ONE);
    XtSetArg(args[0], XtNlabel, s1);
    XtSetValues(seascapebutton, args, ONE);
    XtFree(s1);
    XtFree(s2);

    if (set_new_orientation(current_page)) {
	layout_ghostview();
	show_page(current_page);
    }
}

/* Set new page media.  If new page media is different, update app_resources */
/* and redisplay page. */
void
set_pagemedia(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    int new_pagemedia = (int) client_data;
    if (new_pagemedia >= base_papersize) {
	default_pagemedia = new_pagemedia;
	app_res.force_pagemedia = force_setting;
    } else {
	document_media = new_pagemedia;
	force_document_media = force_setting;
    }
    if (set_new_pagemedia(current_page)) {
	layout_ghostview();
	show_page(current_page);
    }
}

/* track mouse pointer and popup zoom window */
void
track_and_zoom(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    Arg args[20];
    Cardinal num_args;
    Dimension width, height;
    Widget zoom;
    Widget zoomform;
    Widget zoompage;
    Widget zoomdismiss;
    FILE *zoomfile;
    struct stat sbuf;
    GhostviewReturnStruct *p = (GhostviewReturnStruct *)call_data;
    int llx;
    int lly;
    int urx;
    int ury;
    int bottom_margin;
    int left_margin;
    int right_margin;
    int top_margin;
    int i;

    /* locator events have zero width and height */
    if ((p->width == 0) || (p->height == 0)) {
	char buf[32];
	if (!app_res.show_locator) return;
	sprintf(buf, "(%d, %d)", p->psx, p->psy);
	XtSetArg(args[0], XtNlabel, buf);
	XtSetValues(locator, args, ONE);
	return;
    }

    /* If no file, nothing to zoom. */
    if (!psfile) return;

    /* If the file changed, cannot zoom */
    stat(filename, &sbuf);
    if (mtime != sbuf.st_mtime) return;
							num_args = 0;
    XtSetArg(args[num_args], XtNcolormap, cmap);	num_args++;
    if (special_cmap) {
	XtSetArg(args[num_args], XtNbackground, white);	num_args++;
    }
    zoom = XtCreatePopupShell("zoom", topLevelShellWidgetClass,
			      toplevel, args, num_args);

							num_args = 0;
    if (special_cmap) {
	XtSetArg(args[num_args], XtNbackground, white);	num_args++;
    }
    zoomform = XtCreateManagedWidget("form", formWidgetClass,
				     zoom, args, num_args);

    llx = p->psx - p->width/2;
    lly = p->psy - p->height/2;
    urx = p->psx + p->width/2;
    ury = p->psy + p->height/2;

    /* Make sure zoom window doesn't go off the edge of the page */
    if (llx < current_llx) {
	llx = current_llx;
	urx = llx + p->width;
    }
    if (lly < current_lly) {
	lly = current_lly;
	ury = lly + p->height;
    }
    if (urx > current_urx) {
	urx = current_urx;
	llx = urx - p->width;
    }
    if (ury > current_ury) {
	ury = current_ury;
	lly = ury - p->height;
    }
    if (llx < current_llx) {
	llx = current_llx;
    }
    if (lly < current_lly) {
	lly = current_lly;
    }
    bottom_margin = lly - current_lly;
    left_margin = llx - current_llx;
    right_margin = current_urx - urx;
    top_margin = current_ury - ury;

							num_args = 0;
    if (special_cmap) {
	XtSetArg(args[num_args], XtNbackground, white);	num_args++;
    }
    XtSetArg(args[num_args], XtNtop, XtChainTop);	num_args++;
    XtSetArg(args[num_args], XtNbottom, XtChainBottom);	num_args++;
    XtSetArg(args[num_args], XtNleft, XtChainLeft);	num_args++;
    XtSetArg(args[num_args], XtNright, XtChainRight);	num_args++;
    XtSetArg(args[num_args], XtNorientation, current_orientation);
							num_args++;
    XtSetArg(args[num_args], XtNllx, llx);      	num_args++;
    XtSetArg(args[num_args], XtNlly, lly);      	num_args++;
    XtSetArg(args[num_args], XtNurx, urx);      	num_args++;
    XtSetArg(args[num_args], XtNury, ury);      	num_args++;
    XtSetArg(args[num_args], XtNbottomMargin, bottom_margin);
							num_args++;
    XtSetArg(args[num_args], XtNleftMargin, left_margin);
							num_args++;
    XtSetArg(args[num_args], XtNrightMargin, right_margin);
							num_args++;
    XtSetArg(args[num_args], XtNtopMargin, top_margin); num_args++;
    XtSetArg(args[num_args], XtNbottomMargin, bottom_margin);
							num_args++;
    XtSetFloatArg(args[num_args], XtNxdpi, p->xdpi);	num_args++;
    XtSetFloatArg(args[num_args], XtNydpi, p->ydpi);	num_args++;
    if (!toc_text) {
        XtSetArg(args[num_args], XtNfilename, filename);	num_args++;
    }
    zoompage = XtCreateManagedWidget("page", ghostviewWidgetClass,
				     zoomform, args, num_args);
    num_ghosts++;
    XtAddCallback(zoompage, XtNcallback, track_and_zoom, (XtPointer)0);
    XtAddCallback(zoompage, XtNmessageCallback, message, (XtPointer)zoompage);
    XtAddCallback(zoompage, XtNdestroyCallback, destroy_ghost,
		  (XtPointer)zoompage);

							num_args = 0;
    if (special_cmap) {
	XtSetArg(args[num_args], XtNforeground, black);	num_args++;
	XtSetArg(args[num_args], XtNbackground, white);	num_args++;
    }
    XtSetArg(args[num_args], XtNfromVert, zoompage);	num_args++;
    XtSetArg(args[num_args], XtNtop, XtChainBottom);	num_args++;
    XtSetArg(args[num_args], XtNbottom, XtChainBottom);	num_args++;
    XtSetArg(args[num_args], XtNleft, XtChainLeft);	num_args++;
    XtSetArg(args[num_args], XtNright, XtChainRight);	num_args++;
    zoomdismiss = XtCreateManagedWidget("dismiss", commandWidgetClass,
				       zoomform, args, num_args);
    XtAddCallback(zoomdismiss, XtNcallback, destroy, (XtPointer)zoom);

    XtSetArg(args[0], XtNwidth, &width);
    XtGetValues(zoompage, args, ONE);
    XtSetArg(args[0], XtNwidth, width);
    XtSetValues(zoomdismiss, args, ONE);

    XtRealizeWidget(zoom);
    positionpopup(zoom);

							num_args = 0;
    XtSetArg(args[num_args], XtNwidth, &width);		num_args++;
    XtSetArg(args[num_args], XtNheight, &height);	num_args++;
    XtGetValues(zoom, args, num_args);

							      	num_args = 0;
    XtSetArg(args[num_args], XtNminWidth, width);		num_args++;
    XtSetArg(args[num_args], XtNminHeight, height);		num_args++;
    XtSetArg(args[num_args], XtNmaxWidth, width);		num_args++;
    XtSetArg(args[num_args], XtNmaxHeight, height);		num_args++;
    XtSetValues(zoom, args, num_args);
    XSetWMProtocols(XtDisplay(zoom), XtWindow(zoom), &wm_delete_window, 1);
    XtPopup(zoom, XtGrabNone);

    if (toc_text) {
	zoomfile = fopen(filename, "r");
	if (zoomfile == NULL) return;
	GhostviewSendPS(zoompage, zoomfile, doc->beginprolog,
			doc->lenprolog, False);
	GhostviewSendPS(zoompage, zoomfile, doc->beginsetup,
			doc->lensetup, False);
	if (doc->pageorder == DESCEND)
	    i = (doc->numpages - 1) - current_page;
	else
	    i = current_page;
	GhostviewSendPS(zoompage, zoomfile, doc->pages[i].begin,
			doc->pages[i].len, True);
    }
}

/* Process messages from ghostscript */
/* Refresh occurs when window was resized unexpectedly */
void
message(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    int i;
    char *error;

    if (!strcmp((char *) call_data, "Failed")) {
	if ((Widget)client_data == page) {
	    error = "Error: PostScript interpreter failed in main window.\n\n";
	} else {
	    error = "Error: PostScript interpreter failed in zoom window.\n\n";
	}
	output(w, NULL, error);
    } else if (!strcmp((char *) call_data, "BadAlloc")) {
	if ((Widget)client_data == page) {
	    error = 
	       "Warning: Could not allocate backing pixmap in main window.\n\n";
	} else {
	    error = 
	       "Warning: Could not allocate backing pixmap in zoom window.\n\n";
	}
	output(w, NULL, error);
    } else if (!strcmp((char *) call_data, "Refresh")) {
	if (toc_text) {
	    GhostviewSendPS(w, psfile, doc->beginprolog,
			    doc->lenprolog, False);
	    GhostviewSendPS(w, psfile, doc->beginsetup,
			    doc->lensetup, False);
	    if (doc->pageorder == DESCEND)
		i = (doc->numpages - 1) - current_page;
	    else
		i = current_page;
	    GhostviewSendPS(w, psfile, doc->pages[i].begin,
			    doc->pages[i].len, False);
	}
    }
}

/* Take output from ghostscript and display it in the infotext popup window */
void
output(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    Arg args[2];
    XawTextBlock message_block;

    message_block.firstPos = 0;
    message_block.length = strlen(call_data);
    message_block.ptr = call_data;
    message_block.format = FMT8BIT;

    XawTextDisableRedisplay(infotext);

    XtSetArg(args[0], XtNeditType, XawtextAppend);
    XtSetValues(infotext, args, ONE);
    XawTextReplace(infotext, info_length, info_length, &message_block);
    info_length = info_length + message_block.length;

    XtSetArg(args[0], XtNeditType, XawtextRead);
    XtSetArg(args[1], XtNinsertPosition, info_length);
    XtSetValues(infotext, args, TWO);
    XawTextEnableRedisplay(infotext);
    if (!info_up) XtPopup(infopopup, XtGrabNone);
    info_up = True;
}

/* Dismiss popup dialog */
void
okay(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    String name, error;
    Widget dialog;

    dialog = (Widget)client_data;
    name = GetDialogResponse(dialog);
    switch (mode) {
    case PRINT_WHOLE:
    case PRINT_MARKED:
	if (error = print_file(name, (mode == PRINT_WHOLE))) {
	    char *buf = XtMalloc(strlen(error) +
				 strlen(app_res.print_prompt) + 2);
	    sprintf(buf, "%s\n%s", error, app_res.print_prompt);
	    SetDialogPrompt(dialog, buf);
	    XtFree(error);
	    XtFree(buf);
	} else {
	    XtPopdown(XtParent(dialog));
	}
	break;
    case OPEN:
	if (error = open_file(name)) {
	    char *buf = XtMalloc(strlen(error) +
				 strlen(app_res.open_prompt) + 2);
	    sprintf(buf, "%s\n%s", error, app_res.open_prompt);
	    SetDialogPrompt(dialog, buf);
	    XtFree(error);
	    XtFree(buf);
	} else {
	    XtPopdown(XtParent(dialog));
	}
	break;
    case SAVE:
	if (error = save_file(name)) {
	    char *buf = XtMalloc(strlen(error) +
				 strlen(app_res.save_prompt) + 2);
	    sprintf(buf, "%s\n%s", error, app_res.save_prompt);
	    SetDialogPrompt(dialog, buf);
	    XtFree(error);
	    XtFree(buf);
	} else {
	    XtPopdown(XtParent(dialog));
	}
	break;
    }
    XtFree(name);
}

/* Dismiss popup window */
void
dismiss(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XtPopdown((Widget)client_data);
    if ((Widget)client_data == infopopup) info_up = False;
}

/* Destroy popup window */
void
destroy(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XtDestroyWidget((Widget)client_data);
}

/* destroy callback for Ghostview widgets. */
/* The disable interpreter call ensures that ghostscript is killed. */
/* One the count goes to 0, we are sure that all forked processes have */
/* been killed and that we can safely exit. */
void
destroy_ghost(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    GhostviewDisableInterpreter((Widget) client_data);
    num_ghosts--;
    if (num_ghosts) return;
    if (dying) old_Xerror(XtDisplay(w), &bomb);
    XtDestroyApplicationContext(app_con);
    exit(0);
}
