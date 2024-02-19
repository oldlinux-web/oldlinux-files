/*
 * Ghostview.c -- Ghostview widget.
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

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include <X11/Xos.h>
#include "GhostviewP.h"
#include <ctype.h>

#ifndef XlibSpecificationRelease
typedef char *XPointer;
#endif

#include <signal.h>
#ifdef SIGNALRETURNSINT
#define SIGVAL int
#else
#define SIGVAL void
#endif

#ifdef NON_BLOCKING_IO
#include <fcntl.h>
/* if POSIX O_NONBLOCK is not available, use O_NDELAY */
#if !defined(O_NONBLOCK) && defined(O_NDELAY)
#define O_NONBLOCK O_NDELAY
#endif
#endif

#include <errno.h>
/* BSD 4.3 errno.h does not declare errno */
extern int errno;
/* Both error returns are checked for non-blocking I/O. */
/* Manufacture the other error code if only one exists. */
#if !defined(EWOULDBLOCK) && defined(EAGAIN)
#define EWOULDBLOCK EAGAIN
#endif
#if !defined(EAGAIN) && defined(EWOULDBLOCK)
#define EAGAIN EWOULDBLOCK
#endif

#ifndef VMS
/* GV_BUFSIZ is set to the minimum POSIX PIPE_BUF to ensure that
 * nonblocking writes to ghostscript will work properly.
 */
#define GV_BUFSIZ 512
#else /* VMS */
/*
** GV_BUFSIZ is the maximum length line we can handle, so we up it to 1024
*/
#define GV_BUFSIZ 1024
#endif /* VMS */

/* length calculates string length at compile time */
/* can only be used with character constants */
#define length(a) (sizeof(a)-1)
#define iscomment(a, b) (strncmp(a, b, length(b)) == 0)

static void ComputeXdpi();
static void ComputeYdpi();

static XtResource resources[] = {
#define offset(field) XtOffsetOf(GhostviewRec, ghostview.field)
    { XtNbottomMargin, XtCMargin, XtRInt, sizeof(int),
	  offset(bottom_margin), XtRImmediate, (XtPointer)0 },
    { XtNbusyCursor, XtCCursor, XtRCursor, sizeof(XtPointer),
	  offset(busy_cursor), XtRString, "watch" },
    { XtNcallback, XtCCallback, XtRCallback, sizeof(XtPointer),
	  offset(callback), XtRCallback, (XtPointer)NULL },
    { XtNcursor, XtCCursor, XtRCursor, sizeof(XtPointer),
	  offset(cursor), XtRString, "crosshair" },
    { XtNfilename, XtCFilename, XtRString, sizeof(String),
	  offset(filename), XtRString, (XtPointer)NULL },
    { XtNinterpreter, XtCInterpreter, XtRString, sizeof(String),
	  offset(interpreter), XtRString, "gs" },
    { XtNleftMargin, XtCMargin, XtRInt, sizeof(int),
	  offset(left_margin), XtRImmediate, (XtPointer)0 },
    { XtNllx, XtCBoundingBox, XtRInt, sizeof(int),
	  offset(llx), XtRImmediate, (XtPointer)0 },
    { XtNlly, XtCBoundingBox, XtRInt, sizeof(int),
	  offset(lly), XtRImmediate, (XtPointer)0 },
    { XtNmessageCallback, XtCCallback, XtRCallback, sizeof(XtPointer),
	  offset(message_callback), XtRCallback, (XtPointer)NULL },
    { XtNorientation, XtCOrientation, XtRPageOrientation,
	  sizeof(XtPageOrientation), offset(orientation), XtRImmediate,
	  (XtPointer)XtPageOrientationPortrait },
    { XtNoutputCallback, XtCCallback, XtRCallback, sizeof(XtPointer),
	  offset(output_callback), XtRCallback, (XtPointer)NULL },
    { XtNpreload, XtCPreload, XtRString, sizeof(String),
	  offset(preload), XtRString, (XtPointer)NULL },
    { XtNquiet, XtCQuiet, XtRBoolean, sizeof(Boolean),
	  offset(quiet), XtRImmediate, (XtPointer)True },
    { XtNrightMargin, XtCMargin, XtRInt, sizeof(int),
	  offset(right_margin), XtRImmediate, (XtPointer)0 },
    { XtNtopMargin, XtCMargin, XtRInt, sizeof(int),
	  offset(top_margin), XtRImmediate, (XtPointer)0 },
    { XtNuseBackingPixmap, XtCUseBackingPixmap, XtRBoolean, sizeof(Boolean),
	  offset(use_bpixmap), XtRImmediate, (XtPointer)True },
    { XtNurx, XtCBoundingBox, XtRInt, sizeof(int),
	  offset(urx), XtRImmediate, (XtPointer)612 },
    { XtNury, XtCBoundingBox, XtRInt, sizeof(int),
	  offset(ury), XtRImmediate, (XtPointer)792 },
    { XtNxdpi, XtCResolution, XtRFloat, sizeof(float),
	  offset(xdpi), XtRCallProc, (XtPointer)ComputeXdpi },
    { XtNydpi, XtCResolution, XtRFloat, sizeof(float),
	  offset(ydpi), XtRCallProc, (XtPointer)ComputeYdpi },
#undef offset
};

static void Message();
static void Notify();
static void Input();
static void Output();

static void ClassInitialize();
static void ClassPartInitialize();
static void Initialize();
static void Realize();
static void Destroy();
static void Resize();
static Boolean SetValues();
static XtGeometryResult QueryGeometry();

static void Layout();
static Boolean ComputeSize();
static void ChangeSize();
static Boolean Setup();
static void StartInterpreter();
static void StopInterpreter();
static void InterpreterFailed();

static XtActionsRec actions[] =
{
    {"message",	Message},
    {"notify",	Notify},
};

/* notify takes zero to four parameters.  The first two give the width and
 * height of the zoom requested in the default user coordinate system.
 * If they are omitted, a default value of 72 is provided.  If the second
 * parameter is omitted, the zoom area is assumed to be a square.
 * The next two parameters give the desired resolution of the zoom window.
 * If they are omitted, a default value of 300 is provided. If the four
 * parameter is omitted, the y resolution is assumed to be equal to the
 * x resolution.
 */
static char translations[] =
"<Message>:		message()	\n\
<Btn1Down>:		notify(72)	\n\
<Btn2Down>:		notify(108)	\n\
<Btn3Down>:		notify(144)	\n\
";

GhostviewClassRec ghostviewClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) &coreClassRec,
    /* class_name		*/	"Ghostview",
    /* widget_size		*/	sizeof(GhostviewRec),
    /* class_initialize		*/	ClassInitialize,
    /* class_part_initialize	*/	ClassPartInitialize,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	Realize,
    /* actions			*/	actions,
    /* num_actions		*/	XtNumber(actions),
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	Destroy,
    /* resize			*/	Resize,
    /* expose			*/	NULL,
    /* set_values		*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	translations,
    /* query_geometry		*/	QueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  },
  { /* ghostview fields */
    /* ghostview		*/	NULL,
    /* next			*/	NULL,
    /* page			*/	NULL,
    /* done			*/	NULL
  }
};

WidgetClass ghostviewWidgetClass = (WidgetClass)&ghostviewClassRec;

/* Procedures that compute the default xdpi and ydpi from display parameters */

static void
ComputeXdpi(w, offset, value)
    Widget w;
    int offset;
    XrmValue *value;
{
    static float xdpi;
    xdpi = 25.4 * WidthOfScreen(XtScreen(w)) / WidthMMOfScreen(XtScreen(w));
    value->addr = (XtPointer) &xdpi;
}

static void
ComputeYdpi(w, offset, value)
    Widget w;
    int offset;
    XrmValue *value;
{
    static float ydpi;
    ydpi = 25.4 * HeightOfScreen(XtScreen(w)) / HeightMMOfScreen(XtScreen(w));
    value->addr = (XtPointer) &ydpi;
}

/* Message action routine.
 * Passes ghostscript message events back to application via
 * the message callback.  It also marks the interpreter as
 * being not busy at the end of page, and stops the interpreter
 * when it send a "done" message.
 */
static void
Message(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;		/* unused */
    Cardinal *num_params;	/* unused */
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    GhostviewWidgetClass gvc = (GhostviewWidgetClass) XtClass(w);

    gvw->ghostview.mwin = event->xclient.data.l[0];
    if (event->xclient.message_type ==
	XmuInternAtom(XtDisplay(w), gvc->ghostview_class.page)) {
	gvw->ghostview.busy = False;
	XDefineCursor(XtDisplay(gvw), XtWindow(gvw), gvw->ghostview.cursor);
	XtCallCallbackList(w, gvw->ghostview.message_callback, "Page");
    } else if (event->xclient.message_type ==
	       XmuInternAtom(XtDisplay(w), gvc->ghostview_class.done)) {
	StopInterpreter(w);
	XtCallCallbackList(w, gvw->ghostview.message_callback, "Done");
    }
}

