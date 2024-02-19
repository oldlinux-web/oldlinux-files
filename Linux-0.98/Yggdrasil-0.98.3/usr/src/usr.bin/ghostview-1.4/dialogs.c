/*
 * dialogs.c -- Dialog box for ghostview.
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
#include <X11/Xos.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Command.h>
#include "gv.h"

static String okay_accelerators =
    "#override\n\
     <Key>Return: set() notify() unset()\n";

/* Create a dialog widget */
/* It is just a form widget with
 *   a label prompt
 *   a text response
 *   an oky button
 *   a cancel button */
Widget
CreateDialog(parent, name, okay_callback, cancel_callback)
    Widget parent;
    String name;
    XtCallbackProc okay_callback;
    XtCallbackProc cancel_callback;
{
    Widget form, prompt, response, okay, cancel;
    Arg args[20];
    Cardinal num_args;

							num_args = 0;
    if (special_cmap) {
	XtSetArg(args[num_args], XtNbackground, white);	num_args++;
    }
    form = XtCreateManagedWidget(name, formWidgetClass, parent, args, num_args);

							num_args = 0;
    if (special_cmap) {
	XtSetArg(args[num_args], XtNforeground, black);	num_args++;
	XtSetArg(args[num_args], XtNbackground, white);	num_args++;
    }
    XtSetArg(args[num_args], XtNtop, XtChainTop);	num_args++;
    XtSetArg(args[num_args], XtNbottom, XtChainTop);	num_args++;
    XtSetArg(args[num_args], XtNleft, XtChainLeft);	num_args++;
    XtSetArg(args[num_args], XtNright, XtChainLeft);	num_args++;
    XtSetArg(args[num_args], XtNresizable, True);	num_args++;
    XtSetArg(args[num_args], XtNborderWidth, 0);	num_args++;
    prompt = XtCreateManagedWidget("prompt", labelWidgetClass,
				   form, args, num_args);

							num_args = 0;
    if (special_cmap) {
	XtSetArg(args[num_args], XtNforeground, black);	num_args++;
	XtSetArg(args[num_args], XtNbackground, white);	num_args++;
    }
    XtSetArg(args[num_args], XtNfromVert, prompt);	num_args++;
    XtSetArg(args[num_args], XtNtop, XtChainTop);	num_args++;
    XtSetArg(args[num_args], XtNbottom, XtChainTop);	num_args++;
    XtSetArg(args[num_args], XtNleft, XtChainLeft);	num_args++;
    XtSetArg(args[num_args], XtNright, XtChainLeft);	num_args++;
    XtSetArg(args[num_args], XtNresizable, True);	num_args++;
    XtSetArg(args[num_args], XtNeditType, XawtextEdit);	num_args++;
    XtSetArg(args[num_args], XtNresize, XawtextResizeWidth);	num_args++;
    XtSetArg(args[num_args], XtNstring, "");		num_args++;
    response = XtCreateManagedWidget("response", asciiTextWidgetClass,
				     form, args, num_args);

							num_args = 0;
    if (special_cmap) {
	XtSetArg(args[num_args], XtNforeground, black);	num_args++;
	XtSetArg(args[num_args], XtNbackground, white);	num_args++;
    }
    XtSetArg(args[num_args], XtNfromVert, response);	num_args++;
    XtSetArg(args[num_args], XtNtop, XtChainTop);	num_args++;
    XtSetArg(args[num_args], XtNbottom, XtChainTop);	num_args++;
    XtSetArg(args[num_args], XtNleft, XtChainLeft);	num_args++;
    XtSetArg(args[num_args], XtNright, XtChainLeft);	num_args++;
    XtSetArg(args[num_args], XtNresizable, True);	num_args++;
    XtSetArg(args[num_args], XtNaccelerators,
	     XtParseAcceleratorTable(okay_accelerators));	num_args++;
    okay = XtCreateManagedWidget("okay", commandWidgetClass,
				 form, args, num_args);
    XtAddCallback(okay, XtNcallback, okay_callback, form);

							num_args = 0;
    if (special_cmap) {
	XtSetArg(args[num_args], XtNforeground, black);	num_args++;
	XtSetArg(args[num_args], XtNbackground, white);	num_args++;
    }
    XtSetArg(args[num_args], XtNfromVert, response);	num_args++;
    XtSetArg(args[num_args], XtNfromHoriz, okay);	num_args++;
    XtSetArg(args[num_args], XtNtop, XtChainTop);	num_args++;
    XtSetArg(args[num_args], XtNbottom, XtChainTop);	num_args++;
    XtSetArg(args[num_args], XtNleft, XtChainLeft);	num_args++;
    XtSetArg(args[num_args], XtNright, XtChainLeft);	num_args++;
    XtSetArg(args[num_args], XtNresizable, True);	num_args++;
    cancel = XtCreateManagedWidget("cancel", commandWidgetClass,
				   form, args, num_args);
    XtAddCallback(cancel, XtNcallback, cancel_callback, parent);

    XtInstallAccelerators(response, okay);
    XtSetKeyboardFocus(form, response);

    return form;
}

/* get the prompt from the dialog box.  Used a startup time to
 * save away the initial prompt */
String
GetDialogPrompt(w)
    Widget w;
{
    Arg args[1];
    Widget label;
    String s;

    label = XtNameToWidget(w, "prompt");
    XtSetArg(args[0], XtNlabel, &s);
    XtGetValues(label, args, ONE);
    return XtNewString(s);
}

/* set the prompt.  This is used to put error information in the prompt */
void
SetDialogPrompt(w, newprompt)
    Widget w;
    String newprompt;
{
    Arg args[1];
    Widget label;

    label = XtNameToWidget(w, "prompt");
    XtSetArg(args[0], XtNlabel, newprompt);
    XtSetValues(label, args, ONE);
}

/* get what the user typed */
String
GetDialogResponse(w)
    Widget w;
{
    Arg args[1];
    Widget response;
    String s;

    response = XtNameToWidget(w, "response");
    XtSetArg(args[0], XtNstring, &s);
    XtGetValues(response, args, ONE);
    return XtNewString(s);
}

/* set the default reponse */
void
SetDialogResponse(w, s)
    Widget w;
    String s;
{
    Arg args[3];
    Widget response;
    XFontStruct *font;
    Dimension width, leftMargin, rightMargin;

    response = XtNameToWidget(w, "response");
    XtSetArg(args[0], XtNfont, &font);
    XtSetArg(args[1], XtNleftMargin, &leftMargin);
    XtSetArg(args[2], XtNrightMargin, &rightMargin);
    XtGetValues(response, args, THREE);
    width = font->max_bounds.width * strlen(s) + leftMargin + rightMargin;

    XtSetArg(args[0], XtNstring, s);
    XtSetArg(args[1], XtNwidth, width);
    XtSetValues(response, args, TWO);
    XawTextSetInsertionPoint(response, strlen(s));

}

/* clear the response */
void
ClearDialogResponse(w)
    Widget w;
{
    Arg args[2];
    Widget response;

    response = XtNameToWidget(w, "response");
    XtSetArg(args[0], XtNstring, "");
    XtSetArg(args[1], XtNwidth, 100);
    XtSetValues(response, args, TWO);
}
