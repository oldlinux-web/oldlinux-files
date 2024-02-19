/*
 * ps.c -- Postscript scanning and copying routines.
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
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef BUFSIZ
#define BUFSIZ 1024
#endif
#include <ctype.h>
#include <X11/Xos.h>		/* #includes the appropriate <string.h> */
#include "ps.h"

#ifdef BSD4_2
#define memset(a,b,c) bzero(a,c)
#endif

/* length calculates string length at compile time */
/* can only be used with character constants */
#define length(a) (sizeof(a)-1)
#define iscomment(a, b)	(strncmp(a, b, length(b)) == 0)

	/* list of standard paper sizes from Adobe's PPD. */

struct documentmedia papersizes[] = {
    "Letter",		 612,  792,
    "LetterSmall",	 612,  792,
    "Tabloid",		 792, 1224,
    "Ledger",		1224,  792,
    "Legal",		 612, 1008,
    "Statement",	 396,  612,
    "Executive",	 540,  720,
    "A3",		 842, 1190,
    "A4",		 595,  842,
    "A4Small",		 595,  842,
    "A5",		 420,  595,
    "B4",		 729, 1032,
    "B5",		 516,  729,
    "Folio",		 612,  936,
    "Quarto",		 610,  780,
    "10x14",		 720, 1008,
    NULL,		   0,    0
};


static char *readline();
static char *gettextline();
static char *gettext();
static int  isblankline();

/*
 *	psscan -- scan the PostScript file for document structuring comments.
 *
 *	This scanner is designed to retrieve the information necessary for
 *	the ghostview previewer.  It will scan files that conform to any
 *	version (1.0, 2.0, 2.1, or 3.0) of the document structuring conventions.
 *	It does not really care which version of comments the file contains.
 *	(The comments are largely upward compatible.)  It will scan a number
 *	of non-conforming documents.  (You could have part of the document
 *	conform to V2.0 and the rest conform to V3.0.  It would be similar
 *	to the DC-2 1/2+, it would look funny but it can still fly.)
 *
 *	This routine returns a pointer to the document structure.
 *	The structure contains the information relevant to previewing.
 *      These include EPSF flag (to tell if the file is a encapsulated figure),
 *      Page Media (for the Page Size), Bounding Box (to minimize backing
 *      pixmap size or determine window size for encapsulated PostScript), 
 *      Orientation of Paper (for default transformation matrix), and
 *      Page Order.  The title and CreationDate are also retrieved to
 *      help identify the document.
 *
 *      The following comments are examined:
 *
 *      Header section: 
 *      Must start with %!PS-Adobe-.  Version numbers ignored.
 *
 *      %!PS-Adobe-* [EPSF-*]
 *      %%BoundingBox: <int> <int> <int> <int>|(atend)
 *      %%CreationDate: <textline>
 *      %%Orientation: Portrait|Landscape|(atend)
 *      %%Pages: <uint> [<int>]|(atend)
 *      %%PageOrder: Ascend|Descend|Special|(atend)
 *      %%Title: <textline>
 *      %%DocumentMedia: <text> <real> <real> <real> <text> <text>
 *      %%DocumentPageSizes: <text>
 *      %%EndComments
 *
 *      Note: Either the 3.0 or 2.0 syntax for %%Pages is accepted.
 *            Also either the 2.0 %%DocumentPageSizes or the 3.0
 *            %%DocumentMedia comments are accepted as well.
 *
 *      The header section ends either explicitly with %%EndComments or
 *      implicitly with any line that does not begin with %X where X is
 *      a not whitespace character.
 *
 *      If the file is encapsulated PostScript the optional Preview section
 *      is next:
 *
 *      %%BeginPreview
 *      %%EndPreview
 *
 *      This section explicitly begins and ends with the above comments.
 *
 *      Next the Defaults section for version 3 page defaults:
 *
 *      %%BeginDefaults
 *      %%PageBoundingBox: <int> <int> <int> <int>
 *      %%PageOrientation: Portrait|Landscape
 *      %%PageMedia: <text>
 *      %%EndDefaults
 *
 *      This section explicitly begins and ends with the above comments.
 *
 *      The prolog section either explicitly starts with %%BeginProlog or
 *      implicitly with any nonblank line.
 *
 *      %%BeginProlog
 *      %%EndProlog
 *
 *      The Prolog should end with %%EndProlog, however the proglog implicitly
 *      ends when %%BeginSetup, %%Page, %%Trailer or %%EOF are encountered.
 *
 *      The Setup section is where the version 2 page defaults are found.
 *      This section either explicitly begins with %%BeginSetup or implicitly
 *      with any noblank line after the Prolog.
 *
 *      %%BeginSetup
 *      %%PageBoundingBox: <int> <int> <int> <int>
 *      %%PageOrientation: Portrait|Landscape
 *      %%PaperSize: <text>
 *      %%EndSetup
 *
 *      The Setup should end with %%EndSetup, however the setup implicitly
 *      ends when %%Page, %%Trailer or %%EOF are encountered.
 *
 *      Next each page starts explicitly with %%Page and ends implicitly with
 *      %%Page or %%Trailer or %%EOF.  The following comments are recognized:
 *
 *      %%Page: <text> <uint>
 *      %%PageBoundingBox: <int> <int> <int> <int>|(atend)
 *      %%PageOrientation: Portrait|Landscape
 *      %%PageMedia: <text>
 *      %%PaperSize: <text>
 *
 *      The tralier section start explicitly with %%Trailer and end with %%EOF.
 *      The following comment are examined with the proper (atend) notation
 *      was used in the header:
 *
 *      %%Trailer
 *      %%BoundingBox: <int> <int> <int> <int>|(atend)
 *      %%Orientation: Portrait|Landscape|(atend)
 *      %%Pages: <uint> [<int>]|(atend)
 *      %%PageOrder: Ascend|Descend|Special|(atend)
 *      %%EOF
 *
 *
 *  + A DC-3 received severe damage to one of its wings.  The wing was a total
 *    loss.  There was no replacement readily available, so the mechanic
 *    installed a wing from a DC-2.
 */

