/*
 * Ghostview.h -- Public header file for Ghostview widget.
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

#ifndef _Ghostview_h
#define _Ghostview_h
/* Be sure that FILE* is defined */
#include <stdio.h>

/****************************************************************
 *
 * Ghostview widget
 *
 ****************************************************************/

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		XtDefaultBackground
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 bottomMargin	     Margin		int		0
 busyCursor	     Cursor		Cursor		watch
 callback	     Callback		Pointer		NULL
 cursor		     Cursor		Cursor		crosshair
 destroyCallback     Callback		Pointer		NULL
 filename	     Filename		String		"-"
 height		     Height		Dimension	0
 interpreter	     Interpreter	String		"gs"
 leftMargin	     Margin		int		0
 llx		     BoundingBox	Int		0
 lly		     BoundingBox	Int		0
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 messageCallback     Callback		Pointer		NULL
 orientation	     Orientation	PageOrientation	Portrait
 outputCallback      Callback		Pointer		NULL
 preload	     Preload		String		NULL
 quiet		     Quiet		Boolean		True
 rightMargin	     Margin		int		0
 topMargin	     Margin		int		0
 urx		     BoundingBox	Int		612
 ury		     BoundingBox	Int		792
 useBackingPixmap    UseBackingPixmap	Boolean		True
 width		     Width		Dimension	0
 x		     Position		Position	0
 xdpi		     Resolution		Float		**
 y		     Position		Position	0
 ydpi		     Resolution		Float		**

 ** automatically calculated from display parameters if width and
    height are not set.

*/

/* define any special resource names here that are not in <X11/StringDefs.h> */

#define XtNbusyCursor "busyCursor"
#define XtNcursor "cursor"
#define XtNfilename "filename"
#define XtNinterpreter "interpreter"
#define XtNmessageCallback "messageCallback"
#define XtNoutputCallback "outputCallback"
#define XtNpreload "preload"
#define XtNquiet "quiet"
#define XtNllx "llx"
#define XtNlly "lly"
#define XtNurx "urx"
#define XtNury "ury"
#define XtNuseBackingPixmap "useBackingPixmap"
#define XtNxdpi "xdpi"
#define XtNydpi "ydpi"
#define XtNrightMargin "rightMargin"
#define XtNleftMargin "leftMargin"
#define XtNbottomMargin "bottomMargin"
#define XtNtopMargin "topMargin"

#define XtCBoundingBox "BoundingBox"
#define XtCFilename "Filename"
#define XtCInterpreter "Interpreter"
#define XtCPreload "Preload"
#define XtCQuiet "Quiet"
#define XtCResolution "Resolution"
#define XtCUseBackingPixmap "UseBackingPixmap"

#define XtRPageOrientation "PageOrientation"

/******************************************************************************
 * XmuCvtStringToPageOrientation
 */
/* Number represents clockwise rotation of the paper in degrees */
typedef enum {
    XtPageOrientationPortrait	=   0,	/* Normal portrait orientation */
    XtPageOrientationLandscape	=  90,	/* Normal landscape orientation */
    XtPageOrientationUpsideDown = 180,	/* Don't think this will be used much */
    XtPageOrientationSeascape	= 270	/* Landscape rotated the other way */
} XtPageOrientation;
#define XtEportrait "portrait"
#define XtElandscape "landscape"
#define XtEupsideDown "upside-down"
#define XtEseascape "seascape"
#define XtRPageOrientation "PageOrientation"
extern Boolean XmuCvtStringToPageOrientation();

/* declare specific GhostviewWidget class and instance datatypes */

typedef struct _GhostviewClassRec*	GhostviewWidgetClass;
typedef struct _GhostviewRec*		GhostviewWidget;

/* declare the class constant */

extern WidgetClass ghostviewWidgetClass;

/* Public routines */

extern void GhostviewEnableInterpreter(
#if NeedFunctionPrototypes
    Widget	/* w */
#endif
);

extern void GhostviewDisableInterpreter(
#if NeedFunctionPrototypes
    Widget	/* w */
#endif
);

extern Boolean GhostviewIsInterpreterReady(
#if NeedFunctionPrototypes
    Widget	/* w */
#endif
);

extern Boolean GhostviewIsInterpreterRunning(
#if NeedFunctionPrototypes
    Widget	/* w */
#endif
);

extern Pixmap GhostviewGetBackingPixmap(
#if NeedFunctionPrototypes
    Widget	/* w */
#endif
);

extern Boolean GhostviewSendPS(
#if NeedFunctionPrototypes
    Widget	/* widget */,
    FILE*	/* fp */,
    long	/* begin */,
    unsigned int	/* len */,
    Bool	/* close */
#endif
);

extern Boolean GhostviewNextPage(
#if NeedFunctionPrototypes
    Widget	/* w */
#endif
);

/* The structure returned by the regular callback */

typedef struct _GhostviewReturnStruct {
    int width, height;
    int	psx, psy;
    float xdpi, ydpi;
} GhostviewReturnStruct;

#endif /* _Ghostview_h */
