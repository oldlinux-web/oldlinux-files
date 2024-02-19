/*
 * actions.c -- X11 actions for ghostview.
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

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Scrollbar.h>
#include "gv.h"
#include "ps.h"

/* Popup the copyright window */
void
gv_copyright(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    popup(w, (XtPointer)copyrightpopup, NULL);
}

/* Call the quit callback to stop ghostview */
void
gv_quit(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    quit_ghostview(w, NULL, NULL);
}

/* Popup the open file dialog box. */
void
gv_open(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    popup_dialog(w, (XtPointer)OPEN, NULL);
}

/* Popup the open file dialog box. */
void
gv_reopen(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    if (!XtIsSensitive(reopenbutton)) return;
    reopen_file(w, NULL, NULL);
}

/* Popup the save file dialog box. */
void
gv_save(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    if (!XtIsSensitive(savebutton)) return;
    popup_dialog(w, (XtPointer)SAVE, NULL);
}

/* Popup the print file dialog box. */
void
gv_print_whole(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    if (!XtIsSensitive(printwholebutton)) return;
    popup_dialog(w, (XtPointer)PRINT_WHOLE, NULL);
}

/* Popup the print file dialog box. */
void
gv_print_marked(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    if (!XtIsSensitive(printmarkedbutton)) return;
    popup_dialog(w, (XtPointer)PRINT_MARKED, NULL);
}

/* Call the prev_page callback */
void
gv_prev(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    if (!XtIsSensitive(prevbutton)) return;
    prev_page(w, NULL, NULL);
}

/* Call the this_page callback */
void
gv_show(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    if (!XtIsSensitive(showbutton)) return;
    this_page(w, NULL, NULL);
}

/* Call the next_page callback */
void
gv_next(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    if (!XtIsSensitive(nextbutton)) return;
    next_page(w, NULL, NULL);
}

/* Call the center_page callback */
void
gv_center(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    if (!XtIsSensitive(centerbutton)) return;
    center_page(w, NULL, NULL);
}

/* Call the mark_page callback */
void
gv_mark(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    if (!XtIsSensitive(markbutton)) return;
    mark_page(w, NULL, NULL);
}

/* Call the unmark_page callback */
void
gv_unmark(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    if (!XtIsSensitive(unmarkbutton)) return;
    unmark_page(w, NULL, NULL);
}

/* Get the magstep from the parameter string and
 * call the set_magstep callback with that magstep */
void
gv_set_magstep(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    int i;

    if (*num_params < 1) return;
    i = atoi(params[0]);
    set_magstep(w, (XtPointer)i, NULL);
}

/* Increment the magstep and
 * call the set_magstep callback with that magstep */
void
gv_increase_magstep(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    int i;

    i = app_res.magstep + 1;
    if (i <= app_res.maximum_magstep)
	set_magstep(w, (XtPointer)i, NULL);
}

/* Decrement the magstep and
 * call the set_magstep callback with that magstep */
void
gv_decrease_magstep(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    int i;

    i = app_res.magstep - 1;
    if (i >= app_res.minimum_magstep)
	set_magstep(w, (XtPointer)i, NULL);
}

/* Set orientation action routine.  Converts text parameter
 * to XtPageOrientation and all set_orientation callback */
void
gv_set_orientation(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    XrmValue from, to;
    XtPageOrientation orient;

    if (*num_params < 1) return;
    from.size = sizeof(String);
    from.addr = params[0];
    to.size = 0;
    to.addr = NULL;
    if (XmuCvtStringToPageOrientation(XtDisplay(w), NULL, ZERO,
				      &from, &to, NULL)) {
	orient = *(XtPageOrientation *)(to.addr);
	set_orientation(w, (XtPointer)orient, NULL);
    }
}

/* Call the swap_landscape callback */
void
gv_swap_landscape(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    swap_landscape(w, NULL, NULL);
}

/* Set pagemedia action routine.  Converts text parameter
 * to index into the pagemedia widgets and calls the set_pagemedia
 * callback. */