struct document *
psscan(file)
    FILE *file;
{
    struct document *doc;
    int bb_set = NONE;
    int pages_set = NONE;
    int page_order_set = NONE;
    int orientation_set = NONE;
    int page_bb_set = NONE;
    int page_media_set = NONE;
    int preread;		/* flag which tells the readline isn't needed */
    int i;
    unsigned int maxpages = 0;
    unsigned int nextpage = 1;	/* Next expected page */
    unsigned int thispage;
    int ignore = 0;		/* whether to ignore page ordinals */
    char *label;
    char line[PSLINELENGTH];	/* 255 characters + 1 newline + 1 NULL */
    char text[PSLINELENGTH];	/* Temporary storage for text */
    long position;		/* Position of the current line */
    unsigned int line_len; 	/* Length of the current line */
    unsigned int section_len;	/* Place to accumulate the section length */
    char *next_char;		/* 1st char after text returned by gettext() */
    char *cp;
    struct documentmedia *dmp;

    rewind(file);
    if (readline(line, sizeof line, file, &position, &line_len) == NULL) {
	fprintf(stderr, "Warning: empty file.\n");
	return(NULL);
    }

    /* Header comments */

    if (iscomment(line,"%!PS-Adobe-")) {
	doc = (struct document *) malloc(sizeof(struct document));
	if (doc == NULL) {
	    fprintf(stderr, "Fatal Error: Dynamic memory exhausted.\n");
	    exit(-1);
	}
	memset(doc, 0, sizeof(struct document));
	sscanf(line, "%*s %s", text);
	doc->epsf = iscomment(text, "EPSF-");
	doc->beginheader = position;
	section_len = line_len;
    } else {
	return(NULL);
    }

    preread = 0;
    while (preread || readline(line, sizeof line, file, &position, &line_len)) {
	if (!preread) section_len += line_len;
	preread = 0;
	if (iscomment(line, "%%EndComments") ||
	    line[0] != '%' ||
	    (!isprint(line[1]) || line[1] == ' ' ||
	     line[1] == '\t' || line[1] == '\n')) {
	    break;
	} else if (!iscomment(line, "%%")) {
	    /* Do nothing */
	} else if (doc->title == NULL && iscomment(line+2, "Title:")) {
	    doc->title = gettextline(line+length("%%Title:"));
	} else if (doc->date == NULL && iscomment(line+2, "CreationDate:")) {
	    doc->date = gettextline(line+length("%%CreationDate:"));
	} else if (bb_set == NONE && iscomment(line+2, "BoundingBox:")) {
	    sscanf(line+length("%%BoundingBox:"), "%s", text);
	    if (strcmp(text, "(atend)") == 0) {
		bb_set = ATEND;
	    } else {
		if (sscanf(line+length("%%BoundingBox:"), "%d %d %d %d",
			   &(doc->boundingbox[LLX]),
			   &(doc->boundingbox[LLY]),
			   &(doc->boundingbox[URX]),
			   &(doc->boundingbox[URY])) == 4)
		    bb_set = 1;
		else {
		    float fllx, flly, furx, fury;
		    if (sscanf(line+length("%%BoundingBox:"), "%f %f %f %f",
			       &fllx, &flly, &furx, &fury) == 4) {
			bb_set = 1;
			doc->boundingbox[LLX] = fllx;
			doc->boundingbox[LLY] = flly;
			doc->boundingbox[URX] = furx;
			doc->boundingbox[URY] = fury;
			if (fllx < doc->boundingbox[LLX])
			    doc->boundingbox[LLX]--;
			if (flly < doc->boundingbox[LLY])
			    doc->boundingbox[LLY]--;
			if (furx > doc->boundingbox[URX])
			    doc->boundingbox[URX]++;
			if (fury > doc->boundingbox[URY])
			    doc->boundingbox[URY]++;
		    }
		}
	    }
	} else if (orientation_set == NONE &&
		   iscomment(line+2, "Orientation:")) {
	    sscanf(line+length("%%Orientation:"), "%s", text);
	    if (strcmp(text, "(atend)") == 0) {
		orientation_set = ATEND;
	    } else if (strcmp(text, "Portrait") == 0) {
		doc->orientation = PORTRAIT;
		orientation_set = 1;
	    } else if (strcmp(text, "Landscape") == 0) {
		doc->orientation = LANDSCAPE;
		orientation_set = 1;
	    }
	} else if (page_order_set == NONE && iscomment(line+2, "PageOrder:")) {
	    sscanf(line+length("%%PageOrder:"), "%s", text);
	    if (strcmp(text, "(atend)") == 0) {
		page_order_set = ATEND;
	    } else if (strcmp(text, "Ascend") == 0) {
		doc->pageorder = ASCEND;
		page_order_set = 1;
	    } else if (strcmp(text, "Descend") == 0) {
		doc->pageorder = DESCEND;
		page_order_set = 1;
	    } else if (strcmp(text, "Special") == 0) {
		doc->pageorder = SPECIAL;
		page_order_set = 1;
	    }
	} else if (pages_set == NONE && iscomment(line+2, "Pages:")) {
	    sscanf(line+length("%%Pages:"), "%s", text);
	    if (strcmp(text, "(atend)") == 0) {
		pages_set = ATEND;
	    } else {
		switch (sscanf(line+length("%%Pages:"), "%d %d",
			       &maxpages, &i)) {
		    case 2:
			if (page_order_set == NONE) {
			    if (i == -1) {
				doc->pageorder = DESCEND;
				page_order_set = 1;
			    } else if (i == 0) {
				doc->pageorder = SPECIAL;
				page_order_set = 1;
			    } else if (i == 1) {
				doc->pageorder = ASCEND;
				page_order_set = 1;
			    }
			}
		    case 1:
			doc->pages = (struct page *) calloc(maxpages,
							sizeof(struct page));
			if (doc->pages == NULL) {
			    fprintf(stderr,
				    "Fatal Error: Dynamic memory exhausted.\n");
			    exit(-1);
			}
		}
	    }
	} else if (doc->nummedia == NONE &&
		   iscomment(line+2, "DocumentMedia:")) {
	    float w, h;
	    doc->media = (struct documentmedia *)
			 malloc(sizeof (struct documentmedia));
	    if (doc->media == NULL) {
		fprintf(stderr, "Fatal Error: Dynamic memory exhausted.\n");
		exit(-1);
	    }
	    doc->media[0].name = gettext(line+length("%%DocumentMedia:"),
					 &next_char);
	    if (doc->media[0].name != NULL) {
		if (sscanf(next_char, "%f %f", &w, &h) == 2) {
		    doc->media[0].width = w + 0.5;
		    doc->media[0].height = h + 0.5;
		}
		if (doc->media[0].width != 0 && doc->media[0].height != 0)
		    doc->nummedia = 1;
		else
		    free(doc->media[0].name);
	    }
	    preread=1;
	    while (readline(line, sizeof line, file, &position, &line_len) &&
		   iscomment(line, "%%+")) {
		section_len += line_len;
		doc->media = (struct documentmedia *)
			     realloc(doc->media,
				     (doc->nummedia+1)*
				     sizeof (struct documentmedia));
		if (doc->media == NULL) {
		    fprintf(stderr, "Fatal Error: Dynamic memory exhausted.\n");
		    exit(-1);
		}
		doc->media[doc->nummedia].name = gettext(line+length("%%+"),
							 &next_char);
		if (doc->media[doc->nummedia].name != NULL) {
		    if (sscanf(next_char, "%f %f", &w, &h) == 2) {
			doc->media[doc->nummedia].width = w + 0.5;
			doc->media[doc->nummedia].height = h + 0.5;
		    }
		    if (doc->media[doc->nummedia].width != 0 &&
			doc->media[doc->nummedia].height != 0) doc->nummedia++;
		    else
			free(doc->media[doc->nummedia].name);
		}
	    }
	    section_len += line_len;
	    if (doc->nummedia != 0) doc->default_page_media = doc->media;
	} else if (doc->nummedia == NONE &&
		   iscomment(line+2, "DocumentPaperSizes:")) {

	    doc->media = (struct documentmedia *)
			 malloc(sizeof (struct documentmedia));
	    if (doc->media == NULL) {
		fprintf(stderr, "Fatal Error: Dynamic memory exhausted.\n");
		exit(-1);
	    }
	    doc->media[0].name = gettext(line+length("%%DocumentPaperSizes:"),
					 &next_char);
	    if (doc->media[0].name != NULL) {
		doc->media[0].width = 0;
		doc->media[0].height = 0;
		for (dmp=papersizes; dmp->name != NULL; dmp++) {
		    /* Note: Paper size comment uses down cased paper size
		     * name.  Case insensitive compares are only used for
		     * PaperSize comments.
		     */
		    if (strcasecmp(doc->media[0].name, dmp->name) == 0) {
			free(doc->media[0].name);
			doc->media[0].name =
				(char *)malloc(strlen(dmp->name)+1);
			if (doc->media[0].name == NULL) {
			    fprintf(stderr,
				    "Fatal Error: Dynamic memory exhausted.\n");
			    exit(-1);
			}
			strcpy(doc->media[0].name, dmp->name);
			doc->media[0].width = dmp->width;
			doc->media[0].height = dmp->height;
			break;
		    }
		}
		if (doc->media[0].width != 0 && doc->media[0].height != 0)
		    doc->nummedia = 1;
		else
		    free(doc->media[0].name);
	    }
	    while (cp = gettext(next_char, &next_char)) {
		doc->media = (struct documentmedia *)
			     realloc(doc->media,
				     (doc->nummedia+1)*
				     sizeof (struct documentmedia));
		if (doc->media == NULL) {
		    fprintf(stderr, "Fatal Error: Dynamic memory exhausted.\n");
		    exit(-1);
		}
		doc->media[doc->nummedia].name = cp;
		doc->media[doc->nummedia].width = 0;
		doc->media[doc->nummedia].height = 0;
		for (dmp=papersizes; dmp->name != NULL; dmp++) {
		    /* Note: Paper size comment uses down cased paper size
		     * name.  Case insensitive compares are only used for
		     * PaperSize comments.
		     */
		    if (strcasecmp(doc->media[doc->nummedia].name,
			       dmp->name) == 0) {
			free(doc->media[doc->nummedia].name);
			doc->media[doc->nummedia].name =
				(char *)malloc(strlen(dmp->name)+1);
			if (doc->media[doc->nummedia].name == NULL) {
			    fprintf(stderr,
				    "Fatal Error: Dynamic memory exhausted.\n");
			    exit(-1);
			}
			strcpy(doc->media[doc->nummedia].name, dmp->name);
			doc->media[doc->nummedia].name = dmp->name;
			doc->media[doc->nummedia].width = dmp->width;
			doc->media[doc->nummedia].height = dmp->height;
			break;
		    }
		}
		if (doc->media[doc->nummedia].width != 0 &&
		    doc->media[doc->nummedia].height != 0) doc->nummedia++;
		else
		    free(doc->media[doc->nummedia].name);
	    }
	    preread=1;
	    while (readline(line, sizeof line, file, &position, &line_len) &&
		   iscomment(line, "%%+")) {
		section_len += line_len;
		next_char = line + length("%%+");
		while (cp = gettext(next_char, &next_char)) {
		    doc->media = (struct documentmedia *)
				 realloc(doc->media,
					 (doc->nummedia+1)*
					 sizeof (struct documentmedia));
		    if (doc->media == NULL) {
			fprintf(stderr,
				"Fatal Error: Dynamic memory exhausted.\n");
			exit(-1);
		    }
		    doc->media[doc->nummedia].name = cp;
		    doc->media[doc->nummedia].width = 0;
		    doc->media[doc->nummedia].height = 0;
		    for (dmp=papersizes; dmp->name != NULL; dmp++) {
			/* Note: Paper size comment uses down cased paper size
			 * name.  Case insensitive compares are only used for
			 * PaperSize comments.
			 */
			if (strcasecmp(doc->media[doc->nummedia].name,
				   dmp->name) == 0) {
			    doc->media[doc->nummedia].width = dmp->width;
			    doc->media[doc->nummedia].height = dmp->height;
			    break;
			}
		    }
		    if (doc->media[doc->nummedia].width != 0 &&
			doc->media[doc->nummedia].height != 0) doc->nummedia++;
		    else
			free(doc->media[doc->nummedia].name);
		}
	    }
	    section_len += line_len;
	    if (doc->nummedia != 0) doc->default_page_media = doc->media;
	}
    }

    if (iscomment(line, "%%EndComments")) {
	readline(line, sizeof line, file, &position, &line_len);
	section_len += line_len;
    }
    doc->endheader = position;
    doc->lenheader = section_len - line_len;

    /* Optional Preview comments for encapsulated PostScript files */ 

    while (isblankline(line) &&
	   readline(line, sizeof line, file, &position, &line_len)) {
    }

    if (doc->epsf && iscomment(line, "%%BeginPreview")) {
	doc->beginpreview = position;
	section_len = line_len;
	while (readline(line, sizeof line, file, &position, &line_len) &&
	       !iscomment(line, "%%EndPreview")) {
	    section_len += line_len;
	}
	section_len += line_len;
	readline(line, sizeof line, file, &position, &line_len);
	section_len += line_len;
	doc->endpreview = position;
	doc->lenpreview = section_len - line_len;
    }

    /* Page Defaults for Version 3.0 files */

    while (isblankline(line) &&
	   readline(line, sizeof line, file, &position, &line_len)) {
    }

    if (iscomment(line, "%%BeginDefaults")) {
	doc->begindefaults = position;
	section_len = line_len;
	while (readline(line, sizeof line, file, &position, &line_len) &&
	       !iscomment(line, "%%EndDefaults")) {
	    section_len += line_len;
	    if (!iscomment(line, "%%")) {
		/* Do nothing */
	    } else if (doc->default_page_orientation == NONE &&
		iscomment(line+2, "PageOrientation:")) {
		sscanf(line+length("%%PageOrientation:"), "%s", text);
		if (strcmp(text, "Portrait") == 0) {
		    doc->default_page_orientation = PORTRAIT;
		} else if (strcmp(text, "Landscape") == 0) {
		    doc->default_page_orientation = LANDSCAPE;
		}
	    } else if (page_media_set == NONE &&
		       iscomment(line+2, "PageMedia:")) {
		cp = gettext(line+length("%%PageMedia:"), NULL);
		for (dmp = doc->media, i=0; i<doc->nummedia; i++, dmp++) {
		    if (strcmp(cp, dmp->name) == 0) {
			doc->default_page_media = dmp;
			page_media_set = 1;
			break;
		    }
		}
		free(cp);
	    } else if (page_bb_set == NONE &&
		       iscomment(line+2, "PageBoundingBox:")) {
		if (sscanf(line+length("%%PageBoundingBox:"), "%d %d %d %d",
			   &(doc->default_page_boundingbox[LLX]),
			   &(doc->default_page_boundingbox[LLY]),
			   &(doc->default_page_boundingbox[URX]),
			   &(doc->default_page_boundingbox[URY])) == 4)
		    page_bb_set = 1;
		else {
		    float fllx, flly, furx, fury;
		    if (sscanf(line+length("%%PageBoundingBox:"), "%f %f %f %f",
			       &fllx, &flly, &furx, &fury) == 4) {
			page_bb_set = 1;
			doc->default_page_boundingbox[LLX] = fllx;
			doc->default_page_boundingbox[LLY] = flly;
			doc->default_page_boundingbox[URX] = furx;
			doc->default_page_boundingbox[URY] = fury;
			if (fllx < doc->default_page_boundingbox[LLX])
			    doc->default_page_boundingbox[LLX]--;
			if (flly < doc->default_page_boundingbox[LLY])
			    doc->default_page_boundingbox[LLY]--;
			if (furx > doc->default_page_boundingbox[URX])
			    doc->default_page_boundingbox[URX]++;
			if (fury > doc->default_page_boundingbox[URY])
			    doc->default_page_boundingbox[URY]++;
		    }
		}
	    }
	}
	section_len += line_len;
	readline(line, sizeof line, file, &position, &line_len);
	section_len += line_len;
	doc->enddefaults = position;
	doc->lendefaults = section_len - line_len;
    }

    /* Document Prolog */

    while (isblankline(line) &&
	   readline(line, sizeof line, file, &position, &line_len)) {
    }

    if (!iscomment(line, "%%BeginSetup") && !iscomment(line, "%%Page:") &&
	!iscomment(line, "%%Trailer") && !iscomment(line, "%%EOF")) {
	doc->beginprolog = position;
	section_len = line_len;
	preread = 1;

	while ((preread ||
		readline(line, sizeof line, file, &position, &line_len)) &&
	       !iscomment(line, "%%EndProlog") &&
	       !iscomment(line, "%%BeginSetup") &&
	       !iscomment(line, "%%Page:") && !iscomment(line, "%%Trailer") &&
	       !iscomment(line, "%%EOF")) {
	    if (!preread) section_len += line_len;
	    preread = 0;
	}
	section_len += line_len;
	if (iscomment(line, "%%EndProlog")) {
	    readline(line, sizeof line, file, &position, &line_len);
	    section_len += line_len;
	}
	doc->endprolog = position;
	doc->lenprolog = section_len - line_len;
    }

    /* Document Setup,  Page Defaults found here for Version 2 files */

    while (isblankline(line) &&
	   readline(line, sizeof line, file, &position, &line_len)) {
    }

    if (!iscomment(line, "%%Page:") && !iscomment(line, "%%Trailer") &&
	!iscomment(line, "%%EOF")) {
	doc->beginsetup = position;
	section_len = line_len;
	preread = 1;
	while ((preread ||
		readline(line, sizeof line, file, &position, &line_len)) &&
	       !iscomment(line, "%%EndSetup") && !iscomment(line, "%%Page:") &&
	       !iscomment(line, "%%Trailer") && !iscomment(line, "%%EOF")) {
	    if (!preread) section_len += line_len;
	    preread = 0;
	    if (!iscomment(line, "%%")) {
		/* Do nothing */
	    } else if (doc->default_page_orientation == NONE &&
		iscomment(line+2, "PageOrientation:")) {
		sscanf(line+length("%%PageOrientation:"), "%s", text);
		if (strcmp(text, "Portrait") == 0) {
		    doc->default_page_orientation = PORTRAIT;
		} else if (strcmp(text, "Landscape") == 0) {
		    doc->default_page_orientation = LANDSCAPE;
		}
	    } else if (page_media_set == NONE &&
		       iscomment(line+2, "PaperSize:")) {
		cp = gettext(line+length("%%PaperSize:"), NULL);
		for (dmp = doc->media, i=0; i<doc->nummedia; i++, dmp++) {
		    /* Note: Paper size comment uses down cased paper size
		     * name.  Case insensitive compares are only used for
		     * PaperSize comments.
		     */
		    if (strcasecmp(cp, dmp->name) == 0) {
			doc->default_page_media = dmp;
			page_media_set = 1;
			break;
		    }
		}
		free(cp);
	    } else if (page_bb_set == NONE &&
		       iscomment(line+2, "PageBoundingBox:")) {
		if (sscanf(line+length("%%PageBoundingBox:"), "%d %d %d %d",
			   &(doc->default_page_boundingbox[LLX]),
			   &(doc->default_page_boundingbox[LLY]),
			   &(doc->default_page_boundingbox[URX]),
			   &(doc->default_page_boundingbox[URY])) == 4)
		    page_bb_set = 1;
		else {
		    float fllx, flly, furx, fury;
		    if (sscanf(line+length("%%PageBoundingBox:"), "%f %f %f %f",
			       &fllx, &flly, &furx, &fury) == 4) {
			page_bb_set = 1;
			doc->default_page_boundingbox[LLX] = fllx;
			doc->default_page_boundingbox[LLY] = flly;
			doc->default_page_boundingbox[URX] = furx;
			doc->default_page_boundingbox[URY] = fury;
			if (fllx < doc->default_page_boundingbox[LLX])
			    doc->default_page_boundingbox[LLX]--;
			if (flly < doc->default_page_boundingbox[LLY])
			    doc->default_page_boundingbox[LLY]--;
			if (furx > doc->default_page_boundingbox[URX])
			    doc->default_page_boundingbox[URX]++;
			if (fury > doc->default_page_boundingbox[URY])
			    doc->default_page_boundingbox[URY]++;
		    }
		}
	    }
	}
	section_len += line_len;
	if (iscomment(line, "%%EndSetup")) {
	    readline(line, sizeof line, file, &position, &line_len);
	    section_len += line_len;
	}
	doc->endsetup = position;
	doc->lensetup = section_len - line_len;
    }

    /* Individual Pages */

    while (isblankline(line) &&
	   readline(line, sizeof line, file, &position, &line_len)) {
    }

newpage:
    while (iscomment(line, "%%Page:")) {
	if (maxpages == 0) {
	    maxpages = 1;
	    doc->pages = (struct page *) calloc(maxpages, sizeof(struct page));
	    if (doc->pages == NULL) {
		fprintf(stderr,
			"Fatal Error: Dynamic memory exhausted.\n");
		exit(-1);
	    }
	}
	label = gettext(line+length("%%Page:"), &next_char);
	if (sscanf(next_char, "%d", &thispage) != 1) thispage = 0;
	if (nextpage == 1) {
	    ignore = thispage != 1;
	}
	if (!ignore && thispage != nextpage) {
	    free(label);
	    doc->numpages--;
	    goto continuepage;
	}
	nextpage++;
	if (doc->numpages == maxpages) {
	    maxpages++;
	    doc->pages = (struct page *)
			 realloc(doc->pages, maxpages*sizeof (struct page));
	    if (doc->pages == NULL) {
		fprintf(stderr, "Fatal Error: Dynamic memory exhausted.\n");
		exit(-1);
	    }
	}
	memset(&(doc->pages[doc->numpages]), 0, sizeof(struct page));
	page_bb_set = NONE;
	doc->pages[doc->numpages].label = label;
	doc->pages[doc->numpages].begin = position;
	section_len = line_len;
continuepage:
	while (readline(line, sizeof line, file, &position, &line_len) &&
	       !iscomment(line, "%%Page:") && !iscomment(line, "%%Trailer") &&
	       !iscomment(line, "%%EOF")) {
	    section_len += line_len;
	    if (!iscomment(line, "%%")) {
		/* Do nothing */
	    } else if (doc->pages[doc->numpages].orientation == NONE &&
		iscomment(line+2, "PageOrientation:")) {
		sscanf(line+length("%%PageOrientation:"), "%s", text);
		if (strcmp(text, "Portrait") == 0) {
		    doc->pages[doc->numpages].orientation = PORTRAIT;
		} else if (strcmp(text, "Landscape") == 0) {
		    doc->pages[doc->numpages].orientation = LANDSCAPE;
		}
	    } else if (doc->pages[doc->numpages].media == NULL &&
		       iscomment(line+2, "PageMedia:")) {
		cp = gettext(line+length("%%PageMedia:"), NULL);
		for (dmp = doc->media, i=0; i<doc->nummedia; i++, dmp++) {
		    if (strcmp(cp, dmp->name) == 0) {
			doc->pages[doc->numpages].media = dmp;
			break;
		    }
		}
		free(cp);
	    } else if (doc->pages[doc->numpages].media == NULL &&
		       iscomment(line+2, "PaperSize:")) {
		cp = gettext(line+length("%%PaperSize:"), NULL);
		for (dmp = doc->media, i=0; i<doc->nummedia; i++, dmp++) {
		    /* Note: Paper size comment uses down cased paper size
		     * name.  Case insensitive compares are only used for
		     * PaperSize comments.
		     */
		    if (strcasecmp(cp, dmp->name) == 0) {
			doc->pages[doc->numpages].media = dmp;
			break;
		    }
		}
		free(cp);
	    } else if ((page_bb_set == NONE || page_bb_set == ATEND) &&
		       iscomment(line+2, "PageBoundingBox:")) {
		sscanf(line+length("%%PageBoundingBox:"), "%s", text);
		if (strcmp(text, "(atend)") == 0) {
		    page_bb_set = ATEND;
		} else {
		    if (sscanf(line+length("%%PageBoundingBox:"), "%d %d %d %d",
			    &(doc->pages[doc->numpages].boundingbox[LLX]),
			    &(doc->pages[doc->numpages].boundingbox[LLY]),
			    &(doc->pages[doc->numpages].boundingbox[URX]),
			    &(doc->pages[doc->numpages].boundingbox[URY])) == 4)
			if (page_bb_set == NONE) page_bb_set = 1;
		    else {
			float fllx, flly, furx, fury;
			if (sscanf(line+length("%%PageBoundingBox:"),
				   "%f %f %f %f",
				   &fllx, &flly, &furx, &fury) == 4) {
			    if (page_bb_set == NONE) page_bb_set = 1;
			    doc->pages[doc->numpages].boundingbox[LLX] = fllx;
			    doc->pages[doc->numpages].boundingbox[LLY] = flly;
			    doc->pages[doc->numpages].boundingbox[URX] = furx;
			    doc->pages[doc->numpages].boundingbox[URY] = fury;
			    if (fllx <
				    doc->pages[doc->numpages].boundingbox[LLX])
				doc->pages[doc->numpages].boundingbox[LLX]--;
			    if (flly <
				    doc->pages[doc->numpages].boundingbox[LLY])
				doc->pages[doc->numpages].boundingbox[LLY]--;
			    if (furx >
				    doc->pages[doc->numpages].boundingbox[URX])
				doc->pages[doc->numpages].boundingbox[URX]++;
			    if (fury >
				    doc->pages[doc->numpages].boundingbox[URY])
				doc->pages[doc->numpages].boundingbox[URY]++;
			}
		    }
		}
	    }
	}
	section_len += line_len;
	doc->pages[doc->numpages].end = position;
	doc->pages[doc->numpages].len = section_len - line_len;
	doc->numpages++;
    }

    /* Document Trailer */

    doc->begintrailer = position;
    section_len = line_len;

    preread = 1;
    while ((preread ||
	    readline(line, sizeof line, file, &position, &line_len)) &&
	   !iscomment(line, "%%EOF")) {
	if (!preread) section_len += line_len;
	preread = 0;
	if (!iscomment(line, "%%")) {
	    /* Do nothing */
	} else if (iscomment(line+2, "Page:")) {
	    free(gettext(line+length("%%Page:"), &next_char));
	    if (sscanf(next_char, "%d", &thispage) != 1) thispage = 0;
	    if (!ignore && thispage == nextpage) {
		doc->pages[doc->numpages-1].end = position;
		doc->pages[doc->numpages-1].len += section_len - line_len;
		goto newpage;
	    }
	} else if (bb_set == ATEND && iscomment(line+2, "BoundingBox:")) {
	    if (sscanf(line+length("%%BoundingBox:"), "%d %d %d %d",
		       &(doc->boundingbox[LLX]),
		       &(doc->boundingbox[LLY]),
		       &(doc->boundingbox[URX]),
		       &(doc->boundingbox[URY])) != 4) {
		float fllx, flly, furx, fury;
		if (sscanf(line+length("%%BoundingBox:"), "%f %f %f %f",
			   &fllx, &flly, &furx, &fury) == 4) {
		    doc->boundingbox[LLX] = fllx;
		    doc->boundingbox[LLY] = flly;
		    doc->boundingbox[URX] = furx;
		    doc->boundingbox[URY] = fury;
		    if (fllx < doc->boundingbox[LLX])
			doc->boundingbox[LLX]--;
		    if (flly < doc->boundingbox[LLY])
			doc->boundingbox[LLY]--;
		    if (furx > doc->boundingbox[URX])
			doc->boundingbox[URX]++;
		    if (fury > doc->boundingbox[URY])
			doc->boundingbox[URY]++;
		}
	    }
	} else if (orientation_set == ATEND &&
		   iscomment(line+2, "Orientation:")) {
	    sscanf(line+length("%%Orientation:"), "%s", text);
	    if (strcmp(text, "Portrait") == 0) {
		doc->orientation = PORTRAIT;
	    } else if (strcmp(text, "Landscape") == 0) {
		doc->orientation = LANDSCAPE;
	    }
	} else if (page_order_set == ATEND && iscomment(line+2, "PageOrder:")) {
	    sscanf(line+length("%%PageOrder:"), "%s", text);
	    if (strcmp(text, "Ascend") == 0) {
		doc->pageorder = ASCEND;
	    } else if (strcmp(text, "Descend") == 0) {
		doc->pageorder = DESCEND;
	    } else if (strcmp(text, "Special") == 0) {
		doc->pageorder = SPECIAL;
	    }
	} else if (pages_set == ATEND && iscomment(line+2, "Pages:")) {
	    if (sscanf(line+length("%%Pages:"), "%*u %d", &i) == 1) {
		if (page_order_set == NONE) {
		    if (i == -1) doc->pageorder = DESCEND;
		    else if (i == 0) doc->pageorder = SPECIAL;
		    else if (i == 1) doc->pageorder = ASCEND;
		}
	    }
	}
    }
    section_len += line_len;
    if (iscomment(line, "%%EOF")) {
	readline(line, sizeof line, file, &position, &line_len);
	section_len += line_len;
    }
    doc->endtrailer = position;
    doc->lentrailer = section_len - line_len;

    section_len = line_len;
    preread = 1;
    while (preread ||
	   readline(line, sizeof line, file, &position, &line_len)) {
	if (!preread) section_len += line_len;
	preread = 0;
	if (iscomment(line, "%%Page:")) {
	    free(gettext(line+length("%%Page:"), &next_char));
	    if (sscanf(next_char, "%d", &thispage) != 1) thispage = 0;
	    if (!ignore && thispage == nextpage) {
		doc->pages[doc->numpages-1].end = position;
		doc->pages[doc->numpages-1].len += doc->lentrailer +
						   section_len - line_len;
		goto newpage;
	    }
	}
    }
    return doc;
}