/* Notify action routine.
 * Calculates where the user clicked in the default user coordinate system.
 * Call the callbacks with the point of click and size of zoom window
 * requested.
 */
static void
Notify(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    GhostviewReturnStruct ret_val;

    /* notify takes zero to four parameters.  The first two give the width and
     * height of the zoom requested in the default user coordinate system.
     * If they are omitted, a default value of 72 is provided.  If the second
     * parameter is omitted, the zoom area is assumed to be a square.
     * The next two parameters give the desired resolution of the zoom window.
     * If they are omitted, a default value of 300 is provided. If the four
     * parameter is omitted, the y resolution is assumed to be equal to the
     * x resolution.
     */
    switch (*num_params) {
    case 0:
	ret_val.width = ret_val.height = 72;
	ret_val.xdpi = ret_val.ydpi = 300;
	break;
    case 1:
	ret_val.width = ret_val.height = atoi(params[0]);
	ret_val.xdpi = ret_val.ydpi = 300;
	break;
    case 2:
	ret_val.width = atoi(params[0]);
	ret_val.height = atoi(params[1]);
	ret_val.xdpi = ret_val.ydpi = 300;
	break;
    case 3:
	ret_val.width = atoi(params[0]);
	ret_val.height = atoi(params[1]);
	ret_val.xdpi = ret_val.ydpi = atoi(params[2]);
	break;
    default:
	ret_val.width = atoi(params[0]);
	ret_val.height = atoi(params[1]);
	ret_val.xdpi = atoi(params[2]);
	ret_val.ydpi = atoi(params[3]);
	break;
    }

    switch (gvw->ghostview.orientation) {
    case XtPageOrientationPortrait:
	ret_val.psx = gvw->ghostview.llx + 
		      event->xbutton.x * 72.0 / gvw->ghostview.xdpi;
	ret_val.psy = gvw->ghostview.ury - 
		      event->xbutton.y * 72.0 / gvw->ghostview.ydpi;
	break;
    case XtPageOrientationLandscape:
	ret_val.psx = gvw->ghostview.llx + 
		      event->xbutton.y * 72.0 / gvw->ghostview.ydpi;
	ret_val.psy = gvw->ghostview.lly + 
		      event->xbutton.x * 72.0 / gvw->ghostview.xdpi;
	break;
    case XtPageOrientationUpsideDown:
	ret_val.psx = gvw->ghostview.urx - 
		      event->xbutton.x * 72.0 / gvw->ghostview.xdpi;
	ret_val.psy = gvw->ghostview.lly + 
		      event->xbutton.y * 72.0 / gvw->ghostview.ydpi;
	break;
    case XtPageOrientationSeascape:
	ret_val.psx = gvw->ghostview.urx - 
		      event->xbutton.y * 72.0 / gvw->ghostview.ydpi;
	ret_val.psy = gvw->ghostview.ury - 
		      event->xbutton.x * 72.0 / gvw->ghostview.xdpi;
	break;
    }
    XtCallCallbackList(w, gvw->ghostview.callback, (XtPointer) &ret_val);
}

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

static Boolean broken_pipe = False;

static SIGVAL
CatchPipe(i)
    int i;
{
    broken_pipe = True;
#ifdef SIGNALRETURNSINT
    return 0;
#endif
}

#ifndef VMS

/* Input - Feed data to ghostscript's stdin.
 * Write bytes to ghostscript using non-blocking I/O.
 * Also, pipe signals are caught during writing.  The return
 * values are checked and the appropriate action is taken.  I do
 * this at this low level, because it may not be appropriate for
 * SIGPIPE to be caught for the overall application.
 */

static void
Input(client_data, source, id)
    XtPointer client_data;
    int *source;
    XtInputId *id;
{
    Widget w = (Widget) client_data;
    GhostviewWidget gvw = (GhostviewWidget) w;
    int bytes_written;
    SIGVAL (*oldsig)();

    oldsig = signal(SIGPIPE, CatchPipe);

#ifdef NON_BLOCKING_IO
    do {
#endif

	if (gvw->ghostview.buffer_bytes_left == 0) {

	    /* Get a new section if required */
	    if (gvw->ghostview.ps_input && gvw->ghostview.bytes_left == 0) {
		struct record_list *ps_old = gvw->ghostview.ps_input;
		gvw->ghostview.ps_input = ps_old->next;
		if (ps_old->close) fclose(ps_old->fp);
		XtFree((char *)ps_old);
	    }

	    /* Have to seek at the beginning of each section */
	    if (gvw->ghostview.ps_input &&
		gvw->ghostview.ps_input->seek_needed) {
		if (gvw->ghostview.ps_input->len > 0)
		    fseek(gvw->ghostview.ps_input->fp,
			  gvw->ghostview.ps_input->begin, SEEK_SET);
		gvw->ghostview.ps_input->seek_needed = False;
		gvw->ghostview.bytes_left = gvw->ghostview.ps_input->len;
	    }

	    if (gvw->ghostview.bytes_left > GV_BUFSIZ) {
		gvw->ghostview.buffer_bytes_left =
			fread(gvw->ghostview.input_buffer,
			      sizeof (char), GV_BUFSIZ,
			      gvw->ghostview.ps_input->fp);
	    } else if (gvw->ghostview.bytes_left > 0) {
		gvw->ghostview.buffer_bytes_left =
			fread(gvw->ghostview.input_buffer,
			      sizeof (char), gvw->ghostview.bytes_left,
			      gvw->ghostview.ps_input->fp);
	    } else {
		gvw->ghostview.buffer_bytes_left = 0;
	    }
	    if (gvw->ghostview.bytes_left > 0 &&
		gvw->ghostview.buffer_bytes_left == 0) {
		InterpreterFailed(w);	/* Error occurred */
	    }
	    gvw->ghostview.input_buffer_ptr = gvw->ghostview.input_buffer;
	    gvw->ghostview.bytes_left -= gvw->ghostview.buffer_bytes_left;
	}

	if (gvw->ghostview.buffer_bytes_left > 0) {
	    bytes_written = write(gvw->ghostview.interpreter_input,
				  gvw->ghostview.input_buffer_ptr,
				  gvw->ghostview.buffer_bytes_left);

	    if (broken_pipe) {
		broken_pipe = False;
		InterpreterFailed(w);		/* Something bad happened */
	    } else if (bytes_written == -1) {
		if ((errno != EWOULDBLOCK) && (errno != EAGAIN)) {
		    InterpreterFailed(w);	/* Something bad happened */
		}
	    } else {
		gvw->ghostview.buffer_bytes_left -= bytes_written;
		gvw->ghostview.input_buffer_ptr += bytes_written;
	    }
	}
#ifdef NON_BLOCKING_IO
    } while(gvw->ghostview.ps_input &&
	    gvw->ghostview.buffer_bytes_left == 0);
#endif
    signal(SIGPIPE, oldsig);
    if (gvw->ghostview.ps_input == NULL &&
	gvw->ghostview.buffer_bytes_left == 0) {
	if (gvw->ghostview.interpreter_input_id != None) {
	    XtRemoveInput(gvw->ghostview.interpreter_input_id);
	    gvw->ghostview.interpreter_input_id = None;
	}
    }
}

/* Output - receive I/O from ghostscript's stdout and stderr.
 * Pass this to the application via the output_callback. */
static void
Output(client_data, source, id)
    XtPointer client_data;
    int *source;
    XtInputId *id;
{
    Widget w = (Widget) client_data;
    GhostviewWidget gvw = (GhostviewWidget) w;
    char buf[GV_BUFSIZ+1];
    int bytes = 0;

    if (*source == gvw->ghostview.interpreter_output) {
	bytes = read(gvw->ghostview.interpreter_output, buf, GV_BUFSIZ);
	if (bytes == 0) { /* EOF occurred */
	    close(gvw->ghostview.interpreter_output);
	    gvw->ghostview.interpreter_output = -1;
	    XtRemoveInput(gvw->ghostview.interpreter_output_id);
	    return;
	} else if (bytes == -1) {
	    InterpreterFailed(w);		/* Something bad happened */
	    return;
	}
    } else if (*source == gvw->ghostview.interpreter_error) {
	bytes = read(gvw->ghostview.interpreter_error, buf, GV_BUFSIZ);
	if (bytes == 0) { /* EOF occurred */
	    close(gvw->ghostview.interpreter_error);
	    gvw->ghostview.interpreter_error = -1;
	    XtRemoveInput(gvw->ghostview.interpreter_error_id);
	    return;
	} else if (bytes == -1) {
	    InterpreterFailed(w);		/* Something bad happened */
	    return;
	}
    }
    if (bytes > 0) {
	buf[bytes] = '\0';
	XtCallCallbackList(w, gvw->ghostview.output_callback, (XtPointer) buf);
    }
}