void
gv_set_pagemedia(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    int i;

    if (*num_params < 1) return;

    /* First check pagemedia defined within the document */
    if (doc && doc->nummedia) {
	for (i = 0; i < doc->nummedia; i++) {
	    if (!strcmp(params[0], doc->media[i].name)) {
		set_pagemedia(w, (XtPointer)i, NULL);
		break;
	    }
	}
    }

    /* Then check the standard ones */
    for (i = 0; papersizes[i].name; i++) {
	if (!strcmp(params[0], papersizes[i].name)) {
    	    set_pagemedia(w, (XtPointer)(base_papersize+i), NULL);
	    break;
	}
    }
}


/* Reset the force flag.  */
/* (force flag is checked when setting orientaion and pagemedia) */
void
gv_default(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    force_setting = False;
}

/* Set the force flag.  */
/* (force flag is checked when setting orientaion and pagemedia) */
void
gv_force(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    force_setting = True;
}

/* Implement WM_DELETE_WINDOW protocol */
void
gv_delete_window(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    if (event->type == ClientMessage &&
	event->xclient.data.l[0] != wm_delete_window) return;
    XtDestroyWidget(w);
}


/* Destroy popup zoom window */
void
gv_delete_zoom(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    XtDestroyWidget(XtParent(w));
}

/* dismiss a popup window */
void
gv_dismiss(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    XtPopdown(w);
    if (w == infopopup) info_up = False;
}

/* scroll main viewport up */
void
gv_scroll_up(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    Arg args[2];
    Widget scroll;
    float top, shown;

    scroll = XtNameToWidget(pageview, "vertical");
    if (scroll) {
	XtSetArg(args[0], XtNshown, &shown);
	XtSetArg(args[1], XtNtopOfThumb, &top);
	XtGetValues(scroll, args, TWO);

	top = top - shown;
	if (top < 0.0) top = 0.0;
	XtCallCallbacks(scroll, XtNjumpProc, &top);
    }
}

/* scroll main viewport down */
void
gv_scroll_down(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    Arg args[2];
    Widget scroll;
    float top, shown;

    scroll = XtNameToWidget(pageview, "vertical");
    if (scroll) {
	XtSetArg(args[0], XtNshown, &shown);
	XtSetArg(args[1], XtNtopOfThumb, &top);
	XtGetValues(scroll, args, TWO);

	top = top + shown;
	if (top > (1.0 - shown)) top = (1.0 - shown);
	XtCallCallbacks(scroll, XtNjumpProc, &top);
    }
}

/* scroll main viewport left */
void
gv_scroll_left(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    Arg args[2];
    Widget scroll;
    float top, shown;

    scroll = XtNameToWidget(pageview, "horizontal");
    if (scroll) {
	XtSetArg(args[0], XtNshown, &shown);
	XtSetArg(args[1], XtNtopOfThumb, &top);
	XtGetValues(scroll, args, TWO);

	top = top - shown;
	if (top < 0.0) top = 0.0;
	XtCallCallbacks(scroll, XtNjumpProc, &top);
    }
}

/* scroll main viewport right */
void
gv_scroll_right(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    Arg args[2];
    Widget scroll;
    float top, shown;

    scroll = XtNameToWidget(pageview, "horizontal");
    if (scroll) {
	XtSetArg(args[0], XtNshown, &shown);
	XtSetArg(args[1], XtNtopOfThumb, &top);
	XtGetValues(scroll, args, TWO);

	top = top + shown;
	if (top > (1.0 - shown)) top = (1.0 - shown);
	XtCallCallbacks(scroll, XtNjumpProc, &top);
    }
}

/* Pop down locator window */
void
gv_erase_locator(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    Arg args[1];

    if (!app_res.show_locator) return;
    XtSetArg(args[0], XtNlabel, "");
    XtSetValues(locator, args, ONE);
}

/* Check to see if file was updated */
void
gv_check_file(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    struct stat sbuf;

    if (psfile) {
	if (!stat(filename, &sbuf) && mtime != sbuf.st_mtime) {
	    show_page(current_page);
	}
    }
}