/*
 *	psfree -- free dynamic storage associated with document structure.
 */

void
psfree(doc)
    struct document *doc;
{
    int i;

    if (doc) {
	for (i=0; i<doc->numpages; i++) {
	    if (doc->pages[i].label) free(doc->pages[i].label);
	}
	for (i=0; i<doc->nummedia; i++) {
	    if (doc->media[i].name) free(doc->media[i].name);
	}
	if (doc->title) free(doc->title);
	if (doc->date) free(doc->date);
	if (doc->pages) free(doc->pages);
	if (doc->media) free(doc->media);
	free(doc);
    }
}

/*
 * gettextine -- skip over white space and return the rest of the line.
 *               If the text begins with '(' return the text string
 *		 using gettext().
 */

static char *
gettextline(line)
    char *line;
{
    char *cp;

    while (*line && (*line == ' ' || *line == '\t')) line++;
    if (*line == '(') {
	return gettext(line, NULL);
    } else {
	if (strlen(line) == 0) return NULL;
	cp = (char *) malloc(strlen(line));
	if (cp == NULL) {
	    fprintf(stderr, "Fatal Error: Dynamic memory exhausted.\n");
	    exit(-1);
	}
	strncpy(cp, line, strlen(line)-1);
	cp[strlen(line)-1] = '\0';
	return cp;
    }
}