#endif /* VMS */

/* Register the type converter required for the PageOrientation. */
/* This routine is called exactly once. */
static void
ClassInitialize()
{
    XtSetTypeConverter(XtRString, XtRPageOrientation,
		       XmuCvtStringToPageOrientation, NULL, 0,
		       XtCacheAll, NULL);
}

/* Get atoms needed to communicate with ghostscript. */
/* This routine is called once per display. */
static void
ClassPartInitialize(class)
    WidgetClass class;
{
    GhostviewWidgetClass gvc = (GhostviewWidgetClass)class;
    gvc->ghostview_class.ghostview = XmuMakeAtom("GHOSTVIEW");
    gvc->ghostview_class.next = XmuMakeAtom("NEXT");
    gvc->ghostview_class.page = XmuMakeAtom("PAGE");
    gvc->ghostview_class.done = XmuMakeAtom("DONE");
}

/* Initialize private state. */

static void
Initialize(request, new, args, num_args)
    Widget request, new;
    ArgList args;		/* unused */
    Cardinal *num_args;	/* unused */
{
    XGCValues	values;
    XtGCMask	mask;
    GhostviewWidget ngvw = (GhostviewWidget) new;
    GhostviewWidget rgvw = (GhostviewWidget) request;

    values.foreground = new->core.background_pixel;
    mask = GCForeground;
    ngvw->ghostview.gc = XtGetGC(new, mask, &values);
    ngvw->ghostview.mwin = None;
    ngvw->ghostview.disable_start = False;
    ngvw->ghostview.interpreter_pid = -1;
    ngvw->ghostview.input_buffer = NULL;
    ngvw->ghostview.bytes_left = 0;
#ifndef VMS
    ngvw->ghostview.input_buffer_ptr = NULL;
    ngvw->ghostview.buffer_bytes_left = 0;
#endif
    ngvw->ghostview.ps_input = NULL;
    ngvw->ghostview.interpreter_input = -1;
    ngvw->ghostview.interpreter_output = -1;
#ifndef VMS
    ngvw->ghostview.interpreter_error = -1;
    ngvw->ghostview.interpreter_input_id = None;
    ngvw->ghostview.interpreter_output_id = None;
    ngvw->ghostview.interpreter_error_id = None;
#else /* VMS */
    memset(ngvw->ghostview.interpreter_input_iosb, 0, 8);
    memset(ngvw->ghostview.interpreter_output_iosb, 0, 8);
    ngvw->ghostview.output_buffer = NULL;
#endif /* VMS */
    ngvw->ghostview.gs_width = 0;
    ngvw->ghostview.gs_height = 0;
    ngvw->ghostview.changed = False;
    ngvw->ghostview.busy = False;

    /* Compute window size */
    Layout(new, (rgvw->core.width == 0), (rgvw->core.height == 0));
}

/* Create Window and start interpreter if needed */
static void
Realize(w, valueMask, attributes)
    Widget w;
    Mask *valueMask;
    XSetWindowAttributes *attributes;
{
    GhostviewWidget gvw = (GhostviewWidget) w;

    if (gvw->ghostview.cursor != None) {
	attributes->cursor = gvw->ghostview.cursor;
	*valueMask |= CWCursor;
    }

    XtCreateWindow(w, (unsigned int) InputOutput, (Visual *) CopyFromParent,
		   *valueMask, attributes);

    Setup(w);
}

/* Destroy routine: kill the interpreter and release the GC */
static void
Destroy(w)
    Widget w;
{
    GhostviewWidget gvw = (GhostviewWidget) w;

    StopInterpreter(w);
    XtReleaseGC(w, gvw->ghostview.gc);
    if (gvw->ghostview.input_buffer) XtFree(gvw->ghostview.input_buffer);
#ifdef VMS
    if (gvw->ghostview.output_buffer) XtFree(gvw->ghostview.output_buffer);
#endif VMS
    if (gvw->core.background_pixmap != XtUnspecifiedPixmap)
	XFreePixmap(XtDisplay(w), gvw->core.background_pixmap);
}

/* Process resize request.  Requested size cannot be changed.
 * NOTE: This routine may be called before the widget is realized.
 * (It was a surprise to me.)
 * If the widget is realized, start a new interpreter by calling Setup().
 * If Setup() actually started a new interpreter and it is taking input
 * from stdin, send a refresh message to the application.  This is the
 * only way that the application can be notified that it needs to resend
 * the input because someone forced a new window size on the widget.
 */
static void
Resize(w)
    Widget w;
{
    Layout(w, False, False);
    if (!XtIsRealized(w)) return;
    if (Setup(w)) {
	GhostviewWidget gvw = (GhostviewWidget) w;
	if (gvw->ghostview.filename == NULL) {
	    XtCallCallbackList(w, gvw->ghostview.message_callback, "Refresh");
	}
    }
}

/* SetValues routine.  Set new private state, based on changed values
 * in the widget.  Always returns False, because redisplay is never needed.
 */
static Boolean
SetValues(current, request, new)
    Widget current, request, new;
{
    GhostviewWidget cgvw = (GhostviewWidget) current;
    GhostviewWidget rgvw = (GhostviewWidget) request;
    GhostviewWidget ngvw = (GhostviewWidget) new;
    String cfilename;
    String rfilename;
    String cpreload;
    String rpreload;

    cfilename = cgvw->ghostview.filename;
    if (cfilename == NULL) cfilename = "(null)";
    rfilename = rgvw->ghostview.filename;
    if (rfilename == NULL) rfilename = "(null)";
    cpreload = cgvw->ghostview.preload;
    if (cpreload == NULL) cpreload = "(null)";
    rpreload = rgvw->ghostview.preload;
    if (rpreload == NULL) rpreload = "(null)";

    if (XtIsRealized(new) && !ngvw->ghostview.busy &&
	(cgvw->ghostview.cursor != ngvw->ghostview.cursor)) {
	XDefineCursor(XtDisplay(new), XtWindow(new), ngvw->ghostview.cursor);
    }
    if (XtIsRealized(new) && ngvw->ghostview.busy &&
	(cgvw->ghostview.busy_cursor != ngvw->ghostview.busy_cursor)) {
	XDefineCursor(XtDisplay(new), XtWindow(new),
		      ngvw->ghostview.busy_cursor);
    }
    if ((cgvw->core.width != rgvw->core.width) ||
	(cgvw->core.height != rgvw->core.height) ||
	strcmp(cgvw->ghostview.interpreter, rgvw->ghostview.interpreter) ||
	strcmp(cpreload, rpreload) ||
	(cgvw->ghostview.quiet != rgvw->ghostview.quiet) ||
	strcmp(cfilename, rfilename) ||
	(cgvw->ghostview.orientation != rgvw->ghostview.orientation) ||
	(cgvw->ghostview.use_bpixmap != rgvw->ghostview.use_bpixmap) ||
	(cgvw->ghostview.xdpi != rgvw->ghostview.xdpi) ||
	(cgvw->ghostview.ydpi != rgvw->ghostview.ydpi) ||
	(cgvw->ghostview.bottom_margin != rgvw->ghostview.bottom_margin) ||
	(cgvw->ghostview.left_margin != rgvw->ghostview.left_margin) ||
	(cgvw->ghostview.right_margin != rgvw->ghostview.right_margin) ||
	(cgvw->ghostview.top_margin != rgvw->ghostview.top_margin) ||
	(cgvw->ghostview.llx != rgvw->ghostview.llx) ||
	(cgvw->ghostview.lly != rgvw->ghostview.lly) ||
	(cgvw->ghostview.urx != rgvw->ghostview.urx) ||
	(cgvw->ghostview.ury != rgvw->ghostview.ury)) {

	ngvw->ghostview.changed = True;
	Layout(new, True, True);
    }

    if (ngvw->ghostview.changed && XtIsRealized(current)) Setup(new);
    return(False);
}

/*	Function Name: QueryGeometry
 *	Description: This tells the parent what size we would like to be
 *                   given certain constraints.
 *	Arguments: w - the widget.
 *                 intended - what the parent intends to do with us.
 *                 requested - what we want to happen.
 */

