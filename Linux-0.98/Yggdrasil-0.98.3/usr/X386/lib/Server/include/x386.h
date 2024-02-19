/* $Header: /home/x_cvs/mit/server/ddx/x386/common/x386.h,v 1.15 1992/08/29 10:08:26 dawes Exp $ */
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * $Header: /proj/X11/mit/server/ddx/x386/RCS/x386.h,v 1.1 1991/06/02 22:36:07 root Exp $
 */

#ifndef _X386_H
#define _X386_H

/*
 * structure common for all modes
 */
typedef struct _DispM {
  struct _DispM	*prev,*next;
  char		*name;              /* identifier of this mode */
  int		Clock;              /* doclock */
  int           HDisplay;           /* horizontal timing */
  int           HSyncStart;
  int           HSyncEnd;
  int           HTotal;
  int           VDisplay;           /* vertical timing */
  int           VSyncStart;
  int           VSyncEnd;
  int           VTotal;
  int           Flags;
} DisplayModeRec, *DisplayModePtr;

#define V_PHSYNC    0x0001
#define V_NHSYNC    0x0002
#define V_PVSYNC    0x0004
#define V_NVSYNC    0x0008
#define V_INTERLACE 0x0010
#define V_DBLSCAN   0x0020

#define MAXCLOCKS   32

/*
 * the graphic device
 */
typedef struct {
  Bool           configured;
  int            index;
  Bool           (* Probe)();
  Bool           (* Init)();
  void           (* EnterLeaveVT)();
  void           (* EnterLeaveMonitor)();
  void           (* EnterLeaveCursor)();
  void           (* AdjustFrame)();
  void           (* SwitchMode)();
  void           (* PrintIdent)();
  int            depth;
  int            bitsPerPixel;
  int            defaultVisual;
  int            virtualX,virtualY; 
  int            frameX0, frameY0, frameX1, frameY1;
  char           *vendor;
  char           *chipset;
  int            clocks;
  int            clock[MAXCLOCKS];
  int            videoRam;
  int            width, height;            /* real display dimensions */
  unsigned long  speedup;                  /* Use SpeedUp code */
  DisplayModePtr modes;
} ScrnInfoRec, *ScrnInfoPtr;

#define VGA_DRIVER  1
#define V256_DRIVER 2
#define WGA_DRIVER  3
#define XGA_DRIVER  4

#define ENTER       1
#define LEAVE       0

/* SpeedUp options */

#define SPEEDUP_FILLBOX		1
#define SPEEDUP_FILLRECT	2
#define	SPEEDUP_BITBLT		4
#define SPEEDUP_LINE		8
#define SPEEDUP_TEGBLT8      0x10
#define SPEEDUP_RECTSTIP     0x20

/*
 * This is the routines where SpeedUp is quicker than fX386.  The problem is
 * that the SpeedUp fillbox is better for drawing vertical and horizontal
 * line segments, and the fX386 version is significantly better for
 * more general lines
 */
#define SPEEDUP_BEST		(SPEEDUP_FILLRECT | SPEEDUP_BITBLT | \
				 SPEEDUP_LINE | SPEEDUP_TEGBLT8 | \
				 SPEEDUP_RECTSTIP)
/*
#define SPEEDUP_BEST		(SPEEDUP_FILLBOX | SPEEDUP_FILLRECT | \
				 SPEEDUP_BITBLT | SPEEDUP_LINE | \
                                 SPEEDUP_TEGBLT8 | SPEEDUP_RECTSTIP)
*/

/*
 * SpeedUp routines which are not dependent on the screen virtual resolution
 */
#ifndef SPEEDUP_ANYWIDTH
#define SPEEDUP_ANYWIDTH	(SPEEDUP_FILLRECT | SPEEDUP_BITBLT | \
                                 SPEEDUP_LINE | SPEEDUP_FILLBOX)
#endif

/*
 * SpeedUp routines which are not dependent on ET4000
 */
#ifndef SPEEDUP_ANYCHIPSET
#define SPEEDUP_ANYCHIPSET	(SPEEDUP_TEGBLT8 | SPEEDUP_RECTSTIP)
#endif

/* All SpeedUps */
#define SPEEDUP_ALL		(SPEEDUP_FILLBOX | SPEEDUP_FILLRECT | \
				 SPEEDUP_BITBLT | SPEEDUP_LINE | \
                                 SPEEDUP_TEGBLT8 | SPEEDUP_RECTSTIP)

/* SpeedUp flags used if SpeedUp is not in Xconfig */
#define SPEEDUP_DEFAULT		SPEEDUP_ALL

extern Bool        x386VTSema;

#endif /* _X386_H */