/*
 *	gettext -- return the next text string on the line.
 *		   return NULL if nothing is present.
 */

static char *
gettext(line, next_char)
    char *line;
    char **next_char;
{
    char text[PSLINELENGTH];	/* Temporary storage for text */
    char *cp;
    int quoted=0;

    while (*line && (*line == ' ' || *line == '\t')) line++;
    cp = text;
    if (*line == '(') {
	int level = 0;
	quoted=1;
	line++;
	while (*line && !(*line == ')' && level == 0 )) {
	    if (*line == '\\') {
		if (*(line+1) == 'n') {
		    *cp++ = '\n';
		    line += 2;
		} else if (*(line+1) == 'r') {
		    *cp++ = '\r';
		    line += 2;
		} else if (*(line+1) == 't') {
		    *cp++ = '\t';
		    line += 2;
		} else if (*(line+1) == 'b') {
		    *cp++ = '\b';
		    line += 2;
		} else if (*(line+1) == 'f') {
		    *cp++ = '\f';
		    line += 2;
		} else if (*(line+1) == '\\') {
		    *cp++ = '\\';
		    line += 2;
		} else if (*(line+1) == '(') {
		    *cp++ = '(';
		    line += 2;
		} else if (*(line+1) == ')') {
		    *cp++ = ')';
		    line += 2;
		} else if (*(line+1) >= '0' && *(line+1) <= '9') {
		    if (*(line+2) >= '0' && *(line+2) <= '9') {
			if (*(line+3) >= '0' && *(line+3) <= '9') {
			    *cp++ = ((*(line+1) - '0')*8 + *(line+2) - '0')*8 +
				    *(line+3) - '0';
			    line += 4;
			} else {
			    *cp++ = (*(line+1) - '0')*8 + *(line+2) - '0';
			    line += 3;
			}
		    } else {
			*cp++ = *(line+1) - '0';
			line += 2;
		    }
		} else {
		    line++;
		    *cp++ = *line++;
		}
	    } else if (*line == '(') {
		level++;
		*cp++ = *line++;
	    } else if (*line == ')') {
		level--;
		*cp++ = *line++;
	    } else {
		*cp++ = *line++;
	    }
	}
    } else {
	while (*line && !(*line == ' ' || *line == '\t' || *line == '\n'))
	    *cp++ = *line++;
    }
    *cp = '\0';
    if (next_char) *next_char = line;
    if (!quoted && strlen(text) == 0) return NULL;
    cp = (char *) malloc(strlen(text)+1);
    if (cp == NULL) {
	fprintf(stderr, "Fatal Error: Dynamic memory exhausted.\n");
	exit(-1);
    }
    strcpy(cp, text);
    return cp;
}