static XtGeometryResult 
QueryGeometry(w, intended, requested)
Widget w;
XtWidgetGeometry *intended, *requested;
{
    Dimension new_width, new_height;
    Boolean change, width_req, height_req;
    
    width_req = intended->request_mode & CWWidth;
    height_req = intended->request_mode & CWHeight;

    if (width_req)
      new_width = intended->width;
    else
      new_width = w->core.width;

    if (height_req)
      new_height = intended->height;
    else
      new_height = w->core.height;

    requested->request_mode = 0;
    
/*
 * We only care about our height and width.
 */

    if (!width_req && !height_req)
      return(XtGeometryYes);
    
    change = ComputeSize(w, !width_req, !height_req, &new_width, &new_height);

    requested->request_mode |= CWWidth;
    requested->width = new_width;
    requested->request_mode |= CWHeight;
    requested->height = new_height;

    if (change)
        return(XtGeometryAlmost);
    return(XtGeometryYes);
}

/* Layout the widget. */

static void
Layout(w, xfree, yfree)
    Widget w;
    Boolean xfree, yfree;
{
    Dimension width = w->core.width;
    Dimension height = w->core.height;
    Boolean different_size = ComputeSize(w, xfree, yfree, &width, &height);
    if (different_size) ChangeSize(w, width, height);
}

/* Compute new size of window, sets xdpi and ydpi if necessary.
 * returns True if new window size is different */
static Boolean
ComputeSize(w, xfree, yfree, width, height)
    Widget w;
    Boolean xfree, yfree;	/* Am I allowed to change width or height */
    Dimension *width, *height;
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    Dimension new_width = *width;
    Dimension new_height = *height;
    float newxdpi, newydpi;
    Boolean change;

    if (xfree && yfree) {
	/* width and height can be changed, calculate window size according */
	/* to xpdi and ydpi */
	switch (gvw->ghostview.orientation) {
	case XtPageOrientationPortrait:
	case XtPageOrientationUpsideDown:
	    new_width = (gvw->ghostview.urx - gvw->ghostview.llx) / 72.0 *
			 gvw->ghostview.xdpi + 0.5;
	    new_height = (gvw->ghostview.ury - gvw->ghostview.lly) / 72.0 *
			  gvw->ghostview.ydpi + 0.5;
	    break;
	case XtPageOrientationLandscape:
	case XtPageOrientationSeascape:
	    new_width = (gvw->ghostview.ury - gvw->ghostview.lly) / 72.0 *
			 gvw->ghostview.xdpi + 0.5;
	    new_height = (gvw->ghostview.urx - gvw->ghostview.llx) / 72.0 *
			  gvw->ghostview.ydpi + 0.5;
	    break;
	}
    } else if (xfree) {
	/* height is fixed.  Preserve aspect ratio by recomputing */
	/* ydpi and xdpi */
	switch (gvw->ghostview.orientation) {
	case XtPageOrientationPortrait:
	case XtPageOrientationUpsideDown:
	    newydpi = gvw->core.height * 72.0 /
		      (gvw->ghostview.ury - gvw->ghostview.lly);
	    newxdpi = newydpi * gvw->ghostview.xdpi / gvw->ghostview.ydpi;
	    gvw->ghostview.xdpi = newxdpi;
	    gvw->ghostview.ydpi = newydpi;
	    new_width = (gvw->ghostview.urx - gvw->ghostview.llx) / 72.0 *
			 gvw->ghostview.xdpi + 0.5;
	    break;
	case XtPageOrientationLandscape:
	case XtPageOrientationSeascape:
	    newydpi = gvw->core.height * 72.0 /
		      (gvw->ghostview.urx - gvw->ghostview.llx);
	    newxdpi = newydpi * gvw->ghostview.xdpi / gvw->ghostview.ydpi;
	    gvw->ghostview.xdpi = newxdpi;
	    gvw->ghostview.ydpi = newydpi;
	    new_width = (gvw->ghostview.ury - gvw->ghostview.lly) / 72.0 *
			 gvw->ghostview.xdpi + 0.5;
	    break;
	}
    } else if (yfree) {
	/* width is fixed.  Preserve aspect ratio by recomputing */
	/* xdpi and ydpi */
	switch (gvw->ghostview.orientation) {
	case XtPageOrientationPortrait:
	case XtPageOrientationUpsideDown:
	    newxdpi = gvw->core.width * 72.0 /
		      (gvw->ghostview.urx - gvw->ghostview.llx);
	    newydpi = newxdpi * gvw->ghostview.ydpi / gvw->ghostview.xdpi;
	    gvw->ghostview.xdpi = newxdpi;
	    gvw->ghostview.ydpi = newydpi;
	    new_height = (gvw->ghostview.ury - gvw->ghostview.lly) / 72.0 *
			  gvw->ghostview.ydpi + 0.5;
	    break;
	case XtPageOrientationLandscape:
	case XtPageOrientationSeascape:
	    newxdpi = gvw->core.width * 72.0 /
		      (gvw->ghostview.ury - gvw->ghostview.lly);
	    newydpi = newxdpi * gvw->ghostview.ydpi / gvw->ghostview.xdpi;
	    gvw->ghostview.xdpi = newxdpi;
	    gvw->ghostview.ydpi = newydpi;
	    new_height = (gvw->ghostview.urx - gvw->ghostview.llx) / 72.0 *
			  gvw->ghostview.ydpi + 0.5;
	    break;
	}
    } else {
	/* height and width are fixed.  Just have to live with it. */
	switch (gvw->ghostview.orientation) {
	case XtPageOrientationPortrait:
	case XtPageOrientationUpsideDown:
	    gvw->ghostview.xdpi = gvw->core.width * 72.0 /
				  (gvw->ghostview.urx - gvw->ghostview.llx);
	    gvw->ghostview.ydpi = gvw->core.height * 72.0 /
				  (gvw->ghostview.ury - gvw->ghostview.lly);
	    break;
	case XtPageOrientationLandscape:
	case XtPageOrientationSeascape:
	    gvw->ghostview.xdpi = gvw->core.width * 72.0 /
				  (gvw->ghostview.ury - gvw->ghostview.lly);
	    gvw->ghostview.ydpi = gvw->core.height * 72.0 /
				  (gvw->ghostview.urx - gvw->ghostview.llx);
	    break;
	}
    }

    change = (new_width != *width) || (new_height != *height);
    *width = new_width;
    *height = new_height;
    return (change);
}

/*	Function Name: ChangeSize.
 *	Description: Request a size change.
 *	Arguments: w - the widget to try change the size of.
 */

static void
ChangeSize(w, width, height)
Widget w;
Dimension width, height;
{
    XtWidgetGeometry request, reply;
    Boolean changed = False;

    request.request_mode = CWWidth | CWHeight;
    request.width = width;
    request.height = height;
    
    switch ( XtMakeGeometryRequest(w, &request, &reply) ) {
    case XtGeometryYes:
	changed = True;
        break;
    case XtGeometryNo:
        break;
    case XtGeometryAlmost:
	ComputeSize(w, (request.height != reply.height),
		       (request.width != reply.width),
		       &(reply.width), &(reply.height));
	request = reply;
	switch (XtMakeGeometryRequest(w, &request, &reply) ) {
	case XtGeometryYes:
	    changed = True;
	    break;
	case XtGeometryNo:
	    break;
	case XtGeometryAlmost:
	    request = reply;
	    ComputeSize(w, FALSE, FALSE, &(request.width), &(request.height));
	    request.request_mode = CWWidth | CWHeight;
	    XtMakeGeometryRequest(w, &request, &reply);
	    changed = True;
	    break;
	}
	break;
    }

    /* If success, setup the widet for the new size. */
    if (changed && XtIsRealized(w)) Setup(w);
}

/* Catch the alloc error when there is not enough resources for the
 * backing pixmap.  Automatically shut off backing pixmap and let the
 * user know when this happens.
 */
static Boolean alloc_error;
static XErrorHandler oldhandler;

static int
catch_alloc (dpy, err)
Display *dpy;
XErrorEvent *err;
{
    if (err->error_code == BadAlloc) {
	alloc_error = True;
    }
    if (alloc_error) return 0;
    oldhandler(dpy, err);
}

/* Setup - sets up the backing pixmap, and GHOSTVIEW property and
 * starts interpreter if needed.
 * NOTE: the widget must be realized before calling Setup().
 * Returns True if a new interpreter was started, False otherwise.
 */
 
