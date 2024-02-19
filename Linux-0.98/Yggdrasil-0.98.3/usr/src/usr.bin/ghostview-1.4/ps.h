/*
 * ps.h -- Include file for PostScript routines.
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

#ifndef NeedFunctionPrototypes
#if defined(FUNCPROTO) || defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
#define NeedFunctionPrototypes 1
#else
#define NeedFunctionPrototypes 0
#endif /* __STDC__ */
#endif /* NeedFunctionPrototypes */

	/* Constants used to index into the bounding box array. */

#define LLX 0
#define LLY 1
#define URX 2
#define URY 3

	/* Constants used to store keywords that are scanned. */
	/* NONE is not a keyword, it tells when a field was not set */

enum {ATEND = -1, NONE = 0, PORTRAIT, LANDSCAPE, ASCEND, DESCEND, SPECIAL};

#define PSLINELENGTH 257	/* 255 characters + 1 newline + 1 NULL */

struct document {
    int  epsf;				/* Encapsulated PostScript flag. */
    char *title;			/* Title of document. */
    char *date;				/* Creation date. */
    int  pageorder;			/* ASCEND, DESCEND, SPECIAL */
    long beginheader, endheader;	/* offsets into file */
    unsigned int lenheader;
    long beginpreview, endpreview;
    unsigned int lenpreview;
    long begindefaults, enddefaults;
    unsigned int lendefaults;
    long beginprolog, endprolog;
    unsigned int lenprolog;
    long beginsetup, endsetup;
    unsigned int lensetup;
    long begintrailer, endtrailer;
    unsigned int lentrailer;
    int  boundingbox[4];
    int  default_page_boundingbox[4];
    int  orientation;			/* PORTRAIT, LANDSCAPE */
    int  default_page_orientation;	/* PORTRAIT, LANDSCAPE */
    unsigned int nummedia;
    struct documentmedia *media;
    struct documentmedia *default_page_media;
    unsigned int numpages;
    struct page *pages;
};

struct page {
    char *label;
    int  boundingbox[4];
    struct documentmedia *media;
    int  orientation;			/* PORTRAIT, LANDSCAPE */
    long begin, end;			/* offsets into file */
    unsigned int len;
};

struct documentmedia {
    char *name;
    int width, height;
};

	/* list of standard paper sizes from Adobe's PPD. */

extern struct documentmedia papersizes[];

	/* scans a PostScript file and return a pointer to the document
	   structure.  Returns NULL if file does not Conform to commenting
	   conventions . */

#if NeedFunctionPrototypes
struct document *psscan(FILE *);
#else
struct document *psscan();
#endif

	/* free data structure malloc'ed by psscan */

#if NeedFunctionPrototypes
void psfree(struct document *);
#else
void psfree();
#endif

	/* Copy a portion of the PostScript file */

#if NeedFunctionPrototypes
void pscopy(FILE *from, FILE *to, long begin, long end);
#else
void pscopy();
#endif

	/* Copy a portion of the PostScript file upto a comment */

#if NeedFunctionPrototypes
char *pscopyuntil(FILE *from, FILE *to, long begin, long end,
		  const char *comment);
#else
char *pscopyuntil();
#endif