/*
 *	readline -- Read the next line in the postscript file.
 *                  Automatically skip over data (as indicated by
 *                  %%BeginBinary/%%EndBinary or %%BeginData/%%EndData
 *		    comments.)
 *		    Also, skip over included documents (as indicated by
 *		    %%BeginDocument/%%EndDocument comments.)
 */

static char *
readline(line, size, fp, position, line_len)
    char *line;
    int size;
    FILE *fp;
    long *position;
    unsigned int *line_len;
{
    char text[PSLINELENGTH];	/* Temporary storage for text */
    char save[PSLINELENGTH];	/* Temporary storage for text */
    char *cp;
    unsigned int num;
    unsigned int nbytes;
    int i;
    char buf[BUFSIZ];

    if (position) *position = ftell(fp);
    cp = fgets(line, size, fp);
    *line_len = cp ? strlen(line) : 0;
    if (iscomment(line, "%%BeginDocument:")) {
	strcpy(save, line+7);
	while (readline(line, size, fp, NULL, &nbytes) &&
	       !iscomment(line, "%%EndDocument")) {
	    *line_len += nbytes;
	}
	*line_len += nbytes;
	strcpy(line+2, save);
    } else if (iscomment(line, "%%BeginFeature:")) {
	strcpy(save, line+7);
	while (readline(line, size, fp, NULL, &nbytes) &&
	       !iscomment(line, "%%EndFeature")) {
	    *line_len += nbytes;
	}
	*line_len += nbytes;
	strcpy(line+2, save);
    } else if (iscomment(line, "%%BeginFile:")) {
	strcpy(save, line+7);
	while (readline(line, size, fp, NULL, &nbytes) &&
	       !iscomment(line, "%%EndFile")) {
	    *line_len += nbytes;
	}
	*line_len += nbytes;
	strcpy(line+2, save);
    } else if (iscomment(line, "%%BeginFont:")) {
	strcpy(save, line+7);
	while (readline(line, size, fp, NULL, &nbytes) &&
	       !iscomment(line, "%%EndFont")) {
	    *line_len += nbytes;
	}
	*line_len += nbytes;
	strcpy(line+2, save);
    } else if (iscomment(line, "%%BeginProcSet:")) {
	strcpy(save, line+7);
	while (readline(line, size, fp, NULL, &nbytes) &&
	       !iscomment(line, "%%EndProcSet")) {
	    *line_len += nbytes;
	}
	*line_len += nbytes;
	strcpy(line+2, save);
    } else if (iscomment(line, "%%BeginResource:")) {
	strcpy(save, line+7);
	while (readline(line, size, fp, NULL, &nbytes) &&
	       !iscomment(line, "%%EndResource")) {
	    *line_len += nbytes;
	}
	*line_len += nbytes;
	strcpy(line+2, save);
    } else if (iscomment(line, "%%BeginData:")) {
	text[0] = '\0';
	strcpy(save, line+7);
	if (sscanf(line+length("%%BeginData:"), "%d %*s %s", &num, text) >= 1) {
	    if (strcmp(text, "Lines") == 0) {
		for (i=0; i < num; i++) {
		    cp = fgets(line, size, fp);
		    *line_len += cp ? strlen(line) : 0;
		}
	    } else {
		while (num > BUFSIZ) {
		    fread(buf, sizeof (char), BUFSIZ, fp);
		    *line_len += BUFSIZ;
		    num -= BUFSIZ;
		}
		fread(buf, sizeof (char), num, fp);
		*line_len += num;
	    }
	}
	while (readline(line, size, fp, NULL, &nbytes) &&
	       !iscomment(line, "%%EndData")) {
	    *line_len += nbytes;
	}
	*line_len += nbytes;
	strcpy(line+2, save);
    } else if (iscomment(line, "%%BeginBinary:")) {
	strcpy(save, line+7);
	if(sscanf(line+length("%%BeginBinary:"), "%d", &num) == 1) {
	    while (num > BUFSIZ) {
		fread(buf, sizeof (char), BUFSIZ, fp);
		*line_len += BUFSIZ;
		num -= BUFSIZ;
	    }
	    fread(buf, sizeof (char), num, fp);
	    *line_len += num;
	}
	while (readline(line, size, fp, NULL, &nbytes) &&
	       !iscomment(line, "%%EndBinary")) {
	    *line_len += nbytes;
	}
	*line_len += nbytes;
	strcpy(line+2, save);
    }
    return cp;
}