static Boolean
Setup(w)
    Widget w;
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    GhostviewWidgetClass gvc = (GhostviewWidgetClass) XtClass(w);
    char buf[GV_BUFSIZ];
    Pixmap bpixmap;
    XSetWindowAttributes xswa;

    if (!gvw->ghostview.changed &&
	(gvw->core.width == gvw->ghostview.gs_width) &&
	(gvw->core.height == gvw->ghostview.gs_height)) return False;

    StopInterpreter(w);

    if ((gvw->core.width != gvw->ghostview.gs_width) ||
	(gvw->core.height != gvw->ghostview.gs_height) ||
	(!gvw->ghostview.use_bpixmap)) {
	if (gvw->core.background_pixmap != XtUnspecifiedPixmap) {
	    XFreePixmap(XtDisplay(w), gvw->core.background_pixmap);
	    gvw->core.background_pixmap = XtUnspecifiedPixmap;
	    XSetWindowBackgroundPixmap(XtDisplay(w), XtWindow(w), None);
	}
    }

    if (gvw->ghostview.use_bpixmap) {
	if (gvw->core.background_pixmap == XtUnspecifiedPixmap) {
	    /* Get a Backing Pixmap, but be ready for the BadAlloc. */
	    oldhandler = XSetErrorHandler(catch_alloc);
	    alloc_error = False;
	    bpixmap = XCreatePixmap(XtDisplay(w), XtWindow(w),
				    gvw->core.width, gvw->core.height,
				    gvw->core.depth);
	    XSync(XtDisplay(w), False);  /* Force the error */
	    if (alloc_error) {
		XtCallCallbackList(w, gvw->ghostview.message_callback,
				   "BadAlloc");
		if (bpixmap != None) {
		    XFreePixmap(XtDisplay(w), bpixmap);
		    XSync(XtDisplay(w), False);  /* Force the error */
		    bpixmap = None;
		}
	    }
	    oldhandler = XSetErrorHandler(oldhandler);
	    if (bpixmap != None) {
		gvw->core.background_pixmap = bpixmap;
		XSetWindowBackgroundPixmap(XtDisplay(w), XtWindow(w),
					   gvw->core.background_pixmap);
	    }
	} else {
	    bpixmap = gvw->core.background_pixmap;
	}
    } else {
	if (gvw->core.background_pixmap != XtUnspecifiedPixmap) {
	    XFreePixmap(XtDisplay(w), gvw->core.background_pixmap);
	    gvw->core.background_pixmap = XtUnspecifiedPixmap;
	    XSetWindowBackgroundPixmap(XtDisplay(w), XtWindow(w), None);
	}
	bpixmap = None;
    }

    if (bpixmap != None) {
	xswa.backing_store = NotUseful;
	XChangeWindowAttributes(XtDisplay(w), XtWindow(w),
				CWBackingStore, &xswa);
    } else {
	xswa.backing_store = Always;
	XChangeWindowAttributes(XtDisplay(w), XtWindow(w),
				CWBackingStore, &xswa);
    }

    gvw->ghostview.gs_width = gvw->core.width;
    gvw->ghostview.gs_height = gvw->core.height;

    sprintf(buf, "%d %d %d %d %d %d %g %g %d %d %d %d",
	    bpixmap, gvw->ghostview.orientation,
	    gvw->ghostview.llx, gvw->ghostview.lly,
	    gvw->ghostview.urx, gvw->ghostview.ury,
	    gvw->ghostview.xdpi, gvw->ghostview.ydpi,
	    gvw->ghostview.left_margin, gvw->ghostview.bottom_margin,
	    gvw->ghostview.right_margin, gvw->ghostview.top_margin);
    XChangeProperty(XtDisplay(w), XtWindow(w),
		   XmuInternAtom(XtDisplay(w), gvc->ghostview_class.ghostview),
		   XA_STRING, 8, PropModeReplace,
		   (unsigned char *)buf, strlen(buf));

    StartInterpreter(w);
    return True;
}

#ifndef VMS

/* This routine starts the interpreter.  It sets the DISPLAY and 
 * GHOSTVIEW environment variables.  The GHOSTVIEW environment variable
 * contains the Window that ghostscript should write on.
 *
 * This routine also opens pipes for stdout and stderr and initializes
 * application input events for them.  If input to ghostscript is not
 * from a file, a pipe for stdin is created.  This pipe is setup for
 * non-blocking I/O so that the user interface never "hangs" because of
 * a write to ghostscript.
 */
static void
StartInterpreter(w)
    Widget w;
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    int	std_in[2];
    int	std_out[2];
    int	std_err[2];
    char buf[GV_BUFSIZ];
#define NUM_ARGS 100
    char *argv[NUM_ARGS];
    char *preload = NULL;
    char *cptr;
    int argc = 0;
    int ret;

    StopInterpreter(w);

    /* Clear the window before starting a new interpreter. */
    if (gvw->core.background_pixmap != XtUnspecifiedPixmap) {
	XFillRectangle(XtDisplay(w), gvw->core.background_pixmap,
		       gvw->ghostview.gc,
		       0, 0, gvw->core.width, gvw->core.height);
    }
    XClearArea(XtDisplay(w), XtWindow(w),
	       0, 0, gvw->core.width, gvw->core.height, False);

    if (gvw->ghostview.disable_start) return;

    argv[argc++] = gvw->ghostview.interpreter;
    if (gvw->ghostview.quiet) argv[argc++] = "-dQUIET";
    argv[argc++] = "-dNOPAUSE";
    if (gvw->ghostview.preload) {
	cptr = preload = XtNewString(gvw->ghostview.preload);
	while (isspace(*cptr)) cptr++;
	while (*cptr) {
	    argv[argc++] = cptr;
	    while (*cptr && !isspace(*cptr)) cptr++;
	    *cptr++ = '\0';
	    if (argc + 2 >= NUM_ARGS) {
		fprintf(stderr, "Too many files to preload.\n");
		exit(1);
	    }
	    while (isspace(*cptr)) cptr++;
	}
    }
    argv[argc++] = "-";
    argv[argc++] = NULL;

    if (gvw->ghostview.filename == NULL) {
	ret = pipe(std_in);
	if (ret == -1) {
	    perror("Could not create pipe");
	    exit(1);
	}
    } else if (strcmp(gvw->ghostview.filename, "-")) {
	std_in[0] = open(gvw->ghostview.filename, O_RDONLY, 0);
    }
    ret = pipe(std_out);
    if (ret == -1) {
	perror("Could not create pipe");
	exit(1);
    }
    ret = pipe(std_err);
    if (ret == -1) {
	perror("Could not create pipe");
	exit(1);
    }

    gvw->ghostview.changed = False;
    gvw->ghostview.busy = True;
    XDefineCursor(XtDisplay(gvw), XtWindow(gvw), gvw->ghostview.busy_cursor);
#if defined(SYSV) || defined(USG)
#define vfork fork
#endif
    gvw->ghostview.interpreter_pid = vfork();

    if (gvw->ghostview.interpreter_pid == 0) { /* child */
	close(std_out[0]);
	close(std_err[0]);
	dup2(std_out[1], 1);
	close(std_out[1]);
	dup2(std_err[1], 2);
	close(std_err[1]);
	sprintf(buf, "%d", XtWindow(w));
	setenv("GHOSTVIEW", buf, True);
	setenv("DISPLAY", XDisplayString(XtDisplay(w)), True);
	if (gvw->ghostview.filename == NULL) {
	    close(std_in[1]);
	    dup2(std_in[0], 0);
	    close(std_in[0]);
	} else if (strcmp(gvw->ghostview.filename, "-")) {
	    dup2(std_in[0], 0);
	    close(std_in[0]);
	}
	execvp(argv[0], argv);
	sprintf(buf, "Exec of %s failed", argv[0]);
	perror(buf);
	_exit(1);
    } else {
	if (gvw->ghostview.filename == NULL) {
#ifdef NON_BLOCKING_IO
	    int result;
#endif
	    close(std_in[0]);

#ifdef NON_BLOCKING_IO
	    result = fcntl(std_in[1], F_GETFL, 0);
	    result = result | O_NONBLOCK;
	    result = fcntl(std_in[1], F_SETFL, result);
#endif
	    gvw->ghostview.interpreter_input = std_in[1];
	    gvw->ghostview.interpreter_input_id = None;
	} else if (strcmp(gvw->ghostview.filename, "-")) {
	    close(std_in[0]);
	}
	close(std_out[1]);
	gvw->ghostview.interpreter_output = std_out[0];
	gvw->ghostview.interpreter_output_id = 
	    XtAppAddInput(XtWidgetToApplicationContext(w), std_out[0],
			  (XtPointer)XtInputReadMask, Output, (XtPointer)w);
	close(std_err[1]);
	gvw->ghostview.interpreter_error = std_err[0];
	gvw->ghostview.interpreter_error_id = 
	    XtAppAddInput(XtWidgetToApplicationContext(w), std_err[0],
			  (XtPointer)XtInputReadMask, Output, (XtPointer)w);
    }
    if (preload) XtFree(preload);
}

