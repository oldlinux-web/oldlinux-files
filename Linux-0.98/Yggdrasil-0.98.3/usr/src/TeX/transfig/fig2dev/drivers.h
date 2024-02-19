/*
 * TransFig: Facility for Translating Fig code
 * Copyright (c) 1991 Micah Beck, Cornell University
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Cornell University not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Cornell University makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * CORNELL UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL CORNELL UNIVERSITY BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

extern struct driver dev_box;
extern struct driver dev_epic;
extern struct driver dev_ibmgl;
extern struct driver dev_latex;
extern struct driver dev_pic;
extern struct driver dev_pictex;
extern struct driver dev_ps;
extern struct driver dev_pstex;
extern struct driver dev_pstex_t;
extern struct driver dev_textyl;
extern struct driver dev_tpic;

struct 
	{char *name; struct driver *dev;}
	drivers[]
	= {
		{"box",		&dev_box}, 
#ifdef EPIC
		{"epic",	&dev_epic},
		{"eepic",	&dev_epic},
		{"eepicemu",	&dev_epic},
#endif
#ifdef IBMGL
		{"ibmgl",	&dev_ibmgl},
#endif
#ifdef LATEX
		{"latex",	&dev_latex},
#endif
#ifdef PIC
		{"pic",		&dev_pic},
#endif
#ifdef PICTEX
		{"pictex",	&dev_pictex},
#endif
#ifdef PS
		{"ps",		&dev_ps},
#endif
#ifdef PSTEX
		{"pstex",	&dev_pstex},
		{"pstex_t",	&dev_pstex_t},
#endif
#ifdef TEXTYL
		{"textyl",	&dev_textyl},
#endif
#ifdef TPIC
		{"tpic",	&dev_tpic},
#endif
		{"",		NULL}
	};