/*
 *	pscopy -- copy lines of Postscript from a section of one file
 *		  to another file.
 *                Automatically switch to binary copying whenever
 *                %%BeginBinary/%%EndBinary or %%BeginData/%%EndData
 *		  comments are encountered.
 */

void
pscopy(from, to, begin, end)
    FILE *from;
    FILE *to;
    long begin;			/* set negative to avoid initial seek */
    long end;
{
    char line[PSLINELENGTH];	/* 255 characters + 1 newline + 1 NULL */
    char text[PSLINELENGTH];	/* Temporary storage for text */
    unsigned int num;
    int i;
    char buf[BUFSIZ];

    if (begin >= 0) fseek(from, begin, SEEK_SET);
    while (ftell(from) < end) {

	fgets(line, sizeof line, from);
	fputs(line, to);

	if (iscomment(line, "%%BeginData:")) {
	    text[0] = '\0';
	    if (sscanf(line+length("%%BeginData:"),
		       "%d %*s %s", &num, text) >= 1) {
		if (strcmp(text, "Lines") == 0) {
		    for (i=0; i < num; i++) {
			fgets(line, sizeof line, from);
			fputs(line, to);
		    }
		} else {
		    while (num > BUFSIZ) {
			fread(buf, sizeof (char), BUFSIZ, from);
			fwrite(buf, sizeof (char), BUFSIZ, to);
			num -= BUFSIZ;
		    }
		    fread(buf, sizeof (char), num, from);
		    fwrite(buf, sizeof (char), num, to);
		}
	    }
	} else if (iscomment(line, "%%BeginBinary:")) {
	    if(sscanf(line+length("%%BeginBinary:"), "%d", &num) == 1) {
		while (num > BUFSIZ) {
		    fread(buf, sizeof (char), BUFSIZ, from);
		    fwrite(buf, sizeof (char), BUFSIZ, to);
		    num -= BUFSIZ;
		}
		fread(buf, sizeof (char), num, from);
		fwrite(buf, sizeof (char), num, to);
	    }
	}
    }
}