/* Stop the interperter, if present, and remove any Input sources. */
/* Also reset the busy state. */
static void
StopInterpreter(w)
    Widget w;
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    if (gvw->ghostview.interpreter_pid >= 0) {
	kill(gvw->ghostview.interpreter_pid, SIGTERM);
	wait(0);
	gvw->ghostview.interpreter_pid = -1;
    }
    if (gvw->ghostview.interpreter_input >= 0) {
	close(gvw->ghostview.interpreter_input);
	gvw->ghostview.interpreter_input = -1;
	if (gvw->ghostview.interpreter_input_id != None) {
	    XtRemoveInput(gvw->ghostview.interpreter_input_id);
	    gvw->ghostview.interpreter_input_id = None;
	}
	while (gvw->ghostview.ps_input) {
	    struct record_list *ps_old = gvw->ghostview.ps_input;
	    gvw->ghostview.ps_input = ps_old->next;
	    if (ps_old->close) fclose(ps_old->fp);
	    XtFree((char *)ps_old);
	}
    }
    if (gvw->ghostview.interpreter_output >= 0) {
	close(gvw->ghostview.interpreter_output);
	gvw->ghostview.interpreter_output = -1;
	XtRemoveInput(gvw->ghostview.interpreter_output_id);
    }
    if (gvw->ghostview.interpreter_error >= 0) {
	close(gvw->ghostview.interpreter_error);
	gvw->ghostview.interpreter_error = -1;
	XtRemoveInput(gvw->ghostview.interpreter_error_id);
    }
    gvw->ghostview.busy = False;
    XDefineCursor(XtDisplay(gvw), XtWindow(gvw), gvw->ghostview.cursor);
}

#endif /* VMS */

/* The interpeter failed, Stop what's left and notify application */
static void
InterpreterFailed(w)
    Widget w;
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    StopInterpreter(w);
    XtCallCallbackList(w, gvw->ghostview.message_callback, "Failed");
}

/*
 *	Public Routines
 */

/* GhostviewDisableInterpreter:
 * Stop any interpreter and disable new ones from starting.
 */
void
GhostviewDisableInterpreter(w)
    Widget w;
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    gvw->ghostview.disable_start = True;
    if (XtIsRealized(w)) StopInterpreter(w);
}

/* GhostviewDisableInterpreter:
 * Allow an interpreter to start and start one if the widget is
 * currently realized.
 */
void
GhostviewEnableInterpreter(w)
    Widget w;
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    gvw->ghostview.disable_start = False;
    if (XtIsRealized(w)) StartInterpreter(w);
}

/* GhostviewIsInterpreterReady:
 * Returns True if the interpreter is ready for new input.
 */
Boolean
GhostviewIsInterpreterReady(w)
    Widget w;
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    return gvw->ghostview.interpreter_pid != -1 &&
	   !gvw->ghostview.busy &&
	   gvw->ghostview.ps_input == NULL;
}

/* GhostviewIsInterpreterRunning:
 * Returns True if the interpreter is running.
 */
Boolean
GhostviewIsInterpreterRunning(w)
    Widget w;
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    return gvw->ghostview.interpreter_pid != -1;
}

/* GhostviewGetBackingPixmap:
 * Returns the current backing pixmap.
 */
Pixmap
GhostviewGetBackingPixmap(w)
    Widget w;
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    if (gvw->core.background_pixmap != XtUnspecifiedPixmap)
	return(gvw->core.background_pixmap);
    else
	return(None);
}

#ifndef VMS

/* GhostviewSendPS:
 *   Queue a portion of a PostScript file for output to ghostscript.
 *   fp: FILE * of the file in question.  NOTE: if you have several
 *   Ghostview widgets reading from the same file.  You must open
 *   a unique FILE * for each widget.
 *   SendPS does not actually send the PostScript, it merely queues it
 *   for output.
 *   begin: position in file (returned from ftell()) to start.
 *   len:   number of bytes to write.
 *
 *   If an interpreter is not running, nothing is queued and
 *   False is returned.
 */
Boolean
GhostviewSendPS(w, fp, begin, len, close)
    Widget w;
    FILE *fp;
    long begin;
    unsigned int len;
    Bool close;
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    struct record_list *ps_new;

    if (gvw->ghostview.interpreter_input < 0) return False;
    ps_new = (struct record_list *) XtMalloc(sizeof (struct record_list));
    ps_new->fp = fp;
    ps_new->begin = begin;
    ps_new->len = len;
    ps_new->seek_needed = True;
    ps_new->close = close;
    ps_new->next = NULL;

    if (gvw->ghostview.input_buffer == NULL) {
	gvw->ghostview.input_buffer = XtMalloc(GV_BUFSIZ);
    }

    if (gvw->ghostview.ps_input == NULL) {
	gvw->ghostview.input_buffer_ptr = gvw->ghostview.input_buffer;
	gvw->ghostview.bytes_left = len;
	gvw->ghostview.buffer_bytes_left = 0;
	gvw->ghostview.ps_input = ps_new;
	gvw->ghostview.interpreter_input_id =
		XtAppAddInput(XtWidgetToApplicationContext(w),
			      gvw->ghostview.interpreter_input,
			      (XtPointer)XtInputWriteMask, Input, (XtPointer)w);
    } else {
	struct record_list *p = gvw->ghostview.ps_input;
	while (p->next != NULL) {
	    p = p->next;
	}
	p->next = ps_new;
    }
    return True;
}

#endif /* VMS */

/* GhostviewNextPage:
 *   Tell ghostscript to start the next page.
 *   Returns False if ghostscript is not running, or not ready to start
 *   another page.
 *   If another page is started.  Sets the busy flag and cursor.
 */
Boolean
GhostviewNextPage(w)
    Widget w;
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    GhostviewWidgetClass gvc = (GhostviewWidgetClass) XtClass(w);
    XEvent event;

    if (gvw->ghostview.interpreter_pid < 0) return False;
    if (gvw->ghostview.mwin == None) return False;

    if (!gvw->ghostview.busy) {
	gvw->ghostview.busy = True;
	XDefineCursor(XtDisplay(gvw), XtWindow(gvw),
		      gvw->ghostview.busy_cursor);

	event.xclient.type = ClientMessage;
	event.xclient.display = XtDisplay(w);
	event.xclient.window = gvw->ghostview.mwin;
	event.xclient.message_type =
		XmuInternAtom(XtDisplay(w), gvc->ghostview_class.next);
	event.xclient.format = 32;
	XSendEvent(XtDisplay(w), gvw->ghostview.mwin, False, 0, &event);
	return True;
    } else {
	return False;
    }
}

#define	done(type, value) \
	{							\
	    if (toVal->addr != NULL) {				\
		if (toVal->size < sizeof(type)) {		\
		    toVal->size = sizeof(type);			\
		    return False;				\
		}						\
		*(type*)(toVal->addr) = (value);		\
	    }							\
	    else {						\
		static type static_val;				\
		static_val = (value);				\
		toVal->addr = (XPointer)&static_val;		\
	    }							\
	    toVal->size = sizeof(type);				\
	    return True;					\
	}

/* PageOrienation Conversion Routine.
 * Returns True if Conversion is successful.
 */
Boolean
XmuCvtStringToPageOrientation(dpy, args, num_args, fromVal, toVal, data)
    Display	*dpy;
    XrmValue	*args;		/* unused */
    Cardinal	*num_args;	/* unused */
    XrmValue	*fromVal;
    XrmValue	*toVal;
    XtPointer	*data;		/* unused */
{
    static XrmQuark		XrmQEportrait;
    static XrmQuark		XrmQElandscape;
    static XrmQuark		XrmQEupsideDown;
    static XrmQuark		XrmQEseascape;
    static int			haveQuarks;
    XrmQuark    q;
    char	*str = (XPointer) fromVal->addr;
    char        lowerName[1000];

    if (str == NULL) return False;

    if (!haveQuarks) {
	XrmQEportrait   = XrmStringToQuark(XtEportrait);
	XrmQElandscape  = XrmStringToQuark(XtElandscape);
	XrmQEupsideDown = XrmStringToQuark(XtEupsideDown);
	XrmQEseascape   = XrmStringToQuark(XtEseascape);
	haveQuarks = 1;
    }

    XmuCopyISOLatin1Lowered(lowerName, str);

    q = XrmStringToQuark(lowerName);

    if (q == XrmQEportrait)
	done(XtPageOrientation, XtPageOrientationPortrait);
    if (q == XrmQElandscape)
	done(XtPageOrientation, XtPageOrientationLandscape);
    if (q == XrmQEupsideDown)
	done(XtPageOrientation, XtPageOrientationUpsideDown);
    if (q == XrmQEseascape)
	done(XtPageOrientation, XtPageOrientationSeascape);

    XtDisplayStringConversionWarning(dpy, str, XtRPageOrientation);
    return False;
}

#ifdef VMS

/*
** VMS specific include files
*/
#include <descrip.h>
#include <ssdef.h>
#include <clidef.h>
#include <lnmdef.h>
#include <iodef.h>
#include <dvidef.h>
#include "vms_types.h"

#define ERR_SIGNAL(s) if(!((s) & 1))lib$signal((s), 0, 0)
#define XtEFN 23

struct g_l_i
{
    GhostviewWidget w;
    struct g_l_i *next;
};

typedef struct g_l_i GhostListItem, *GLI_p;

static GhostListItem glhead = {(GhostviewWidget) -1, NULL};
static GLI_p GL = &glhead;
static size_t GLI_Size = sizeof(GhostListItem);
static XtInputId EventId;

/*
** This routine is passed to XtAppAddInput(). It is called whenever the event
** flag number XtEFN is set and the Xt main loop becomes idle. It clears the
** event flag and then scans all the ghostview widgets for completed I/O
** requests, processing each as they are found. We have to do them all because
** there is no way to have Xt dispatch them individually without a window of
** vulnerability that can cause missed events, or by using a separate event
** flag for each I/O stream. Event flags are, unfortunately, a limited
** resource.
*/
static Boolean
IOProcess()
{
    GhostviewWidget gvw;
    GLI_p cur;

    /*
    ** Before we process any I/O's, clear the event flag.
    */
    sys$clref(XtEFN);
    /*
    ** Scan all the ghostview widgets and check for completed I/O's
    */
    for(cur = GL->next; cur; cur = cur->next){
	/*
	** Get the widget and check for I/O complete on either mailbox.
	*/
	gvw = cur->w;
	if(gvw->ghostview.interpreter_input_iosb[0])Input(gvw);
	if(gvw->ghostview.interpreter_output_iosb[0])Output(gvw);
    }
}
    
/*
** This is an AST routine. It is called asynchronously whenever one of our
** mailbox I/O's completes.
*/
static void
IOComplete(client_data)
    XtPointer client_data;
{
    /*
    ** Set the event flag to tell Xt to call IOProcess.
    */
    sys$setef(XtEFN);
}

static void
GLInsert(w)
    GhostviewWidget w;
{
    GLI_p new;
    int first;
    
    /*
    ** Insert this widget after the list head
    */
    first = (GL->next == NULL);
    new = XtMalloc(GLI_Size);
    new->w = w;
    new->next = GL->next;
    GL->next = new;
    /*
    ** If this is the first item on the list, call XtAppAddInput()
    */
    if(first)EventId = XtAppAddInput(XtWidgetToApplicationContext(w), XtEFN, 0, 
	IOProcess, 0);
}

static void
GLRemove(w)
    GhostviewWidget w;
{
    GLI_p prev, cur;
    int last = 0;

    /*
    ** Find and remove this widget from the list.
    */
    prev = GL;
    cur = prev->next;
    while(cur && cur->w != w){
	prev = cur;
	cur = cur->next;
    }
    if(cur){
       prev->next = cur->next;
       XtFree(cur);
       last = (GL->next == NULL);
    }
    /*
    ** If this was the last item on the list, call XtRemoveInput()
    */
    if(last)XtRemoveInput(EventId);
}

/* Input - Feed data to ghostscript's stdin.
 * Write bytes to ghostscript using non-blocking I/O.
 * Also, pipe signals are caught during writing.  The return
 * values are checked and the appropriate action is taken.  I do
 * this at this low level, because it may not be appropriate for
 * SIGPIPE to be caught for the overall application.
 */

static void
Input(gvw)
    GhostviewWidget gvw;
{
    int stat, bbytes;
    char *ch;

    /*
    ** Check for error on previous I/O.
    */
    stat = gvw->ghostview.interpreter_input_iosb[0];
    if(stat != SS$_NORMAL){
	InterpreterFailed(gvw);
    } else {

	/* Get a new section if required */
	
	if (gvw->ghostview.ps_input && gvw->ghostview.bytes_left == 0) {
	    struct record_list *ps_old = gvw->ghostview.ps_input;
	    gvw->ghostview.ps_input = ps_old->next;
	    if (ps_old->close) fclose(ps_old->fp);
	    XtFree((char *)ps_old);
	}
	if(gvw->ghostview.ps_input){
	    /* Have to seek at the beginning of each section */
	    if (gvw->ghostview.ps_input->seek_needed) {
		if (gvw->ghostview.ps_input->len > 0)
		    fseek(gvw->ghostview.ps_input->fp,
			  gvw->ghostview.ps_input->begin, SEEK_SET);
		gvw->ghostview.ps_input->seek_needed = False;
		gvw->ghostview.bytes_left = gvw->ghostview.ps_input->len;
	    }
	    /*
	    ** Read a line from the file.
	    */
	    ch = fgets(gvw->ghostview.input_buffer, GV_BUFSIZ,
		gvw->ghostview.ps_input->fp);
	    if(!ch){
		/*
		** Error, EOF when there's supposed to be data left. 
		*/
		InterpreterFailed(gvw);
	    } else {
		/*
		** Write it to the mailbox.
		*/
		bbytes = strlen(gvw->ghostview.input_buffer);
		gvw->ghostview.bytes_left -= bbytes;
		stat = sys$qio(0, (short)gvw->ghostview.interpreter_input,
		    IO$_WRITEVBLK, &gvw->ghostview.interpreter_input_iosb,
		    IOComplete, 0, gvw->ghostview.input_buffer, bbytes,
		    0, 0, 0, 0);
		ERR_SIGNAL(stat);
	    }
	}
    }
}

/* Output - receive I/O from ghostscript's stdout and stderr.
 * Pass this to the application via the output_callback. */
static void
Output(gvw)
    GhostviewWidget gvw;
{
    char buf[GV_BUFSIZ+1];
    int bytes, stat;

    stat = gvw->ghostview.interpreter_output_iosb[0];
    bytes = gvw->ghostview.interpreter_output_iosb[1];
    if (stat == SS$_NORMAL) {
	/*
	** Got a message. If line complete, pass to the output_callback.
	**
	** HACK ALERT, if bytes is -1 nothing happens, but an I/O is queued.
	** This is our first time code, since Xt doesn't queue the I/O for us
	** under VMS, just watches for completion. In StartInterpreter We setup
	** an IOSB with a success status and -1 bytes so Xt will call us the
	** first time to get the I/O queued.
	*/
	if (bytes == 0) {
	    strcpy(buf, "\n");
	} else if (bytes == 1) {
	    buf[0] = gvw->ghostview.output_buffer[0];
	    buf[1] = '\0';
	} else if (bytes > 1) {
	    /*
	    ** Copy the message to a local buffer and pass it to the callback.
	    */
	    memcpy(buf, gvw->ghostview.output_buffer, bytes);
	    buf[bytes] = '\0';
	}
	if(bytes >= 0)XtCallCallbackList(gvw, gvw->ghostview.output_callback,
	    (XtPointer) buf);
	/*
	** Queue a new read to the mailbox
	*/
	stat = sys$qio(0, (short)gvw->ghostview.interpreter_output,
	    IO$_READVBLK, &gvw->ghostview.interpreter_output_iosb, IOComplete,
	    0, gvw->ghostview.output_buffer, GV_BUFSIZ, 0, 0, 0, 0);
	ERR_SIGNAL(stat);
    } else {
	InterpreterFailed(gvw);		/* Something bad happened */
    }
}

/* This routine starts the interpreter.  It sets the DISPLAY and 
 * GHOSTVIEW environment variables.  The GHOSTVIEW environment variable
 * contains the Window that ghostscript should write on.
 *
 * This routine also opens pipes for stdout and stderr and initializes
 * application input events for them.  If input to ghostscript is not
 * from a file, a pipe for stdin is created.  This pipe is setup for
 * non-blocking I/O so that the user interface never "hangs" because of
 * a write to ghostscript.
 */