/*
 *	pscopyuntil -- copy lines of Postscript from a section of one file
 *		       to another file until a particular comment is reached.
 *                     Automatically switch to binary copying whenever
 *                     %%BeginBinary/%%EndBinary or %%BeginData/%%EndData
 *		       comments are encountered.
 */

char *
pscopyuntil(from, to, begin, end, comment)
    FILE *from;
    FILE *to;
    long begin;			/* set negative to avoid initial seek */
    long end;
#if NeedFunctionPrototypes
    const
#endif
    char *comment;
{
    char line[PSLINELENGTH];	/* 255 characters + 1 newline + 1 NULL */
    char text[PSLINELENGTH];	/* Temporary storage for text */
    unsigned int num;
    int comment_length;
    int i;
    char buf[BUFSIZ];
    char *cp;

    comment_length = strlen(comment);
    if (begin >= 0) fseek(from, begin, SEEK_SET);
    while (ftell(from) < end) {

	fgets(line, sizeof line, from);

	/* iscomment cannot be used here,
	 * because comment_length is not known at compile time. */
	if (strncmp(line, comment, comment_length) == 0) {
	    cp = (char *) malloc(strlen(line)+1);
	    if (cp == NULL) {
		fprintf(stderr, "Fatal Error: Dynamic memory exhausted.\n");
		exit(-1);
	    }
	    strcpy(cp, line);
	    return cp;
	}
	fputs(line, to);
	if (iscomment(line, "%%BeginData:")) {
	    text[0] = '\0';
	    if (sscanf(line+length("%%BeginData:"),
		       "%d %*s %s", &num, text) >= 1) {
		if (strcmp(text, "Lines") == 0) {
		    for (i=0; i < num; i++) {
			fgets(line, sizeof line, from);
			fputs(line, to);
		    }
		} else {
		    while (num > BUFSIZ) {
			fread(buf, sizeof (char), BUFSIZ, from);
			fwrite(buf, sizeof (char), BUFSIZ, to);
			num -= BUFSIZ;
		    }
		    fread(buf, sizeof (char), num, from);
		    fwrite(buf, sizeof (char), num, to);
		}
	    }
	} else if (iscomment(line, "%%BeginBinary:")) {
	    if(sscanf(line+length("%%BeginBinary:"), "%d", &num) == 1) {
		while (num > BUFSIZ) {
		    fread(buf, sizeof (char), BUFSIZ, from);
		    fwrite(buf, sizeof (char), BUFSIZ, to);
		    num -= BUFSIZ;
		}
		fread(buf, sizeof (char), num, from);
		fwrite(buf, sizeof (char), num, to);
	    }
	}
    }
    return NULL;
}

/*
 *	isblankline -- determine whether the line contains nothing but whitespace.
 */

static int
isblankline(line)
    char *line;
{
    while (*line == ' ' || *line == '\t') line++;
    return *line == '\n';
}