static void
StartInterpreter(w)
    Widget w;
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    char buf[GV_BUFSIZ];
    char cmd[512];
    int ret;
    short ch1, ch2;
    char in_mbx_name[65], out_mbx_name[65];
    long pid, nowait = CLI$M_NOWAIT;
    const $DESCRIPTOR(ghostview_desc, "GHOSTVIEW");
    const $DESCRIPTOR(display_desc, "DECW$DISPLAY");
    const $DESCRIPTOR(lnt_desc, "LNM$PROCESS");
    $DESCRIPTOR(in_desc, "");
    $DESCRIPTOR(out_desc, "");
    $DESCRIPTOR(lnm_desc, "");
    $DESCRIPTOR(cmd_desc, cmd);
    ITEM_LIST_3_T(gv_list, 1) = {{{0, LNM$_STRING, buf, NULL}}, 0};
    ITEM_LIST_3_T(dis_list, 1) = {{{0, LNM$_STRING, NULL, NULL}}, 0};
    ITEM_LIST_3_T(dvi_list, 1) = {{{64, DVI$_DEVNAM, NULL, NULL}}, 0};
    IOSB_GET_T dvi_iosb;

    /*
    ** Stop interpreter if running
    */
    StopInterpreter(w);
    /*
    ** Clear the window before starting a new interpreter.
    */
    if (gvw->core.background_pixmap != XtUnspecifiedPixmap) {
	XFillRectangle(XtDisplay(w), gvw->core.background_pixmap,
		       gvw->ghostview.gc,
		       0, 0, gvw->core.width, gvw->core.height);
    }
    XClearArea(XtDisplay(w), XtWindow(w),
	       0, 0, gvw->core.width, gvw->core.height, False);
    /*
    ** Check for disabled.
    */
    if (gvw->ghostview.disable_start) return;
    /*
    ** Build Ghostscript startup command
    */
    strcpy(cmd, gvw->ghostview.interpreter);
    strcat(cmd, " ");
    if (gvw->ghostview.quiet) strcat(cmd, "\"-dQUIET\" ");
    strcat(cmd, "\"-dNOPAUSE\" ");
    if (gvw->ghostview.preload) {
	strcat(cmd, gvw->ghostview.preload);
	strcat(cmd, " ");
    }
    strcat(cmd, "\"-\" ");

    /*
    ** Determine input source.
    */
    if (gvw->ghostview.filename == NULL) {
	/*
	** Create a mailbox to feed input to Ghostscript and get its name.
	*/
	ret = sys$crembx(0, &ch1, GV_BUFSIZ, GV_BUFSIZ, 0, 0, 0, 0);
	ERR_SIGNAL(ret);
	dvi_list.item[0].buffer_p = in_mbx_name;
	ret = sys$getdvi(0, ch1, 0, &dvi_list, &dvi_iosb, 0, 0, 0);
	ERR_SIGNAL(ret); ERR_SIGNAL(dvi_iosb.status);
	in_mbx_name[64] = '\0';
	in_desc.dsc$a_pointer = in_mbx_name;
	in_desc.dsc$w_length = strlen(in_mbx_name);
    } else {
	/*
	** Set up file name to give Ghostscript as standard input.
	*/
	in_desc.dsc$a_pointer = gvw->ghostview.filename;
	in_desc.dsc$w_length = strlen(gvw->ghostview.filename);
    }
    /*
    ** Create mailbox to receive Ghostscript's output
    */
    ret = sys$crembx(0, &ch2, GV_BUFSIZ, GV_BUFSIZ, 0, 0, 0, 0);
    ERR_SIGNAL(ret);
    dvi_list.item[0].buffer_p = out_mbx_name;
    ret = sys$getdvi(0, ch2, 0, &dvi_list, &dvi_iosb, 0, 0, 0);
    ERR_SIGNAL(ret); ERR_SIGNAL(dvi_iosb.status);
    out_mbx_name[64] = '\0';
    out_desc.dsc$a_pointer = out_mbx_name;
    out_desc.dsc$w_length = strlen(out_mbx_name);
    /*
    ** Create GHOSTVIEW and DECW$DISPLAY logical names.
    **
    ** We use CRELNM rather than LIB$SET_LOGICAL because we want these to be
    ** user mode and go away when the program exits. It doesn't matter that we
    ** may set them multiple times, as with the mailbox logicals, since once
    ** Ghostscript starts we don't need them any more.
    */
    sprintf(buf, "%d", XtWindow(w));
    gv_list.item[0].buffer_size = strlen(buf);
    ret = sys$crelnm(0, &lnt_desc, &ghostview_desc, 0, &gv_list);
    ERR_SIGNAL(ret);
    dis_list.item[0].buffer_p = XDisplayString(XtDisplay(w));
    dis_list.item[0].buffer_size = strlen(dis_list.item[0].buffer_p);
    ret = sys$crelnm(0, &lnt_desc, &display_desc, 0, &dis_list);
    ERR_SIGNAL(ret);
    /*
    ** Spawn Ghostscript process
    */
    gvw->ghostview.changed = False;
    gvw->ghostview.busy = True;
    cmd_desc.dsc$w_length = strlen(cmd);
    ret = lib$spawn(&cmd_desc, &in_desc, &out_desc, &nowait, 0, &pid, 0, 0,
	0, 0, 0, 0, 0);
    ERR_SIGNAL(ret);
    XDefineCursor(XtDisplay(gvw), XtWindow(gvw), gvw->ghostview.busy_cursor);
    /*
    ** Everything worked, initialize IOSBs and save info about interpretter.
    */
    gvw->ghostview.interpreter_pid = pid;
    if (gvw->ghostview.filename == NULL) {
	gvw->ghostview.interpreter_input = ch1;
	gvw->ghostview.interpreter_input_iosb[0] = 0;
    }
    gvw->ghostview.interpreter_output = ch2;
    if (gvw->ghostview.output_buffer == NULL) {
	gvw->ghostview.output_buffer = XtMalloc(GV_BUFSIZ);
    }
    GLInsert(gvw);
    /*
    ** Fake a completed I/O so Output will get called to queue the first I/O.
    */
    gvw->ghostview.interpreter_output_iosb[0] = SS$_NORMAL;
    gvw->ghostview.interpreter_output_iosb[1] = -1;
    IOComplete();
}

/* Stop the interperter, if present, and remove any Input sources. */
/* Also reset the busy state. */
static void
StopInterpreter(w)
    Widget w;
{
    int ret;
    
    GhostviewWidget gvw = (GhostviewWidget) w;
    if (gvw->ghostview.interpreter_pid >= 0) {
	ret = sys$delprc(&gvw->ghostview.interpreter_pid, 0);
	if(ret != SS$_NORMAL && ret != SS$_NONEXPR)lib$signal(ret, 0, 0);
	gvw->ghostview.interpreter_pid = -1;
    }
    if (gvw->ghostview.interpreter_input >= 0) {
	(void) sys$dassgn(gvw->ghostview.interpreter_input);
	gvw->ghostview.interpreter_input = -1;
	while (gvw->ghostview.ps_input) {
	    struct record_list *ps_old = gvw->ghostview.ps_input;
	    gvw->ghostview.ps_input = ps_old->next;
	    if (ps_old->close) fclose(ps_old->fp);
	    XtFree((char *)ps_old);
	}
    }
    if (gvw->ghostview.interpreter_output >= 0) {
	(void) sys$dassgn(gvw->ghostview.interpreter_output);
	gvw->ghostview.interpreter_output = -1;
    }
    gvw->ghostview.busy = False;
    XDefineCursor(XtDisplay(gvw), XtWindow(gvw), gvw->ghostview.cursor);
    GLRemove(gvw);
}

/* GhostviewSendPS:
 *   Queue a portion of a PostScript file for output to ghostscript.
 *   fp: FILE * of the file in question.  NOTE: if you have several
 *   Ghostview widgets reading from the same file.  You must open
 *   a unique FILE * for each widget.
 *   SendPS does not actually send the PostScript, it merely queues it
 *   for output.
 *   begin: position in file (returned from ftell()) to start.
 *   len:   number of bytes to write.
 *
 *   If an interpreter is not running, nothing is queued and
 *   False is returned.
 */
Boolean
GhostviewSendPS(w, fp, begin, len, close)
    Widget w;
    FILE *fp;
    long begin;
    unsigned int len;
    Bool close;
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    struct record_list *ps_new;

    if (gvw->ghostview.interpreter_input < 0) return False;
    if(len != 0){
	ps_new = (struct record_list *) XtMalloc(sizeof (struct record_list));
	ps_new->fp = fp;
	ps_new->begin = begin;
	ps_new->len = len;
	ps_new->seek_needed = True;
	ps_new->close = close;
	ps_new->next = NULL;

	if (gvw->ghostview.input_buffer == NULL) {
	    gvw->ghostview.input_buffer = XtMalloc(GV_BUFSIZ);
	}

	if (gvw->ghostview.ps_input == NULL) {
	    gvw->ghostview.bytes_left = len;
	    gvw->ghostview.ps_input = ps_new;
	    /*
	    ** Fake a completed I/O so Input will get called to queue the
	    ** first I/O.
	    */
	    gvw->ghostview.interpreter_input_iosb[0] = SS$_NORMAL;
	    gvw->ghostview.interpreter_input_iosb[1] = -1;
	    IOComplete();
	} else {
	    struct record_list *p = gvw->ghostview.ps_input;
	    while (p->next != NULL) {
		p = p->next;
	    }
	    p->next = ps_new;
	}
    }
    return True;
}
#endif /* VMS */
